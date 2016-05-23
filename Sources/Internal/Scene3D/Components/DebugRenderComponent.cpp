#include "Scene3D/Components/DebugRenderComponent.h"

namespace DAVA
{
DebugRenderComponent::DebugRenderComponent()
    : curDebugFlags(DEBUG_DRAW_NONE)
{
}

DebugRenderComponent::~DebugRenderComponent()
{
}

void DebugRenderComponent::SetDebugFlags(uint32 debugFlags)
{
    curDebugFlags = debugFlags;
}

uint32 DebugRenderComponent::GetDebugFlags() const
{
    return curDebugFlags;
}

Component* DebugRenderComponent::Clone(Entity* toEntity)
{
    DebugRenderComponent* component = new DebugRenderComponent();
    component->SetEntity(toEntity);
    component->curDebugFlags = curDebugFlags;
    return component;
}

void DebugRenderComponent::Serialize(KeyedArchive* archive, SerializationContext* serializationContext)
{
    // Don't need to save
}

void DebugRenderComponent::Deserialize(KeyedArchive* archive, SerializationContext* serializationContext)
{
    // Don't need to load
}
};
