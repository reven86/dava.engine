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


#include "Scene3D/Systems/SoundUpdateSystem.h"
#include "Scene3D/Systems/EventSystem.h"
#include "Scene3D/Entity.h"
#include "Scene3D/Scene.h"
#include "Scene3D/Components/TransformComponent.h"
#include "Scene3D/Components/SoundComponent.h"
#include "Scene3D/Components/ComponentHelpers.h"
#include "Sound/SoundSystem.h"
#include "Sound/SoundEvent.h"
#include "Debug/Stats.h"

namespace DAVA
{

SoundUpdateSystem::AutoTriggerSound::AutoTriggerSound(Entity * _owner, SoundEvent * _sound) :
    owner(_owner),
    soundEvent(_sound)
{
    float32 distance = soundEvent->GetMaxDistance();
    maxSqDistance = distance * distance;
}

SoundUpdateSystem::SoundUpdateSystem(Scene * scene)
:	SceneSystem(scene)
{
	scene->GetEventSystem()->RegisterSystemForEvent(this, EventSystem::WORLD_TRANSFORM_CHANGED);
    scene->GetEventSystem()->RegisterSystemForEvent(this, EventSystem::SOUND_COMPONENT_CHANGED);
}

SoundUpdateSystem::~SoundUpdateSystem()
{
    DVASSERT(sounds.size() == 0);
    pausedEvents.clear();
}

void SoundUpdateSystem::ImmediateEvent(Entity * entity, uint32 event)
{
	if (event == EventSystem::WORLD_TRANSFORM_CHANGED || event == EventSystem::SOUND_COMPONENT_CHANGED)
	{
		const Matrix4 & worldTransform = GetTransformComponent(entity)->GetWorldTransform();
		Vector3 translation = worldTransform.GetTranslationVector();

        SoundComponent * sc = GetSoundComponent(entity);
        DVASSERT(sc);

        uint32 eventsCount = sc->GetEventsCount();
        for(uint32 i = 0; i < eventsCount; ++i)
        {
            SoundEvent * sound = sc->GetSoundEvent(i);
            sound->SetPosition(translation);
            if(sound->IsDirectional())
            {
                Vector3 worldDirection = MultiplyVectorMat3x3(sc->GetLocalDirection(i), worldTransform);
                sound->SetDirection(worldDirection);
            }
            sound->UpdateInstancesPosition();
        }
	}
}
    
void SoundUpdateSystem::Process(float32 timeElapsed)
{
    TIME_PROFILE("SoundUpdateSystem::Process")

    Camera * activeCamera = GetScene()->GetCurrentCamera();

    if(activeCamera)
    {
        SoundSystem * ss = SoundSystem::Instance();
        const Vector3 & listenerPosition = activeCamera->GetPosition();
        ss->SetListenerPosition(listenerPosition);
        ss->SetListenerOrientation(activeCamera->GetDirection(), activeCamera->GetLeft());

        uint32 autoCount = static_cast<uint32>(autoTriggerSounds.size());
        for(uint32 i = 0; i < autoCount; ++i)
        {
            AutoTriggerSound & autoTriggerSound = autoTriggerSounds[i];
            float32 distanceSq = (listenerPosition - autoTriggerSound.owner->GetWorldTransform().GetTranslationVector()).SquareLength();
            if(distanceSq < autoTriggerSound.maxSqDistance)
            {
                if(!autoTriggerSound.soundEvent->IsActive())
                    autoTriggerSound.soundEvent->Trigger();
            }
            else
            {
                if(autoTriggerSound.soundEvent->IsActive())
                    autoTriggerSound.soundEvent->Stop();
            }
        }
    }
}

void SoundUpdateSystem::AddEntity(Entity * entity)
{
    SoundComponent * sc = GetSoundComponent(entity);
    DVASSERT(sc);
    
    uint32 eventsCount = sc->GetEventsCount();
    for(uint32 i = 0; i < eventsCount; ++i)
    {
        if((sc->GetSoundEventFlags(i) & SoundComponent::FLAG_AUTO_DISTANCE_TRIGGER) > 0 && sc->GetSoundEvent(i)->IsActive())
        {
            AddAutoTriggerSound(entity, sc->GetSoundEvent(i));
        }
    }
    
    sounds.push_back(entity);
}

void SoundUpdateSystem::RemoveEntity(Entity * entity)
{
    SoundComponent * sc = GetSoundComponent(entity);
    DVASSERT(sc);

    uint32 eventsCount = sc->GetEventsCount();
    for(uint32 i = 0; i < eventsCount; ++i)
    {
        SoundEvent * sound = sc->GetSoundEvent(i);
        sound->Stop(true);
    }

    RemoveAutoTriggerSound(entity);
    FindAndRemoveExchangingWithLast(sounds, entity);
}

void SoundUpdateSystem::AddAutoTriggerSound(Entity * soundOwner, SoundEvent * sound)
{
    int32 soundsCount = static_cast<int32>(autoTriggerSounds.size());
    for(int32 i = 0; i < soundsCount; ++i)
    {
        if(autoTriggerSounds[i].owner == soundOwner && autoTriggerSounds[i].soundEvent == sound)
        {
            return;
        }
    }

    autoTriggerSounds.push_back(AutoTriggerSound(soundOwner, sound));
}

void SoundUpdateSystem::RemoveAutoTriggerSound(Entity * soundOwner, SoundEvent * sound /* = 0 */)
{
    for(int32 i = static_cast<int32>(autoTriggerSounds.size() - 1); i >= 0; --i)
    {
        if(autoTriggerSounds[i].owner == soundOwner && (sound == 0 || autoTriggerSounds[i].soundEvent == sound))
        {
            RemoveExchangingWithLast(autoTriggerSounds, i);
        }
    }
}
    
void SoundUpdateSystem::Deactivate()
{
    DVASSERT(pausedEvents.size() == 0);
    
    for(auto entity: sounds)
    {
        auto sound = DAVA::GetSoundComponent(entity);
        DVASSERT(sound);
        
        auto eventCount = sound->GetEventsCount();
        for(size_t i = 0; i < eventCount; ++i)
        {
            auto soundEvent = sound->GetSoundEvent(i);
            if(soundEvent->IsActive())
            {
                auto flags = sound->GetSoundEventFlags(i);
                if((flags & DAVA::SoundComponent::FLAG_AUTO_DISTANCE_TRIGGER) == DAVA::SoundComponent::FLAG_AUTO_DISTANCE_TRIGGER)
                {
                    soundEvent->Stop();
                }
                else
                {
                    soundEvent->SetPaused(true);
                    pausedEvents.push_back(soundEvent);
                }
            }
        }
    }
}

void SoundUpdateSystem::Activate()
{
    for(auto soundEvent: pausedEvents)
    {
        soundEvent->SetPaused(false);
    }
    
    pausedEvents.clear();
}



};