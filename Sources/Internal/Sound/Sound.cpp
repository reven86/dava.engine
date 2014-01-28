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

#include "Sound/Sound.h"
#include "Sound/SoundSystem.h"
#include "Sound/SoundGroup.h"
#include "Sound/FMODUtils.h"

namespace DAVA
{
#if !defined(__DAVAENGINE_HTML5__)
FMOD_RESULT F_CALLBACK SoundInstanceEndPlaying(FMOD_CHANNEL *channel, FMOD_CHANNEL_CALLBACKTYPE type, void *commanddata1, void *commanddata2);
#endif

#if defined(__DAVAENGINE_HTML5__)
Map<int, Sound*> Sound::channelsUserData;
Map<int, Message> callbacksMap;
int s_nFreeSoundID = 1;
#endif
    
#if !defined(__DAVAENGINE_HTML5__)
Map<String, FMOD::Sound*> soundMap;
Map<FMOD::Sound*, int32> soundRefsMap;
#endif

Sound * Sound::Create(const FilePath & fileName, eType type, const FastName & groupName, int32 priority /* = 128 */)
{
	return CreateWithFlags(fileName, type, groupName, 0, priority);
}

Sound * Sound::Create3D(const FilePath & fileName, eType type, const FastName & groupName, int32 priority)
{
	return CreateWithFlags(fileName, type, groupName, FMOD_3D, priority);
}

Sound * Sound::CreateWithFlags(const FilePath & fileName, eType type, const FastName & groupName, int32 flags, int32 priority)
{
	Sound * sound = new Sound(fileName, type, priority);

    if(flags & FMOD_3D)
        sound->is3d = true;

#if !defined(__DAVAENGINE_HTML5__)
    Map<String, FMOD::Sound*>::iterator it;
    it = soundMap.find(fileName.GetAbsolutePathname());
    if (it != soundMap.end())
    {
        sound->fmodSound = it->second;
        soundRefsMap[sound->fmodSound]++;
    }
#endif

#if defined(__DAVAENGINE_HTML5__)
    if(!sound->soundChunk)
#else
    if(!sound->fmodSound)
#endif
    {
        File * file = File::Create(fileName, File::OPEN | File::READ);
        if(!file)
        {
            SafeRelease(sound);
            return 0;
        }

        int32 fileSize = file->GetSize();
        if(!fileSize)
        {
            SafeRelease(sound);
            return 0;
        }

        sound->soundData = new uint8[fileSize];
        file->Read(sound->soundData, fileSize);
        SafeRelease(file);

#if defined(__DAVAENGINE_HTML5__)
        SDL_RWops * ops = SDL_RWFromConstMem(sound->soundData, fileSize);
        sound->soundChunk = Mix_LoadWAV_RW(ops, 0);
        SDL_FreeRW(ops);
        sound->nSoundID = s_nFreeSoundID++;
        sound->SetSoundGroup(groupName);
        sound->SetLoopCount(0);
        
#else
        FMOD_CREATESOUNDEXINFO exInfo;
        memset(&exInfo, 0, sizeof(FMOD_CREATESOUNDEXINFO));
        exInfo.cbsize = sizeof(FMOD_CREATESOUNDEXINFO);
        exInfo.length = fileSize;

        switch (type)
        {
        case TYPE_STATIC:
            FMOD_VERIFY(SoundSystem::Instance()->fmodSystem->createSound((char *)sound->soundData, FMOD_LOOP_NORMAL | FMOD_OPENMEMORY | flags, &exInfo, &sound->fmodSound));
            SafeDelete(sound->soundData);
            break;
        case TYPE_STREAMED:
            FMOD_VERIFY(SoundSystem::Instance()->fmodSystem->createStream((char *)sound->soundData, FMOD_LOOP_NORMAL | FMOD_OPENMEMORY | flags, &exInfo, &sound->fmodSound));
            break;
        }

        sound->SetSoundGroup(groupName);
        sound->SetLoopCount(0);

#if !defined DONT_USE_DEFAULT_3D_SOUND_SETTINGS
        if( sound->is3d && sound->fmodSound )
            FMOD_VERIFY( sound->fmodSound->set3DMinMaxDistance(12.0f, 1000.0f) );
#endif

        soundMap[sound->fileName.GetAbsolutePathname()] = sound->fmodSound;
        soundRefsMap[sound->fmodSound] = 1;
#endif
    }

#if !defined(__DAVAENGINE_HTML5__)
	FMOD_VERIFY(SoundSystem::Instance()->fmodSystem->createChannelGroup(0, &sound->fmodInstanceGroup));
#endif

	return sound;
}
Sound::Sound(const FilePath & _fileName, eType _type, int32 _priority)
:	fileName(_fileName),
	type(_type),
	priority(_priority),
	is3d(false),
#if defined(__DAVAENGINE_HTML5__)
    nLoopCount(0),
    fVolume(1.0),
    nChannelID(-1),
    soundChunk(NULL),
    nSoundID(-1),
    soundGroup(NULL),
    nPlayedCallbacks(-1),
#endif
    soundData(0),
    fmodSound(0),
    fmodInstanceGroup(0)
{
}

Sound::~Sound()
{
    SafeDeleteArray(soundData);

#if defined(__DAVAENGINE_HTML5__)
    soundGroup->sounds.erase(this);
#endif
    
#if !defined(__DAVAENGINE_HTML5__)
    if(fmodInstanceGroup)
        FMOD_VERIFY(fmodInstanceGroup->release());
#endif
}

int32 Sound::Release()
{
    if(GetRetainCount() == 1)
    {
#if !defined(__DAVAENGINE_HTML5__)
        soundRefsMap[fmodSound]--;
        if(soundRefsMap[fmodSound] == 0)
        {
            soundMap.erase(fileName.GetAbsolutePathname());
            soundRefsMap.erase(fmodSound);
            FMOD_VERIFY(fmodSound->release());
        }
#endif
    }

    return BaseObject::Release();
}
void Sound::SetSoundGroup(const FastName & groupName)
{
	SoundGroup * soundGroup = SoundSystem::Instance()->CreateSoundGroup(groupName);
	if(soundGroup)
	{
#if defined(__DAVAENGINE_HTML5__)
        soundGroup->sounds.insert(this);
        this->soundGroup = soundGroup;
#else
		FMOD_VERIFY(fmodSound->setSoundGroup(soundGroup->fmodSoundGroup));
#endif
	}
}

void Sound::Play(const Message & msg)
{
#if defined(__DAVAENGINE_HTML5__)
    nChannelID = Mix_PlayChannel(-1, soundChunk, nLoopCount);
    if(nChannelID != -1)
    {
        if(soundGroup)
        {
            Mix_Volume(nChannelID, (int)(soundGroup->GetVolume()*fVolume*MIX_MAX_VOLUME));
        }
        else
        {
            Mix_Volume(nChannelID, (int)(fVolume*MIX_MAX_VOLUME));
        }
        std::map<int, Sound*>::iterator it = Sound::channelsUserData.find(nChannelID);
        if(it != channelsUserData.end())
        {
            it->second = this;
        }
        else
        {
            Sound::channelsUserData.insert(std::pair<int, Sound*>(nChannelID, this));
        }
        if(!msg.IsEmpty())
        {
            Map<int, Message>::iterator it = callbacksMap.find(nChannelID);
            if(it != callbacksMap.end())
            {
                it->second = msg;
            }
            else
            {
                callbacksMap.insert(std::pair<int, Message>(nChannelID, msg));
            }
        }
    }
#else
	FMOD::Channel * fmodInstance = 0;
	FMOD_VERIFY(SoundSystem::Instance()->fmodSystem->playSound(FMOD_CHANNEL_FREE, fmodSound, true, &fmodInstance)); //start sound paused
	FMOD_VECTOR pos = {position.x, position.y, position.z};
	FMOD_VERIFY(fmodInstance->setPriority(priority));
	FMOD_VERIFY(fmodInstance->setCallback(SoundInstanceEndPlaying));
	FMOD_VERIFY(fmodInstance->setUserData(this));
	FMOD_VERIFY(fmodInstance->setChannelGroup(fmodInstanceGroup));

    if(fmodInstance && !msg.IsEmpty())
        callbacks[fmodInstance] = msg;

	if(is3d)
		FMOD_VERIFY(fmodInstance->set3DAttributes(&pos, 0));

	FMOD_VERIFY(fmodInstance->setPaused(false));

    Retain();
#endif
}

void Sound::SetPosition(const Vector3 & _position)
{
	position = _position;
}

void Sound::UpdateInstancesPosition()
{
#if defined(__DAVAENGINE_HTML5__)
#else
	if(is3d)
	{
		FMOD_VECTOR pos = {position.x, position.y, position.z};
		int32 instancesCount = 0;
		FMOD_VERIFY(fmodInstanceGroup->getNumChannels(&instancesCount));
		for(int32 i = 0; i < instancesCount; i++)
		{
			FMOD::Channel * inst = 0;
			FMOD_VERIFY(fmodInstanceGroup->getChannel(i, &inst));
			FMOD_VERIFY(inst->set3DAttributes(&pos, 0));
		}
	}
#endif
}

Sound::eType Sound::GetType() const
{
	return type;
}

void Sound::SetVolume(float32 volume)
{
#if defined(__DAVAENGINE_HTML5__)
    fVolume = volume;
    if(nChannelID != -1)
    {
        if(soundGroup)
        {
            Mix_Volume(nChannelID, (int)(soundGroup->GetVolume()*fVolume*MIX_MAX_VOLUME));
        }
        else
        {
            Mix_Volume(nChannelID, (int)(fVolume*MIX_MAX_VOLUME));
        }
    }
#else
	FMOD_VERIFY(fmodInstanceGroup->setVolume(volume));
#endif
}

float32	Sound::GetVolume()
{
	float32 volume = 0;
#if defined(__DAVAENGINE_HTML5__)
    volume = fVolume;
#else
	FMOD_VERIFY(fmodInstanceGroup->getVolume(&volume));
#endif
	return volume;
}

void Sound::Pause(bool isPaused)
{
#if defined(__DAVAENGINE_HTML5__)
    Mix_Pause(nChannelID);
#else
    FMOD_VERIFY(fmodInstanceGroup->setPaused(isPaused));
#endif
}

bool Sound::IsPaused()
{
	bool isPaused = false;
#if defined(__DAVAENGINE_HTML5__)
    isPaused = (Mix_Paused(nChannelID) != 0);
#else
	FMOD_VERIFY(fmodInstanceGroup->getPaused(&isPaused));
#endif
	return isPaused;
}

void Sound::Stop()
{
#if defined(__DAVAENGINE_HTML5__)
    Mix_HaltChannel(nChannelID);
#else
    FMOD_VERIFY(fmodInstanceGroup->stop());
#endif
}

int32 Sound::GetLoopCount() const
{
	int32 loopCount;
#if defined(__DAVAENGINE_HTML5__)
    loopCount = nLoopCount;
#else
	FMOD_VERIFY(fmodSound->getLoopCount(&loopCount));
#endif
	return loopCount;
}

void Sound::SetLoopCount(int32 loopCount)
{
#if defined(__DAVAENGINE_HTML5__)
    nLoopCount = loopCount;
#else
	FMOD_VERIFY(fmodSound->setLoopCount(loopCount));
#endif
}

#if defined(__DAVAENGINE_HTML5__)
void Sound::PerformCallback()
{
    nPlayedCallbacks++;
    if(nPlayedCallbacks == nLoopCount)
    {
        Map<int, Message>::iterator it = callbacksMap.find(nChannelID);
        if(it != callbacksMap.end())
        {
            it->second(this);
            callbacksMap.erase(it);
        }
        //SoundSystem::Instance()->ReleaseOnUpdate(this);
    }
}
    
void SoundChannelFinishedPlaying(int nChannelID)
{
    Sound *sound = 0;
    std::map<int, Sound*>::iterator it = Sound::channelsUserData.find(nChannelID);
    if(it != Sound::channelsUserData.end())
    {
        sound = it->second;
        it->second = NULL;
        if(sound)
        {
            sound->PerformCallback();
        }
    }
}
    
#else
void Sound::PerformCallback(FMOD::Channel * instance)
{
    Map<FMOD::Channel *, Message>::iterator it = callbacks.find(instance);
    if(it != callbacks.end())
    {
        it->second(this);
        callbacks.erase(it);
    }
    
    SoundSystem::Instance()->ReleaseOnUpdate(this);
}
    
FMOD_RESULT F_CALLBACK SoundInstanceEndPlaying(FMOD_CHANNEL *channel, FMOD_CHANNEL_CALLBACKTYPE type, void *commanddata1, void *commanddata2)
{
	if(type == FMOD_CHANNEL_CALLBACKTYPE_END)
	{
		FMOD::Channel *cppchannel = (FMOD::Channel *)channel;
        if(cppchannel)
        {
            Sound * sound = 0;
            FMOD_VERIFY(cppchannel->getUserData((void**)&sound));
            if(sound)
                sound->PerformCallback(cppchannel);
        }
	}

	return FMOD_OK;
}
#endif

};
