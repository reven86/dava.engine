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


#include "Scene3D/Systems/GlobalEventSystem.h"
#include "Scene3D/Systems/EventSystem.h"
#include "Scene3D/Scene.h"
#include "Entity/Component.h"

namespace DAVA
{
    
void GlobalEventSystem::GroupEvent(Scene * scene, Vector<Component *> & components, uint32 event)
{
    scene->GetEventSystem()->GroupNotifyAllSystems(components, event);
}

void GlobalEventSystem::Event(Component * component, uint32 event)
{
    if (component)
    {
        if (component->GetEntity() && component->GetEntity()->GetScene())
        {
            component->GetEntity()->GetScene()->GetEventSystem ()->NotifyAllSystems (component, event);
            return;
        }
        
        List<uint32> & events = eventsCache[component];
        events.push_back(event);
    }
    
}

void GlobalEventSystem::PerformAllEventsFromCache(Component * component)
{
    auto it = eventsCache.find(component);
    if (it != eventsCache.end())
    {
        List<uint32> & list = it->second;
        
        for (List<uint32>::iterator listIt = list.begin(); listIt != list.end();  ++listIt)
        {
            component->GetEntity()->GetScene()->GetEventSystem()->NotifyAllSystems(component, *listIt);
        }
        
        eventsCache.erase(it);
    }
}

void GlobalEventSystem::RemoveAllEvents(Component * component)
{
    auto it = eventsCache.find(component);
    if (it != eventsCache.end())
    {
        eventsCache.erase(it);
    }
}
    
}