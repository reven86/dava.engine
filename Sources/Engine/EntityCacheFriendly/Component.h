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



#ifndef __DAVAENGINE_ENTITY_COMPONENT_H__
#define __DAVAENGINE_ENTITY_COMPONENT_H__

#include "Entity/Pool.h"
#include "Entity/ComponentTypes.h"
#include "Entity/EntityManager.h"
#include <typeinfo>


#define DECLARE_COMPONENT(ComponentName) \
class ComponentName : public Component \
{ \
public:	\
	static ComponentName * Get() { return instance; } \
	\
	static void Create() \
	{   \
		ComponentName * component = new ComponentName(); \
		RegisterComponent(#ComponentName, component); \
		component->Register(); \
	}\
private: \
	static ComponentName * instance; \
	ComponentName() \
	{ \
		instance = this; \
	} \
	inline void Register();	\
\
}; 


#define IMPLEMENT_COMPONENT(ComponentName) \
ComponentName * ComponentName::instance = 0;

namespace DAVA 
{

class Component
{
public:
	static void RegisterComponent(const char * componentName, Component * component); //create or get from cache
	static Component * GetComponent(const char * componentName);
    
    Component()
    {
        //type = ComponentType(); //this duplicates field declaration
        componentsByIndex[type.GetIndex()] = this;
    };
    
    
    template <class T>
    void RegisterData(const char * name)
    {
        EntityManager::CreatePoolAllocator<T>(name);
        dataNames.insert(name);
    }

//	template<class T>
//	TemplatePool<T>* CreatePool(T a, const char * name)
//	{
//		TemplatePool<T> * pool = new TemplatePool<T>(100);
//		pools.push_back(pool);
//        return pool;
//	}
//    

    const ComponentType & GetType() { return type; };
    
    static Component * GetComponentByIndex(uint64 index);
    
    Set<const char*> & GetDataNames() {return dataNames; };

	static Map<const char *, Component * > cache;//<name, component>
    
private:
	ComponentType type;
    Set<const char*> dataNames;
    static Map<uint64, Component*>  componentsByIndex;
};
    
};


#endif // __DAVAENGINE_ENTITY_COMPONENT_H__
