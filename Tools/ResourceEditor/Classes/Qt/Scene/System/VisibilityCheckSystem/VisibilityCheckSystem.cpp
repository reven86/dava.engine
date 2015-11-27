/*==================================================================================
    Copyright (c) 2008, binaryzebra
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    * Neither the name of the binaryzebra nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE binaryzebra AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL binaryzebra BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/

#include "VisibilityCheckSystem.h"
#include "Scene3D/Scene.h"
#include "Scene3D/Entity.h"
#include "Scene3D/Components/TransformComponent.h"
#include "Scene3D/Components/VisibilityCheckComponent.h"
#include "Render/Highlevel/RenderSystem.h"
#include "Render/2D/Systems/RenderSystem2D.h"

using namespace DAVA;

const uint32 renderTargetSize = 1024;

VisibilityCheckSystem::VisibilityCheckSystem(Scene* scene)
    : SceneSystem(scene)
    , cubemapTarget(Texture::CreateFBO(renderTargetSize, renderTargetSize, PixelFormat::FORMAT_RGBA8888, true, rhi::TEXTURE_TYPE_CUBE))
{
}

VisibilityCheckSystem::~VisibilityCheckSystem()
{
}

void VisibilityCheckSystem::AddEntity(Entity* entity)
{
    auto requiredComponent = entity->GetComponent(Component::VISIBILITY_CHECK_COMPONENT);
    if (requiredComponent != nullptr)
    {
        entities.push_back(entity);
        shouldPrerender = true;
    }
}

void VisibilityCheckSystem::RemoveEntity(Entity* entity)
{
    auto i = std::find(entities.begin(), entities.end(), entity);
    if (i != entities.end())
    {
        entities.erase(i);
        shouldPrerender = true;
    }
}

void VisibilityCheckSystem::Process(float32 timeElapsed)
{
    if (entities.empty())
        return;
}

void VisibilityCheckSystem::Draw()
{
    if (entities.empty())
        return;

    for (auto e : entities)
    {
        auto visibilityComponent = static_cast<VisibilityCheckComponent*>(e->GetComponent(Component::VISIBILITY_CHECK_COMPONENT));
        if (!visibilityComponent->IsPointSetValid())
        {
            visibilityComponent->BuildPointSet();
        }
    }

    UpdatePointSet();

    auto rs = GetScene()->GetRenderSystem();
    auto dbg = rs->GetDebugDrawer();
    for (auto e : entities)
    {
        auto worldTransform = e->GetWorldTransform();
        auto visibilityComponent = static_cast<VisibilityCheckComponent*>(e->GetComponent(Component::VISIBILITY_CHECK_COMPONENT));
        Vector3 position = worldTransform.GetTranslationVector();
        Vector3 direction = MultiplyVectorMat3x3(Vector3(0.0f, 0.0f, 1.0f), worldTransform);
        dbg->DrawCircle(position, direction, visibilityComponent->GetRadius(), 36, Color::White, RenderHelper::DRAW_WIRE_DEPTH);
    }

    if (!CacheIsValid())
    {
        BuildCache();
        shouldPrerender = true;
    }

    if (shouldPrerender)
    {
        Prerender();
    }

    auto fromCamera = GetScene()->GetCurrentCamera();

    size_t index = 0;
    for (const auto& point : controlPoints)
    {
        if (currentPointIndex >= index)
        {
            dbg->DrawIcosahedron(point, 0.1f, Color(1.0f, 1.0f, 0.5f, 1.0f), RenderHelper::DRAW_WIRE_DEPTH);
        }

        if (currentPointIndex == index)
        {
            Color clr(0.0f, 0.0f, 0.0f, 0.0f);
            clr.r = 1.0f / static_cast<float>(controlPoints.size() + 1);
            renderPass.RenderToCubemapFromPoint(rs, fromCamera, cubemapTarget, point);
            renderPass.RenderVisibilityToTexture(rs, fromCamera, cubemapTarget, renderTarget, point, clr);
        }

        ++index;
    }

    Rect dstRect(0.0f, Renderer::GetFramebufferHeight(), Renderer::GetFramebufferWidth(), -Renderer::GetFramebufferHeight());
    RenderSystem2D::Instance()->DrawTexture(renderTarget, RenderSystem2D::DEFAULT_2D_TEXTURE_ADDITIVE_MATERIAL, Color::White, dstRect);
    currentPointIndex = std::min(controlPoints.size(), currentPointIndex + 1);
}

void VisibilityCheckSystem::UpdatePointSet()
{
    controlPoints.clear();

    for (auto e : entities)
    {
        auto worldTransform = e->GetWorldTransform();
        Vector3 position = worldTransform.GetTranslationVector();

        auto visibilityComponent = static_cast<VisibilityCheckComponent*>(e->GetComponent(Component::VISIBILITY_CHECK_COMPONENT));
        for (const auto& pt : visibilityComponent->GetPoints())
        {
            controlPoints.push_back(position + MultiplyVectorMat3x3(pt, worldTransform));
        }
    }
}

void VisibilityCheckSystem::Prerender()
{
    bool shouldCreateRenderTarget = (renderTarget == nullptr);

    if (shouldCreateRenderTarget)
    {
        CreateRenderTarget();
    }

    renderPass.PreRenderScene(GetScene()->GetRenderSystem(), GetScene()->GetCurrentCamera(), renderTarget);

    currentPointIndex = 0;
    shouldPrerender = false;
}

void VisibilityCheckSystem::CreateRenderTarget()
{
    if (renderTarget != nullptr)
    {
        SafeRelease(renderTarget);
    }

    renderTarget = Texture::CreateFBO(Renderer::GetFramebufferWidth(), Renderer::GetFramebufferHeight(),
                                      PixelFormat::FORMAT_RGBA8888, true, rhi::TEXTURE_TYPE_2D);
}

bool VisibilityCheckSystem::CacheIsValid()
{
    Size2i vpSize(Renderer::GetFramebufferWidth(), Renderer::GetFramebufferHeight());
    if (vpSize != stateCache.viewportSize)
        return false;

    auto cam = GetScene()->GetCurrentCamera();
    if (cam != stateCache.camera)
        return false;

    auto currentMatrix = cam->GetViewProjMatrix();
    if (memcmp(currentMatrix.data, stateCache.viewprojMatrix.data, sizeof(Matrix4)) != 0)
        return false;

    return true;
}

void VisibilityCheckSystem::BuildCache()
{
    stateCache.viewportSize = Size2i(Renderer::GetFramebufferWidth(), Renderer::GetFramebufferHeight());
    stateCache.camera = GetScene()->GetCurrentCamera();
    stateCache.viewprojMatrix = stateCache.camera->GetViewProjMatrix();
}
