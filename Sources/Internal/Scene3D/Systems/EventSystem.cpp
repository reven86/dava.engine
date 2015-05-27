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



#include "Scene3D/Systems/EventSystem.h"
#include "Entity/SceneSystem.h"
#include "Scene3D/Entity.h"

namespace DAVA
{
    
void EventSystem::RegisterSystemForEvent(SceneSystem * system, uint32 event)
{
	registeredSystems[event].push_back(system);
}

void EventSystem::UnregisterSystemForEvent(SceneSystem * system, uint32 event)
{
	Vector<SceneSystem*> & container = registeredSystems[event];
	uint32 size = static_cast<uint32>(container.size());
	for(uint32 i = 0; i < size; ++i)
	{
		if(container[i] == system)
		{
			container[i] = container[size-1];
			container.pop_back();
			return;
		}
	}
}
    
void EventSystem::GroupNotifyAllSystems(Vector<Component*> & components, uint32 event)
{
	Vector<SceneSystem*> & container = registeredSystems[event];
	uint32 size = static_cast<uint32>(container.size());
	for(uint32 i = 0; i < size; ++i)
	{
        SceneSystem * system = container[i];
        uint64 requiredComponentFlags = system->GetRequiredComponents();

        uint32 componentsVectorSize = static_cast<uint32>(components.size());
        for (uint32 k = 0; k < componentsVectorSize; ++k)
        {
            Component * comp = components[k];
            Entity * entity = comp->GetEntity();
            uint64 componentsInEntity = entity->GetAvailableComponentFlags();

            if ((requiredComponentFlags & componentsInEntity) == requiredComponentFlags)
                system->ImmediateEvent(comp, event);
        }
    }
    
}
    
void EventSystem::NotifyAllSystems(Component * component, uint32 event)
{
	Vector<SceneSystem*> & container = registeredSystems[event];
	uint32 size = static_cast<uint32>(container.size());
    uint64 componentsInEntity = component->GetEntity()->GetAvailableComponentFlags();
	for(uint32 i = 0; i < size; ++i)
	{
        SceneSystem * system = container[i];
        uint64 requiredComponentFlags = system->GetRequiredComponents();
        if ((requiredComponentFlags & componentsInEntity) == requiredComponentFlags)
            system->ImmediateEvent(component, event);
    }
}

void EventSystem::NotifySystem(SceneSystem * system, Component * component, uint32 event)
{
	Vector<SceneSystem*> & container = registeredSystems[event];
	uint32 size = static_cast<uint32>(container.size());
	for(uint32 i = 0; i < size; ++i)
	{
		if(system == container[i])
		{
			system->ImmediateEvent(component, event);
			return;
		}
	}
}

}