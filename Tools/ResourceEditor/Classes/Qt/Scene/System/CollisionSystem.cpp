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


#include "Base/AlignedAllocator.h"
#include "Scene/System/CollisionSystem.h"
#include "Scene/System/CollisionSystem/CollisionRenderObject.h"
#include "Scene/System/CollisionSystem/CollisionLandscape.h"
#include "Scene/System/CollisionSystem/CollisionBox.h"
#include "Scene/System/CameraSystem.h"
#include "Scene/System/SelectionSystem.h"
#include "Scene/SceneEditor2.h"

#include "Commands2/EntityParentChangeCommand.h"
#include "Commands2/InspMemberModifyCommand.h"

#include "Settings/SettingsManager.h"

// framework
#include "Scene3D/Components/ComponentHelpers.h"
#include "Scene3D/Components/TransformComponent.h"
#include "Scene3D/Scene.h"

#define SIMPLE_COLLISION_BOX_SIZE 1.0f

ENUM_DECLARE(CollisionSystemDrawMode)
{
    //ENUM_ADD(CS_DRAW_OBJECTS);
    ENUM_ADD(CS_DRAW_OBJECTS_SELECTED);
    ENUM_ADD(CS_DRAW_OBJECTS_RAYTEST);
    //ENUM_ADD(CS_DRAW_LAND);
    ENUM_ADD(CS_DRAW_LAND_RAYTEST);
    //ENUM_ADD(CS_DRAW_LAND_COLLISION);
}

SceneCollisionSystem::SceneCollisionSystem(DAVA::Scene* scene)
    : DAVA::SceneSystem(scene)
{
    btVector3 worldMin(-1000, -1000, -1000);
    btVector3 worldMax(1000, 1000, 1000);

    objectsCollConf = new btDefaultCollisionConfiguration();
    objectsCollDisp = DAVA::CreateObjectAligned<btCollisionDispatcher, 16>(objectsCollConf);
    objectsBroadphase = new btAxisSweep3(worldMin, worldMax);
    objectsDebugDrawer = new SceneCollisionDebugDrawer(scene->GetRenderSystem()->GetDebugDrawer());
    objectsDebugDrawer->setDebugMode(btIDebugDraw::DBG_DrawWireframe);
    objectsCollWorld = new btCollisionWorld(objectsCollDisp, objectsBroadphase, objectsCollConf);
    objectsCollWorld->setDebugDrawer(objectsDebugDrawer);

    landCollConf = new btDefaultCollisionConfiguration();
    landCollDisp = DAVA::CreateObjectAligned<btCollisionDispatcher, 16>(landCollConf);
    landBroadphase = new btAxisSweep3(worldMin, worldMax);
    landDebugDrawer = new SceneCollisionDebugDrawer(scene->GetRenderSystem()->GetDebugDrawer());
    landDebugDrawer->setDebugMode(btIDebugDraw::DBG_DrawWireframe);
    landCollWorld = new btCollisionWorld(landCollDisp, landBroadphase, landCollConf);
    landCollWorld->setDebugDrawer(landDebugDrawer);

    scene->GetEventSystem()->RegisterSystemForEvent(this, EventSystem::SWITCH_CHANGED);
}

SceneCollisionSystem::~SceneCollisionSystem()
{
    if (GetScene())
    {
        GetScene()->GetEventSystem()->UnregisterSystemForEvent(this, EventSystem::SWITCH_CHANGED);
    }

    for (const auto& etc : entityToCollision)
    {
        delete etc.second;
    }

    DAVA::SafeDelete(objectsCollWorld);
    DAVA::SafeDelete(objectsBroadphase);
    DAVA::SafeDelete(objectsDebugDrawer);
    DAVA::SafeDelete(objectsCollConf);

    DAVA::SafeDelete(landCollWorld);
    DAVA::SafeDelete(landDebugDrawer);
    DAVA::SafeDelete(landBroadphase);
    DAVA::SafeDelete(landCollConf);

    DAVA::DestroyObjectAligned(objectsCollDisp);
    DAVA::DestroyObjectAligned(landCollDisp);
}

void SceneCollisionSystem::SetDrawMode(int mode)
{
    drawMode = mode;
}

int SceneCollisionSystem::GetDrawMode() const
{
    return drawMode;
}

