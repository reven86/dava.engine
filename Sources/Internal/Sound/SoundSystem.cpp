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

#include "Base/FastName.h"
#include "Sound/SoundSystem.h"
#include "Sound/SoundGroup.h"
#include "Sound/SoundEvent.h"
#include "Sound/SoundEventCategory.h"
#include "Sound/VolumeAnimatedObject.h"
#include "Sound/FMODUtils.h"

#ifdef __DAVAENGINE_IPHONE__
#include "fmodiphone.h"
#endif

namespace DAVA
{
	
const FastName SoundSystem::SOUND_GROUP_FX = FastName("FX");

SoundSystem::SoundSystem(int32 maxChannels)
{
	FMOD_VERIFY(FMOD::EventSystem_Create(&fmodEventSystem));
	FMOD_VERIFY(fmodEventSystem->getSystemObject(&fmodSystem));
	FMOD_VERIFY(fmodEventSystem->init(maxChannels, FMOD_INIT_NORMAL, 0));
    FMOD_VERIFY(fmodSystem->set3DSettings(1.f, 1.f, 1.0f));
}

SoundSystem::~SoundSystem()
{
	for(FastNameMap<SoundGroup*>::iterator it = soundGroups.begin(); it != soundGroups.end(); ++it)
	{
		SoundGroup * soundGroup = it->second;
		SafeRelease(soundGroup);
	}
    soundGroups.clear();

	FMOD_VERIFY(fmodEventSystem->release());
}

void SoundSystem::LoadFEV(const FilePath & filePath)
{
	FMOD_VERIFY(fmodEventSystem->load(filePath.GetAbsolutePathname().c_str(), 0, 0));
}

SoundEvent * SoundSystem::CreateSoundEvent(const String & eventPath)
{
	FMOD::Event * fmodEvent = 0;
	FMOD_VERIFY(fmodEventSystem->getEvent(eventPath.c_str(), FMOD_EVENT_DEFAULT, &fmodEvent));
	if(fmodEvent)
		return new SoundEvent(fmodEvent);
	else
		return 0;
}

void SoundSystem::Update()
{
    int32 size = animatedObjects.size();
    for(int32 i = 0; i < size; i++)
        animatedObjects[i]->Update();

	fmodEventSystem->update();

    size = soundsToReleaseOnUpdate.size();
    for(int32 i = 0; i < size; i++)
        SafeRelease(soundsToReleaseOnUpdate[i]);
    soundsToReleaseOnUpdate.clear();
}

void SoundSystem::Suspend()
{
	/*
#ifdef __DAVAENGINE_ANDROID__
	for(FastNameMap<SoundGroup*>::iterator it = soundGroups.begin(); it != soundGroups.end(); ++it)
	{
		SoundGroup * soundGroup = it.GetValue();
		soundGroup->Stop();
	}
#endif
*/
}

void SoundSystem::Resume()
{
#ifdef __DAVAENGINE_IPHONE__
    FMOD_IPhone_RestoreAudioSession();
#endif
}

void SoundSystem::SetListenerPosition(const Vector3 & position)
{
	FMOD_VECTOR pos = {position.x, position.y, position.z};
	FMOD_VERIFY(fmodEventSystem->set3DListenerAttributes(0, &pos, 0, 0, 0));
}

void SoundSystem::SetListenerOrientation(const Vector3 & at, const Vector3 & left)
{
	Vector3 atNorm = at;
	atNorm.Normalize();
	Vector3 upNorm = at.CrossProduct(left);
	upNorm.Normalize();

	FMOD_VECTOR fmodAt = {atNorm.x, atNorm.y, atNorm.z};
	FMOD_VECTOR fmodUp = {upNorm.x, upNorm.y, upNorm.z};
	FMOD_VERIFY(fmodEventSystem->set3DListenerAttributes(0, 0, 0, &fmodAt, &fmodUp));
}

SoundGroup * SoundSystem::GetSoundGroup(const FastName & groupName)
{
	if(soundGroups.count(groupName))
		return soundGroups[groupName];
    else
        return 0;
}

SoundGroup * SoundSystem::CreateSoundGroup(const FastName & groupName)
{
	SoundGroup * group = 0;
	if(soundGroups.count(groupName))
	{
		group = soundGroups[groupName];
	}
    else
    {
        group = new SoundGroup();
        soundGroups.Insert(groupName, group);
    }

	return group;
}

ScopedPtr<SoundEventCategory> SoundSystem::GetSoundEventCategory(const String & category)
{
	FMOD::EventCategory * fmodCategory = 0;
	FMOD_VERIFY(fmodEventSystem->getCategory(category.c_str(), &fmodCategory));

	if(fmodCategory)
		return ScopedPtr<SoundEventCategory>(new SoundEventCategory(fmodCategory));
	else
		return ScopedPtr<SoundEventCategory>(0);
}

void SoundSystem::ReleaseOnUpdate(Sound * sound)
{
    soundsToReleaseOnUpdate.push_back(sound);
}

void SoundSystem::AddVolumeAnimatedObject(VolumeAnimatedObject * object)
{
	animatedObjects.push_back(object);
}

void SoundSystem::RemoveVolumeAnimatedObject(VolumeAnimatedObject * object)
{
	Vector<VolumeAnimatedObject *>::iterator it = std::find(animatedObjects.begin(), animatedObjects.end(), object);
	if(it != animatedObjects.end())
		animatedObjects.erase(it);
}

};
