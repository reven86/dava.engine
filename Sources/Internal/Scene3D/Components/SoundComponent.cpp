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



#include "Scene3D/Components/SoundComponent.h"
#include "Sound/SoundEvent.h"
#include "FileSystem/KeyedArchive.h"
#include "Scene3D/Systems/EventSystem.h"
#include "Scene3D/Systems/GlobalEventSystem.h"

namespace DAVA 
{
	REGISTER_CLASS(SoundComponent)

SoundComponent::SoundComponent()
{
    soundEvent = 0;
}

SoundComponent::~SoundComponent()
{
    SafeRelease(soundEvent);
}
    
void SoundComponent::SetSoundEvent(SoundEvent * sEvent)
{
	SafeRelease(soundEvent);
    soundEvent = SafeRetain(sEvent);
}
    
SoundEvent * SoundComponent::GetSoundEvent()
{
    return soundEvent;
}

const String & SoundComponent::GetEventName()
{
	return eventName;
}

void SoundComponent::SetEventName(const String & _eventName)
{
	DVASSERT(_eventName != "");

	eventName = _eventName;
	GlobalEventSystem::Instance()->Event(entity, EventSystem::SOUND_CHANGED);
}

Component * SoundComponent::Clone(Entity * toEntity)
{
    SoundComponent * component = new SoundComponent();
	component->SetEntity(toEntity);

    //TODO: Do not forget ot check what does it means.
    component->soundEvent->fmodEvent = soundEvent->fmodEvent;
	component->eventName = eventName;
    return component;
}

void SoundComponent::Serialize(KeyedArchive *archive, SerializationContext *serializationContext)
{
	Component::Serialize(archive, serializationContext);

	if(archive != 0 && soundEvent != 0)
	{
		archive->SetString("sc.eventName", eventName);
	}
}

void SoundComponent::Deserialize(KeyedArchive *archive, SerializationContext *serializationContext)
{
	if(archive)
	{
		SetEventName(archive->GetString("sc.eventName"));
	}

	Component::Deserialize(archive, serializationContext);
}

};
