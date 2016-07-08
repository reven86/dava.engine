#include "Scene3D/Components/UpdatableComponent.h"
#include "Scene3D/Systems/UpdateSystem.h"
#include "Scene3D/Entity.h"
#include "Scene3D/Scene.h"

namespace DAVA
{
UpdatableComponent::UpdatableComponent()
    : updatableObject(0)
{
}

Component* UpdatableComponent::Clone(Entity* toEntity)
{
    UpdatableComponent* newComponent = new UpdatableComponent();
    newComponent->SetEntity(toEntity);

    newComponent->SetUpdatableObject(updatableObject);

    return newComponent;
}

void UpdatableComponent::SetUpdatableObject(IUpdatable* _updatableObject)
{
    updatableObject = _updatableObject;

    if (entity)
    {
        entity->GetScene()->updatableSystem->RemoveEntity(entity);
        entity->GetScene()->updatableSystem->AddEntity(entity);
    }
}

IUpdatable* UpdatableComponent::GetUpdatableObject()
{
    return updatableObject;
}
}