const SelectableObjectGroup::CollectionType& SceneCollisionSystem::ObjectsRayTest(const DAVA::Vector3& from, const DAVA::Vector3& to)
{
    // check if cache is available
    if (rayIntersectCached && (lastRayFrom == from) && (lastRayTo == to))
    {
        return rayIntersectedEntities;
    }

    // no cache. start ray new ray test
    lastRayFrom = from;
    lastRayTo = to;
    rayIntersectedEntities.clear();

    btVector3 btFrom(from.x, from.y, from.z);
    btVector3 btTo(to.x, to.y, to.z);

    btCollisionWorld::AllHitsRayResultCallback btCallback(btFrom, btTo);
    objectsCollWorld->rayTest(btFrom, btTo, btCallback);

    int collidedObjects = btCallback.m_collisionObjects.size();
    if (btCallback.hasHit() && (collidedObjects > 0))
    {
        // TODO: sort values inside btCallback, instead of creating separate vector for them
        using Hits = DAVA::Vector<std::pair<btCollisionObject*, btScalar>>;
        Hits hits(collidedObjects);
        btCallback.m_collisionObjects.size();
        for (int i = 0; i < collidedObjects; ++i)
        {
            hits[i].first = btCallback.m_collisionObjects[i];
            hits[i].second = btCallback.m_hitFractions[i];
        }
        std::sort(hits.begin(), hits.end(), [&btFrom](const Hits::value_type& l, const Hits::value_type& r) {
            return l.second < r.second;
        });

        for (const auto& hit : hits)
        {
            auto entity = collisionToEntity[hit.first];
            rayIntersectedEntities.emplace_back(entity);
            rayIntersectedEntities.back().SetBoundingBox(GetBoundingBox(entity));
        }
    }

    rayIntersectCached = true;
    return rayIntersectedEntities;
}

const SelectableObjectGroup::CollectionType& SceneCollisionSystem::ObjectsRayTestFromCamera()
{
    DAVA::Vector3 traceFrom;
    DAVA::Vector3 traceTo;

    SceneCameraSystem* cameraSystem = ((SceneEditor2*)GetScene())->cameraSystem;
    cameraSystem->GetRayTo2dPoint(lastMousePos, 1000.0f, traceFrom, traceTo);

    return ObjectsRayTest(traceFrom, traceTo);
}

bool SceneCollisionSystem::LandRayTest(const DAVA::Vector3& from, const DAVA::Vector3& to, DAVA::Vector3& intersectionPoint)
{
    DAVA::Vector3 ret;

    // check if cache is available
    if (landIntersectCached && lastLandRayFrom == from && lastLandRayTo == to)
    {
        intersectionPoint = lastLandCollision;
        return landIntersectCachedResult;
    }

    // no cache. start new ray test
    lastLandRayFrom = from;
    lastLandRayTo = to;
    landIntersectCached = true;
    landIntersectCachedResult = false;

    DAVA::Vector3 rayDirection = to - from;
    DAVA::float32 rayLength = rayDirection.Length();
    rayDirection.Normalize();
    rayDirection *= Min(5.0f, rayLength);
    float stepSize = rayDirection.Length();

    btVector3 btStep(rayDirection.x, rayDirection.y, rayDirection.z);
    btVector3 btFrom(from.x, from.y, from.z);
    while ((rayLength - stepSize) >= std::numeric_limits<float>::epsilon())
    {
        btVector3 btTo = btFrom + btStep;

        btCollisionWorld::ClosestRayResultCallback btCallback(btFrom, btTo);
        landCollWorld->rayTest(btFrom, btTo, btCallback);

        if (btCallback.hasHit())
        {
            btVector3 hitPoint = btCallback.m_hitPointWorld;
            ret = DAVA::Vector3(hitPoint.x(), hitPoint.y(), hitPoint.z());
            landIntersectCachedResult = true;
            break;
        }

        btFrom = btTo;
        rayLength -= stepSize;
    }

    lastLandCollision = ret;
    intersectionPoint = ret;

    return landIntersectCachedResult;
}

bool SceneCollisionSystem::LandRayTestFromCamera(DAVA::Vector3& intersectionPoint)
{
    SceneCameraSystem* cameraSystem = ((SceneEditor2*)GetScene())->cameraSystem;

    DAVA::Vector3 camPos = cameraSystem->GetCameraPosition();
    DAVA::Vector3 camDir = cameraSystem->GetPointDirection(lastMousePos);

    DAVA::Vector3 traceFrom = camPos;
    DAVA::Vector3 traceTo = traceFrom + camDir * 1000.0f;

    return LandRayTest(traceFrom, traceTo, intersectionPoint);
}

