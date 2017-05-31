#include "Base/AlignedAllocator.h"
#include "Math/AABBox3.h"
#include "Functional/Function.h"
#include "Scene/System/CollisionSystem.h"
#include "Scene/System/CollisionSystem/CollisionRenderObject.h"
#include "Scene/System/CollisionSystem/CollisionLandscape.h"
#include "Scene/System/CollisionSystem/CollisionBox.h"
#include "Scene/System/CameraSystem.h"
#include "Scene/SceneEditor2.h"

#include "Commands2/Base/RECommandNotificationObject.h"
#include "Commands2/TransformCommand.h"
#include "Commands2/ParticleEditorCommands.h"
#include "Commands2/EntityParentChangeCommand.h"
#include "Commands2/CreatePlaneLODCommand.h"
#include "Commands2/DeleteLODCommand.h"
#include "Commands2/InspMemberModifyCommand.h"
#include "Commands2/ConvertToBillboardCommand.h"

#include "Settings/SettingsManager.h"

#include "Classes/Selection/Selection.h"

// framework
#include "Scene3D/Components/ComponentHelpers.h"
#include "Scene3D/Components/TransformComponent.h"
#include "Scene3D/Components/SingleComponents/TransformSingleComponent.h"
#include "Scene3D/Scene.h"
#include "Commands2/SetFieldValueCommand.h"

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

    scene->GetEventSystem()->RegisterSystemForEvent(this, DAVA::EventSystem::SWITCH_CHANGED);
}

SceneCollisionSystem::~SceneCollisionSystem()
{
    if (GetScene())
    {
        GetScene()->GetEventSystem()->UnregisterSystemForEvent(this, DAVA::EventSystem::SWITCH_CHANGED);
    }

    for (const auto& etc : objectToCollision)
    {
        delete etc.second;
    }
    objectToCollision.clear();

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

const SelectableGroup::CollectionType& SceneCollisionSystem::ObjectsRayTest(const DAVA::Vector3& from, const DAVA::Vector3& to)
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
            auto entity = collisionToObject[hit.first];
            rayIntersectedEntities.emplace_back(entity);
            DAVA::AABBox3 bbox = GetBoundingBox(entity);
            if (!bbox.IsEmpty())
            {
                rayIntersectedEntities.back().SetBoundingBox(bbox);
            }
        }
    }

    rayIntersectCached = true;
    return rayIntersectedEntities;
}

const SelectableGroup::CollectionType& SceneCollisionSystem::ObjectsRayTestFromCamera()
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
    rayDirection *= DAVA::Min(5.0f, rayLength);
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

void SceneCollisionSystem::UpdateCollisionObject(const Selectable& object)
{
    if (object.CanBeCastedTo<DAVA::Entity>())
    {
        auto entity = object.AsEntity();
        RemoveEntity(entity);
        AddEntity(entity);
    }
    else
    {
        objectsToRemove.insert(object.GetContainedObject());
        objectsToAdd.insert(object.GetContainedObject());
    }
}

DAVA::AABBox3 SceneCollisionSystem::GetBoundingBox(Selectable::Object* object) const
{
    DVASSERT(object != nullptr);

    DAVA::AABBox3 aabox;
    if (objectToCollision.count(object) != 0)
    {
        CollisionBaseObject* collObj = objectToCollision.at(object);
        if (collObj != nullptr)
        {
            aabox = collObj->object.GetBoundingBox();

            Selectable wrapper(object);
            if (wrapper.CanBeCastedTo<DAVA::Entity>())
            {
                auto entity = wrapper.AsEntity();
                for (DAVA::int32 i = 0, e = entity->GetChildrenCount(); i < e; ++i)
                {
                    aabox.AddAABBox(GetBoundingBox(entity->GetChild(i)));
                }
            }
        }
    }

    return aabox;
}

void SceneCollisionSystem::AddCollisionObject(Selectable::Object* obj, CollisionBaseObject* collision)
{
    if (collision == nullptr)
        return;

    DVASSERT(obj != nullptr);
    if (objectToCollision.count(obj) > 0)
    {
        DestroyFromObject(obj);
    }
    objectToCollision[obj] = collision;
    collisionToObject[collision->btObject] = obj;
}

