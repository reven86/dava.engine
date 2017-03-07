#include "WASDControllerComponent.h"
#include "Reflection/ReflectionRegistrator.h"

namespace DAVA
{
DAVA_VIRTUAL_REFLECTION_IMPL(WASDControllerComponent)
{
    ReflectionRegistrator<WASDControllerComponent>::Begin()
    .End();
}

Component* WASDControllerComponent::Clone(Entity* toEntity)
{
    WASDControllerComponent* component = new WASDControllerComponent();
    component->SetEntity(toEntity);

    return component;
}
};
