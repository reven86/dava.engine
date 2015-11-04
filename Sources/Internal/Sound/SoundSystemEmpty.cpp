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


#include "Sound/SoundSystem.h"

#ifndef DAVA_FMOD

namespace DAVA
{
    Mutex SoundSystem::soundGroupsMutex;

    SoundSystem::SoundSystem()
    {
    }
    
    SoundSystem::~SoundSystem()
    {
    }
    
    SoundEvent* SoundSystem::CreateSoundEventByID(const FastName & eventName, const FastName & groupName)
    {
        return 0;
    }
    
    SoundEvent* SoundSystem::CreateSoundEventFromFile(const FilePath & fileName, const FastName & groupName, uint32 createFlags, int32 priority)
    {
        return 0;
    }

    void SoundSystem::SerializeEvent(const SoundEvent *sEvent, KeyedArchive *toArchive)
    {
    }
    
    SoundEvent* SoundSystem::DeserializeEvent(KeyedArchive *archive)
    {
        return 0;
    }
    
    SoundEvent* SoundSystem::CloneEvent(const SoundEvent *sEvent)
    {
        return 0;
    }
    
    void SoundSystem::Update(float32 timeElapsed)
    {
    }
    
    void SoundSystem::Suspend()
    {
    }
    
    void SoundSystem::Resume()
    {
    }
    
    void SoundSystem::SetCurrentLocale(const String &langID)
    {
    }

    String SoundSystem::GetCurrentLocale() const
    {
        return String();
    }

    void SoundSystem::SetListenerPosition(const Vector3 &position)
    {
    }
    
    void SoundSystem::SetListenerOrientation(const Vector3 &forward, const Vector3 &left)
    {
    }
    
    void SoundSystem::SetGroupVolume(const FastName &groupName, float32 volume)
    {
    }
    
    float32 SoundSystem::GetGroupVolume(const FastName & groupName)
    {
        return 0.0f;
    }
    
    void SoundSystem::InitFromQualitySettings()
    {
    }
    
    void SoundSystem::ParseSFXConfig(const FilePath & configPath)
    {
    }
    
}//DAVA

#endif //DAVA_FMOD
