#include "Scene3D/Components/TransformComponent.h"
#include "Scene3D/Entity.h"
#include "Scene3D/Scene.h"
#include "Scene3D/Systems/EventSystem.h"
#include "Scene3D/Systems/GlobalEventSystem.h"

namespace DAVA
{
Component* TransformComponent::Clone(Entity* toEntity)
{
    TransformComponent* newTransform = new TransformComponent();
    newTransform->SetEntity(toEntity);
    newTransform->localMatrix = localMatrix;
    newTransform->worldMatrix = worldMatrix;
    newTransform->parent = this->parent;

    return newTransform;
}

void TransformComponent::SetLocalTransform(const Matrix4* transform)
{
    localMatrix = *transform;
    if (!parent)
    {
        worldMatrix = *transform;
    }

    GlobalEventSystem::Instance()->Event(this, EventSystem::LOCAL_TRANSFORM_CHANGED);
}

void TransformComponent::SetParent(Entity* node)
{
    parent = node;

    if (node)
    {
        parentMatrix = (static_cast<TransformComponent*>(node->GetComponent(Component::TRANSFORM_COMPONENT)))->GetWorldTransformPtr();
    }
    else
    {
        parentMatrix = 0;
    }

    GlobalEventSystem::Instance()->Event(this, EventSystem::TRANSFORM_PARENT_CHANGED);
}

Matrix4& TransformComponent::ModifyLocalTransform()
{
    GlobalEventSystem::Instance()->Event(this, EventSystem::LOCAL_TRANSFORM_CHANGED);
    return localMatrix;
}

void TransformComponent::Serialize(KeyedArchive* archive, SerializationContext* serializationContext)
{
    Component::Serialize(archive, serializationContext);

    if (NULL != archive)
    {
        archive->SetMatrix4("tc.localMatrix", localMatrix);
        archive->SetMatrix4("tc.worldMatrix", worldMatrix);
    }
}

void TransformComponent::Deserialize(KeyedArchive* archive, SerializationContext* sceneFile)
{
    if (NULL != archive)
    {
        localMatrix = archive->GetMatrix4("tc.localMatrix", Matrix4::IDENTITY);
        worldMatrix = archive->GetMatrix4("tc.worldMatrix", Matrix4::IDENTITY);
    }

    Component::Deserialize(archive, sceneFile);
}
};