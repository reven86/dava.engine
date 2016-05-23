#include "RotationControllerComponent.h"

namespace DAVA
{
Component* RotationControllerComponent::Clone(Entity* toEntity)
{
    RotationControllerComponent* component = new RotationControllerComponent();
    component->SetEntity(toEntity);

    return component;
}
};
