
#include "FMODFileSoundEvent.h"
#include "FMODSoundSystem.h"
#include "Scene3D/Entity.h"

namespace DAVA
{
Map<FilePath, FMOD::Sound*> soundMap;
Map<FMOD::Sound*, int32> soundRefsMap;

Mutex FMODFileSoundEvent::soundMapMutex;

FMODFileSoundEvent* FMODFileSoundEvent::CreateWithFlags(const FilePath& fileName, uint32 flags, int32 priority, FMODSoundSystem* rootSoundSystem)
{
    FMODFileSoundEvent* sound = new FMODFileSoundEvent(fileName, flags, priority, rootSoundSystem);

    FMOD_MODE fmodMode = FMOD_DEFAULT;

    if (flags & SOUND_EVENT_CREATE_3D)
        fmodMode |= FMOD_3D;

    if (flags & SOUND_EVENT_CREATE_LOOP)
        fmodMode |= FMOD_LOOP_NORMAL;

    if (flags & SOUND_EVENT_CREATE_STREAM)
        fmodMode |= FMOD_CREATESTREAM;

    soundMapMutex.Lock();
    Map<FilePath, FMOD::Sound*>::iterator it;
    it = soundMap.find(fileName);
    if (it != soundMap.end())
    {
        sound->fmodSound = it->second;
        soundRefsMap[sound->fmodSound]++;
    }
    else
    {
        if (rootSoundSystem->fmodSystem)
        {
            FMOD_VERIFY(rootSoundSystem->fmodSystem->createSound(fileName.GetStringValue().c_str(), fmodMode, 0, &sound->fmodSound));
        }

        if (!sound->fmodSound)
        {
            soundMapMutex.Unlock();
            SafeRelease(sound);
            return nullptr;
        }

        if (flags & SOUND_EVENT_CREATE_LOOP)
            sound->SetLoopCount(-1);

        soundMap[sound->fileName] = sound->fmodSound;
        soundRefsMap[sound->fmodSound] = 1;
    }
    soundMapMutex.Unlock();

    if (rootSoundSystem->fmodSystem)
    {
        FMOD_VERIFY(rootSoundSystem->fmodSystem->createChannelGroup(0, &sound->fmodInstanceGroup));
        FMOD_VERIFY(rootSoundSystem->masterChannelGroup->addGroup(sound->fmodInstanceGroup));
    }

    return sound;
}

FMODFileSoundEvent::FMODFileSoundEvent(const FilePath& _fileName, uint32 _flags, int32 _priority, FMODSoundSystem* rootSoundSystem)
    : fileName(_fileName)
    , priority(_priority)
    , flags(_flags)
    , fmodSound(0)
    , fmodInstanceGroup(0)
    , soundSystem(rootSoundSystem)
{
}

FMODFileSoundEvent::~FMODFileSoundEvent()
{
    if (fmodInstanceGroup)
        FMOD_VERIFY(fmodInstanceGroup->release());

    soundSystem->RemoveSoundEventFromGroups(this);
}

int32 FMODFileSoundEvent::Release()
{
    if (GetRetainCount() == 1)
    {
        soundMapMutex.Lock();
        soundRefsMap[fmodSound]--;
        if (soundRefsMap[fmodSound] == 0)
        {
            soundMap.erase(fileName);
            soundRefsMap.erase(fmodSound);
            FMOD_VERIFY(fmodSound->release());
        }
        soundMapMutex.Unlock();
    }

    return BaseObject::Release();
}

bool FMODFileSoundEvent::Trigger()
{
    if (nullptr == soundSystem->fmodSystem)
        return false;

    FMOD::Channel* fmodInstance = nullptr;
    FMOD_VERIFY(soundSystem->fmodSystem->playSound(FMOD_CHANNEL_FREE, fmodSound, true, &fmodInstance)); //start sound paused
    if (fmodInstance && fmodInstanceGroup)
    {
        FMOD_VERIFY(fmodInstance->setPriority(priority));
        FMOD_VERIFY(fmodInstance->setCallback(SoundInstanceEndPlaying));
        FMOD_VERIFY(fmodInstance->setUserData(this));
        FMOD_VERIFY(fmodInstance->setChannelGroup(fmodInstanceGroup));

        if (flags & SOUND_EVENT_CREATE_3D)
            FMOD_VERIFY(fmodInstance->set3DAttributes(reinterpret_cast<FMOD_VECTOR*>(&position), 0));

        FMOD_VERIFY(fmodInstanceGroup->setPaused(false));
        FMOD_VERIFY(fmodInstance->setPaused(false));

        Retain();

        PerformEvent(EVENT_TRIGGERED);

        return true;
    }

    return false;
}

void FMODFileSoundEvent::SetPosition(const Vector3& _position)
{
    const bool isFinite = std::isfinite(_position.x) && std::isfinite(_position.y) && std::isfinite(_position.z);
    DVASSERT(isFinite);

    if (isFinite)
    {
        position = _position;
    }
    else
    {
        Logger::Error("[FMODFileSoundEvent::SetPosition] Invalid vector was given: (%f, %f, %f), ignoring", _position.x, _position.y, _position.z);
    }
}

void FMODFileSoundEvent::UpdateInstancesPosition()
{
    if (flags & SOUND_EVENT_CREATE_3D && fmodInstanceGroup)
    {
        int32 instancesCount = 0;
        FMOD_VERIFY(fmodInstanceGroup->getNumChannels(&instancesCount));
        for (int32 i = 0; i < instancesCount; i++)
        {
            FMOD::Channel* inst = 0;
            FMOD_VERIFY(fmodInstanceGroup->getChannel(i, &inst));
            FMOD_VERIFY(inst->set3DAttributes(reinterpret_cast<FMOD_VECTOR*>(&position), 0));
        }
    }
}

void FMODFileSoundEvent::SetVolume(float32 _volume)
{
    volume = _volume;
    if (fmodInstanceGroup)
    {
        FMOD_VERIFY(fmodInstanceGroup->setVolume(volume));
    }
}

void FMODFileSoundEvent::SetSpeed(float32 _speed)
{
    speed = _speed;
    if (fmodInstanceGroup != nullptr)
    {
        FMOD_VERIFY(fmodInstanceGroup->setPitch(speed));
    }
}

bool FMODFileSoundEvent::IsActive() const
{
    if (fmodInstanceGroup)
    {
        int32 numChanels = 0;
        FMOD_VERIFY(fmodInstanceGroup->getNumChannels(&numChanels));

        bool isPaused = false;
        FMOD_VERIFY(fmodInstanceGroup->getPaused(&isPaused));

        return numChanels != 0 && !isPaused;
    }

    return false;
}

void FMODFileSoundEvent::Stop(bool force /* = false */)
{
    if (fmodInstanceGroup)
    {
        FMOD_VERIFY(fmodInstanceGroup->stop());
    }
}

int32 FMODFileSoundEvent::GetLoopCount()
{
    int32 loopCount = 0;
    if (fmodInstanceGroup)
    {
        FMOD_VERIFY(fmodSound->getLoopCount(&loopCount));
    }
    return loopCount;
}

void FMODFileSoundEvent::SetLoopCount(int32 loopCount)
{
    if (fmodInstanceGroup)
    {
        FMOD_VERIFY(fmodSound->setLoopCount(loopCount));
    }
}

void FMODFileSoundEvent::SetPaused(bool paused)
{
    if (fmodInstanceGroup)
    {
        FMOD_VERIFY(fmodInstanceGroup->setPaused(paused));
    }
}

FMOD_RESULT F_CALLBACK FMODFileSoundEvent::SoundInstanceEndPlaying(FMOD_CHANNEL* channel, FMOD_CHANNEL_CALLBACKTYPE type, void* commanddata1, void* commanddata2)
{
    if (type == FMOD_CHANNEL_CALLBACKTYPE_END)
    {
        FMOD::Channel* cppchannel = reinterpret_cast<FMOD::Channel*>(channel);
        if (cppchannel)
        {
            FMODFileSoundEvent* sound = 0;
            FMOD_VERIFY(cppchannel->getUserData(reinterpret_cast<void**>(&sound)));
            if (sound)
            {
                sound->PerformEvent(EVENT_END);
                sound->soundSystem->ReleaseOnUpdate(sound);
            }
        }
    }

    return FMOD_OK;
}
};
