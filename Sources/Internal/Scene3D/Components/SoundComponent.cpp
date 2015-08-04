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
#include "TransformComponent.h"
#include "Sound/SoundSystem.h"
#include "Sound/SoundEvent.h"
#include "Base/FastName.h"
#include "ComponentHelpers.h"
#include "Scene3D/Entity.h"
#include "Scene3D/Scene.h"
#include "Scene3D/Systems/EventSystem.h"
#include "Scene3D/Systems/GlobalEventSystem.h"
#include "Scene3D/Systems/SoundUpdateSystem.h"
#include "Utils/Utils.h"

namespace DAVA
{

#ifdef DAVA_FMOD

SoundComponent::SoundComponent()
{}

SoundComponent::~SoundComponent()
{
    RemoveAllEvents();
}

void SoundComponent::AddSoundEvent(SoundEvent * _event, uint32 flags /*= 0*/, const Vector3 & direction /*= Vector3(1.f, 0.f, 0.f)*/)
{
    DVASSERT(_event);

    SafeRetain(_event);
    events.push_back(SoundComponentElement(_event, flags, direction));

    GlobalEventSystem::Instance()->Event(this, EventSystem::SOUND_COMPONENT_CHANGED);
}

void SoundComponent::RemoveSoundEvent(SoundEvent * event)
{
    uint32 eventCount = static_cast<uint32>(events.size());
    for(uint32 i = 0; i < eventCount; ++i)
    {
        if(events[i].soundEvent == event)
        {
            events[i].soundEvent->Stop(true);
            SafeRelease(events[i].soundEvent);
            RemoveExchangingWithLast(events, i);

            return;
        }
    }
}

void SoundComponent::RemoveAllEvents()
{
    uint32 eventsCount = static_cast<uint32>(events.size());
    for(uint32 i = 0; i < eventsCount; ++i)
    {
        events[i].soundEvent->Stop(true);
        SafeRelease(events[i].soundEvent);
    }

    events.clear();
}

void SoundComponent::Trigger()
{
    uint32 eventsCount = static_cast<uint32>(events.size());
    for(uint32 i = 0; i < eventsCount; ++i)
        Trigger(i);
}

void SoundComponent::Stop()
{
    uint32 eventsCount = static_cast<uint32>(events.size());
    for(uint32 i = 0; i < eventsCount; ++i)
        Stop(i);
}

void SoundComponent::Trigger(uint32 index)
{
    DVASSERT(index < events.size());

    SoundComponentElement & sound = events[index];
    sound.soundEvent->Trigger();

    if((sound.flags & SoundComponent::FLAG_AUTO_DISTANCE_TRIGGER) && entity && entity->GetScene())
    {
        entity->GetScene()->soundSystem->AddAutoTriggerSound(entity, sound.soundEvent);
    }
}

void SoundComponent::Stop(uint32 index)
{
    DVASSERT(index < events.size());

    SoundComponentElement & sound = events[index];
    sound.soundEvent->Stop();

    if((sound.flags & SoundComponent::FLAG_AUTO_DISTANCE_TRIGGER) && entity && entity->GetScene())
    {
        entity->GetScene()->soundSystem->RemoveAutoTriggerSound(entity, sound.soundEvent);
    }
}

void SoundComponent::SetSoundEventFlags(uint32 index, uint32 flags)
{
    DVASSERT(index < (uint32)events.size());

    if(events[index].flags != flags)
    {
        Stop(index);
        events[index].flags = flags;

        GlobalEventSystem::Instance()->Event(this, EventSystem::SOUND_COMPONENT_CHANGED);
    }
}

void SoundComponent::SetLocalDirection(uint32 eventIndex, const Vector3 & direction)
{
    DVASSERT(eventIndex < (uint32)events.size());
    events[eventIndex].localDirection = direction;
}

void SoundComponent::SetLocalDirection(const DAVA::Vector3 &direction)
{
    uint32 eventsCount = static_cast<uint32>(events.size());
    for(uint32 i = 0; i < eventsCount; ++i)
        SetLocalDirection(i, direction);
}
    
Component * SoundComponent::Clone(Entity * toEntity)
{
    SoundComponent * soundComponent = new SoundComponent();
    soundComponent->SetEntity(toEntity);
    
    SoundSystem * soundSystem = SoundSystem::Instance();
    int32 eventCount = static_cast<int32>(events.size());
    for(int32 i = 0; i < eventCount; ++i)
    {
        SoundEvent * clonedEvent = soundSystem->CloneEvent(events[i].soundEvent);
        soundComponent->AddSoundEvent(clonedEvent, events[i].flags, events[i].localDirection);
        clonedEvent->Release();
    }

    return soundComponent;
}

void SoundComponent::Serialize(KeyedArchive *archive, SerializationContext *serializationContext)
{
    Component::Serialize(archive, serializationContext);

    if(archive)
    {
        uint32 eventsCount = static_cast<uint32>(events.size());
        archive->SetUInt32("sc.eventCount", eventsCount);
        for(uint32 i = 0; i < eventsCount; ++i)
        {
            KeyedArchive* eventArchive = new KeyedArchive();

            SoundSystem::Instance()->SerializeEvent(events[i].soundEvent, eventArchive);
            eventArchive->SetUInt32("sce.flags", events[i].flags);
            eventArchive->SetVector3("sce.localDirection", events[i].localDirection);

            archive->SetArchive(KeyedArchive::GenKeyFromIndex(i), eventArchive);
            SafeRelease(eventArchive);
        }
    }
}

void SoundComponent::Deserialize(KeyedArchive *archive, SerializationContext *serializationContext)
{
    events.clear();

    if(archive)
    {
        uint32 eventsCount = archive->GetUInt32("sc.eventCount");
        for(uint32 i = 0; i < eventsCount; ++i)
        {
            KeyedArchive* eventArchive = archive->GetArchive(KeyedArchive::GenKeyFromIndex(i));
            SoundEvent * sEvent = SoundSystem::Instance()->DeserializeEvent(eventArchive);
            AddSoundEvent(sEvent, eventArchive->GetUInt32("sce.flags"), eventArchive->GetVector3("sce.localDirection", Vector3(1.f, 0.f, 0.f)));
            SafeRelease(sEvent);
        }
    }

    Component::Deserialize(archive, serializationContext);
}

#else

//no FMOD, no sound component
SoundComponent::SoundComponent() {}
SoundComponent::~SoundComponent() {}

Component * SoundComponent::Clone(Entity * toEntity) { return nullptr; }

void SoundComponent::Serialize(KeyedArchive *archive, SerializationContext *serializationContext) {}
void SoundComponent::Deserialize(KeyedArchive *archive, SerializationContext *serializationContext) {}

void SoundComponent::Trigger() {}
void SoundComponent::Stop() {}
void SoundComponent::Trigger(uint32 index) {}
void SoundComponent::Stop(uint32 index) {}

void SoundComponent::SetSoundEventFlags(uint32 eventIndex, uint32 flags) {}

void SoundComponent::AddSoundEvent(SoundEvent * _event, uint32 flags, const Vector3 & direction) {}
void SoundComponent::RemoveSoundEvent(SoundEvent * event) {}
void SoundComponent::RemoveAllEvents() {}

void SoundComponent::SetLocalDirection(uint32 eventIndex, const Vector3 & direction) {}
void SoundComponent::SetLocalDirection(const DAVA::Vector3 &direction) {}

#endif // !DAVA_FMOD

};