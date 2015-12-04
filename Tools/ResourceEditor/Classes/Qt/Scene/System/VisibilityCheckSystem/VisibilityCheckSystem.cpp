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

    for (DAVA::uint32 i = 0; i < CubemapsCount; ++i)
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
    for (uint32 i = 0; i < CubemapsCount; ++i)
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
        entities.push_back(entity);
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
    auto i = std::find(entities.begin(), entities.end(), entity);
    if (i != entities.end())
    {
        entities.erase(i);
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
    for (auto e : entities)
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
    for (auto e : entities)
    {
        auto visibilityComponent = static_cast<DAVA::VisibilityCheckComponent*>(e->GetComponent(DAVA::Component::VISIBILITY_CHECK_COMPONENT));
        if (visibilityComponent->IsEnabled())
        {
            auto worldTransform = e->GetWorldTransform();
            DAVA::Vector3 position = worldTransform.GetTranslationVector();
            DAVA::Vector3 direction = MultiplyVectorMat3x3(DAVA::Vector3(0.0f, 0.0f, 1.0f), worldTransform);
            dbg->DrawCircle(position, direction, visibilityComponent->GetRadius(), 36, DAVA::Color::White, DAVA::RenderHelper::DRAW_WIRE_DEPTH);
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
    for (DAVA::uint32 cm = 0; (cm < CubemapsCount) && (currentPointIndex < controlPoints.size()); ++cm, ++currentPointIndex)
    {
        const auto& point = controlPoints[currentPointIndex];
        renderer.RenderToCubemapFromPoint(rs, fromCamera, cubemapTarget[cm], point.point);
        renderer.RenderVisibilityToTexture(rs, fromCamera, cubemapTarget[cm], renderTarget, point);
    }

    DAVA::Rect dstRect(0.0f, Renderer::GetFramebufferHeight(), Renderer::GetFramebufferWidth(), -Renderer::GetFramebufferHeight());
    DAVA::RenderSystem2D::Instance()->DrawTexture(renderTarget, RenderSystem2D::DEFAULT_2D_TEXTURE_ADDITIVE_MATERIAL, Color::White, dstRect);
}

void VisibilityCheckSystem::UpdatePointSet()
{
    DAVA::Landscape* landscape = DAVA::FindLandscape(GetScene());

    controlPoints.clear();

    for (auto e : entities)
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
                if (shouldSnap && (PlacePointOnLandscape(transformedPoint, landscape)))
                {
                    DAVA::Vector3 dxPoint = transformedPoint + Vector3(1.0f, 0.0f, 0.0f);
                    DAVA::Vector3 dyPoint = transformedPoint + Vector3(0.0f, 1.0f, 0.0f);
                    PlacePointOnLandscape(dxPoint, landscape);
                    PlacePointOnLandscape(dyPoint, landscape);

                    normal = (dxPoint - transformedPoint).CrossProduct(dyPoint - transformedPoint);
                    normal.Normalize();

                    transformedPoint.z += snapHeight;
                }

                controlPoints.emplace_back(transformedPoint, normal, visibilityComponent->GetNormalizedColor(), upAngle, dnAngle, maxDist);
            }
        }
    }
}

bool VisibilityCheckSystem::PlacePointOnLandscape(DAVA::Vector3& point, DAVA::Landscape* landscape)
{
    Vector3 normalizedCoord = (point - landscape->GetBoundingBox().min) / (landscape->GetBoundingBox().max - landscape->GetBoundingBox().min);
    if ((normalizedCoord.x < 0.0f) || (normalizedCoord.y < 0.0f) || (normalizedCoord.x >= 1.0f) || (normalizedCoord.y >= 1.0f))
        return false;

    Vector2 hmPoint = normalizedCoord.xy() * static_cast<float>(landscape->GetHeightmap()->Size());
    point.z = GetHeightAtHeightmapPoint(hmPoint.x, hmPoint.y, landscape);
    return true;
}

void VisibilityCheckSystem::Prerender()
{
    bool shouldCreateRenderTarget = (renderTarget == nullptr);

    if (shouldCreateRenderTarget)
    {
        CreateRenderTarget();
    }

    renderer.PreRenderScene(GetScene()->GetRenderSystem(), GetScene()->GetCurrentCamera(), renderTarget);

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

    if (renderObjectToEntity.count(object) == 0)
        return false;

    auto entity = renderObjectToEntity.at(object);

    KeyedArchive* customProps = GetCustomPropertiesArchieve(entity);
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

float VisibilityCheckSystem::GetHeightAtHeightmapPoint(DAVA::float32 fx, DAVA::float32 fy, DAVA::Landscape* landscape)
{
    auto hmSize = landscape->GetHeightmap()->Size();
    const auto hmData = landscape->GetHeightmap()->Data();
    int32 x = std::min(hmSize - 1, static_cast<int32>(fx));
    int32 y = std::min(hmSize - 1, static_cast<int32>(fy));
    int nextX = DAVA::Min(x + 1, hmSize - 1);
    int nextY = DAVA::Min(y + 1, hmSize - 1);
    int i00 = x + y * hmSize;
    int i01 = nextX + y * hmSize;
    int i10 = x + nextY * hmSize;
    int i11 = nextX + nextY * hmSize;
    float h00 = static_cast<float>(hmData[i00]);
    float h01 = static_cast<float>(hmData[i01]);
    float h10 = static_cast<float>(hmData[i10]);
    float h11 = static_cast<float>(hmData[i11]);
    float dx = fx - static_cast<float>(x);
    float dy = fy - static_cast<float>(y);
    float h0 = h00 * (1.0f - dx) + h01 * dx;
    float h1 = h10 * (1.0f - dx) + h11 * dx;
    float h = h0 * (1.0f - dy) + h1 * dy;
    return h * landscape->GetLandscapeHeight() / static_cast<float>(Heightmap::MAX_VALUE);
}