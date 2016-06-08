#include "EntityFamily.h"
#include "Entity.h"

namespace DAVA
{
BaseFamilyRepository<EntityFamily> EntityFamily::repository;

EntityFamily::EntityFamily(const Vector<Component*>& components)
    : BaseFamily<Component>(components)
{
}

EntityFamily* EntityFamily::GetOrCreate(const Vector<Component*>& components)
{
    return repository.GetOrCreate(EntityFamily(components));
}

void EntityFamily::Release(EntityFamily*& family)
{
    repository.ReleaseFamily(family);
    family = nullptr;
}
}
