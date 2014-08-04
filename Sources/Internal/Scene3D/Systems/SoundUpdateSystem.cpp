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

namespace DAVA
{

SoundUpdateSystem::AutoTriggerSound::AutoTriggerSound(SoundComponent * _component, SoundEvent * _sound) :
    component(_component),
    soundEvent(_sound),
    wasTriggered(false)
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
}

void SoundUpdateSystem::ImmediateEvent(Entity * entity, uint32 event)
{
	if (event == EventSystem::WORLD_TRANSFORM_CHANGED || event == EventSystem::SOUND_COMPONENT_CHANGED)
	{
		const Matrix4 & worldTransform = GetTransformComponent(entity)->GetWorldTransform();
		Vector3 translation = worldTransform.GetTranslationVector();

        SoundComponent * sc = GetSoundComponent(entity);
        DVASSERT(sc);

        bool hasAutoTrigger = false;
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

            hasAutoTrigger |= ((sc->GetSoundEventFlags(i) & SoundComponent::FLAG_AUTO_DISTANCE_TRIGGER) > 0);
        }

        if(event == EventSystem::SOUND_COMPONENT_CHANGED)
        {
            bool needRebuild = false;
            if(hasAutoTrigger)
                needRebuild = componentsWithAutoTrigger.insert(sc).second;
            else
                needRebuild = (componentsWithAutoTrigger.erase(sc) != 0);

            if(needRebuild)
                RebuildAutoTriggerArray();
        }
	}
}
    
void SoundUpdateSystem::Process(float32 timeElapsed)
{
    Camera * activeCamera = GetScene()->GetCurrentCamera();

    if(activeCamera)
    {
        SoundSystem * ss = SoundSystem::Instance();
        const Vector3 & listenerPosition = activeCamera->GetPosition();
        ss->SetListenerPosition(listenerPosition);
        ss->SetListenerOrientation(activeCamera->GetDirection(), activeCamera->GetLeft());

        uint32 autoCount = autoTriggerSounds.size();
        for(uint32 i = 0; i < autoCount; ++i)
        {
            AutoTriggerSound & autoTriggerSound = autoTriggerSounds[i];
            if(autoTriggerSound.wasTriggered)
            {
                float32 distanceSq = (listenerPosition - autoTriggerSound.component->GetEntity()->GetWorldTransform().GetTranslationVector()).SquareLength();
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
}

void SoundUpdateSystem::RebuildAutoTriggerArray()
{
    Map<SoundEvent *, bool> wasTriggeredMap;
    uint32 autoCount = autoTriggerSounds.size();
    for(uint32 i = 0; i < autoCount; ++i)
    {
        wasTriggeredMap[autoTriggerSounds[i].soundEvent] = autoTriggerSounds[i].wasTriggered;
    }
    autoTriggerSounds.clear();

    Set<SoundComponent *>::iterator it = componentsWithAutoTrigger.begin();
    Set<SoundComponent *>::iterator itEnd = componentsWithAutoTrigger.end();
    for(; it != itEnd; ++it)
    {
        SoundComponent * sc = (*it);
        uint32 eventsCount = sc->GetEventsCount();
        for(uint32 i = 0; i < eventsCount; ++i)
        {
            SoundEvent * sEvent = sc->GetSoundEvent(i);
            sEvent->RemoveEvent(SoundEvent::EVENT_TRIGGERED, Message(this, &SoundUpdateSystem::OnSoundTriggeredCallback));
            sEvent->AddEvent(SoundEvent::EVENT_TRIGGERED, Message(this, &SoundUpdateSystem::OnSoundTriggeredCallback));

            AutoTriggerSound autoTrigger(sc, sEvent);
            if(wasTriggeredMap.find(sEvent) != wasTriggeredMap.end())
                autoTrigger.wasTriggered = wasTriggeredMap[sEvent];

            autoTrigger.wasTriggered |= sEvent->IsActive();

            autoTriggerSounds.push_back(autoTrigger);
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
        if((sc->GetSoundEventFlags(i) & SoundComponent::FLAG_AUTO_DISTANCE_TRIGGER) > 0)
        {
            componentsWithAutoTrigger.insert(sc);
            RebuildAutoTriggerArray();
            break;
        }
    }
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

    if(componentsWithAutoTrigger.erase(sc))
        RebuildAutoTriggerArray();
}

void SoundUpdateSystem::OnSoundTriggeredCallback(BaseObject * caller, void * param, void *callerData)
{
    uint32 autoCount = autoTriggerSounds.size();
    for(uint32 i = 0; i < autoCount; ++i)
    {
        if(autoTriggerSounds[i].soundEvent == caller)
            autoTriggerSounds[i].wasTriggered = true;
    }
}

};