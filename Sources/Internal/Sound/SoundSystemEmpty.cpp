#include "Sound/SoundSystem.h"

#ifndef DAVA_FMOD

#include "Engine/EngineModule.h"

namespace DAVA
{
Mutex SoundSystem::soundGroupsMutex;

#if defined(__DAVAENGINE_COREV2__)
SoundSystem::SoundSystem(Engine* e)
    : engine(e)
#else
SoundSystem::SoundSystem()
#endif
{
}

SoundSystem::~SoundSystem()
{
}

SoundStream* SoundSystem::CreateSoundStream(SoundStreamDelegate* streamDelegate, uint32 channelsCount)
{
    return nullptr;
}

SoundEvent* SoundSystem::CreateSoundEventByID(const FastName& eventName, const FastName& groupName)
{
    return nullptr;
}

SoundEvent* SoundSystem::CreateSoundEventFromFile(const FilePath& fileName, const FastName& groupName, uint32 createFlags, int32 priority)
{
    return nullptr;
}

void SoundSystem::SerializeEvent(const SoundEvent* sEvent, KeyedArchive* toArchive)
{
}

SoundEvent* SoundSystem::DeserializeEvent(KeyedArchive* archive)
{
    return nullptr;
}

SoundEvent* SoundSystem::CloneEvent(const SoundEvent* sEvent)
{
    return nullptr;
}

void SoundSystem::Update(float32 timeElapsed)
{
}

void SoundSystem::Suspend()
{
}

void SoundSystem::Resume()
{
}

void SoundSystem::Mute(bool value)
{
}

void SoundSystem::SetCurrentLocale(const String& langID)
{
}

String SoundSystem::GetCurrentLocale() const
{
    return String();
}

void SoundSystem::SetListenerPosition(const Vector3& position)
{
}

void SoundSystem::SetListenerOrientation(const Vector3& forward, const Vector3& left)
{
}

void SoundSystem::SetGroupVolume(const FastName& groupName, float32 volume)
{
}

float32 SoundSystem::GetGroupVolume(const FastName& groupName)
{
    return 0.0f;
}

void SoundSystem::InitFromQualitySettings()
{
}

void SoundSystem::SetDebugMode(bool debug)
{
}

bool SoundSystem::IsDebugModeOn() const
{
    return false;
}

void SoundSystem::ParseSFXConfig(const FilePath& configPath)
{
}

} //DAVA

#endif //DAVA_FMOD
