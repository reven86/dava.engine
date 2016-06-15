#ifndef __DAVAENGINE_TRANSFORM_COMPONENT_H__
#define __DAVAENGINE_TRANSFORM_COMPONENT_H__

#include "Base/BaseTypes.h"
#include "Scene3D/Systems/TransformSystem.h"
#include "Entity/Component.h"
#include "Scene3D/SceneFile/SerializationContext.h"

namespace DAVA
{
class Entity;

class TransformComponent : public Component
{
protected:
    virtual ~TransformComponent();

public:
    TransformComponent();

    IMPLEMENT_COMPONENT_TYPE(TRANSFORM_COMPONENT)

    inline Matrix4* GetWorldTransformPtr();
    inline const Matrix4& GetWorldTransform();
    inline const Matrix4& GetLocalTransform();
    Matrix4& ModifyLocalTransform();

    inline int32 GetIndex();

    void SetLocalTransform(const Matrix4* transform);
    void SetParent(Entity* node);

    Component* Clone(Entity* toEntity) override;
    void Serialize(KeyedArchive* archive, SerializationContext* serializationContext) override;
    void Deserialize(KeyedArchive* archive, SerializationContext* serializationContext) override;

private:
    Matrix4 localMatrix;
    Matrix4 worldMatrix;
    Matrix4* parentMatrix;
    Entity* parent; //Entity::parent should be removed

    int32 index;

    friend class TransformSystem;

public:
    INTROSPECTION_EXTEND(TransformComponent, Component,
                         MEMBER(localMatrix, "Local Transform", I_SAVE | I_VIEW)
                         MEMBER(worldMatrix, "World Transform", I_SAVE | I_VIEW)
                         MEMBER(parentMatrix, "Parent Matrix", I_SAVE)
                         )
};

const Matrix4& TransformComponent::GetWorldTransform()
{
    return worldMatrix;
}

const Matrix4& TransformComponent::GetLocalTransform()
{
    return localMatrix;
}

int32 TransformComponent::GetIndex()
{
    return index;
}

Matrix4* TransformComponent::GetWorldTransformPtr()
{
    return &worldMatrix;
}
};

#endif //__DAVAENGINE_TRANSFORM_COMPONENT_H__
