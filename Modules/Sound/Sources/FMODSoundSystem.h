#pragma once

#include "Base/Singleton.h"
#include "Base/BaseTypes.h"
#include "Base/BaseObject.h"
#include "Base/FastName.h"
#include "Base/EventDispatcher.h"
#include "Concurrency/Mutex.h"
#include "FileSystem/FilePath.h"
#include "Sound/SoundEvent.h"
#include "Sound/SoundStream.h"
#include "Sound/SoundSystem.h"

namespace FMOD
{
class EventGroup;
class System;
class EventSystem;
class EventProject;
class ChannelGroup;
};

namespace DAVA
{
class FMODFileSoundEvent;
class FMODSoundEvent;

class Engine;
class Component;
class FMODSoundSystem : public SoundSystem
{
    static Mutex soundGroupsMutex;

public:
    FMODSoundSystem(Engine* e);
    Engine* engine = nullptr;

    ~FMODSoundSystem();

    SoundStream* CreateSoundStream(SoundStreamDelegate* streamDelegate, uint32 channelsCount);

    SoundEvent* CreateSoundEventByID(const FastName& eventName, const FastName& groupName);
    SoundEvent* CreateSoundEventFromFile(const FilePath& fileName, const FastName& groupName, uint32 createFlags = SoundEvent::SOUND_EVENT_CREATE_DEFAULT, int32 priority = 128);

    void SerializeEvent(const SoundEvent* sEvent, KeyedArchive* toArchive);
    SoundEvent* DeserializeEvent(KeyedArchive* archive);
    SoundEvent* CloneEvent(const SoundEvent* sEvent);

    void Mute(bool value);

    void SetCurrentLocale(const String& langID);
    String GetCurrentLocale() const;

    void SetListenerPosition(const Vector3& position);
    void SetListenerOrientation(const Vector3& forward, const Vector3& left);

    void SetAllGroupsVolume(float32 volume);
    void SetGroupVolume(const FastName& groupName, float32 volume);
    float32 GetGroupVolume(const FastName& groupName) const;

    void SetAllGroupsSpeed(float32 speed);
    void SetGroupSpeed(const FastName& groupName, float32 speed);
    float32 GetGroupSpeed(const FastName& groupName) const;

    void InitFromQualitySettings();

    void SetDebugMode(bool debug = true);
    bool IsDebugModeOn() const;

protected:
    void ParseSFXConfig(const FilePath& configPath);

protected:
    struct SoundGroup
    {
        SoundGroup()
            : volume(1.0f)
            , speed(1.0f)
        {
        }

        FastName name;
        float32 volume;
        float32 speed;
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
    void OnUpdate(float32 timeElapsed);
    void OnSuspend();
    void OnResume();

    size_t onUpdateToken = 0;
    size_t onSuspendToken = 0;
    size_t onResumeToken = 0;

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
};
};
