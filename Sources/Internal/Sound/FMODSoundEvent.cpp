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

#include "Sound/FMODSoundEvent.h"
#include "Sound/SoundSystem.h"
#include "Scene3D/Entity.h"

#include "Concurrency/Thread.h"
#include "Utils/StringFormat.h"

namespace DAVA
{
    
static const FastName FMOD_SYSTEM_EVENTANGLE_PARAMETER("(event angle)");
    
FMODSoundEvent::FMODSoundEvent(const FastName & _eventName) :
    is3D(false)
{
    DVASSERT(_eventName.c_str()[0] != '/');
    eventName = _eventName;

    if (SoundSystem::Instance()->fmodEventSystem)
    {
        FMOD::Event * fmodEventInfo = nullptr;
        SoundSystem::Instance()->fmodEventSystem->getEvent(eventName.c_str(), FMOD_EVENT_INFOONLY, &fmodEventInfo);
        if (fmodEventInfo)
        {
            FMOD_MODE mode = 0;
            fmodEventInfo->getPropertyByIndex(FMOD_EVENTPROPERTY_MODE, &mode);
            is3D = (mode == FMOD_3D);

            InitParamsMap();

            isDirectional = IsParameterExists(FMOD_SYSTEM_EVENTANGLE_PARAMETER);
        }
    }
}

FMODSoundEvent::~FMODSoundEvent()
{
	DVASSERT(fmodEventInstances.size() == 0);

    SoundSystem::Instance()->RemoveSoundEventFromGroups(this);
}

bool FMODSoundEvent::Trigger()
{
    SoundSystem * soundSystem = SoundSystem::Instance();
    FMOD::EventSystem * fmodEventSystem = soundSystem->fmodEventSystem;
    
    if (fmodEventSystem == nullptr)
        return false;

    if(is3D)
    {
        FMOD::Event * fmodEventInfo = nullptr;
        FMOD_VERIFY(fmodEventSystem->getEvent(eventName.c_str(), FMOD_EVENT_INFOONLY, &fmodEventInfo));
        if(fmodEventInfo)
        {
            FMOD_VERIFY(fmodEventInfo->set3DAttributes((FMOD_VECTOR*)&position, 0, isDirectional ? (FMOD_VECTOR*)&direction : nullptr));
            FMOD_VERIFY(fmodEventInfo->setVolume(volume));
            ApplyParamsToEvent(fmodEventInfo);
        }
    }
    
    FMOD::Event * fmodEvent = nullptr;
    FMOD_RESULT result = fmodEventSystem->getEvent(eventName.c_str(), FMOD_EVENT_DEFAULT, &fmodEvent);

    if(result == FMOD_OK)
    {
        ApplyParamsToEvent(fmodEvent);

		FMOD_VERIFY(fmodEvent->setVolume(volume));
        FMOD_RESULT startResult = fmodEvent->start();

		if(startResult == FMOD_OK)
		{
			FMOD_VERIFY(fmodEvent->setCallback(FMODEventCallback, this));
			fmodEventInstances.push_back(fmodEvent);
			Retain();
            PerformEvent(EVENT_TRIGGERED);
            return true;
		}
		else if(startResult != FMOD_ERR_EVENT_FAILED) //'just fail' max playbacks behavior
		{
			Logger::Error("[FMODSoundEvent::Trigger()] Failed to start event by %d on eventID: %s", startResult, eventName.c_str());
		}
    }
    else if(result != FMOD_ERR_EVENT_FAILED) //'just fail' max playbacks behavior
    {
        Logger::Error("[FMODSoundEvent::Trigger()] Failed to retrieve event by %d on eventID: %s", result, eventName.c_str());
    }

    return false;
}

void FMODSoundEvent::SetPosition(const Vector3 & _position)
{
    position = _position;
}

void FMODSoundEvent::SetDirection(const Vector3 & _direction)
{
    direction = _direction;
}

void FMODSoundEvent::SetVelocity(const Vector3 & velocity)
{
    if(is3D && fmodEventInstances.size())
	{
		Vector<FMOD::Event *> instancesCopy(fmodEventInstances);
		size_t instancesCount = instancesCopy.size();
		for(size_t i = 0; i < instancesCount; ++i)
		{
            FMOD_VERIFY(instancesCopy[i]->set3DAttributes(0, (FMOD_VECTOR*)&velocity, 0));
		}
    }
}

void FMODSoundEvent::SetVolume(float32 _volume)
{
    if(volume != _volume)
    {
        volume = _volume;

		Vector<FMOD::Event *> instancesCopy(fmodEventInstances);
		size_t instancesCount = instancesCopy.size();
		for(size_t i = 0; i < instancesCount; ++i)
		{
            FMOD_VERIFY(instancesCopy[i]->setVolume(volume));
		}
    }
}
    
void FMODSoundEvent::UpdateInstancesPosition()
{
    if(is3D)
    {
		Vector<FMOD::Event *> instancesCopy(fmodEventInstances);
		size_t instancesCount = instancesCopy.size();
		for(size_t i = 0; i < instancesCount; ++i)
		{
            FMOD_VERIFY(instancesCopy[i]->set3DAttributes((FMOD_VECTOR*)&position, 0, isDirectional ? (FMOD_VECTOR*)&direction : NULL));
		}
    }
}
    
void FMODSoundEvent::Stop(bool force /* = false */)
{
    SoundSystem * soundSystem = SoundSystem::Instance();

	Vector<FMOD::Event *> instancesCopy(fmodEventInstances);
	size_t instancesCount = instancesCopy.size();
	for(size_t i = 0; i < instancesCount; ++i)
	{
        FMOD::Event * fEvent = instancesCopy[i];
		FMOD_VERIFY(fEvent->setCallback(0, 0));
        FMOD_VERIFY(fEvent->stop(force));

		PerformEvent(SoundEvent::EVENT_END);
		soundSystem->ReleaseOnUpdate(this);
	}
	fmodEventInstances.clear();
}

bool FMODSoundEvent::IsActive() const
{
    return fmodEventInstances.size() != 0;
}

void FMODSoundEvent::SetPaused(bool paused)
{
	size_t instancesCount = fmodEventInstances.size();
	for(size_t i = 0; i < instancesCount; ++i)
        FMOD_VERIFY(fmodEventInstances[i]->setPaused(paused));
}
    
void FMODSoundEvent::SetParameterValue(const FastName & paramName, float32 value)
{
    paramsValues[paramName] = value;

	Vector<FMOD::Event *> instancesCopy(fmodEventInstances);
	size_t instancesCount = instancesCopy.size();
	for(size_t i = 0; i < instancesCount; ++i)
	{
        FMOD::EventParameter * param = 0;
        FMOD_VERIFY(instancesCopy[i]->getParameter(paramName.c_str(), &param));
        if(param)
            FMOD_VERIFY(param->setValue(value));
    }
}

float32 FMODSoundEvent::GetParameterValue(const FastName & paramName)
{
    return paramsValues[paramName];
}

bool FMODSoundEvent::IsParameterExists(const FastName & paramName)
{
    return paramsValues.find(paramName) != paramsValues.end();
}

void FMODSoundEvent::ApplyParamsToEvent(FMOD::Event *event)
{
    FastNameMap<float32>::iterator it = paramsValues.begin();
    FastNameMap<float32>::iterator itEnd = paramsValues.end();
    for(;it != itEnd; ++it)
    {
        FMOD::EventParameter * param = 0;
        FMOD_VERIFY(event->getParameter(it->first.c_str(), &param));
        if(param)
        {
            FMOD_VERIFY(param->setValue(it->second));
        }
        else
        {
            Logger::Error("Event: %s, Param: %s", eventName.c_str(), it->first.c_str());
        }
    }
}

void FMODSoundEvent::InitParamsMap()
{
    Vector<SoundEvent::SoundEventParameterInfo> paramsInfo;
    GetEventParametersInfo(paramsInfo);
    for(int32 i = 0; i < (int32)paramsInfo.size(); ++i)
    {
        const SoundEvent::SoundEventParameterInfo & info = paramsInfo[i];
        paramsValues[FastName(info.name)] = info.minValue;
    }
}

void FMODSoundEvent::PerformCallback(FMOD::Event * fmodEvent)
{
    Vector<FMOD::Event *>::iterator it = std::find(fmodEventInstances.begin(), fmodEventInstances.end(), fmodEvent);
    if(it != fmodEventInstances.end())
    {
		PerformEvent(SoundEvent::EVENT_END);
		fmodEventInstances.erase(it);
		SoundSystem::Instance()->ReleaseOnUpdate(this);
    }
}

void FMODSoundEvent::GetEventParametersInfo(Vector<SoundEventParameterInfo> & paramsInfo) const
{
    paramsInfo.clear();

    FMOD::Event * event = nullptr;
    if(fmodEventInstances.size())
    {
        event = fmodEventInstances[0];
    }
    else
    {
        FMOD::EventSystem * fmodEventSystem = SoundSystem::Instance()->fmodEventSystem;
        if (fmodEventSystem)
        {
            FMOD_VERIFY(fmodEventSystem->getEvent(eventName.c_str(), FMOD_EVENT_INFOONLY, &event));
        }
    }

    if(!event)
        return;

    int32 paramsCount = 0;
    FMOD_VERIFY(event->getNumParameters(&paramsCount));
    paramsInfo.reserve(paramsCount);
    for(int32 i = 0; i < paramsCount; i++)
    {
        FMOD::EventParameter * param = 0;
        FMOD_VERIFY(event->getParameterByIndex(i, &param));
        if(!param)
            continue;

        char * paramName = 0;
        FMOD_VERIFY(param->getInfo(0, &paramName));

        SoundEventParameterInfo pInfo;
        pInfo.name = String(paramName);
        FMOD_VERIFY(param->getRange(&pInfo.minValue, &pInfo.maxValue));

        paramsInfo.push_back(pInfo);
    }
}

String FMODSoundEvent::GetEventName() const
{
     return String(eventName.c_str());
}

float32 FMODSoundEvent::GetMaxDistance() const
{
    float32 distance = 0;
    FMOD::EventSystem * fmodEventSystem = SoundSystem::Instance()->fmodEventSystem;
    FMOD::Event * fmodEventInfo = nullptr;

    if (fmodEventSystem)
    {
        FMOD_VERIFY(fmodEventSystem->getEvent(eventName.c_str(), FMOD_EVENT_INFOONLY, &fmodEventInfo));
    }
    if(fmodEventInfo)
    {
        FMOD_VERIFY(fmodEventInfo->getPropertyByIndex(FMOD_EVENTPROPERTY_3D_MAXDISTANCE, &distance));
    }

    return distance;
}

FMOD_RESULT F_CALLBACK FMODSoundEvent::FMODEventCallback(FMOD_EVENT *event, FMOD_EVENT_CALLBACKTYPE type, void *param1, void *param2, void *userdata)
{
    if(type == FMOD_EVENT_CALLBACKTYPE_STOLEN || type == FMOD_EVENT_CALLBACKTYPE_EVENTFINISHED)
	{
		DVASSERT_MSG(Thread::IsMainThread(), DAVA::Format("FMOD Callback type %d", type).c_str());

        FMOD::Event * fEvent = (FMOD::Event *)event;
        FMODSoundEvent * sEvent = (FMODSoundEvent *)userdata;
        if(sEvent && fEvent)
		{
			FMOD_VERIFY(fEvent->setCallback(0, 0));
			sEvent->PerformCallback(fEvent);
		}
    }
    return FMOD_OK;
}
    
};

#endif //DAVA_FMOD