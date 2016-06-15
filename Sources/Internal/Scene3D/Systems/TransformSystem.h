#ifndef __DAVAENGINE_TRANSFORM_SYSTEM_H__
#define __DAVAENGINE_TRANSFORM_SYSTEM_H__

#include "Base/BaseTypes.h"
#include "Math/MathConstants.h"
#include "Math/Matrix4.h"
#include "Base/Singleton.h"
#include "Entity/SceneSystem.h"

namespace DAVA
{
class Entity;
class Transform;

class TransformSystem : public SceneSystem
{
public:
    TransformSystem(Scene* scene);
    ~TransformSystem();

    void ImmediateEvent(Component* component, uint32 event) override;
    void AddEntity(Entity* entity) override;
    void RemoveEntity(Entity* entity) override;

    void LinkTransform(int32 parentIndex, int32 childIndex);
    void UnlinkTransform(int32 childIndex);

    void Process(float32 timeElapsed) override;

private:
    void SortAndThreadSplit();

    Vector<Entity*> updatableEntities;
    Vector<Component*> sendEvent;

    void EntityNeedUpdate(Entity* entity);
    void HierahicAddToUpdate(Entity* entity);
    void FindNodeThatRequireUpdate(Entity* entity);
    void TransformAllChildEntities(Entity* entity);
    void RecursiveTransformCheck(Entity* entity);

    void HierahicFindUpdatableTransform(Entity* entity, bool forcedUpdate = false);

    int32 passedNodes;
    int32 multipliedNodes;
};
};

#endif //__DAVAENGINE_TRANSFORM_SYSTEM_H__
