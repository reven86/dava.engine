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

#include "SoundComponent.h"
#include "Sound/SoundSystem.h"
#include "Sound/SoundEvent.h"
#include "Base/FastName.h"

namespace DAVA
{

REGISTER_CLASS(SoundComponent)

SoundComponent::SoundComponent() :
    localDirection(1.f, 0.f, 0.f)
{
    TAG_SWITCH(MemoryManager::TAG_COMPONENTS)
}

SoundComponent::~SoundComponent()
{
    TAG_SWITCH(MemoryManager::TAG_COMPONENTS)
    
    RemoveAllEvents();
}

void SoundComponent::AddSoundEvent(SoundEvent * _event)
{
    TAG_SWITCH(MemoryManager::TAG_COMPONENTS)
    
    DVASSERT(_event);

    SafeRetain(_event);
    events.push_back(_event);
}

void SoundComponent::RemoveSoundEvent(SoundEvent * event)
{
    TAG_SWITCH(MemoryManager::TAG_COMPONENTS)
    
    Vector<SoundEvent *>::iterator it = events.begin();
    Vector<SoundEvent *>::iterator itEnd = events.end();
    for(; it != itEnd; ++it)
    {
        if((*it) == event)
        {
            events.erase(it);
            SafeRelease(event);
            return;
        }
    }
}

void SoundComponent::RemoveAllEvents()
{
    TAG_SWITCH(MemoryManager::TAG_COMPONENTS)
    
    uint32 eventsCount = events.size();
    for(uint32 i = 0; i < eventsCount; ++i)
        SafeRelease(events[i]);

    events.clear();
}

void SoundComponent::SetLocalDirection(const Vector3 & direction)
{
    TAG_SWITCH(MemoryManager::TAG_COMPONENTS)
    
    localDirection = direction;
}

Component * SoundComponent::Clone(Entity * toEntity)
{
    TAG_SWITCH(MemoryManager::TAG_COMPONENTS)
    
    SoundComponent * soundComponent = new SoundComponent();
    soundComponent->SetEntity(toEntity);
    
    int32 eventCount = events.size();
    for(int32 i = 0; i < eventCount; ++i)
        soundComponent->AddSoundEvent(events[i]);
    
    soundComponent->localDirection = localDirection;

    return soundComponent;
}

void SoundComponent::Serialize(KeyedArchive *archive, SerializationContext *serializationContext)
{
    TAG_SWITCH(MemoryManager::TAG_COMPONENTS)
    
    Component::Serialize(archive, serializationContext);

    if(archive)
    {
        uint32 eventsCount = events.size();
        archive->SetUInt32("sc.eventCount", eventsCount);
        for(uint32 i = 0; i < eventsCount; ++i)
        {
            KeyedArchive* eventArchive = new KeyedArchive();
            SoundSystem::Instance()->SerializeEvent(events[i], eventArchive);
            archive->SetArchive(KeyedArchive::GenKeyFromIndex(i), eventArchive);
            SafeRelease(eventArchive);
        }

        archive->SetVector3("sc.localDirection", localDirection);
    }
}

void SoundComponent::Deserialize(KeyedArchive *archive, SerializationContext *serializationContext)
{
    TAG_SWITCH(MemoryManager::TAG_COMPONENTS)
    
    events.clear();

    if(archive)
    {
        uint32 eventsCount = archive->GetUInt32("sc.eventCount");
        for(uint32 i = 0; i < eventsCount; ++i)
        {
            KeyedArchive* eventArchive = archive->GetArchive(KeyedArchive::GenKeyFromIndex(i));
            SoundEvent * sEvent = SoundSystem::Instance()->DeserializeEvent(eventArchive);
            AddSoundEvent(sEvent);
            SafeRelease(sEvent);
        }

        localDirection = archive->GetVector3("sc.localDirection", Vector3(1.f, 0.f, 0.f));
    }

    Component::Deserialize(archive, serializationContext);
}

};