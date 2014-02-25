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

#ifndef __DAVAENGINE_SOUND_SYSTEM_H__
#define __DAVAENGINE_SOUND_SYSTEM_H__

#include "Base/Singleton.h"
#include "Base/BaseTypes.h"
#include "Base/BaseMath.h"
#include "Base/ScopedPtr.h"
#include "Base/FastNameMap.h"
#include "Sound/Sound.h"

namespace FMOD
{
class System;
class EventSystem;
};

namespace DAVA
{
class SoundGroup;
class SoundEvent;
class Animation;
class SoundEventCategory;
class VolumeAnimatedObject;
class SoundSystem : public Singleton<SoundSystem>
{
public:
	
	static const FastName SOUND_GROUP_FX;
	
public:
	
	SoundSystem(int32 maxChannels);
	virtual ~SoundSystem();

	void Update();
	void Suspend();
	void Resume();

	void SetListenerPosition(const Vector3 & position);
	void SetListenerOrientation(const Vector3 & at, const Vector3 & left);

	SoundEvent * CreateSoundEvent(const String & eventPath);

	void LoadFEV(const FilePath & filePath);

	SoundGroup * GetSoundGroup(const FastName & groupName);
	ScopedPtr<SoundEventCategory> GetSoundEventCategory(const String & category);

	void AddVolumeAnimatedObject(VolumeAnimatedObject * object);
	void RemoveVolumeAnimatedObject(VolumeAnimatedObject * object);

private:
	SoundGroup * CreateSoundGroup(const FastName & groupName);

    void ReleaseOnUpdate(Sound * sound);

	FMOD::System * fmodSystem;
	FMOD::EventSystem * fmodEventSystem;

    Vector<Sound *> soundsToReleaseOnUpdate;

	FastNameMap<SoundGroup*> soundGroups;
	Vector<VolumeAnimatedObject*> animatedObjects;

friend class SoundGroup;
friend class Sound;
};



};

#endif //__DAVAENGINE_SOUND_SYSTEM_H__
