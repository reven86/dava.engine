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



#ifndef __DAVAENGINE_SOUND_EVENT_H__
#define __DAVAENGINE_SOUND_EVENT_H__

#include "Base/BaseTypes.h"
#include "Base/BaseMath.h"
#include "Base/EventDispatcher.h"
#include "Sound/VolumeAnimatedObject.h"

namespace FMOD
{
	class Event;
};

namespace DAVA
{
class SoundComponent;
class SoundEvent : public VolumeAnimatedObject
{
public:
	enum eEvent
	{
		EVENT_STARTED = 0,	//Called when an event is started. FMOD_EVENT_CALLBACKTYPE_EVENTSTARTED
		EVENT_FINISHED,		//Called when an event is stopped for any reason. FMOD_EVENT_CALLBACKTYPE_EVENTFINISHED
		EVENT_SYNCPOINT,	//Called when a syncpoint is encountered. Can be from wav file markers. FMOD_EVENT_CALLBACKTYPE_SYNCPOINT

		EVENT_COUNT
	};

	void SetVolume(float32 volume);
	float32	GetVolume();

	void Play();
	void Pause(bool isPaused);
	bool IsPaused();
	void Stop();
	void PerformCallback(eEvent eventType);

	void SetPosition(const Vector3 & position);

private:
	SoundEvent(FMOD::Event * fmodEvent);
	~SoundEvent();

	FMOD::Event * fmodEvent;

	IMPLEMENT_EVENT_DISPATCHER(eventDispatcher);

friend class SoundSystem;
friend class SoundComponent;
};

};

#endif