DAVA::Landscape* SceneCollisionSystem::GetLandscape() const
{
    return DAVA::GetLandscape(curLandscapeEntity);
}

void SceneCollisionSystem::UpdateCollisionObject(DAVA::Entity* entity)
{
    RemoveEntity(entity);
    AddEntity(entity);
}

void SceneCollisionSystem::RemoveCollisionObject(DAVA::Entity* entity)
{
    RemoveEntity(entity);
}

DAVA::AABBox3 SceneCollisionSystem::GetBoundingBox(DAVA::Entity* entity)
{
    DAVA::AABBox3 aabox = DAVA::AABBox3(DAVA::Vector3(0, 0, 0), 1.0f);

    if (entity != nullptr)
    {
        CollisionBaseObject* collObj = entityToCollision[entity];
        if (collObj == nullptr)
        {
            for (int32 i = 0, e = entity->GetChildrenCount(); i < e; ++i)
            {
                aabox.AddAABBox(GetBoundingBox(entity->GetChild(i)));
            }
        }
        else
        {
            aabox = collObj->boundingBox;
        }
    }

    return aabox;
}

void SceneCollisionSystem::Process(DAVA::float32 timeElapsed)
{
    // check in there are entities that should be added or removed
    if (!(entitiesToAdd.empty() && entitiesToRemove.empty()))
    {
        for (auto obj : entitiesToRemove)
        {
            DestroyFromEntity(obj);
        }

        for (auto obj : entitiesToAdd)
        {
            SelectableObject wrapper(obj);
            if (wrapper.CanBeCastedTo<DAVA::Entity>())
            {
                BuildFromEntity(wrapper.Cast<DAVA::Entity>());
            }
            else
            {
                // TODO : build from other types
            }
        }

        entitiesToAdd.clear();
        entitiesToRemove.clear();
    }

    // reset ray cache on new frame
    rayIntersectCached = false;

    drawMode = SettingsManager::GetValue(Settings::Scene_CollisionDrawMode).AsInt32();
    if (drawMode & CS_DRAW_LAND_COLLISION)
    {
        DAVA::Vector3 tmp;
        LandRayTestFromCamera(tmp);
    }
}

void SceneCollisionSystem::Input(DAVA::UIEvent* event)
{
    // don't have to update last mouse pos when event is not from the mouse
    if (UIEvent::Device::MOUSE == event->device)
    {
        lastMousePos = event->point;
    }
}

void SceneCollisionSystem::Draw()
{
    RenderHelper* drawer = GetScene()->GetRenderSystem()->GetDebugDrawer();

    if (drawMode & CS_DRAW_LAND)
    {
        landCollWorld->debugDrawWorld();
    }

    if (drawMode & CS_DRAW_LAND_RAYTEST)
    {
        drawer->DrawLine(lastLandRayFrom, lastLandRayTo, DAVA::Color(0, 1.0f, 0, 1.0f));
    }

    if (drawMode & CS_DRAW_LAND_COLLISION)
    {
        drawer->DrawIcosahedron(lastLandCollision, 0.5f, DAVA::Color(0, 1.0f, 0, 1.0f), RenderHelper::DRAW_SOLID_DEPTH);
    }

    if (drawMode & CS_DRAW_OBJECTS)
    {
        objectsCollWorld->debugDrawWorld();
    }

    if (drawMode & CS_DRAW_OBJECTS_RAYTEST)
    {
        drawer->DrawLine(lastRayFrom, lastRayTo, DAVA::Color(1.0f, 0, 0, 1.0f));
    }

    if (drawMode & CS_DRAW_OBJECTS_SELECTED)
    {
        // current selected entities
        SceneSelectionSystem* selectionSystem = ((SceneEditor2*)GetScene())->selectionSystem;
        if (NULL != selectionSystem)
        {
            for (const auto& item : selectionSystem->GetSelection().GetContent())
            {
                // get collision object for solid selected entity
                CollisionBaseObject* cObj = entityToCollision[item.GetContainedObject()];

                // if no collision object for solid selected entity,
                // try to get collision object for real selected entity
                if (NULL == cObj)
                {
                    cObj = entityToCollision[item.GetContainedObject()];
                }

                if (NULL != cObj && NULL != cObj->btObject)
                {
                    objectsCollWorld->debugDrawObject(cObj->btObject->getWorldTransform(), cObj->btObject->getCollisionShape(), btVector3(1.0f, 0.65f, 0.0f));
                }
            }
        }
    }
}

