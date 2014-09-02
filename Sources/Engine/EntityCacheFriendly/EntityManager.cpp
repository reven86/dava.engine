/*==================================================================================
    Copyright (c) 2008, binaryzebra
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    * Neither the name of the binaryzebra nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE binaryzebra AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL binaryzebra BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/



#include "Entity/Entity.h"
#include "Entity/EntityManager.h"
#include "Entity/Component.h"
#include "Math/AABBox3.h"

namespace DAVA 
{

    
Map<const char *, Pool *> EntityManager::poolAllocators;

Entity * EntityManager::CreateEntity()
{
	Entity * entity = new Entity(this);
	AddToGlobalList(entity);
	return entity;
}

void EntityManager::DestroyEntity(Entity * & entity)
{
	deleteEntities.push_back(entity);
	entity = 0; //seems to be dead now
}
    
void EntityManager::AddComponent(Entity * entity, Component * component)
{
    const ComponentType & addType = component->GetType();

	Map<Entity*, EntityFamilyType>::iterator it = newFamilyEntities.find(entity);
	if(newFamilyEntities.end() != it)
	{
		EntityFamilyType newFamilyType = EntityFamilyType::AddComponentType(it->second, addType);
		it->second = newFamilyType;
	}
	else
	{
		EntityFamilyType oldFamilyType = entity->GetFamily();
		EntityFamilyType newFamilyType = EntityFamilyType::AddComponentType(oldFamilyType, addType);
		newFamilyEntities[entity] = newFamilyType;
	}
}

void EntityManager::AddComponent(Entity * entity, const char * componentName)
{
	//Map<const char *, Component * >::iterator it = Component::cache.find(componentName);
	//if(it != Component::cache.end())
	//{
	//	AddComponent(entity, it->second);
	//}
}

void EntityManager::RemoveComponent(Entity * entity, Component * component)
{
	const ComponentType & addType = component->GetType();

	Map<Entity*, EntityFamilyType>::iterator it = newFamilyEntities.find(entity);
	if(newFamilyEntities.end() != it)
	{
		EntityFamilyType newFamilyType = EntityFamilyType::RemoveComponentType(it->second, addType);
		it->second = newFamilyType;
	}
	else
	{
		EntityFamilyType oldFamilyType = entity->GetFamily();
		EntityFamilyType newFamilyType = EntityFamilyType::RemoveComponentType(oldFamilyType, addType);
		newFamilyEntities[entity] = newFamilyType;
	}
}

void EntityManager::Flush()
{
	FlushChangeFamily();
	FlushDestroy();
}

void EntityManager::FlushDestroy()
{
	int32 size = deleteEntities.size();

	for(int32 i = 0; i < size; ++i)
	{
		Entity * entity = deleteEntities[i];
		Map<uint64, EntityFamily*>::iterator it = families.find(entity->GetFamily().GetBit());
		if(it != families.end())
		{
			EntityFamily * family = it->second;
			family->DeleteEntity(entity);
			RemoveFromGlobalList(entity);

			delete(entity);
		}
		else
		{
			//entity with no family
		}
	}

	deleteEntities.clear();
}

void EntityManager::FlushChangeFamily()
{
	Map<Entity*, EntityFamilyType>::iterator it = newFamilyEntities.begin();
	Map<Entity*, EntityFamilyType>::iterator itEnd = newFamilyEntities.end();
	for(; it != itEnd; ++it)
	{
		Entity * entity = it->first;
		ProcessAddRemoveComponent(entity, entity->GetFamily(), it->second);
		entity->changeState = 0;
	}

	newFamilyEntities.clear();
}



void EntityManager::ProcessAddRemoveComponent(Entity * entity, const EntityFamilyType & oldFamilyType, const EntityFamilyType & newFamilyType)
{
    EntityFamily * oldFamily = GetFamilyByType(oldFamilyType); 
    EntityFamily * newFamily = GetFamilyByType(newFamilyType); 
        
    /*
        Если тип не равен 0, то есть если мы не удалили последний компонент. 
     */
    if (!newFamilyType.IsEmpty() && (newFamily == 0))
    {
        newFamily = new EntityFamily(this, newFamilyType);
            
        families[newFamilyType.GetBit()] = newFamily;
        
        // Require refactoring, because depends on internal structure of FamilyType / ComponentType.
        uint64 bit = newFamilyType.GetBit();
        for (uint64 idx = 0; idx < 64; ++idx)
        {
            if (bit & ((int64)1 << idx))
            {
                Component * comp = Component::GetComponentByIndex(idx);
                familiesWithComponent.insert(std::pair<Component*, EntityFamily*>(comp, newFamily));
            }
        }
    }
    
    if (oldFamily && newFamily)
    {
		newFamily->MoveFromFamily(oldFamily, entity);
    }
	else if (!newFamily)
    {
        oldFamily->DeleteEntity(entity);
    }
	else if (!oldFamily)
    {
        newFamily->NewEntity(entity);
    }

	entity->SetFamily(newFamilyType);
	entity->SetIndexInFamily(newFamily->GetSize()-1);
}

