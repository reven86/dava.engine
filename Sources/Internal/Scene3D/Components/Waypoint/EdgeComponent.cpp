#include "Scene3D/Components/Waypoint/EdgeComponent.h"
#include "Scene3D/Entity.h"

namespace DAVA
{
EdgeComponent::EdgeComponent()
    : Component()
    , nextEntity(NULL)
    , properties(NULL)
{
    properties = new KeyedArchive();
}

EdgeComponent::~EdgeComponent()
{
    SafeRelease(properties);
    SafeRelease(nextEntity);
}

EdgeComponent::EdgeComponent(const EdgeComponent& cp)
    : Component(cp)
{
    SetNextEntity(cp.nextEntity);
    SetProperties(cp.GetProperties());
}

Component* EdgeComponent::Clone(Entity* toEntity)
{
    EdgeComponent* newComponent = new EdgeComponent();
    newComponent->SetEntity(toEntity);
    newComponent->SetNextEntity(GetNextEntity());
    newComponent->SetProperties(properties);

    return newComponent;
}

void EdgeComponent::Serialize(KeyedArchive* archive, SerializationContext* serializationContext)
{
    DVASSERT(false);
}

void EdgeComponent::Deserialize(KeyedArchive* archive, SerializationContext* serializationContext)
{
    DVASSERT(false);
}

void EdgeComponent::SetProperties(KeyedArchive* archieve)
{
    SafeRelease(properties);
    properties = new KeyedArchive(*archieve);
}

void EdgeComponent::SetNextEntity(Entity* _entity)
{
    if (nextEntity != _entity)
    {
        SafeRelease(nextEntity);
        nextEntity = SafeRetain(_entity);
    }
}

void EdgeComponent::SetNextEntityName(const FastName& name)
{
    //do nothing
}

const FastName EdgeComponent::GetNextEntityName() const
{
    FastName nextEntityName;
    if (nextEntity)
    {
        nextEntityName = nextEntity->GetName();
    }

    return nextEntityName;
}

void EdgeComponent::SetNextEntityTag(int32 tag)
{
    //do nothing
}

int32 EdgeComponent::GetNextEntityTag() const
{
    int32 tag = 0;
    if (nextEntity)
    {
        tag = nextEntity->GetTag();
    }

    return tag;
}
}