void SceneCollisionSystem::ProcessCommand(const Command2* command, bool redo)
{
    if (NULL != command)
    {
        DAVA::Entity* entity = command->GetEntity();
        switch (command->GetId())
        {
        case CMDID_TRANSFORM:
            UpdateCollisionObject(entity);
            break;
        case CMDID_ENTITY_CHANGE_PARENT:
        {
            EntityParentChangeCommand* cmd = (EntityParentChangeCommand*)command;
            if (redo)
            {
                if (NULL != cmd->newParent)
                {
                    UpdateCollisionObject(entity);
                }
            }
            else
            {
                if (NULL != cmd->oldParent)
                {
                    UpdateCollisionObject(entity);
                }
                else
                {
                    RemoveCollisionObject(entity);
                }
            }
        }
        break;
        case CMDID_LANDSCAPE_SET_HEIGHTMAP:
        case CMDID_HEIGHTMAP_MODIFY:
            UpdateCollisionObject(curLandscapeEntity);
            break;

        case CMDID_LOD_CREATE_PLANE:
        case CMDID_LOD_DELETE:
        {
            UpdateCollisionObject(command->GetEntity());
            break;
        }

        case CMDID_INSP_MEMBER_MODIFY:
        {
            const InspMemberModifyCommand* cmd = static_cast<const InspMemberModifyCommand*>(command);
            if (String("heightmapPath") == cmd->member->Name().c_str())
            {
                UpdateCollisionObject(curLandscapeEntity);
            }
        }
        break;

        default:
            break;
        }
    }
}

void SceneCollisionSystem::ImmediateEvent(DAVA::Component* component, DAVA::uint32 event)
{
    if (EventSystem::SWITCH_CHANGED == event)
    {
        UpdateCollisionObject(component->GetEntity());
    }
}

void SceneCollisionSystem::AddEntity(DAVA::Entity* entity)
{
    if (NULL != entity)
    {
        entitiesToRemove.erase(entity);
        entitiesToAdd.insert(entity);

        // build collision object for entity childs
        for (int i = 0; i < entity->GetChildrenCount(); ++i)
        {
            AddEntity(entity->GetChild(i));
        }
    }
}

void SceneCollisionSystem::RemoveEntity(DAVA::Entity* entity)
{
    if (NULL != entity)
    {
        entitiesToAdd.erase(entity);
        entitiesToRemove.insert(entity);

        // destroy collision object for entities childs
        for (int i = 0; i < entity->GetChildrenCount(); ++i)
        {
            RemoveEntity(entity->GetChild(i));
        }
    }
}

CollisionBaseObject* SceneCollisionSystem::BuildFromEntity(DAVA::Entity* entity)
{
    CollisionBaseObject* cObj = nullptr;
    DAVA::float32 debugBoxScale = SIMPLE_COLLISION_BOX_SIZE * SettingsManager::GetValue(Settings::Scene_DebugBoxScale).AsFloat();
    DAVA::float32 debugBoxUserScale = SIMPLE_COLLISION_BOX_SIZE * SettingsManager::GetValue(Settings::Scene_DebugBoxUserScale).AsFloat();
    DAVA::float32 debugBoxWaypointScale = SIMPLE_COLLISION_BOX_SIZE * SettingsManager::GetValue(Settings::Scene_DebugBoxWaypointScale).AsFloat();
    DAVA::float32 debugBoxParticleScale = SIMPLE_COLLISION_BOX_SIZE * SettingsManager::GetValue(Settings::Scene_DebugBoxParticleScale).AsFloat();

    bool isLandscape = false;

    DAVA::Landscape* landscape = DAVA::GetLandscape(entity);
    if (landscape != nullptr)
    {
        isLandscape = true;
        cObj = new CollisionLandscape(entity, landCollWorld, landscape);
    }

    DAVA::ParticleEffectComponent* particleEffect = DAVA::GetEffectComponent(entity);
    if ((cObj == nullptr) && (particleEffect != nullptr))
    {
        cObj = new CollisionBox(entity, objectsCollWorld, entity->GetWorldTransform().GetTranslationVector(), debugBoxParticleScale);
    }

    DAVA::RenderObject* renderObject = DAVA::GetRenderObject(entity);
    if ((cObj == nullptr) && (renderObject != nullptr) && entity->IsLodMain(0))
    {
        RenderObject::eType objType = renderObject->GetType();
        if ((objType != RenderObject::TYPE_SPRITE) && (objType != RenderObject::TYPE_VEGETATION))
        {
            cObj = new CollisionRenderObject(entity, objectsCollWorld, renderObject);
        }
    }

    DAVA::Camera* camera = DAVA::GetCamera(entity);
    if ((cObj == nullptr) && (camera != nullptr))
    {
        cObj = new CollisionBox(entity, objectsCollWorld, camera->GetPosition(), debugBoxScale);
    }

    // build simple collision box for all other entities, that has more than two components
    if ((cObj == nullptr) && (entity != nullptr))
    {
        if ((entity->GetComponent(DAVA::Component::SOUND_COMPONENT) != nullptr) ||
            (entity->GetComponent(DAVA::Component::LIGHT_COMPONENT) != nullptr) ||
            (entity->GetComponent(DAVA::Component::WIND_COMPONENT) != nullptr))
        {
            cObj = new CollisionBox(entity, objectsCollWorld, entity->GetWorldTransform().GetTranslationVector(), debugBoxScale);
        }
        else if (entity->GetComponent(DAVA::Component::USER_COMPONENT) != nullptr)
        {
            cObj = new CollisionBox(entity, objectsCollWorld, entity->GetWorldTransform().GetTranslationVector(), debugBoxUserScale);
        }
        else if (GetWaypointComponent(entity) != nullptr)
        {
            cObj = new CollisionBox(entity, objectsCollWorld, entity->GetWorldTransform().GetTranslationVector(), debugBoxWaypointScale);
        }
    }

    if (cObj != nullptr)
    {
        if (entityToCollision.count(entity) > 0)
        {
            DestroyFromEntity(entity);
        }

        entityToCollision[entity] = cObj;
        collisionToEntity[cObj->btObject] = entity;
    }

    if (isLandscape)
    {
        curLandscapeEntity = entity;
    }

    return cObj;
}

