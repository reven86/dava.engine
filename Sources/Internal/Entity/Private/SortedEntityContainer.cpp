#include "Entity/SortedEntityContainer.h"
#include "Scene3D/Entity.h"
#include "Scene3D/EntityFamily.h"
#include "Debug/DVAssert.h"

namespace DAVA
{
void SortedEntityContainer::Push(Entity* entity)
{
    DVASSERT(entity);
    map[entity->GetFamily()].push_back(entity);
}

void SortedEntityContainer::Clear()
{
    map.clear();
}
}