EntityFamily * EntityManager::GetFamilyByType(const EntityFamilyType & familyType)
{
    Map<uint64, EntityFamily*>::iterator familyIterator = families.find(familyType.GetBit());
    if (familyIterator != families.end())
    {
        return familyIterator->second;
    }   
    return 0;
}
    
EntityFamily * EntityManager::GetFamily(Component * c0, ...)
{
    va_list list;
    
    va_start(list, c0);
    
    uint64 bit = c0->GetType().GetBit();
    while(1)
    {
        Component * cNext = va_arg(list, Component*);
        if (!cNext)break;
        bit |= cNext->GetType().GetBit();
    }
    va_end(list);

    Map<uint64, EntityFamily*>::iterator familyIterator = families.find(bit);
    if (familyIterator != families.end())
    {
        return familyIterator->second;
    }
    return 0;
}
    
Pool * EntityManager::CreatePool(const char * dataName, int32 maxSize)
{
	Pool * pool = 0;

    Map<const char *, Pool *>::iterator poolsIt = poolAllocators.find(dataName);
    if (poolsIt != poolAllocators.end())
    {
        Pool * newPool = poolsIt->second->CreateCopy(maxSize);

        Pool * prevPool = 0;
        Map<const char *, Pool*>::iterator find = pools.find(dataName);
        if(pools.end() != find)
        {
            prevPool = find->second;
        }
        newPool->SetNext(prevPool);
        pools[dataName] = newPool;
		pool = newPool;
    }
	
	return pool;
}

void EntityManager::Dump()
{
	Logger::Info("============================");
	Logger::Info("EntityManager dump");
	Logger::Info("============================");
	Logger::Info("Pools:");
	Logger::Info("============================");
	
	Map<const char *, Pool *>::iterator poolIterator;
	for(poolIterator = pools.begin(); poolIterator != pools.end(); ++poolIterator)
	{
		const char * poolName = poolIterator->first;
		Pool * pool = poolIterator->second;
		Logger::Info("Pool \"%s\" of type %s", poolName, typeid(*pool).name());
		Logger::Info("----------------------------");
		
		while(pool)
		{
			int32 count = pool->GetCount();
			int32 maxCount = pool->GetMaxCount();
			Logger::Info("    subpool of family %lld (%d elements, %d maxCount, %d bytes)", pool->GetEntityFamily()->family.GetBit(), count, maxCount, maxCount*pool->typeSizeof);
			for(int32 i = 0; i < count; ++i)
			{
				pool->DumpElement(i);
			}

			pool = pool->GetNext();
			Logger::Info("    ----------------------------");
		}
	}
}

Vector<Entity*> & EntityManager::GetAllEntities()
{
	return entities;
}


//TODO: those are slow debug(editor?) functions for global view
void EntityManager::AddToGlobalList(Entity * entity)
{
	entities.push_back(entity);
}

void EntityManager::RemoveFromGlobalList(Entity * entity)
{
	int32 size = entities.size();
	for(int32 i = 0; i < size; ++i)
	{
		if(entity == entities[i])
		{
			entities[i] = entities[size-1];
			entities.pop_back();
			return;
		}
	}
}

};
