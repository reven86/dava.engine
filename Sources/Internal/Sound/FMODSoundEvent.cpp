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

namespace DAVA
{
    
static const FastName FMOD_SYSTEM_EVENTANGLE_PARAMETER("(event angle)");
    
FMODSoundEvent::FMODSoundEvent(const FastName & _eventName) :
    is3D(false)
{
    DVASSERT(_eventName.c_str()[0] != '/');
    eventName = _eventName;

    FMOD::Event * fmodEventInfo = 0;
    FMOD_VERIFY(SoundSystem::Instance()->fmodEventSystem->getEvent(eventName.c_str(), FMOD_EVENT_INFOONLY, &fmodEventInfo));
    if(fmodEventInfo)
    {
        FMOD_MODE mode = 0;
        fmodEventInfo->getPropertyByIndex(FMOD_EVENTPROPERTY_MODE, &mode);
        is3D = (mode == FMOD_3D);

        InitParamsMap();

        isDirectional = IsParameterExists(FMOD_SYSTEM_EVENTANGLE_PARAMETER);
    }
    else
    {
        Logger::Error(eventName.c_str());
    }

}

FMODSoundEvent::~FMODSoundEvent()
{
    List<FMOD::Event *>::const_iterator itEnd = fmodEventInstances.end();
    for(List<FMOD::Event *>::const_iterator it = fmodEventInstances.begin(); it != itEnd; ++it)
    {
        FMOD::Event * fmodEvent = (*it);
        
        FMOD_VERIFY(fmodEvent->setCallback(0, 0));
        FMOD_VERIFY(fmodEvent->stop());

        SoundSystem::Instance()->CancelCallbackOnUpdate(this, SoundEvent::EVENT_END);
    }

    fmodEventInstances.clear();

    SoundSystem::Instance()->RemoveSoundEventFromGroups(this);
}

bool FMODSoundEvent::Trigger()
{
    SoundSystem * soundSystem = SoundSystem::Instance();
    FMOD::EventSystem * fmodEventSystem = soundSystem->fmodEventSystem;

    List<FMOD::Event *>::const_iterator itEnd = fmodEventInstances.end();
    for(List<FMOD::Event *>::const_iterator it = fmodEventInstances.begin(); it != itEnd; ++it)
        (*it)->setPaused(false);
    
    if(is3D)
    {
        FMOD::Event * fmodEventInfo = 0;
        FMOD_VERIFY(fmodEventSystem->getEvent(eventName.c_str(), FMOD_EVENT_INFOONLY, &fmodEventInfo));
        if(fmodEventInfo)
        {
            FMOD_VERIFY(fmodEventInfo->set3DAttributes((FMOD_VECTOR*)&position, 0, isDirectional ? (FMOD_VECTOR*)&direction : NULL));
            FMOD_VERIFY(fmodEventInfo->setVolume(volume));
            ApplyParamsToEvent(fmodEventInfo);
        }
    }
    
    FMOD::Event * fmodEvent = 0;
    FMOD_RESULT result = fmodEventSystem->getEvent(eventName.c_str(), FMOD_EVENT_DEFAULT, &fmodEvent);
    if(result == FMOD_OK)
    {
        ApplyParamsToEvent(fmodEvent);
        fmodEventInstances.push_back(fmodEvent);

        FMOD_VERIFY(fmodEvent->setVolume(volume));
        FMOD_VERIFY(fmodEvent->setCallback(FMODEventCallback, this));
        FMOD_VERIFY(fmodEvent->start());

        Retain();
    }
    else if(result != FMOD_ERR_EVENT_FAILED) //'just fail' max playbacks behavior
    {
        Logger::Error("[FMODSoundEvent::Trigger()] Failed by %d on eventID: %s", result, eventName.c_str());
    }
    
    return fmodEvent != 0;
}

void FMODSoundEvent::SetPosition(const Vector3 & _position)
{
    position = _position;
}

void FMODSoundEvent::SetDirection(const Vector3 & _direction)
{
    direction = _direction;
}

void FMODSoundEvent::SetVolume(float32 _volume)
{
    if(volume != _volume)
    {
        volume = _volume;

        List<FMOD::Event *>::const_iterator itEnd = fmodEventInstances.end();
        for(List<FMOD::Event *>::const_iterator it = fmodEventInstances.begin(); it != itEnd; ++it)
            FMOD_VERIFY((*it)->setVolume(volume));
    }
}
    
void FMODSoundEvent::UpdateInstancesPosition()
{
    if(is3D)
    {
        SoundSystem * soundSystem = SoundSystem::Instance();

        List<FMOD::Event *>::const_iterator itEnd = fmodEventInstances.end();
        for(List<FMOD::Event *>::const_iterator it = fmodEventInstances.begin(); it != itEnd; ++it)
            FMOD_VERIFY((*it)->set3DAttributes((FMOD_VECTOR*)&position, 0, isDirectional ? (FMOD_VECTOR*)&direction : NULL));
    }
}
    
void FMODSoundEvent::Stop()
{
    SoundSystem * soundSystem = SoundSystem::Instance();

    List<FMOD::Event *>::const_iterator itEnd = fmodEventInstances.end();
    for(List<FMOD::Event *>::const_iterator it = fmodEventInstances.begin(); it != itEnd; ++it)
    {
        FMOD::Event * fEvent = (*it);
        FMOD_VERIFY(fEvent->setCallback(0, 0));
        FMOD_VERIFY(fEvent->stop());
        soundSystem->ReleaseOnUpdate(this);
        PerformEvent(SoundEvent::EVENT_END);
    }
    fmodEventInstances.clear();
}

bool FMODSoundEvent::IsActive() const
{
    return fmodEventInstances.size() != 0;
}

void FMODSoundEvent::Pause()
{
    List<FMOD::Event *>::const_iterator itEnd = fmodEventInstances.end();
    for(List<FMOD::Event *>::const_iterator it = fmodEventInstances.begin(); it != itEnd; ++it)
        (*it)->setPaused(true);
}
    
void FMODSoundEvent::SetParameterValue(const FastName & paramName, float32 value)
{
    paramsValues[paramName] = value;
    
    List<FMOD::Event *>::const_iterator itEnd = fmodEventInstances.end();
    for(List<FMOD::Event *>::const_iterator it = fmodEventInstances.begin(); it != itEnd; ++it)
    {
        FMOD::EventParameter * param = 0;
        FMOD_VERIFY((*it)->getParameter(paramName.c_str(), &param));
        if(param)
        {
            FMOD_VERIFY(param->setValue(value));
        }
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
            FMOD_VERIFY(param->setValue(it->second));
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

void FMODSoundEvent::PerformCallback(FMOD::Event * fmodEvent, eSoundEventCallbackType callbackType)
{
    List<FMOD::Event *>::iterator it = std::find(fmodEventInstances.begin(), fmodEventInstances.end(), fmodEvent);
    if(it != fmodEventInstances.end())
    {
        SoundSystem * soundSystem = SoundSystem::Instance();
        soundSystem->ReleaseOnUpdate(this);
        soundSystem->PerformCallbackOnUpdate(this, callbackType);

        (*it)->setCallback(0, 0);
        fmodEventInstances.erase(it);
    }
}

void FMODSoundEvent::GetEventParametersInfo(Vector<SoundEventParameterInfo> & paramsInfo) const
{
    paramsInfo.clear();

    FMOD::EventSystem * fmodEventSystem = SoundSystem::Instance()->fmodEventSystem;
    FMOD::Event * event = 0;
    fmodEventSystem->getEvent(eventName.c_str(), FMOD_EVENT_INFOONLY, &event);
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

FMOD_RESULT F_CALLBACK FMODSoundEvent::FMODEventCallback(FMOD_EVENT *event, FMOD_EVENT_CALLBACKTYPE type, void *param1, void *param2, void *userdata)
{
    if(type == FMOD_EVENT_CALLBACKTYPE_STOLEN || type == FMOD_EVENT_CALLBACKTYPE_EVENTFINISHED)
    {
        FMOD::Event * fEvent = (FMOD::Event *)event;
        FMODSoundEvent * sEvent = (FMODSoundEvent *)userdata;
        if(sEvent && fEvent)
            sEvent->PerformCallback(fEvent, SoundEvent::EVENT_END);
    }
    return FMOD_OK;
}
    
};

#endif //DAVA_FMOD