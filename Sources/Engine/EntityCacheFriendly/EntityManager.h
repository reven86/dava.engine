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



#ifndef __DAVAENGINE_ENTITY_MANAGER_H__
#define __DAVAENGINE_ENTITY_MANAGER_H__

#include "Base/BaseTypes.h"
#include "Base/Singleton.h"
#include "Entity/Pool.h"
#include "Entity/EntityFamily.h"
#include <typeinfo>

namespace DAVA 
{

class Component;
class Entity;    
class EntityManager : public Singleton<EntityManager>
{
public:
    Entity * CreateEntity();
	void DestroyEntity(Entity * & entity);
	
    void AddComponent(Entity * entity, Component * component);
	void AddComponent(Entity * entity, const char * componentName);

    void RemoveComponent(Entity * entity, Component * component);
	
    EntityFamily * GetFamilyByType(const EntityFamilyType & familyType);
    EntityFamily * GetFamily(Component * c0, ...);
    
    template<class T>
    TemplatePool<T> * GetLinkedTemplatePools(const char * dataName);
    

    
    template<class T>
    static void CreatePoolAllocator(const char * dataPoolName);
    // ??? 
    template<class T>
    static void ReleasePoolAllocator(const char * dataPoolName);

    static Map<const char *, Pool *> & GetPoolAllocators() { return poolAllocators; };
    
    Pool * CreatePool(const char * dataName, int32 size);
        
	void Dump();

	void Flush(); //process newFamilyEntities

	Vector<Entity*> & GetAllEntities();

	//TODO: GC(); //remove empty EntityFamilies
    
private:

    Map<uint64, EntityFamily*> families;
    std::multimap<Component*, EntityFamily*> familiesWithComponent; // all families with given component
    
    static Map<const char *, Pool *> poolAllocators;
    Map<const char *, Pool *> pools;
    
	void ProcessAddRemoveComponent(Entity * entity, const EntityFamilyType & oldType, const EntityFamilyType & newType);

	Map<Entity*, EntityFamilyType> newFamilyEntities;
	Vector<Entity*> deleteEntities;

	void FlushChangeFamily();
	void FlushDestroy();

	void AddToGlobalList(Entity * entity);
	void RemoveFromGlobalList(Entity * entity);
	Vector<Entity*> entities;
};
    
    
    
template<class T>
void EntityManager::CreatePoolAllocator(const char * dataPoolName)
{
    Map<const char *, Pool *>::iterator it = poolAllocators.find(dataPoolName);
    if (it != poolAllocators.end())
    {
        if (typeid(TemplatePool<T>) != typeid(*(it->second)))
        {
            DVASSERT("Data type not valid" && 0);
        }
    }else
    {
        Pool * pool = new TemplatePool<T>(1);
        poolAllocators[dataPoolName] = pool;
    }
}
    
template<class T>
TemplatePool<T> * EntityManager::GetLinkedTemplatePools(const char * dataName)
{
    Pool * pool = 0;
    Map<const char *, Pool*>::iterator find = pools.find(dataName);
    if(pools.end() != find)
    {
        pool = find->second;
    }
    // TODO: fast_cast
    return dynamic_cast<TemplatePool<T> *>(pool);
}


};

#endif // __DAVAENGINE_ENTITY_MANAGER_H__
