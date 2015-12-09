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
#include "../../SceneSignals.h"

#include "Scene3D/Scene.h"
#include "Scene3D/Entity.h"
#include "Scene3D/Components/TransformComponent.h"
#include "Scene3D/Components/VisibilityCheckComponent.h"

#include "Render/Highlevel/RenderSystem.h"
#include "Render/2D/Systems/RenderSystem2D.h"

VisibilityCheckSystem::VisibilityCheckSystem(DAVA::Scene* scene)
    : DAVA::SceneSystem(scene)
{
    renderer.SetDelegate(this);

    const DAVA::uint32 renderTargetSize = 2048;

    for (DAVA::uint32 i = 0; i < CUBEMAPS_COUNT; ++i)
    {
        cubemapTarget[i] = DAVA::Texture::CreateFBO(renderTargetSize, renderTargetSize,
                                                    DAVA::PixelFormat::FORMAT_RGBA8888, true, rhi::TEXTURE_TYPE_CUBE);
    }

    QObject::connect(SceneSignals::Instance(), &SceneSignals::CommandExecuted, [this](Scene*, const Command2*, bool) {
        shouldPrerender = true;
    });

    QObject::connect(SceneSignals::Instance(), &SceneSignals::NonModifyingEventEmitted, [this]() {
        shouldPrerender = true;
    });
}

VisibilityCheckSystem::~VisibilityCheckSystem()
{
    for (uint32 i = 0; i < CUBEMAPS_COUNT; ++i)
    {
        SafeRelease(cubemapTarget[i]);
    }
}

void VisibilityCheckSystem::RegisterEntity(DAVA::Entity* entity)
{
    auto visibilityComponent = entity->GetComponent(DAVA::Component::VISIBILITY_CHECK_COMPONENT);
    auto renderComponent = DAVA::GetRenderComponent(entity);
    if ((renderComponent != nullptr) || (visibilityComponent != nullptr))
    {
        AddEntity(entity);
    }
}

void VisibilityCheckSystem::UnregisterEntity(DAVA::Entity* entity)
{
    auto visibilityComponent = entity->GetComponent(DAVA::Component::VISIBILITY_CHECK_COMPONENT);
    auto renderComponent = DAVA::GetRenderComponent(entity);
    if ((renderComponent != nullptr) || (visibilityComponent != nullptr))
    {
        RemoveEntity(entity);
    }
}

void VisibilityCheckSystem::AddEntity(DAVA::Entity* entity)
{
    auto requiredComponent = entity->GetComponent(DAVA::Component::VISIBILITY_CHECK_COMPONENT);
    if (requiredComponent != nullptr)
    {
        entitiesWithVisibilityComponent.push_back(entity);
        shouldPrerender = true;
    }

    auto renderComponent = DAVA::GetRenderComponent(entity);
    if (renderComponent != nullptr)
    {
        auto ro = renderComponent->GetRenderObject();
        renderObjectToEntity[ro] = entity;
    }
}

void VisibilityCheckSystem::RemoveEntity(DAVA::Entity* entity)
{
    auto i = std::find(entitiesWithVisibilityComponent.begin(), entitiesWithVisibilityComponent.end(), entity);
    if (i != entitiesWithVisibilityComponent.end())
    {
        entitiesWithVisibilityComponent.erase(i);
        shouldPrerender = true;
    }

    auto renderComponent = DAVA::GetRenderComponent(entity);
    if (renderComponent != nullptr)
    {
        auto ro = renderComponent->GetRenderObject();
        renderObjectToEntity.erase(ro);
    }
}

