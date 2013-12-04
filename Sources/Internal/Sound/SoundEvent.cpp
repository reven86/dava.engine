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



#include "Sound/SoundEvent.h"
#include "Sound/SoundSystem.h"
#include "Sound/FMODUtils.h"

namespace DAVA
{
FMOD_RESULT F_CALLBACK SoundEventCallback(FMOD_EVENT *  event, FMOD_EVENT_CALLBACKTYPE  type, void *  param1, void *  param2, void *  userdata);

SoundEvent::SoundEvent(FMOD::Event * _fmodEvent) :
	fmodEvent(_fmodEvent),
	eventDispatcher( new EventDispatcher() )
{
	FMOD_VERIFY(fmodEvent->setCallback(SoundEventCallback, this));
}

SoundEvent::~SoundEvent()
{
	FMOD::EventGroup * parent = 0;
	FMOD_VERIFY(fmodEvent->getParentGroup(&parent));
	if(parent)
	{
		FMOD_VERIFY(parent->freeEventData(fmodEvent));
	}
}

void SoundEvent::Play()
{
	FMOD_VERIFY(fmodEvent->start());
}

void SoundEvent::Stop()
{
	FMOD_VERIFY(fmodEvent->stop());
}

void SoundEvent::SetPosition(const Vector3 & position)
{
	FMOD_VECTOR pos = {position.x, position.y, position.z};
	FMOD_VERIFY(fmodEvent->set3DAttributes(&pos, 0));
}

void SoundEvent::SetVolume(float32 volume)
{
	FMOD_VERIFY(fmodEvent->setVolume(volume));
}

float32	SoundEvent::GetVolume()
{
	float32 volume = 0;
	FMOD_VERIFY(fmodEvent->getVolume(&volume));
	return volume;
}

void SoundEvent::Pause(bool isPaused)
{
	FMOD_VERIFY(fmodEvent->setPaused(isPaused));
}

bool SoundEvent::IsPaused()
{
	bool isPaused = false;
	FMOD_VERIFY(fmodEvent->getPaused(&isPaused));
	return isPaused;
}

void SoundEvent::PerformCallback(eEvent eventType)
{
	eventDispatcher->PerformEvent(eventType, this);
}

FMOD_RESULT F_CALLBACK SoundEventCallback(FMOD_EVENT * event, FMOD_EVENT_CALLBACKTYPE  type, void * param1, void * param2, void * userdata)
{
	SoundEvent * soundEvent = (SoundEvent *)userdata;
	switch(type)
	{
	case FMOD_EVENT_CALLBACKTYPE_EVENTSTARTED:
		soundEvent->PerformCallback(SoundEvent::EVENT_STARTED);
		break;
	case FMOD_EVENT_CALLBACKTYPE_EVENTFINISHED:
		soundEvent->PerformCallback(SoundEvent::EVENT_FINISHED);
		break;
	case FMOD_EVENT_CALLBACKTYPE_SYNCPOINT:
		soundEvent->PerformCallback(SoundEvent::EVENT_SYNCPOINT);
		break;
	case FMOD_EVENT_CALLBACKTYPE_SOUNDDEF_SELECTINDEX:
		break;
	case FMOD_EVENT_CALLBACKTYPE_SOUNDDEF_CREATE:
		break;
	case FMOD_EVENT_CALLBACKTYPE_SOUNDDEF_RELEASE:
		break;
	default:
		break;
	}

	return FMOD_OK;
}

};
