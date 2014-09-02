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



#include "Entity.h"
#include "EntityManager.h"

namespace DAVA 
{

Entity::Entity(EntityManager * _manager)
:	manager(_manager),
	family(0),
	indexInFamily(-1),
	changeState(0)
{
}


Entity::~Entity()
{
}

void Entity::AddComponent(Component * component)
{
    manager->AddComponent(this, component);

	changeState |= FAMILY_CHANGED;
}

void Entity::AddComponent(const char * component)
{
	//manager->AddComponent(this, component);
}

void Entity::RemoveComponent(Component * component)
{
	manager->RemoveComponent(this, component);

	changeState |= FAMILY_CHANGED;
}

//uint32 Entity::CalculateFamily()
//{
//	family = 0;
//    // TODO: Check how to write correct STL code, with size_type. Without 64 => 32 bit conversions.
//	uint32 componentsCount = (uint32)components.size();
//	for(uint32 i = 0; i < componentsCount; ++i)
//	{
//		family |= (1 << components[i]->type);
//	}
//
//	return family;
//}

void Entity::SetFamily(EntityFamilyType newFamily)
{
    family = newFamily;
}

const EntityFamilyType & Entity::GetFamily()
{
	return family;
}

void Entity::SetIndexInFamily(int32 _index)
{
	indexInFamily = _index;
}

int32 Entity::GetIndexInFamily()
{
	return indexInFamily;
}

int32 Entity::GetDataCount()
{
	EntityFamily * myFamily = manager->GetFamilyByType(family);
	return myFamily->pools.size();
}

const char * Entity::GetDataName(int32 dataIndex)
{
	//TODO: Dizz: this is awful, maybe move pool name to Pool class
	EntityFamily * myFamily = manager->GetFamilyByType(family);
	Map<const char *, Pool*>::iterator it = myFamily->poolByDataName.begin();

	while(dataIndex > 0)
	{
		dataIndex--;
		it++;
	}
	return it->first;
}


};
