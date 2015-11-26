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
    }
}

void VisibilityCheckSystem::RemoveEntity(Entity* entity)
{
    auto i = std::find(entities.begin(), entities.end(), entity);
    if (i != entities.end())
    {
        entities.erase(i);
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

    if (shouldPrerender)
    {
        Prerender();
    }

    for (const auto& point : controlPoints)
    {
        dbg->DrawIcosahedron(point, 0.1f, Color(1.0f, 1.0f, 0.5f, 1.0f), RenderHelper::DRAW_WIRE_DEPTH);
        renderPass.RenderToCubemapFromPoint(rs, cubemapTarget, point);
        renderPass.RenderToOverlayTexture(rs, cubemapTarget, renderTarget, point);
        break;
    }

    RenderSystem2D::Instance()->DrawTexture(renderTarget, RenderSystem2D::DEFAULT_2D_TEXTURE_NOBLEND_MATERIAL, Color::White);
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

    renderPass.PreRenderScene(GetScene()->GetRenderSystem(), renderTarget);

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