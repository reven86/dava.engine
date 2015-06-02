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

#ifdef DAVA_FMOD

#include "Sound/FMODFileSoundEvent.h"
#include "Sound/SoundSystem.h"
#include "Scene3D/Entity.h"

namespace DAVA
{
Map<FilePath, FMOD::Sound*> soundMap;
Map<FMOD::Sound*, int32> soundRefsMap;

Mutex FMODFileSoundEvent::soundMapMutex;

FMODFileSoundEvent * FMODFileSoundEvent::CreateWithFlags(const FilePath & fileName, uint32 flags, int32 priority /* = 128 */)
{
    SoundSystem * soundSystem = SoundSystem::Instance();
    
	FMODFileSoundEvent * sound = new FMODFileSoundEvent(fileName, flags, priority);

    FMOD_MODE fmodMode = FMOD_DEFAULT;
    
	if(flags & SOUND_EVENT_CREATE_3D)
        fmodMode |= FMOD_3D;
    
    if(flags & SOUND_EVENT_CREATE_LOOP)
        fmodMode |= FMOD_LOOP_NORMAL;
    
    if(flags & SOUND_EVENT_CREATE_STREAM)
        fmodMode |= FMOD_CREATESTREAM;
    
    soundMapMutex.Lock();
    Map<FilePath, FMOD::Sound*>::iterator it;
    it = soundMap.find(fileName);
    if (it != soundMap.end())
    {
        sound->fmodSound = it->second;
        soundRefsMap[sound->fmodSound]++;
    }
    else
    {
        if (soundSystem->fmodSystem)
        {
            FMOD_VERIFY(soundSystem->fmodSystem->createSound(fileName.GetStringValue().c_str(), fmodMode, 0, &sound->fmodSound));
        }

        if(!sound->fmodSound)
        {
            soundMapMutex.Unlock();
            SafeRelease(sound);
            return nullptr;
        }

        if(flags & SOUND_EVENT_CREATE_LOOP)
            sound->SetLoopCount(-1);

        soundMap[sound->fileName] = sound->fmodSound;
        soundRefsMap[sound->fmodSound] = 1;
    }
    soundMapMutex.Unlock();

    if (soundSystem->fmodSystem)
    {
        FMOD_VERIFY(soundSystem->fmodSystem->createChannelGroup(0, &sound->fmodInstanceGroup));
        FMOD_VERIFY(soundSystem->masterChannelGroup->addGroup(sound->fmodInstanceGroup));
    }
    
	return sound;
}
    
FMODFileSoundEvent::FMODFileSoundEvent(const FilePath & _fileName, uint32 _flags, int32 _priority) :
    fileName(_fileName),
    priority(_priority),
    flags(_flags),
    fmodSound(0),
    fmodInstanceGroup(0)
{
}

FMODFileSoundEvent::~FMODFileSoundEvent()
{
    if(fmodInstanceGroup)
        FMOD_VERIFY(fmodInstanceGroup->release());

    SoundSystem::Instance()->RemoveSoundEventFromGroups(this);
}

int32 FMODFileSoundEvent::Release()
{
    if(GetRetainCount() == 1)
    {
        soundMapMutex.Lock();
        soundRefsMap[fmodSound]--;
        if(soundRefsMap[fmodSound] == 0)
        {
            soundMap.erase(fileName);
            soundRefsMap.erase(fmodSound);
            FMOD_VERIFY(fmodSound->release());
        }
        soundMapMutex.Unlock();
    }

    return BaseObject::Release();
}

bool FMODFileSoundEvent::Trigger()
{
    if (nullptr == SoundSystem::Instance()->fmodSystem)
        return false;

    FMOD::Channel * fmodInstance = nullptr;
    FMOD_VERIFY(SoundSystem::Instance()->fmodSystem->playSound(FMOD_CHANNEL_FREE, fmodSound, true, &fmodInstance)); //start sound paused
    if (fmodInstance && fmodInstanceGroup)
    {
        FMOD_VERIFY(fmodInstance->setPriority(priority));
        FMOD_VERIFY(fmodInstance->setCallback(SoundInstanceEndPlaying));
        FMOD_VERIFY(fmodInstance->setUserData(this));
        FMOD_VERIFY(fmodInstance->setChannelGroup(fmodInstanceGroup));

        if (flags & SOUND_EVENT_CREATE_3D)
            FMOD_VERIFY(fmodInstance->set3DAttributes((FMOD_VECTOR*)&position, 0));

        FMOD_VERIFY(fmodInstanceGroup->setPaused(false));
        FMOD_VERIFY(fmodInstance->setPaused(false));

        Retain();

        PerformEvent(EVENT_TRIGGERED);

        return true;
    }

    return false;
}

void FMODFileSoundEvent::SetPosition(const Vector3 & _position)
{
    position.x = _position.x;
    position.y = _position.y;
    position.z = _position.z;
}

void FMODFileSoundEvent::UpdateInstancesPosition()
{
    if (flags & SOUND_EVENT_CREATE_3D && fmodInstanceGroup)
	{
		int32 instancesCount = 0;
		FMOD_VERIFY(fmodInstanceGroup->getNumChannels(&instancesCount));
		for(int32 i = 0; i < instancesCount; i++)
		{
			FMOD::Channel * inst = 0;
			FMOD_VERIFY(fmodInstanceGroup->getChannel(i, &inst));
			FMOD_VERIFY(inst->set3DAttributes((FMOD_VECTOR*)&position, 0));
		}
	}
}

void FMODFileSoundEvent::SetVolume(float32 _volume)
{
    volume = _volume;
    if (fmodInstanceGroup)
    {
        FMOD_VERIFY(fmodInstanceGroup->setVolume(volume));
    }
}

bool FMODFileSoundEvent::IsActive() const
{
    if (fmodInstanceGroup)
    {
        int32 numChanels = 0;
        FMOD_VERIFY(fmodInstanceGroup->getNumChannels(&numChanels));

        bool isPaused = false;
        FMOD_VERIFY(fmodInstanceGroup->getPaused(&isPaused));

        return numChanels != 0 && !isPaused;
    }
    
    return false;
}

void FMODFileSoundEvent::Stop(bool force /* = false */)
{
    if (fmodInstanceGroup)
    {
        FMOD_VERIFY(fmodInstanceGroup->stop());
    }
}

int32 FMODFileSoundEvent::GetLoopCount()
{
	int32 loopCount = 0;
    if (fmodInstanceGroup)
    {
        FMOD_VERIFY(fmodSound->getLoopCount(&loopCount));
    }
	return loopCount;
}

void FMODFileSoundEvent::SetLoopCount(int32 loopCount)
{
    if (fmodInstanceGroup)
    {
        FMOD_VERIFY(fmodSound->setLoopCount(loopCount));
    }
}

void FMODFileSoundEvent::SetPaused(bool paused)
{
    if (fmodInstanceGroup)
    {
        FMOD_VERIFY(fmodInstanceGroup->setPaused(paused));
    }
}

FMOD_RESULT F_CALLBACK FMODFileSoundEvent::SoundInstanceEndPlaying(FMOD_CHANNEL *channel, FMOD_CHANNEL_CALLBACKTYPE type, void *commanddata1, void *commanddata2)
{
	if(type == FMOD_CHANNEL_CALLBACKTYPE_END)
	{
		FMOD::Channel *cppchannel = (FMOD::Channel *)channel;
        if(cppchannel)
        {
            FMODFileSoundEvent * sound = 0;
            FMOD_VERIFY(cppchannel->getUserData((void**)&sound));
            if(sound)
			{
                sound->PerformEvent(EVENT_END);
				SoundSystem::Instance()->ReleaseOnUpdate(sound);
			}
        }
	}

	return FMOD_OK;
}

};

#endif //DAVA_FMOD
