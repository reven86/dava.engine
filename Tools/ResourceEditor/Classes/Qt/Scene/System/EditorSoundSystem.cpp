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

#include "Scene/System/EditorSoundSystem.h"
#include "Scene3D/Components/SoundComponent.h"
#include "Scene3D/Components/ComponentHelpers.h"
#include "Utils/Utils.h"
#include "Debug/DVAssert.h"

EditorSoundSystem::EditorSoundSystem(DAVA::Scene * scene)
    : DAVA::SceneSystem(scene)
{
}

EditorSoundSystem::~EditorSoundSystem()
{
    DVASSERT(sounds.size() == 0);

    pausedEvents.clear();
}

void EditorSoundSystem::AddEntity(DAVA::Entity * entity)
{
    sounds.push_back(entity);
}

void EditorSoundSystem::RemoveEntity(DAVA::Entity * entity)
{
    DAVA::FindAndRemoveExchangingWithLast(sounds, entity);
}

void EditorSoundSystem::Pause()
{
    DVASSERT(pausedEvents.size() == 0);
    
    for(auto entity: sounds)
    {
        auto sound = DAVA::GetSoundComponent(entity);
        DVASSERT(sound);
        
        auto eventCount = sound->GetEventsCount();
        for(auto i = 0; i < eventCount; ++i)
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

void EditorSoundSystem::Start()
{
    for(auto soundEvent: pausedEvents)
    {
        soundEvent->SetPaused(false);
    }
    
    pausedEvents.clear();
}
