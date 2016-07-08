#include "WASDControllerComponent.h"

namespace DAVA
{
Component* WASDControllerComponent::Clone(Entity* toEntity)
{
    WASDControllerComponent* component = new WASDControllerComponent();
    component->SetEntity(toEntity);

    return component;
}
};
