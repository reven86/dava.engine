#include "Sound/SoundSystem.h"

#ifndef DAVA_FMOD

namespace DAVA
{
Mutex SoundSystem::soundGroupsMutex;

SoundSystem::SoundSystem()
{
}

SoundSystem::~SoundSystem()
{
}

SoundEvent* SoundSystem::CreateSoundEventByID(const FastName& eventName, const FastName& groupName)
{
    return 0;
}

SoundEvent* SoundSystem::CreateSoundEventFromFile(const FilePath& fileName, const FastName& groupName, uint32 createFlags, int32 priority)
{
    return 0;
}

void SoundSystem::SerializeEvent(const SoundEvent* sEvent, KeyedArchive* toArchive)
{
}

SoundEvent* SoundSystem::DeserializeEvent(KeyedArchive* archive)
{
    return 0;
}

SoundEvent* SoundSystem::CloneEvent(const SoundEvent* sEvent)
{
    return 0;
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
