#ifndef __DAVAENGINE_SCENE3D_RENDER_COMPONENT_H__
#define __DAVAENGINE_SCENE3D_RENDER_COMPONENT_H__

#include "Base/BaseTypes.h"
#include "Entity/Component.h"
#include "Render/Highlevel/RenderObject.h"
#include "Scene3D/SceneFile/SerializationContext.h"

namespace DAVA
{
class RenderComponent : public Component
{
protected:
    virtual ~RenderComponent();

public:
    RenderComponent(RenderObject* _object = 0);

    IMPLEMENT_COMPONENT_TYPE(RENDER_COMPONENT);

    void SetRenderObject(RenderObject* object);
    RenderObject* GetRenderObject();

    Component* Clone(Entity* toEntity) override;
    void Serialize(KeyedArchive* archive, SerializationContext* serializationContext) override;
    void Deserialize(KeyedArchive* archive, SerializationContext* serializationContext) override;
    void GetDataNodes(Set<DataNode*>& dataNodes) override;
    void OptimizeBeforeExport() override;

private:
    RenderObject* renderObject;

public:
    INTROSPECTION_EXTEND(RenderComponent, Component,
                         MEMBER(renderObject, "renderObject", I_SAVE | I_VIEW | I_EDIT)
                         );
};
};

#endif //__DAVAENGINE_SCENE3D_RENDER_COMPONENT_H__
