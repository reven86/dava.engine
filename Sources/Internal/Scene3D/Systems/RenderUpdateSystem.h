#ifndef __DAVAENGINE_SCENE3D_RENDERUPDATESYSTEM_H__
#define __DAVAENGINE_SCENE3D_RENDERUPDATESYSTEM_H__

#include "Base/BaseTypes.h"
#include "Base/HashMap.h"
#include "Base/FastNameMap.h"
#include "Entity/SceneSystem.h"

namespace DAVA
{
class RenderPass;
class RenderLayer;
class RenderObject;
class RenderBatch;
class Entity;
class Camera;

/**
    \brief This system is required to transfer all changes from scene to render system and update render object properties.
 */
class RenderUpdateSystem : public SceneSystem
{
public:
    RenderUpdateSystem(Scene* scene);

    void AddEntity(Entity* entity) override;
    void RemoveEntity(Entity* entity) override;
    void Process(float32 timeElapsed) override;

private:
    void UpdateActiveIndexes(Entity* entity, RenderObject* object);
    HashMap<Entity*, RenderObject*> entityObjectMap;
};

} // ns

#endif /* __DAVAENGINE_RENDER_RENDERSYSTEM_H__ */
