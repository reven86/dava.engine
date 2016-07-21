#ifndef __SCENE_COLLISION_SYSTEM_H__
#define __SCENE_COLLISION_SYSTEM_H__

#include "Scene/SelectableGroup.h"
#include "Scene/SceneTypes.h"

// bullet
#include "bullet/btBulletCollisionCommon.h"

// framework
#include "UI/UIEvent.h"
#include "Entity/SceneSystem.h"
#include "Render/Highlevel/Landscape.h"
#include "Render/RenderHelper.h"

class RECommand;
class CollisionBaseObject;
class SceneCollisionDebugDrawer;
enum CollisionSystemDrawMode
{
    CS_DRAW_NOTHING = 0x0,

    CS_DRAW_OBJECTS = 0x1,
    CS_DRAW_OBJECTS_SELECTED = 0x2,
    CS_DRAW_OBJECTS_RAYTEST = 0x4,

    CS_DRAW_LAND = 0x10,
    CS_DRAW_LAND_RAYTEST = 0x20,
    CS_DRAW_LAND_COLLISION = 0x40,

    CS_DRAW_DEFAULT = CS_DRAW_NOTHING,
    CS_DRAW_ALL = 0xFFFFFFFF
};

class SceneCollisionSystem : public DAVA::SceneSystem
{
    friend class SceneEditor2;
    friend class EntityModificationSystem;

public:
    SceneCollisionSystem(DAVA::Scene* scene);
    ~SceneCollisionSystem();

    void SetDrawMode(int mode);
    int GetDrawMode() const;

    DAVA::AABBox3 GetBoundingBox(Selectable::Object* object);

    const SelectableGroup::CollectionType& ObjectsRayTest(const DAVA::Vector3& from, const DAVA::Vector3& to);
    const SelectableGroup::CollectionType& ObjectsRayTestFromCamera();

    bool LandRayTest(const DAVA::Vector3& from, const DAVA::Vector3& to, DAVA::Vector3& intersectionPoint);
    bool LandRayTestFromCamera(DAVA::Vector3& intersectionPoint);

    DAVA::Landscape* GetLandscape() const;

    void UpdateCollisionObject(const Selectable& object);

    void Process(DAVA::float32 timeElapsed) override;
    void Input(DAVA::UIEvent* event) override;

    const SelectableGroup& ClipObjectsToPlanes(const DAVA::Vector<DAVA::Plane>& planes);

    void EnableSystem();

private:
    void Draw();

    void ProcessCommand(const RECommand* command, bool redo);

    void ImmediateEvent(DAVA::Component* component, DAVA::uint32 event) override;
    void AddEntity(DAVA::Entity* entity) override;
    void RemoveEntity(DAVA::Entity* entity) override;

    void DestroyFromObject(Selectable::Object* entity);
    void AddCollisionObject(Selectable::Object* obj, CollisionBaseObject* collision);

    using TCallBack = DAVA::Function<void(Selectable::Object*, CollisionBaseObject*)>;
    void EnumerateObjectHierarchy(const Selectable& object, bool createCollision, const TCallBack& callback);

    DAVA::Vector3 lastRayFrom;
    DAVA::Vector3 lastRayTo;
    DAVA::Vector2 lastMousePos;
    DAVA::Vector3 lastLandRayFrom;
    DAVA::Vector3 lastLandRayTo;
    DAVA::Vector3 lastLandCollision;
    DAVA::Set<Selectable::Object*> objectsToAdd;
    DAVA::Set<Selectable::Object*> objectsToRemove;
    DAVA::Map<Selectable::Object*, CollisionBaseObject*> objectToCollision;
    DAVA::Map<btCollisionObject*, Selectable::Object*> collisionToObject;
    DAVA::Entity* curLandscapeEntity = nullptr;
    SelectableGroup::CollectionType rayIntersectedEntities;
    SelectableGroup planeClippedObjects;
    btDefaultCollisionConfiguration* objectsCollConf = nullptr;
    btCollisionDispatcher* objectsCollDisp = nullptr;
    btAxisSweep3* objectsBroadphase = nullptr;
    btCollisionWorld* objectsCollWorld = nullptr;
    SceneCollisionDebugDrawer* objectsDebugDrawer = nullptr;
    btDefaultCollisionConfiguration* landCollConf = nullptr;
    btCollisionDispatcher* landCollDisp = nullptr;
    btAxisSweep3* landBroadphase = nullptr;
    btCollisionWorld* landCollWorld = nullptr;
    SceneCollisionDebugDrawer* landDebugDrawer = nullptr;
    int drawMode = CS_DRAW_DEFAULT;
    bool rayIntersectCached = false;
    bool landIntersectCached = false;
    bool landIntersectCachedResult = false;

    bool enabled = false;
};

class SceneCollisionDebugDrawer : public btIDebugDraw
{
public:
    SceneCollisionDebugDrawer(DAVA::RenderHelper* _drawer);
    ~SceneCollisionDebugDrawer();

    void drawLine(const btVector3& from, const btVector3& to, const btVector3& color) override;
    void drawContactPoint(const btVector3& PointOnB, const btVector3& normalOnB, btScalar distance, int lifeTime, const btVector3& color) override;
    void reportErrorWarning(const char* warningString) override;
    void draw3dText(const btVector3& location, const char* textString) override;
    void setDebugMode(int debugMode) override;
    int getDebugMode() const override;

protected:
    int dbgMode;
    DAVA::RenderHelper* drawer;
};

#endif // __SCENE_COLLISION_SYSTEM_H__