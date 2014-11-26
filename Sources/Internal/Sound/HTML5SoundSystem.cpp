#if defined(__DAVAENGINE_HTML5__)

#include "Sound/SoundSystem.h"

namespace DAVA
{

SoundSystem::SoundSystem()
{
}

SoundSystem::~SoundSystem()
{
}

void SoundSystem::SetCurrentLocale(const String & langID)
{
}

SoundEvent * SoundSystem::CreateSoundEventFromFile(const FilePath & fileName, const FastName & groupName, uint32 flags /* = SOUND_EVENT_DEFAULT */, int32 priority /* = 128 */)
{
	return NULL;
}

void SoundSystem::InitFromQualitySettings()
{
}

void SoundSystem::SetGroupVolume(const FastName & groupName, float32 volume)
{
}

void SoundSystem::SerializeEvent(const SoundEvent * sEvent, KeyedArchive *toArchive)
{
}

void SoundSystem::Resume()
{
}

void SoundSystem::SetListenerPosition(const Vector3 & position)
{
}

SoundEvent * SoundSystem::DeserializeEvent(KeyedArchive *archive)
{
}

void SoundSystem::Suspend()
{
}

float32 SoundSystem::GetGroupVolume(const FastName & groupName)
{
    return -1.f;
}

void SoundSystem::Update(float32 timeElapsed)
{
}

void SoundSystem::SetListenerOrientation(const Vector3 & forward, const Vector3 & left)
{
}

SoundEvent * SoundSystem::CreateSoundEventByID(const FastName & eventName, const FastName & groupName)
{
	return NULL;
}

};
#endif