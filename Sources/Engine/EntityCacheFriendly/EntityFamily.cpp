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



#include "Entity/EntityFamily.h"
#include "Entity/Component.h"
#include "Entity/Entity.h"
#include "Entity/EntityManager.h"

namespace DAVA
{

EntityFamily::EntityFamily(EntityManager * _manager, EntityFamilyType _family)
{
    manager = _manager;
    family = _family;
    
    // Require refactoring, because depends on internal structure of FamilyType / ComponentType.
    Set<const char*> dataNamesForAllComponents;
    uint64 bit = family.GetBit();
    for (uint64 idx = 0; idx < 64; ++idx)
    {
        if (bit & ((int64)1 << idx))
        {
            Component * comp = Component::GetComponentByIndex(idx);
            dataNamesForAllComponents.insert(comp->GetDataNames().begin(), comp->GetDataNames().end());
        }
    }
    
    pools.reserve(dataNamesForAllComponents.size()); //this changes size of vector
    
    currentSize = 0;
	maxSize = 10;

    for (Set<const char*>::iterator it = dataNamesForAllComponents.begin(); it != dataNamesForAllComponents.end(); ++it)
    {
        Pool * newPool = manager->CreatePool(*it, maxSize);
		newPool->SetEntityFamily(this);
        pools.push_back(newPool);
        poolByDataName[*it] = newPool;
    }       
}
    
Pool * EntityFamily::GetPoolByDataName(const char * dataName)
{
    Map<const char *, Pool*>::iterator it = poolByDataName.find(dataName);
    if (it != poolByDataName.end())
    {
        return it->second;
    }
    return 0;
}
    
void EntityFamily::NewEntity(Entity * entity)
{
    //DVASSERT(entity->GetFamily() == EntityFamilyType(0));
    
    if (currentSize >= maxSize)
    {
        // Resize all pools
        for (uint32 poolIndex = 0; poolIndex < pools.size(); ++poolIndex)
        {
            pools[poolIndex]->Resize(currentSize + 10);
        }
		maxSize += 10;
    }

    currentSize++;
	entities.push_back(entity);
    for (uint32 poolIndex = 0; poolIndex < pools.size(); ++poolIndex)
    {
        pools[poolIndex]->length++;
    }
}
    
void EntityFamily::DeleteEntity(Entity * entity)
{
    // We can't only delete entity that in the family. If it's not something went wrong
    //DVASSERT(entity->GetFamily().GetBit() == this->family.GetBit());
    //DVASSERT(entity->indexInFamily >= 0);
    //DVASSERT(entity->indexInFamily < currentSize);
    
	int32 oldIndex = entity->GetIndexInFamily();
	int32 lastIndex = currentSize - 1;
    for (uint32 poolIndex = 0; poolIndex < pools.size(); ++poolIndex)
    {
		if(pools[poolIndex]->length > 1)
		{
			pools[poolIndex]->MoveElement(lastIndex, oldIndex);
		}
        pools[poolIndex]->length--;
    }

	entities[oldIndex] = entities[lastIndex];
	entities[oldIndex]->SetIndexInFamily(oldIndex);
	entities.pop_back();

	currentSize--;
}
    
void EntityFamily::MoveFromFamily(EntityFamily * oldFamily, Entity * entity)
{
    int32 oldIndex = entity->GetIndexInFamily();
	int32 newIndex = currentSize;

	NewEntity(entity);
    
    for (Map<const char *, Pool*>::iterator currentPoolIt = poolByDataName.begin(); currentPoolIt != poolByDataName.end(); ++currentPoolIt)
    {
		Pool * oldPool = oldFamily->GetPoolByDataName(currentPoolIt->first);

		if(oldPool)
		{
#ifdef __DAVAENGINE_DEBUG__
			Pool * newPool = currentPoolIt->second;
			DVASSERT(typeid(*oldPool) == typeid(*newPool));
#endif
        
			oldPool->MoveElement(oldIndex, oldPool, newIndex);
		}
    }

	oldFamily->DeleteEntity(entity);
    
	//must stay
	entity->SetIndexInFamily(newIndex);
}


/*
void EntityFamily::Flush()
{
	// 

//	for (each moving entity in entities)
//	{
//		MoveToFamily(newFamily)
//	}
//
//	for (each deleted entity in entities)
//	{
//		Delete(entity);
//	}
//
//	if (currentSize + addedEntitiesSize > maxSize)
//		ReallocatePools(currentSize + addedEntitiesSize);
//
//	for (each added entity)
//	{
//		Add(entity);
//	}

	
	Set<String name>

	for each data in set
	{
		if 
		PoolSystem::Instance()->CreatePool(entities.size());
	}

}
 */

};
