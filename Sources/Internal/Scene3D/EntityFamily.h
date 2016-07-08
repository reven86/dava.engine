#ifndef __DAVAENGINE_ENTITY_FAMILY_H__
#define __DAVAENGINE_ENTITY_FAMILY_H__

#include "Entity/BaseFamily.h"
#include "Entity/Component.h"

namespace DAVA
{
class EntityFamily : public BaseFamily<Component>
{
private:
    EntityFamily(const Vector<Component*>& components);

public:
    static EntityFamily* GetOrCreate(const Vector<Component*>& components);
    static void Release(EntityFamily*& family);

private:
    static BaseFamilyRepository<EntityFamily> repository;
};
}

#endif //__DAVAENGINE_ENTITY_FAMILY_H_
