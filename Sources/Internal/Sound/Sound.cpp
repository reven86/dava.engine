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
FMOD_RESULT F_CALLBACK SoundInstanceEndPlaying(FMOD_CHANNEL *channel, FMOD_CHANNEL_CALLBACKTYPE type, void *commanddata1, void *commanddata2);

#if defined (USE_FILEPATH_IN_MAP)
	typedef Map<FilePath, FMOD::Sound*> SoundMap;
#else //#if defined (USE_FILEPATH_IN_MAP)
	typedef Map<String, FMOD::Sound*> SoundMap;
#endif //#if defined (USE_FILEPATH_IN_MAP)
SoundMap soundMap;

Map<FMOD::Sound*, int32> soundRefsMap;

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

	SoundMap::iterator it = soundMap.find(FILEPATH_MAP_KEY(fileName));
    if (it != soundMap.end())
    {
        sound->fmodSound = it->second;
        soundRefsMap[sound->fmodSound]++;
    }

    if(!sound->fmodSound)
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

		soundMap[FILEPATH_MAP_KEY(sound->fileName)] = sound->fmodSound;
        soundRefsMap[sound->fmodSound] = 1;
    }

	FMOD_VERIFY(SoundSystem::Instance()->fmodSystem->createChannelGroup(0, &sound->fmodInstanceGroup));

	return sound;
}
Sound::Sound(const FilePath & _fileName, eType _type, int32 _priority)
:	fileName(_fileName),
	type(_type),
	priority(_priority),
	is3d(false),
    soundData(0),
    fmodSound(0),
    fmodInstanceGroup(0)
{
}

Sound::~Sound()
{
    SafeDeleteArray(soundData);

    if(fmodInstanceGroup)
        FMOD_VERIFY(fmodInstanceGroup->release());
}

int32 Sound::Release()
{
    if(GetRetainCount() == 1)
    {
        soundRefsMap[fmodSound]--;
        if(soundRefsMap[fmodSound] == 0)
        {
			soundMap.erase(FILEPATH_MAP_KEY(fileName));
            soundRefsMap.erase(fmodSound);
            FMOD_VERIFY(fmodSound->release());
        }
    }

    return BaseObject::Release();
}
void Sound::SetSoundGroup(const FastName & groupName)
{
	SoundGroup * soundGroup = SoundSystem::Instance()->CreateSoundGroup(groupName);
	if(soundGroup)
	{
		FMOD_VERIFY(fmodSound->setSoundGroup(soundGroup->fmodSoundGroup));
	}
}

void Sound::Play(const Message & msg)
{
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
}

void Sound::SetPosition(const Vector3 & _position)
{
	position = _position;
}

void Sound::UpdateInstancesPosition()
{
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
}

Sound::eType Sound::GetType() const
{
	return type;
}

void Sound::SetVolume(float32 volume)
{
    DVASSERT(volume >= 0.f && volume <= 1.f);

	FMOD_VERIFY(fmodInstanceGroup->setVolume(volume));
}

float32	Sound::GetVolume()
{
	float32 volume = 0;
	FMOD_VERIFY(fmodInstanceGroup->getVolume(&volume));
	return volume;
}

void Sound::Pause(bool isPaused)
{
    FMOD_VERIFY(fmodInstanceGroup->setPaused(isPaused));
}

bool Sound::IsPaused()
{
	bool isPaused = false;
	FMOD_VERIFY(fmodInstanceGroup->getPaused(&isPaused));
	return isPaused;
}

void Sound::Stop()
{
    FMOD_VERIFY(fmodInstanceGroup->stop());
}

int32 Sound::GetLoopCount() const
{
	int32 loopCount;
	FMOD_VERIFY(fmodSound->getLoopCount(&loopCount));
	return loopCount;
}

void Sound::SetLoopCount(int32 loopCount)
{
	FMOD_VERIFY(fmodSound->setLoopCount(loopCount));
}

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

};