void VisibilityCheckSystem::Draw()
{
    if (!Renderer::GetOptions()->IsOptionEnabled(RenderOptions::DEBUG_ENABLE_VISIBILITY_SYSTEM))
    {
        return;
    }

    bool shouldRenderOverlay = false;

    for (auto e : entitiesWithVisibilityComponent)
    {
        auto visibilityComponent = static_cast<DAVA::VisibilityCheckComponent*>(e->GetComponent(Component::VISIBILITY_CHECK_COMPONENT));

        if (!visibilityComponent->IsValid())
        {
            if (!visibilityComponent->IsPointSetValid())
            {
                visibilityComponent->BuildPointSet();
            }
            shouldPrerender = true;
            visibilityComponent->SetValid();
        }
    }

    UpdatePointSet();

    auto rs = GetScene()->GetRenderSystem();
    auto dbg = rs->GetDebugDrawer();
    for (auto e : entitiesWithVisibilityComponent)
    {
        auto visibilityComponent = static_cast<DAVA::VisibilityCheckComponent*>(e->GetComponent(DAVA::Component::VISIBILITY_CHECK_COMPONENT));
        if (visibilityComponent->IsEnabled())
        {
            auto worldTransform = e->GetWorldTransform();
            DAVA::Vector3 position = worldTransform.GetTranslationVector();
            DAVA::Vector3 direction = MultiplyVectorMat3x3(DAVA::Vector3(0.0f, 0.0f, 1.0f), worldTransform);
            dbg->DrawCircle(position, direction, visibilityComponent->GetRadius(), 36, DAVA::Color::White, DAVA::RenderHelper::DRAW_WIRE_DEPTH);
            shouldRenderOverlay = true;
        }
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

    for (const auto& point : controlPoints)
    {
        dbg->DrawIcosahedron(point.point, 0.1f, DAVA::Color(1.0f, 1.0f, 0.5f, 1.0f), DAVA::RenderHelper::DRAW_WIRE_DEPTH);
        dbg->DrawLine(point.point, point.point + 2.0f * point.normal, Color(0.25f, 1.0f, 0.25f, 1.0f));
    }

    DAVA::Color clr(std::sqrt(1.0f / static_cast<float>(controlPoints.size() + 1)), 0.0f, 0.0f, 0.0f);
    auto fromCamera = GetScene()->GetCurrentCamera();
    for (DAVA::uint32 cm = 0; (cm < CUBEMAPS_COUNT) && (currentPointIndex < controlPoints.size()); ++cm, ++currentPointIndex)
    {
        const auto& point = controlPoints[currentPointIndex];
        renderer.RenderToCubemapFromPoint(rs, fromCamera, cubemapTarget[cm], point.point);
        renderer.RenderVisibilityToTexture(rs, fromCamera, cubemapTarget[cm], renderTarget, point);
    }

    if (shouldRenderOverlay)
    {
        auto rs2d = DAVA::RenderSystem2D::Instance();
        DAVA::Rect dstRect(0.0f, Renderer::GetFramebufferHeight(), Renderer::GetFramebufferWidth(), -Renderer::GetFramebufferHeight());
        rs2d->DrawTextureWithoutAdjustingRects(renderTarget, RenderSystem2D::DEFAULT_2D_TEXTURE_ADDITIVE_MATERIAL, Color::White, dstRect);
    }
}

void VisibilityCheckSystem::UpdatePointSet()
{
    DAVA::Landscape* landscape = DAVA::FindLandscape(GetScene());

    controlPoints.clear();

    for (auto e : entitiesWithVisibilityComponent)
    {
        auto visibilityComponent = static_cast<VisibilityCheckComponent*>(e->GetComponent(Component::VISIBILITY_CHECK_COMPONENT));
        if (visibilityComponent->IsEnabled())
        {
            auto worldTransform = e->GetWorldTransform();
            DAVA::Vector3 position = worldTransform.GetTranslationVector();
            DAVA::Vector3 normal = MultiplyVectorMat3x3(DAVA::Vector3(0.0f, 0.0f, 1.0f), worldTransform);
            normal.Normalize();
            float upAngle = std::cos((90.0f - visibilityComponent->GetUpAngle()) * PI / 180.0f);
            float dnAngle = -std::cos((90.0f - visibilityComponent->GetDownAngle()) * PI / 180.0f);
            float maxDist = visibilityComponent->GetMaximumDistance();

            bool shouldSnap = visibilityComponent->ShouldPlaceOnLandscape();
            float snapHeight = visibilityComponent->GetHeightAboveLandscape();

            for (const auto& pt : visibilityComponent->GetPoints())
            {
                DAVA::Vector3 transformedPoint = position + MultiplyVectorMat3x3(pt, worldTransform);
                if (shouldSnap && (landscape->GetHeightAtPoint(transformedPoint, transformedPoint.z)))
                {
                    DAVA::Vector3 dxPoint = transformedPoint + Vector3(1.0f, 0.0f, 0.0f);
                    DAVA::Vector3 dyPoint = transformedPoint + Vector3(0.0f, 1.0f, 0.0f);
                    landscape->GetHeightAtPoint(dxPoint, dxPoint.z);
                    landscape->GetHeightAtPoint(dyPoint, dyPoint.z);

                    normal = (dxPoint - transformedPoint).CrossProduct(dyPoint - transformedPoint);
                    normal.Normalize();

                    transformedPoint.z += snapHeight;
                }

                controlPoints.emplace_back(transformedPoint, normal, visibilityComponent->GetNormalizedColor(), upAngle, dnAngle, maxDist);
            }
        }
    }
}

void VisibilityCheckSystem::Prerender()
{
    if (renderTarget == nullptr)
    {
        CreateRenderTarget();
    }

    renderer.PreRenderScene(GetScene()->GetRenderSystem(), GetScene()->GetCurrentCamera(), renderTarget);

    currentPointIndex = 0;
    shouldPrerender = false;
}

void VisibilityCheckSystem::CreateRenderTarget()
{
    SafeRelease(renderTarget);
    renderTarget = Texture::CreateFBO(Renderer::GetFramebufferWidth(), Renderer::GetFramebufferHeight(),
                                      PixelFormat::FORMAT_RGBA8888, true, rhi::TEXTURE_TYPE_2D, false);
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

bool VisibilityCheckSystem::ShouldDrawRenderObject(DAVA::RenderObject* object)
{
    auto type = object->GetType();

    if (type == DAVA::RenderObject::TYPE_LANDSCAPE)
        return true;

    if ((type == DAVA::RenderObject::TYPE_SPEED_TREE) || (type == DAVA::RenderObject::TYPE_SPRITE) ||
        (type == DAVA::RenderObject::TYPE_VEGETATION) || (type == DAVA::RenderObject::TYPE_PARTICLE_EMTITTER))
    {
        return false;
    }

    auto entityIterator = renderObjectToEntity.find(object);
    if (entityIterator == renderObjectToEntity.end())
        return false;

    KeyedArchive* customProps = GetCustomPropertiesArchieve(entityIterator->second);
    if (customProps)
    {
        DAVA::String collisionTypeString = "CollisionType";
        if ((object->GetMaxSwitchIndex() > 0) && (object->GetSwitchIndex() > 0))
        {
            collisionTypeString = "CollisionTypeCrashed";
        }

        const int32 collisiontype = customProps->GetInt32(collisionTypeString, 0);

        if ((ResourceEditor::ESOT_NO_COLISION == collisiontype) ||
            (ResourceEditor::ESOT_TREE == collisiontype) ||
            (ResourceEditor::ESOT_BUSH == collisiontype) ||
            (ResourceEditor::ESOT_FALLING == collisiontype) ||
            (ResourceEditor::ESOT_FRAGILE_PROJ_INV == collisiontype) ||
            (ResourceEditor::ESOT_SPEED_TREE == collisiontype))
        {
            return false;
        }
    }

    return true;
}
