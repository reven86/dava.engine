#ifndef __DAVAENGINE_SOUND_SYSTEM_H__
#define __DAVAENGINE_SOUND_SYSTEM_H__

#include "Base/Singleton.h"
#include "Base/BaseTypes.h"
#include "Base/BaseObject.h"
#include "Base/FastName.h"
#include "FileSystem/FilePath.h"
#include "Base/EventDispatcher.h"
#include "Base/FastName.h"
#include "Sound/SoundEvent.h"
#include "Concurrency/Mutex.h"

#ifdef DAVA_FMOD
namespace FMOD
{
class EventGroup;
class System;
class EventSystem;
class EventProject;
class ChannelGroup;
};
#endif

namespace DAVA
{

#ifdef DAVA_FMOD
class FMODFileSoundEvent;
class FMODSoundEvent;
#endif

class Component;
class SoundSystem : public Singleton<SoundSystem>
{
    static Mutex soundGroupsMutex;

public:
    SoundSystem();
    ~SoundSystem();

    SoundEvent* CreateSoundEventByID(const FastName& eventName, const FastName& groupName);
    SoundEvent* CreateSoundEventFromFile(const FilePath& fileName, const FastName& groupName, uint32 createFlags = SoundEvent::SOUND_EVENT_CREATE_DEFAULT, int32 priority = 128);

    void SerializeEvent(const SoundEvent* sEvent, KeyedArchive* toArchive);
    SoundEvent* DeserializeEvent(KeyedArchive* archive);
    SoundEvent* CloneEvent(const SoundEvent* sEvent);

    void Update(float32 timeElapsed);
    void Suspend();
    void Resume();
    void Mute(bool value);

    void SetCurrentLocale(const String& langID);
    String GetCurrentLocale() const;

    void SetListenerPosition(const Vector3& position);
    void SetListenerOrientation(const Vector3& forward, const Vector3& left);

    void SetGroupVolume(const FastName& groupName, float32 volume);
    float32 GetGroupVolume(const FastName& groupName);

    void InitFromQualitySettings();

    void SetDebugMode(bool debug = true);
    bool IsDebugModeOn() const;

protected:
    void ParseSFXConfig(const FilePath& configPath);

#ifdef DAVA_FMOD
protected:
    struct SoundGroup
    {
        SoundGroup()
            : volume(1.f)
        {
        }

        FastName name;
        float32 volume;
        Vector<SoundEvent*> events;
    };

public:
    void LoadFEV(const FilePath& filePath);
    void UnloadFEV(const FilePath& filePath);
    void UnloadFMODProjects();

    void PreloadFMODEventGroupData(const String& groupName);
    void ReleaseFMODEventGroupData(const String& groupName);
    void ReleaseAllEventWaveData();

    void GetAllEventsNames(Vector<String>& names);

    uint32 GetMemoryUsageBytes() const;
    float32 GetTotalCPUUsage() const;
    int32 GetChannelsUsed() const;
    int32 GetChannelsMax() const;

#ifdef __DAVAENGINE_IPHONE__
    bool IsSystemMusicPlaying();
    void DuckSystemMusic(bool duck);
#endif

protected:
    void GetGroupEventsNamesRecursive(FMOD::EventGroup* group, String& currNamePath, Vector<String>& names);

    void AddSoundEventToGroup(const FastName& groupName, SoundEvent* event);
    void RemoveSoundEventFromGroups(SoundEvent* event);

    void ReleaseOnUpdate(SoundEvent* sound);

    FastName FindGroupByEvent(const SoundEvent* soundEvent);

    Vector<SoundEvent*> soundsToReleaseOnUpdate;

    FMOD::System* fmodSystem = nullptr;
    FMOD::EventSystem* fmodEventSystem = nullptr;

    FMOD::ChannelGroup* masterChannelGroup = nullptr;
    FMOD::ChannelGroup* masterEventChannelGroup = nullptr;

    Vector<SoundGroup> soundGroups;
    Map<FilePath, FMOD::EventProject*> projectsMap;

    Vector<String> toplevelGroups;

    friend class FMODFileSoundEvent;
    friend class FMODSoundEvent;
#ifdef __DAVAENGINE_IPHONE__
    friend class MusicIOSSoundEvent;
#endif
#endif
};
};

#endif //__DAVAENGINE_SOUND_SYSTEM_H__
