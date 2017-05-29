
#include "FMODSoundEvent.h"
#include "FMODSoundSystem.h"
#include "Scene3D/Entity.h"

#include "Concurrency/Thread.h"
#include "Utils/StringFormat.h"

namespace DAVA
{
static const FastName FMOD_SYSTEM_EVENTANGLE_PARAMETER("(event angle)");

static float32 SpeedToPitchInOctaves(const float32 speed)
{
    // For using with FMOD::Event::setPitch & FMOD_EVENT_PITCHUNITS_OCTAVES
    // 0.0f -> default frequency
    // +1 octave -> x2 frequency
    // -1 octave -> x1/2 frequency

    return std::log2f(speed);
}

FMODSoundEvent::FMODSoundEvent(const FastName& _eventName, FMODSoundSystem* rootSoundSystem)
    : is3D(false)
    , soundSystem(rootSoundSystem)
{
    DVASSERT(_eventName.c_str()[0] != '/');
    eventName = _eventName;

    if (soundSystem->fmodEventSystem)
    {
        FMOD::Event* fmodEventInfo = nullptr;
        soundSystem->fmodEventSystem->getEvent(eventName.c_str(), FMOD_EVENT_INFOONLY, &fmodEventInfo);
        if (fmodEventInfo)
        {
            FMOD_MODE mode = 0;
            fmodEventInfo->getPropertyByIndex(FMOD_EVENTPROPERTY_MODE, &mode);
            if (mode == FMOD_3D)
            {
                is3D = true;
            }

            InitParamsMap();

            isDirectional = IsParameterExists(FMOD_SYSTEM_EVENTANGLE_PARAMETER);
        }
    }
}

FMODSoundEvent::~FMODSoundEvent()
{
    DVASSERT(fmodEventInstances.size() == 0);

    soundSystem->RemoveSoundEventFromGroups(this);
}

bool FMODSoundEvent::Trigger()
{
    FMOD::EventSystem* fmodEventSystem = soundSystem->fmodEventSystem;

    if (fmodEventSystem == nullptr)
        return false;

    if (is3D && !fmodEventInstances.empty())
    {
        FMOD::Event* fmodEventInfo = nullptr;
        FMOD_VERIFY(fmodEventSystem->getEvent(eventName.c_str(), FMOD_EVENT_INFOONLY, &fmodEventInfo));
        if (fmodEventInfo)
        {
            // http://stackoverflow.com/questions/570669/checking-if-a-double-or-float-is-nan-in-c

            FMOD_VERIFY(fmodEventInfo->setVolume(volume));
            FMOD_VERIFY(fmodEventInfo->set3DAttributes(reinterpret_cast<FMOD_VECTOR*>(&position), 0, isDirectional ? reinterpret_cast<FMOD_VECTOR*>(&direction) : nullptr));

            ApplyParamsToEvent(fmodEventInfo);
        }
    }

    FMOD::Event* fmodEvent = nullptr;
    FMOD_RESULT result = fmodEventSystem->getEvent(eventName.c_str(), FMOD_EVENT_NONBLOCKING, &fmodEvent);

    if (result == FMOD_OK)
    {
        ApplyParamsToEvent(fmodEvent);

        FMOD_VERIFY(fmodEvent->setVolume(volume));

        const float pitch = SpeedToPitchInOctaves(speed);
        FMOD_VERIFY(fmodEvent->setPitch(pitch, FMOD_EVENT_PITCHUNITS_OCTAVES));

        FMOD_VERIFY(fmodEvent->set3DAttributes(reinterpret_cast<FMOD_VECTOR*>(&position), 0, isDirectional ? reinterpret_cast<FMOD_VECTOR*>(&direction) : nullptr));

        FMOD_RESULT startResult = fmodEvent->start();

        if (startResult == FMOD_OK)
        {
            FMOD_VERIFY(fmodEvent->setCallback(FMODEventCallback, this));
            fmodEventInstances.push_back(fmodEvent);
            Retain();
            PerformEvent(EVENT_TRIGGERED);
            return true;
        }
        else if (startResult != FMOD_ERR_EVENT_FAILED) //'just fail' max playbacks behavior
        {
            Logger::Error("[FMODSoundEvent::Trigger()] Failed to start event by %d on eventID: %s", startResult, eventName.c_str());
        }
    }
    else if (result != FMOD_ERR_EVENT_FAILED) //'just fail' max playbacks behavior
    {
        Logger::Error("[FMODSoundEvent::Trigger()] Failed to retrieve event by %d on eventID: %s", result, eventName.c_str());
    }

    return false;
}

void FMODSoundEvent::SetPosition(const Vector3& _position)
{
    const bool isFinite = std::isfinite(_position.x) && std::isfinite(_position.y) && std::isfinite(_position.z);
    DVASSERT(isFinite);

    if (isFinite)
    {
        position = _position;
    }
    else
    {
        Logger::Error("[FMODSoundEvent::SetPosition] Invalid vector was given: (%f, %f, %f), ignoring", _position.x, _position.y, _position.z);
    }
}

void FMODSoundEvent::SetDirection(const Vector3& _direction)
{
    const bool isFinite = std::isfinite(_direction.x) && std::isfinite(_direction.y) && std::isfinite(_direction.z);
    const bool nonZero = _direction.SquareLength() > 0.0f;
    DVASSERT(isFinite);
    DVASSERT(nonZero);

    if (isFinite && nonZero)
    {
        direction = Normalize(_direction);
    }
    else
    {
        Logger::Error("[FMODSoundEvent::SetDirection] Invalid vector was given: (%f, %f, %f), ignoring", _direction.x, _direction.y, _direction.z);
    }
}

void FMODSoundEvent::SetVelocity(const Vector3& velocity)
{
    if (is3D && fmodEventInstances.size())
    {
        const bool isFinite = std::isfinite(velocity.x) && std::isfinite(velocity.y) && std::isfinite(velocity.z);
        DVASSERT(isFinite);

        if (isFinite)
        {
            Vector<FMOD::Event*> instancesCopy(fmodEventInstances);
            size_t instancesCount = instancesCopy.size();
            for (size_t i = 0; i < instancesCount; ++i)
            {
                FMOD_VERIFY(instancesCopy[i]->set3DAttributes(0, reinterpret_cast<const FMOD_VECTOR*>(&velocity), 0));
            }
        }
        else
        {
            Logger::Error("[FMODSoundEvent::SetVelocity] Invalid vector was given: (%f, %f, %f), ignoring", velocity.x, velocity.y, velocity.z);
        }
    }
}

void FMODSoundEvent::SetVolume(float32 _volume)
{
    if (volume != _volume)
    {
        volume = _volume;

        Vector<FMOD::Event*> instancesCopy(fmodEventInstances);
        size_t instancesCount = instancesCopy.size();
        for (size_t i = 0; i < instancesCount; ++i)
        {
            FMOD_VERIFY(instancesCopy[i]->setVolume(volume));
        }
    }
}

void FMODSoundEvent::SetSpeed(float32 _speed)
{
    if (speed != _speed)
    {
        speed = _speed;

        Vector<FMOD::Event*> instancesCopy(fmodEventInstances);
        size_t instancesCount = instancesCopy.size();
        for (size_t i = 0; i < instancesCount; ++i)
        {
            float pitch = SpeedToPitchInOctaves(speed);
            FMOD_VERIFY(instancesCopy[i]->setPitch(pitch, FMOD_EVENT_PITCHUNITS_OCTAVES));
        }
    }
}

void FMODSoundEvent::UpdateInstancesPosition()
{
    if (is3D)
    {
        Vector<FMOD::Event*> instancesCopy(fmodEventInstances);
        size_t instancesCount = instancesCopy.size();
        for (size_t i = 0; i < instancesCount; ++i)
        {
            FMOD_VERIFY(instancesCopy[i]->set3DAttributes(reinterpret_cast<FMOD_VECTOR*>(&position), 0, isDirectional ? reinterpret_cast<FMOD_VECTOR*>(&direction) : nullptr));
        }
    }
}

void FMODSoundEvent::Stop(bool force /* = false */)
{
    Vector<FMOD::Event*> instancesCopy(fmodEventInstances);
    size_t instancesCount = instancesCopy.size();
    for (size_t i = 0; i < instancesCount; ++i)
    {
        FMOD::Event* fEvent = instancesCopy[i];
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
    for (size_t i = 0; i < instancesCount; ++i)
        FMOD_VERIFY(fmodEventInstances[i]->setPaused(paused));
}

void FMODSoundEvent::SetParameterValue(const FastName& paramName, float32 value)
{
    paramsValues[paramName] = value;

    Vector<FMOD::Event*> instancesCopy(fmodEventInstances);
    size_t instancesCount = instancesCopy.size();
    for (size_t i = 0; i < instancesCount; ++i)
    {
        FMOD::EventParameter* param = 0;
        FMOD_VERIFY(instancesCopy[i]->getParameter(paramName.c_str(), &param));
        if (param)
            FMOD_VERIFY(param->setValue(value));
    }
}

float32 FMODSoundEvent::GetParameterValue(const FastName& paramName)
{
    return paramsValues[paramName];
}

bool FMODSoundEvent::IsParameterExists(const FastName& paramName)
{
    return paramsValues.find(paramName) != paramsValues.end();
}

void FMODSoundEvent::ApplyParamsToEvent(FMOD::Event* event)
{
    FastNameMap<float32>::iterator it = paramsValues.begin();
    FastNameMap<float32>::iterator itEnd = paramsValues.end();
    for (; it != itEnd; ++it)
    {
        FMOD::EventParameter* param = 0;
        FMOD_VERIFY(event->getParameter(it->first.c_str(), &param));
        if (param)
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
    for (const SoundEvent::SoundEventParameterInfo& info : paramsInfo)
    {
        paramsValues[FastName(info.name)] = info.minValue;
    }
}

void FMODSoundEvent::PerformCallback(FMOD::Event* fmodEvent)
{
    Vector<FMOD::Event*>::iterator it = std::find(fmodEventInstances.begin(), fmodEventInstances.end(), fmodEvent);
    if (it != fmodEventInstances.end())
    {
        PerformEvent(SoundEvent::EVENT_END);
        fmodEventInstances.erase(it);
        soundSystem->ReleaseOnUpdate(this);
    }
}

void FMODSoundEvent::GetEventParametersInfo(Vector<SoundEventParameterInfo>& paramsInfo) const
{
    paramsInfo.clear();

    FMOD::Event* event = nullptr;
    if (fmodEventInstances.size())
    {
        event = fmodEventInstances[0];
    }
    else
    {
        FMOD::EventSystem* fmodEventSystem = soundSystem->fmodEventSystem;
        if (fmodEventSystem)
        {
            FMOD_VERIFY(fmodEventSystem->getEvent(eventName.c_str(), FMOD_EVENT_INFOONLY, &event));
        }
    }

    if (!event)
        return;

    int32 paramsCount = 0;
    FMOD_VERIFY(event->getNumParameters(&paramsCount));
    paramsInfo.reserve(paramsCount);
    for (int32 i = 0; i < paramsCount; i++)
    {
        FMOD::EventParameter* param = 0;
        FMOD_VERIFY(event->getParameterByIndex(i, &param));
        if (!param)
            continue;

        char* paramName = 0;
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
    FMOD::EventSystem* fmodEventSystem = soundSystem->fmodEventSystem;
    FMOD::Event* fmodEventInfo = nullptr;

    if (fmodEventSystem)
    {
        FMOD_VERIFY(fmodEventSystem->getEvent(eventName.c_str(), FMOD_EVENT_INFOONLY, &fmodEventInfo));
    }
    if (fmodEventInfo)
    {
        FMOD_VERIFY(fmodEventInfo->getPropertyByIndex(FMOD_EVENTPROPERTY_3D_MAXDISTANCE, &distance));
    }

    return distance;
}

FMOD_RESULT F_CALLBACK FMODSoundEvent::FMODEventCallback(FMOD_EVENT* event, FMOD_EVENT_CALLBACKTYPE type, void* param1, void* param2, void* userdata)
{
    if (type == FMOD_EVENT_CALLBACKTYPE_STOLEN || type == FMOD_EVENT_CALLBACKTYPE_EVENTFINISHED)
    {
        DVASSERT(Thread::IsMainThread(), DAVA::Format("FMOD Callback type %d", type).c_str());

        FMOD::Event* fEvent = reinterpret_cast<FMOD::Event*>(event);
        FMODSoundEvent* sEvent = reinterpret_cast<FMODSoundEvent*>(userdata);
        if (sEvent && fEvent)
        {
            FMOD_VERIFY(fEvent->setCallback(0, 0));
            sEvent->PerformCallback(fEvent);
        }
    }
    return FMOD_OK;
}
};
