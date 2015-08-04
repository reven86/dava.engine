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

#ifndef __DAVAENGINE_FMOD_FILE_SOUND_EVENT_H__
#define __DAVAENGINE_FMOD_FILE_SOUND_EVENT_H__

#include "Base/BaseTypes.h"
#include "Base/BaseObject.h"
#include "Base/BaseMath.h"
#include "FileSystem/FilePath.h"
#include "Base/EventDispatcher.h"
#include "Sound/SoundEvent.h"
#include "Concurrency/Mutex.h"
#include "Sound/FMODUtils.h"

namespace FMOD
{
    class Sound;
    class ChannelGroup;
    class Channel;
};

namespace DAVA
{

class FMODFileSoundEvent : public SoundEvent
{
public:
    static FMOD_RESULT F_CALLBACK SoundInstanceEndPlaying(FMOD_CHANNEL *channel, FMOD_CHANNEL_CALLBACKTYPE type, void *commanddata1, void *commanddata2);

	virtual int32 Release();

	virtual void SetVolume(float32 volume);

    virtual bool IsActive() const;
    virtual bool Trigger();
    virtual void Stop(bool force = false);
    virtual void SetPaused(bool paused);
    
	virtual void SetPosition(const Vector3 & position);
    virtual void SetDirection(const Vector3 & direction) {};
    virtual void UpdateInstancesPosition();
    virtual void SetVelocity(const Vector3 & velocity) {};

	virtual void SetLoopCount(int32 looping); // -1 = infinity
	virtual int32 GetLoopCount();
    
    virtual void SetParameterValue(const FastName & paramName, float32 value) {};
    virtual float32 GetParameterValue(const FastName & paramName) { return 0.f; };
    virtual bool IsParameterExists(const FastName & paramName) {return false; };

    virtual void GetEventParametersInfo(Vector<SoundEventParameterInfo> & paramsInfo) const { return; };

    virtual String GetEventName() const { return fileName.GetFrameworkPath(); };
    virtual float32 GetMaxDistance() const { return -1.f; };

protected:
	FMODFileSoundEvent(const FilePath & fileName, uint32 flags, int32 priority);
	virtual ~FMODFileSoundEvent();

	static FMODFileSoundEvent * CreateWithFlags(const FilePath & fileName, uint32 flags, int32 priority = 128);

    static Mutex soundMapMutex;

	Vector3 position;

	FilePath fileName;
    int32 priority;
    uint32 flags;

	FMOD::Sound * fmodSound;
	FMOD::ChannelGroup * fmodInstanceGroup;

friend class SoundSystem;
};

};

#endif //__DAVAENGINE_FMOD_FILE_SOUND_EVENT_H__

#endif //DAVA_FMOD