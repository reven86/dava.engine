#include "Scene3D/Components/Waypoint/WaypointComponent.h"
#include "Scene3D/Entity.h"

#include "Reflection/ReflectionRegistrator.h"
#include "Reflection/ReflectedMeta.h"

namespace DAVA
{
DAVA_VIRTUAL_REFLECTION_IMPL(WaypointComponent)
{
    ReflectionRegistrator<WaypointComponent>::Begin()
    .Field("pathName", &WaypointComponent::pathName)[M::ReadOnly(), M::DisplayName("Path Name")]
    .Field("properties", &WaypointComponent::properties)[M::DisplayName("Waypoint properties")]
    .End();
}

WaypointComponent::WaypointComponent()
    : Component()
{
    properties = new KeyedArchive();
}

WaypointComponent::~WaypointComponent()
{
    SafeRelease(properties);
}

Component* WaypointComponent::Clone(Entity* toEntity)
{
    WaypointComponent* newComponent = new WaypointComponent();
    newComponent->SetEntity(toEntity);
    newComponent->SetProperties(properties);
    newComponent->SetPathName(pathName);
    newComponent->SetStarting(false);
    return newComponent;
}

void WaypointComponent::Serialize(KeyedArchive* archive, SerializationContext* serializationContext)
{
    DVASSERT(false);
}

void WaypointComponent::Deserialize(KeyedArchive* archive, SerializationContext* serializationContext)
{
    DVASSERT(false);
}

void WaypointComponent::SetProperties(KeyedArchive* archieve)
{
    SafeRelease(properties);
    properties = new KeyedArchive(*archieve);
}
}