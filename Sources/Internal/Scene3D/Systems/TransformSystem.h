#pragma once

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

    void AddEntity(Entity* entity) override;
    void RemoveEntity(Entity* entity) override;

    void Process(float32 timeElapsed) override;

private:
    Vector<Entity*> updatableEntities;
    Vector<Component*> sendEvent;

    void EntityNeedUpdate(Entity* entity);
    void HierahicAddToUpdate(Entity* entity);
    void FindNodeThatRequireUpdate(Entity* entity);
    void TransformAllChildEntities(Entity* entity);
    void HierahicFindUpdatableTransform(Entity* entity, bool forcedUpdate = false);

    int32 passedNodes;
    int32 multipliedNodes;
};
};