void SceneCollisionSystem::Process(DAVA::float32 timeElapsed)
{
    if (!systemIsEnabled)
    {
        return;
    }

    DAVA::TransformSingleComponent* tsc = GetScene()->transformSingleComponent;
    for (DAVA::Entity* entity : tsc->localTransformChanged)
    {
        UpdateCollisionObject(Selectable(entity));
    }
    for (DAVA::Entity* entity : tsc->transformParentChanged)
    {
        UpdateCollisionObject(Selectable(entity));
    }

    // check in there are entities that should be added or removed
    if (!(objectsToAdd.empty() && objectsToRemove.empty()))
    {
        for (auto obj : objectsToRemove)
        {
            Selectable wrapper(obj);
            EnumerateObjectHierarchy(wrapper, false, [this](Selectable::Object* object, CollisionBaseObject* collision)
                                     {
                                         DestroyFromObject(object);
                                     });
        }

        for (auto obj : objectsToAdd)
        {
            Selectable wrapper(obj);
            if (wrapper.CanBeCastedTo<DAVA::Entity>() || wrapper.SupportsTransformType(Selectable::TransformType::Disabled))
            {
                EnumerateObjectHierarchy(wrapper, true, DAVA::MakeFunction(this, &SceneCollisionSystem::AddCollisionObject));
            }
        }

        objectsToAdd.clear();
        objectsToRemove.clear();
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

bool SceneCollisionSystem::Input(DAVA::UIEvent* event)
{
    // don't have to update last mouse pos when event is not from the mouse
    if (DAVA::eInputDevices::MOUSE == event->device)
    {
        lastMousePos = event->point;
    }
    return false;
}

void SceneCollisionSystem::Draw()
{
    DAVA::RenderHelper* drawer = GetScene()->GetRenderSystem()->GetDebugDrawer();

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
        drawer->DrawIcosahedron(lastLandCollision, 0.5f, DAVA::Color(0, 1.0f, 0, 1.0f), DAVA::RenderHelper::DRAW_SOLID_DEPTH);
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
        const SelectableGroup& selection = Selection::GetSelection();
        for (const auto& item : selection.GetContent())
        {
            // get collision object for solid selected entity
            DVASSERT(item.GetContainedObject() != nullptr);
            CollisionBaseObject* cObj = objectToCollision[item.GetContainedObject()];
            if (NULL != cObj && NULL != cObj->btObject)
            {
                objectsCollWorld->debugDrawObject(cObj->btObject->getWorldTransform(), cObj->btObject->getCollisionShape(), btVector3(1.0f, 0.65f, 0.0f));
            }
        }
    }
}

void SceneCollisionSystem::ProcessCommand(const RECommandNotificationObject& commandNotification)
{
    if (commandNotification.MatchCommandIDs({ CMDID_LANDSCAPE_SET_HEIGHTMAP, CMDID_HEIGHTMAP_MODIFY }))
    {
        UpdateCollisionObject(Selectable(curLandscapeEntity));
    }

    static const DAVA::Vector<DAVA::uint32> acceptableCommands =
    {
      CMDID_LOD_CREATE_PLANE,
      CMDID_LOD_DELETE,
      CMDID_INSP_MEMBER_MODIFY,
      CMDID_REFLECTED_FIELD_MODIFY,
      CMDID_PARTICLE_EFFECT_EMITTER_REMOVE,
      CMDID_TRANSFORM,
      CMDID_CONVERT_TO_BILLBOARD
    };

    if (commandNotification.MatchCommandIDs(acceptableCommands) == false)
        return;

    auto processSingleCommand = [this](const RECommand* command, bool redo) {
        if (command->MatchCommandID(CMDID_INSP_MEMBER_MODIFY))
        {
            static const DAVA::String HEIGHTMAP_PATH = "heightmapPath";
            const InspMemberModifyCommand* cmd = static_cast<const InspMemberModifyCommand*>(command);
            if (HEIGHTMAP_PATH == cmd->member->Name().c_str())
            {
                UpdateCollisionObject(Selectable(curLandscapeEntity));
            }
        }
        else if (command->MatchCommandID(CMDID_REFLECTED_FIELD_MODIFY))
        {
            const DAVA::FastName HEIGHTMAP_PATH("heightmapPath");
            const SetFieldValueCommand* cmd = static_cast<const SetFieldValueCommand*>(command);
            const DAVA::Reflection::Field& field = cmd->GetField();
            if (field.key.Cast<DAVA::FastName>(DAVA::FastName("")) == HEIGHTMAP_PATH)
            {
                UpdateCollisionObject(Selectable(curLandscapeEntity));
            }
        }
        else if (command->MatchCommandID(CMDID_LOD_CREATE_PLANE))
        {
            const CreatePlaneLODCommand* createPlaneLODCommand = static_cast<const CreatePlaneLODCommand*>(command);
            UpdateCollisionObject(Selectable(createPlaneLODCommand->GetEntity()));
        }
        else if (command->MatchCommandIDs({ CMDID_LOD_DELETE }))
        {
            const DeleteLODCommand* deleteLODCommand = static_cast<const DeleteLODCommand*>(command);
            UpdateCollisionObject(Selectable(deleteLODCommand->GetEntity()));
        }
        else if (command->MatchCommandID(CMDID_PARTICLE_EFFECT_EMITTER_REMOVE))
        {
            auto cmd = static_cast<const CommandRemoveParticleEmitter*>(command);
            (redo ? objectsToRemove : objectsToAdd).insert(cmd->GetEmitterInstance());
        }
        else if (command->MatchCommandID(CMDID_TRANSFORM))
        {
            auto cmd = static_cast<const TransformCommand*>(command);
            UpdateCollisionObject(cmd->GetTransformedObject());
        }
        else if (command->MatchCommandID(CMDID_CONVERT_TO_BILLBOARD))
        {
            auto cmd = static_cast<const ConvertToBillboardCommand*>(command);
            UpdateCollisionObject(Selectable(cmd->GetEntity()));
        }
    };

    commandNotification.ExecuteForAllCommands(processSingleCommand);
}

void SceneCollisionSystem::ImmediateEvent(DAVA::Component* component, DAVA::uint32 event)
{
    if (!systemIsEnabled)
    {
        return;
    }

    switch (event)
    {
    case DAVA::EventSystem::SWITCH_CHANGED:
    {
        UpdateCollisionObject(Selectable(component->GetEntity()));
        break;
    }
    default:
        break;
    }
}

void SceneCollisionSystem::AddEntity(DAVA::Entity* entity)
{
    if (!systemIsEnabled || entity == nullptr)
        return;

    if (DAVA::GetLandscape(entity) != nullptr)
    {
        curLandscapeEntity = entity;
    }

    objectsToRemove.erase(entity);
    objectsToAdd.insert(entity);

    // build collision object for entity childs
    for (int i = 0; i < entity->GetChildrenCount(); ++i)
    {
        AddEntity(entity->GetChild(i));
    }
}

void SceneCollisionSystem::RemoveEntity(DAVA::Entity* entity)
{
    if (!systemIsEnabled || entity == nullptr)
        return;

    if (curLandscapeEntity == entity)
    {
        curLandscapeEntity = nullptr;
    }

    objectsToAdd.erase(entity);
    objectsToRemove.insert(entity);

    // destroy collision object for entities childs
    for (int i = 0; i < entity->GetChildrenCount(); ++i)
    {
        RemoveEntity(entity->GetChild(i));
    }
}

void SceneCollisionSystem::DestroyFromObject(Selectable::Object* entity)
{
    CollisionBaseObject* cObj = objectToCollision[entity];
    if (cObj != nullptr)
    {
        objectToCollision.erase(entity);
        collisionToObject.erase(cObj->btObject);
        delete cObj;
    }
}

const SelectableGroup& SceneCollisionSystem::ClipObjectsToPlanes(const DAVA::Vector<DAVA::Plane>& planes)
{
    planeClippedObjects.Clear();
    for (const auto& object : objectToCollision)
    {
        if ((object.first != nullptr) && (object.second != nullptr) &&
            (object.second->ClassifyToPlanes(planes) == CollisionBaseObject::ClassifyPlanesResult::ContainsOrIntersects))
        {
            planeClippedObjects.Add(object.first, DAVA::AABBox3());
        }
    }

    return planeClippedObjects;
}

void SceneCollisionSystem::EnableSystem()
{
    EditorSceneSystem::EnableSystem();
    AddEntity(GetScene());
}

namespace CollisionDetails
{
struct CollisionObj
{
    bool isValid = false;
    CollisionBaseObject* collisionObject = nullptr;
};

template <typename T, class... Args>
CollisionObj InitCollision(bool createCollision, Args... args)
{
    CollisionObj result;
    result.isValid = true;
    if (createCollision)
    {
        result.collisionObject = new T(args...);
    }

    return result;
}
}

void SceneCollisionSystem::EnumerateObjectHierarchy(const Selectable& object, bool createCollision, const TCallBack& callback)
{
    DAVA::float32 debugBoxScale = SIMPLE_COLLISION_BOX_SIZE * SettingsManager::GetValue(Settings::Scene_DebugBoxScale).AsFloat();
    DAVA::float32 debugBoxParticleScale = SIMPLE_COLLISION_BOX_SIZE * SettingsManager::GetValue(Settings::Scene_DebugBoxParticleScale).AsFloat();
    if (object.CanBeCastedTo<DAVA::Entity>())
    {
        CollisionDetails::CollisionObj result;
        DAVA::Entity* entity = object.AsEntity();

        DAVA::float32 debugBoxUserScale = SIMPLE_COLLISION_BOX_SIZE * SettingsManager::GetValue(Settings::Scene_DebugBoxUserScale).AsFloat();
        DAVA::float32 debugBoxWaypointScale = SIMPLE_COLLISION_BOX_SIZE * SettingsManager::GetValue(Settings::Scene_DebugBoxWaypointScale).AsFloat();

        DAVA::Landscape* landscape = DAVA::GetLandscape(entity);
        if (landscape != nullptr)
        {
            result = CollisionDetails::InitCollision<CollisionLandscape>(createCollision, entity, landCollWorld, landscape);
        }

        DAVA::ParticleEffectComponent* particleEffect = DAVA::GetEffectComponent(entity);
        if ((result.isValid == false) && (particleEffect != nullptr))
        {
            for (DAVA::int32 i = 0, e = particleEffect->GetEmittersCount(); i < e; ++i)
            {
                EnumerateObjectHierarchy(Selectable(particleEffect->GetEmitterInstance(i)), createCollision, callback);
            }

            result = CollisionDetails::InitCollision<CollisionBox>(createCollision, entity, objectsCollWorld, entity->GetWorldTransform().GetTranslationVector(), debugBoxParticleScale);
        }

        DAVA::RenderObject* renderObject = DAVA::GetRenderObject(entity);
        if ((result.isValid == false) && (renderObject != nullptr) && entity->IsLodMain(0))
        {
            DAVA::RenderObject::eType objType = renderObject->GetType();
            if (objType == DAVA::RenderObject::TYPE_BILLBOARD)
            {
                const DAVA::AABBox3& box = renderObject->GetBoundingBox();
                result = CollisionDetails::InitCollision<CollisionBox>(createCollision, entity, objectsCollWorld, box.GetCenter(), box.GetSize().x);
            }
            else if ((objType != DAVA::RenderObject::TYPE_SPRITE) && (objType != DAVA::RenderObject::TYPE_VEGETATION))
            {
                result = CollisionDetails::InitCollision<CollisionRenderObject>(createCollision, entity, objectsCollWorld, renderObject);
            }
        }

        DAVA::Camera* camera = DAVA::GetCamera(entity);
        if ((result.isValid == false) && (camera != nullptr))
        {
            result = CollisionDetails::InitCollision<CollisionBox>(createCollision, entity, objectsCollWorld, camera->GetPosition(), debugBoxScale);
        }

        // build simple collision box for all other entities, that has more than two components
        if ((result.isValid == false) && (entity != nullptr))
        {
            if ((entity->GetComponent(DAVA::Component::SOUND_COMPONENT) != nullptr) ||
                (entity->GetComponent(DAVA::Component::LIGHT_COMPONENT) != nullptr) ||
                (entity->GetComponent(DAVA::Component::WIND_COMPONENT) != nullptr))
            {
                result = CollisionDetails::InitCollision<CollisionBox>(createCollision, entity, objectsCollWorld, entity->GetWorldTransform().GetTranslationVector(), debugBoxScale);
            }
            else if (entity->GetComponent(DAVA::Component::USER_COMPONENT) != nullptr)
            {
                result = CollisionDetails::InitCollision<CollisionBox>(createCollision, entity, objectsCollWorld, entity->GetWorldTransform().GetTranslationVector(), debugBoxUserScale);
            }
            else if (GetWaypointComponent(entity) != nullptr)
            {
                result = CollisionDetails::InitCollision<CollisionBox>(createCollision, entity, objectsCollWorld, entity->GetWorldTransform().GetTranslationVector(), debugBoxWaypointScale);
            }
        }

        if (result.isValid == true)
        {
            callback(entity, result.collisionObject);
        }
    }
    else
    {
        DAVA::float32 scale = object.CanBeCastedTo<DAVA::ParticleEmitterInstance>() ? debugBoxParticleScale : debugBoxScale;
        Selectable::Object* containedObject = object.GetContainedObject();
        CollisionDetails::CollisionObj result = CollisionDetails::InitCollision<CollisionBox>(createCollision, containedObject, objectsCollWorld, object.GetWorldTransform().GetTranslationVector(), scale);
        callback(containedObject, result.collisionObject);
    }
}

DAVA::AABBox3 SceneCollisionSystem::GetUntransformedBoundingBox(Selectable::Object* entity) const
{
    return GetTransformedBoundingBox(Selectable(entity), DAVA::Matrix4::IDENTITY);
}

DAVA::AABBox3 SceneCollisionSystem::GetTransformedBoundingBox(const Selectable& object, const DAVA::Matrix4& transform) const
{
    DAVA::AABBox3 entityBox = GetBoundingBox(object.GetContainedObject());
    if (object.CanBeCastedTo<DAVA::Entity>())
    {
        // add childs boxes into entity box
        auto entity = object.AsEntity();
        for (DAVA::int32 i = 0; i < entity->GetChildrenCount(); i++)
        {
            Selectable childEntity(entity->GetChild(i));
            DAVA::AABBox3 childBox = GetTransformedBoundingBox(childEntity, childEntity.GetLocalTransform());
            if (childBox.IsEmpty() == false)
            {
                if (entityBox.IsEmpty())
                {
                    entityBox = childBox;
                }
                else
                {
                    entityBox.AddAABBox(childBox);
                }
            }
        }
    }

    DAVA::AABBox3 ret;
    if (entityBox.IsEmpty() == false)
    {
        entityBox.GetTransformedBox(transform, ret);
    }
    return ret;
}

// -----------------------------------------------------------------------------------------------
// debug draw
// -----------------------------------------------------------------------------------------------

SceneCollisionDebugDrawer::SceneCollisionDebugDrawer(DAVA::RenderHelper* _drawer)
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

    drawer->DrawLine(davaFrom, davaTo, davaColor, DAVA::RenderHelper::DRAW_WIRE_DEPTH);
}

void SceneCollisionDebugDrawer::drawContactPoint(const btVector3& PointOnB, const btVector3& normalOnB, btScalar distance, int lifeTime, const btVector3& color)
{
    DAVA::Color davaColor(color.x(), color.y(), color.z(), 1.0f);
    drawer->DrawIcosahedron(DAVA::Vector3(PointOnB.x(), PointOnB.y(), PointOnB.z()), distance / 20.f, davaColor, DAVA::RenderHelper::DRAW_SOLID_DEPTH);
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
