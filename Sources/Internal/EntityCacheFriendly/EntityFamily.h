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



#ifndef __DAVAENGINE_ENTITY_FAMILY_H__
#define __DAVAENGINE_ENTITY_FAMILY_H__

#include "Base/BaseTypes.h"
#include "Entity/ComponentTypes.h"

namespace DAVA
{

class Pool;
class Entity;
class Component;
class EntityManager;    

class EntityFamily
{
public:
	EntityFamily(EntityManager * manager, EntityFamilyType family);
    ~EntityFamily();
    
    Pool * GetPoolByDataName(const char * dataName);

    uint32 GetSize() { return currentSize; };
    template<class T> T * GetPtr(const char * name);

private:
    // Immediate functions. Do what you ask immediatelly. Should be called only from EntityManager
    void NewEntity(Entity * entity);
    void DeleteEntity(Entity * entity);
	void MoveFromFamily(EntityFamily * oldFamily, Entity * entity);
    
    EntityManager * manager;
    EntityFamilyType family;
    Vector<Entity*> entities;
	Vector<Pool*> pools;
    Map<const char *, Pool*> poolByDataName;
    uint32 currentSize;
    uint32 maxSize;
    
    friend class EntityManager;
	friend class Entity;
};

template<class T>
T * EntityFamily::GetPtr(const char * dataName)
{
    Pool * pool = GetPoolByDataName(dataName);
    DVASSERT(pool);

    // TODO: replace to reinterpret cast in release.
    TemplatePool<T> * tPool = dynamic_cast<TemplatePool<T>*>(pool);
    
    return tPool->GetHead();
}

    
};

#endif //__DAVAENGINE_ENTITY_FAMILY_H__
