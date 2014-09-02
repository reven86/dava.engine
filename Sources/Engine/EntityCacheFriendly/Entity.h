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



#ifndef __DAVAENGINE_ENTITY_H__
#define __DAVAENGINE_ENTITY_H__

#include "Base/BaseTypes.h"
#include "Entity/ComponentTypes.h"
#include "Entity/EntityFamily.h"
#include "Entity/EntityManager.h"

namespace DAVA 
{
class Component;
class Entity
{
public:
	enum eChangeState
	{
		FAMILY_CHANGED = 1
	};

	void AddComponent(Component * component);//const char * componentName);
	void AddComponent(const char * component);

	void RemoveComponent(Component * component);

    template<class T>
    void SetData(const char * dataName, const T & value);
    
	template<class T>
	const T * GetData(const char * dataName);

    void SetFamily(EntityFamilyType newFamily);
	const EntityFamilyType & GetFamily();

	void SetIndexInFamily(int32 index);
	int32 GetIndexInFamily();

	EntityManager * GetManager() { return manager; }

	//for navigating through entitie's data
	int32 GetDataCount();
	const char * GetDataName(int32 dataIndex); //data index

private:
	Entity(EntityManager * manager);
	Entity();
	~Entity();
    
	EntityFamilyType  family;
	int32   indexInFamily;

	int32 changeState;

	EntityManager * manager;

	friend class EntityManager;
};


template<class T>
void Entity::SetData(const char * dataName, const T & value)
{
	if(changeState & FAMILY_CHANGED)
	{
		DVASSERT(0 && "Entity::SetData called before manager->Flush()");
		Logger::Error("Entity::SetData called before manager->Flush()");
	}

    EntityFamily * enFamily = manager->GetFamilyByType(family);
	if(0 == enFamily)
	{
		Logger::Error("Entity::SetData enFamily==0");
		return;
	}

    T * t = enFamily->GetPtr<T>(dataName);
    t[indexInFamily] = value;
}

template<class T>
const T * Entity::GetData(const char * dataName)
{
	if(changeState & FAMILY_CHANGED)
	{
		DVASSERT(0 && "Entity::SetData called before manager->Flush()");
		Logger::Error("Entity::SetData called before manager->Flush()");
	}

	EntityFamily * enFamily = manager->GetFamilyByType(family);
	if(0 == enFamily)
	{
		Logger::Error("Entity::SetData enFamily==0");
		return 0;
	}

	T * t = enFamily->GetPtr<T>(dataName);
	
	return &(t[indexInFamily]);
}

};

#endif // __DAVAENGINE_ENTITY_H__