void SceneCollisionSystem::DestroyFromEntity(DAVA::BaseObject* entity)
{
    if (curLandscapeEntity == entity)
    {
        curLandscapeEntity = nullptr;
    }

    CollisionBaseObject* cObj = entityToCollision[entity];
    if (cObj != nullptr)
    {
        entityToCollision.erase(entity);
        collisionToEntity.erase(cObj->btObject);
        delete cObj;
    }
}

const SelectableObjectGroup& SceneCollisionSystem::ClipObjectsToPlanes(DAVA::Plane* planes, DAVA::uint32 numPlanes)
{
    planeClippedObjects.Clear();
    for (const auto& object : entityToCollision)
    {
        if ((object.first != nullptr) && (object.second != nullptr) &&
            (object.second->ClassifyToPlanes(planes, numPlanes) == CollisionBaseObject::ClassifyPlanesResult::ContainsOrIntersects))
        {
            planeClippedObjects.Add(object.first, DAVA::AABBox3());
        }
    }

    return planeClippedObjects;
}

// -----------------------------------------------------------------------------------------------
// debug draw
// -----------------------------------------------------------------------------------------------

SceneCollisionDebugDrawer::SceneCollisionDebugDrawer(RenderHelper* _drawer)
    : dbgMode(0)
    , drawer(_drawer)
{
}

SceneCollisionDebugDrawer::~SceneCollisionDebugDrawer()
{
}

void SceneCollisionDebugDrawer::drawLine(const btVector3& from, const btVector3& to, const btVector3& color)
{
    DAVA::Vector3 davaFrom(from.x(), from.y(), from.z());
    DAVA::Vector3 davaTo(to.x(), to.y(), to.z());
    DAVA::Color davaColor(color.x(), color.y(), color.z(), 1.0f);

    drawer->DrawLine(davaFrom, davaTo, davaColor, RenderHelper::DRAW_WIRE_DEPTH);
}

void SceneCollisionDebugDrawer::drawContactPoint(const btVector3& PointOnB, const btVector3& normalOnB, btScalar distance, int lifeTime, const btVector3& color)
{
    DAVA::Color davaColor(color.x(), color.y(), color.z(), 1.0f);
    drawer->DrawIcosahedron(DAVA::Vector3(PointOnB.x(), PointOnB.y(), PointOnB.z()), distance / 20.f, davaColor, RenderHelper::DRAW_SOLID_DEPTH);
}

void SceneCollisionDebugDrawer::reportErrorWarning(const char* warningString)
{
}

void SceneCollisionDebugDrawer::draw3dText(const btVector3& location, const char* textString)
{
}

void SceneCollisionDebugDrawer::setDebugMode(int debugMode)
{
    dbgMode = debugMode;
}

int SceneCollisionDebugDrawer::getDebugMode() const
{
    return dbgMode;
}
