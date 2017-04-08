#pragma once

#include "Base/BaseTypes.h"
#include "Reflection/Reflection.h"
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
    RenderComponent(RenderObject* _object = nullptr);

    IMPLEMENT_COMPONENT_TYPE(RENDER_COMPONENT);

    void SetRenderObject(RenderObject* object);
    RenderObject* GetRenderObject();

    Component* Clone(Entity* toEntity) override;
    void Serialize(KeyedArchive* archive, SerializationContext* serializationContext) override;
    void Deserialize(KeyedArchive* archive, SerializationContext* serializationContext) override;
    void GetDataNodes(Set<DataNode*>& dataNodes) override;
    void OptimizeBeforeExport() override;

private:
    RenderObject* renderObject = nullptr;

public:
    INTROSPECTION_EXTEND(RenderComponent, Component,
                         MEMBER(renderObject, "renderObject", I_SAVE | I_VIEW | I_EDIT)
                         );

    DAVA_VIRTUAL_REFLECTION(RenderComponent, Component);
};
}
