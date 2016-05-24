#include "Scene3D/Components/UserComponent.h"
#include "Scene3D/Entity.h"

namespace DAVA
{
UserComponent::UserComponent()
{
}

Component* UserComponent::Clone(Entity* toEntity)
{
    UserComponent* uc = new UserComponent();
    uc->SetEntity(toEntity);

    return uc;
}

void UserComponent::Serialize(KeyedArchive* archive, SerializationContext* serializationContext)
{
    Component::Serialize(archive, serializationContext);
}

void UserComponent::Deserialize(KeyedArchive* archive, SerializationContext* serializationContext)
{
    Component::Deserialize(archive, serializationContext);
}
}
