#ifdef DAVA_FMOD

#include "Sound/SoundSystem.h"
#include "FileSystem/FileList.h"
#include "Scene3D/Entity.h"
#include "Scene3D/Systems/QualitySettingsSystem.h"
#include "Sound/FMODUtils.h"
#include "Sound/FMODFileSoundEvent.h"
#include "Sound/FMODSoundEvent.h"
#include "Sound/FMODSoundStream.h"
#include "FileSystem/YamlParser.h"
#include "FileSystem/YamlNode.h"
#include "Debug/ProfilerCPU.h"
#include "Debug/ProfilerMarkerNames.h"
#include "Engine/Engine.h"
#include "Concurrency/LockGuard.h"

#ifdef __DAVAENGINE_IPHONE__
#include "fmodiphone.h"
#include "Sound/iOS/musicios.h"
#endif
#if defined(DAVA_MEMORY_PROFILING_ENABLE)
#include "MemoryManager/MemoryManager.h"
#include "MemoryManager/MemoryProfiler.h"
#endif
#define MAX_SOUND_CHANNELS 48
#define MAX_SOUND_VIRTUAL_CHANNELS 64

namespace DAVA
{
FMOD_RESULT F_CALLBACK DAVA_FMOD_FILE_OPENCALLBACK(const char* name, int unicode, unsigned int* filesize, void** handle, void** userdata);
FMOD_RESULT F_CALLBACK DAVA_FMOD_FILE_READCALLBACK(void* handle, void* buffer, unsigned int sizebytes, unsigned int* bytesread, void* userdata);
FMOD_RESULT F_CALLBACK DAVA_FMOD_FILE_SEEKCALLBACK(void* handle, unsigned int pos, void* userdata);
FMOD_RESULT F_CALLBACK DAVA_FMOD_FILE_CLOSECALLBACK(void* handle, void* userdata);

namespace
{

#if defined(DAVA_MEMORY_PROFILING_ENABLE)
void* F_CALLBACK fmod_tracking_alloc(unsigned int size, FMOD_MEMORY_TYPE /*type*/, const char* /*sourcestr*/)
{
    return MemoryManager::Instance()->Allocate(size, ALLOC_POOL_FMOD);
}

void* F_CALLBACK fmod_tracking_realloc(void* ptr, unsigned int size, FMOD_MEMORY_TYPE /*type*/, const char* /*sourcestr*/)
{
    return MemoryManager::Instance()->Reallocate(ptr, ALLOC_POOL_FMOD);
}

void F_CALLBACK fmod_tracking_free(void* ptr, FMOD_MEMORY_TYPE /*type*/, const char* /*sourcestr*/)
{
    MemoryManager::Instance()->Deallocate(ptr);
}
#endif // DAVA_MEMORY_PROFILING_ENABLE

} // unnamed namespace

static_assert(sizeof(FMOD_VECTOR) == sizeof(Vector3), "Sizes of FMOD_VECTOR and Vector3 do not match");

static const FastName SEREALIZE_EVENTTYPE_EVENTFILE("eventFromFile");
static const FastName SEREALIZE_EVENTTYPE_EVENTSYSTEM("eventFromSystem");

Mutex SoundSystem::soundGroupsMutex;

#if defined(__DAVAENGINE_COREV2__)
SoundSystem::SoundSystem(Engine* e)
    : engine(e)
{
    sigUpdateId = engine->update.Connect(this, &SoundSystem::Update);
#else
SoundSystem::SoundSystem()
{
#endif
    SetDebugMode(false);

#if defined(DAVA_MEMORY_PROFILING_ENABLE)
    FMOD::Memory_Initialize(nullptr, 0, &fmod_tracking_alloc, &fmod_tracking_realloc, &fmod_tracking_free);
#endif

    void* extraDriverData = 0;
#ifdef __DAVAENGINE_IPHONE__
    FMOD_IPHONE_EXTRADRIVERDATA iphoneDriverData;
    Memset(&iphoneDriverData, 0, sizeof(FMOD_IPHONE_EXTRADRIVERDATA));

    iphoneDriverData.sessionCategory = FMOD_IPHONE_SESSIONCATEGORY_AMBIENTSOUND;

    extraDriverData = &iphoneDriverData;
#endif

    FMOD_VERIFY(FMOD::EventSystem_Create(&fmodEventSystem));
    FMOD_VERIFY(fmodEventSystem->getSystemObject(&fmodSystem));
#ifdef __DAVAENGINE_ANDROID__
    FMOD_VERIFY(fmodSystem->setOutput(FMOD_OUTPUTTYPE_AUDIOTRACK));
#endif
    FMOD_VERIFY(fmodSystem->setSoftwareChannels(MAX_SOUND_CHANNELS));

    FMOD_INITFLAGS initFlags = FMOD_INIT_NORMAL;
#ifdef DAVA_FMOD_PROFILE
    initFlags |= FMOD_INIT_ENABLE_PROFILE;
#endif

    FMOD_RESULT initResult = fmodEventSystem->init(MAX_SOUND_VIRTUAL_CHANNELS, initFlags, extraDriverData);

    if (initResult != FMOD_OK)
    {
        Logger::Error("Failed to initialize FMOD: %s", FMOD_ErrorString(initResult));
        FMOD_VERIFY(fmodEventSystem->release());
        fmodEventSystem = nullptr;
        fmodSystem = nullptr;
    }

    if (fmodEventSystem)
    {
        FMOD::EventCategory* masterCategory = nullptr;
        FMOD_VERIFY(fmodEventSystem->getCategory("master", &masterCategory));
        FMOD_VERIFY(masterCategory->getChannelGroup(&masterEventChannelGroup));

        FMOD_VERIFY(fmodSystem->getMasterChannelGroup(&masterChannelGroup));
        FMOD_VERIFY(fmodSystem->setFileSystem(DAVA_FMOD_FILE_OPENCALLBACK, DAVA_FMOD_FILE_CLOSECALLBACK, DAVA_FMOD_FILE_READCALLBACK, DAVA_FMOD_FILE_SEEKCALLBACK, 0, 0, -1));
    }
}

SoundSystem::~SoundSystem()
{
#if defined(__DAVAENGINE_COREV2__)
    engine->update.Disconnect(sigUpdateId);
#endif

    if (fmodEventSystem)
    {
        FMOD_VERIFY(fmodEventSystem->release());
    }
}

void SoundSystem::InitFromQualitySettings()
{
    UnloadFMODProjects();

    QualitySettingsSystem* qSystem = QualitySettingsSystem::Instance();
    FilePath sfxConfig = qSystem->GetSFXQualityConfigPath(qSystem->GetCurSFXQuality());
    if (!sfxConfig.IsEmpty())
    {
        ParseSFXConfig(sfxConfig);
    }
    else
    {
        Logger::Warning("[SoundSystem] No default quality SFX config!");
    }
}

void SoundSystem::SetDebugMode(bool debug)
{
    FMOD::Debug_SetLevel(debug ? FMOD_DEBUG_LEVEL_ALL : FMOD_DEBUG_LEVEL_NONE);
}

bool SoundSystem::IsDebugModeOn() const
{
    FMOD_DEBUGLEVEL debugLevel = 0;
    FMOD::Debug_GetLevel(&debugLevel);
    return debugLevel != FMOD_DEBUG_LEVEL_NONE;
}

SoundStream* SoundSystem::CreateSoundStream(SoundStreamDelegate* streamDelegate, uint32 channelsCount)
{
    FMODSoundStream* fmodStream = new FMODSoundStream(streamDelegate, channelsCount);
    bool isInited = fmodStream->Init(fmodSystem);
    if (!isInited)
    {
        SafeDelete(fmodStream);
    }

    return fmodStream;
}

SoundEvent* SoundSystem::CreateSoundEventByID(const FastName& eventName, const FastName& groupName)
{
    SoundEvent* event = new FMODSoundEvent(eventName);
    AddSoundEventToGroup(groupName, event);

    return event;
}

SoundEvent* SoundSystem::CreateSoundEventFromFile(const FilePath& fileName, const FastName& groupName, uint32 flags /* = SOUND_EVENT_DEFAULT */, int32 priority /* = 128 */)
{
    SoundEvent* event = nullptr;

#ifdef __DAVAENGINE_IPHONE__
    if ((flags & SoundEvent::SOUND_EVENT_CREATE_STREAM) && !(flags & SoundEvent::SOUND_EVENT_CREATE_3D))
    {
        MusicIOSSoundEvent* musicEvent = MusicIOSSoundEvent::CreateMusicEvent(fileName);
        if (musicEvent && (flags & SoundEvent::SOUND_EVENT_CREATE_LOOP))
            musicEvent->SetLoopCount(-1);

        event = musicEvent;
    }
#endif //__DAVAENGINE_IPHONE__

    if (!event)
    {
        event = FMODFileSoundEvent::CreateWithFlags(fileName, flags, priority);
    }

    if (event)
    {
        AddSoundEventToGroup(groupName, event);
    }

    return event;
}

void SoundSystem::SerializeEvent(const SoundEvent* sEvent, KeyedArchive* toArchive)
{
    if (IsPointerToExactClass<FMODFileSoundEvent>(sEvent))
    {
        const FMODFileSoundEvent* sound = static_cast<const FMODFileSoundEvent*>(sEvent);
        toArchive->SetFastName("EventType", SEREALIZE_EVENTTYPE_EVENTFILE);

        toArchive->SetUInt32("flags", sound->flags);
        toArchive->SetInt32("priority", sound->priority);
        toArchive->SetString("filePath", sound->fileName.GetFrameworkPath());
    }
    else if (IsPointerToExactClass<FMODSoundEvent>(sEvent))
    {
        const FMODSoundEvent* sound = static_cast<const FMODSoundEvent*>(sEvent);
        toArchive->SetFastName("EventType", SEREALIZE_EVENTTYPE_EVENTSYSTEM);

        toArchive->SetFastName("eventName", sound->eventName);
    }
#ifdef __DAVAENGINE_IPHONE__
    else if (IsPointerToExactClass<MusicIOSSoundEvent>(sEvent))
    {
        MusicIOSSoundEvent* musicEvent = (MusicIOSSoundEvent*)sEvent;
        toArchive->SetFastName("EventType", SEREALIZE_EVENTTYPE_EVENTFILE);

        uint32 flags = SoundEvent::SOUND_EVENT_CREATE_STREAM;
        if (musicEvent->GetLoopCount() == -1)
            flags |= SoundEvent::SOUND_EVENT_CREATE_LOOP;

        toArchive->SetUInt32("flags", flags);
        toArchive->SetString("filePath", musicEvent->GetEventName());
    }
#endif //__DAVAENGINE_IPHONE__

    FastName groupName = FindGroupByEvent(sEvent);
    if (groupName.IsValid())
        toArchive->SetFastName("groupName", groupName);
}

SoundEvent* SoundSystem::DeserializeEvent(KeyedArchive* archive)
{
    DVASSERT(archive);

    FastName eventType = archive->GetFastName("EventType");
    FastName groupName = archive->GetFastName("groupName", FastName(""));

    if (eventType == SEREALIZE_EVENTTYPE_EVENTFILE)
    {
        uint32 flags = archive->GetUInt32("flags");
        int32 priority = archive->GetInt32("priority");
        FilePath path(archive->GetString("filePath"));

        return CreateSoundEventFromFile(path, groupName, flags, priority);
    }
    else if (eventType == SEREALIZE_EVENTTYPE_EVENTSYSTEM)
    {
        FastName eventName = archive->GetFastName("eventName");

        return CreateSoundEventByID(eventName, groupName);
    }

    return 0;
}

SoundEvent* SoundSystem::CloneEvent(const SoundEvent* sEvent)
{
    DVASSERT(sEvent);

    SoundEvent* clonedSound = 0;
    if (IsPointerToExactClass<FMODFileSoundEvent>(sEvent))
    {
        const FMODFileSoundEvent* sound = static_cast<const FMODFileSoundEvent*>(sEvent);
        clonedSound = CreateSoundEventFromFile(sound->fileName, FindGroupByEvent(sound), sound->flags, sound->priority);
    }
    else if (IsPointerToExactClass<FMODSoundEvent>(sEvent))
    {
        const FMODSoundEvent* sound = static_cast<const FMODSoundEvent*>(sEvent);
        clonedSound = CreateSoundEventByID(sound->eventName, FindGroupByEvent(sound));
    }
#ifdef __DAVAENGINE_IPHONE__
    else if (IsPointerToExactClass<MusicIOSSoundEvent>(sEvent))
    {
        MusicIOSSoundEvent* musicEvent = (MusicIOSSoundEvent*)sEvent;

        uint32 flags = SoundEvent::SOUND_EVENT_CREATE_STREAM;
        if (musicEvent->GetLoopCount() == -1)
            flags |= SoundEvent::SOUND_EVENT_CREATE_LOOP;

        clonedSound = CreateSoundEventFromFile(musicEvent->GetEventName(), FindGroupByEvent(sEvent), flags);
    }
#endif //__DAVAENGINE_IPHONE__

    DVASSERT(clonedSound);
    return clonedSound;
}

FastName SoundSystem::FindGroupByEvent(const SoundEvent* soundEvent)
{
    FastName groupName;
    bool groupWasFound = false;
    Vector<SoundGroup>::iterator it = soundGroups.begin();
    Vector<SoundGroup>::iterator itEnd = soundGroups.end();
    for (; it != itEnd; ++it)
    {
        Vector<SoundEvent*>& events = it->events;
        Vector<SoundEvent*>::const_iterator itEv = events.begin();
        Vector<SoundEvent*>::const_iterator itEvEnd = events.end();
        for (; itEv != itEvEnd; ++itEv)
        {
            if ((*itEv) == soundEvent)
            {
                groupName = it->name;
                groupWasFound = true;
                break;
            }
        }
        if (groupWasFound)
            break;
    }

    return groupName;
}

void SoundSystem::ParseSFXConfig(const FilePath& configPath)
{
    YamlParser* parser = YamlParser::Create(configPath);
    if (parser)
    {
        YamlNode* rootNode = parser->GetRootNode();
        const YamlNode* fmodfevsNode = rootNode->Get("fmod_projects");
        if (fmodfevsNode)
        {
            int32 fevCount = fmodfevsNode->GetCount();
            for (int32 i = 0; i < fevCount; ++i)
            {
                LoadFEV(FilePath(fmodfevsNode->Get(i)->AsString()));
            }
        }
    }
    SafeRelease(parser);
}

void SoundSystem::LoadFEV(const FilePath& filePath)
{
    if (projectsMap.find(filePath) != projectsMap.end())
        return;

    if (fmodEventSystem == nullptr)
        return;

    FMOD::EventProject* project = nullptr;
    FMOD_VERIFY(fmodEventSystem->load(filePath.GetStringValue().c_str(), 0, &project));

    if (project)
    {
        FMOD_EVENT_PROJECTINFO info;
        FMOD_VERIFY(project->getInfo(&info));
        String projectName(info.name);

        int32 groupsCount = 0;
        FMOD_VERIFY(project->getNumGroups(&groupsCount));
        for (int32 i = 0; i < groupsCount; ++i)
        {
            FMOD::EventGroup* group = 0;
            FMOD_VERIFY(project->getGroupByIndex(i, false, &group));

            char* buf = 0;
            FMOD_VERIFY(group->getInfo(0, &buf));
            toplevelGroups.push_back(projectName + "/" + buf);
        }

        projectsMap[filePath] = project;
    }
}

void SoundSystem::UnloadFEV(const FilePath& filePath)
{
    Map<FilePath, FMOD::EventProject*>::iterator it = projectsMap.find(filePath);
    if (it != projectsMap.end())
    {
        FMOD_VERIFY(it->second->release());
        projectsMap.erase(it);
    }
}

void SoundSystem::UnloadFMODProjects()
{
    if (fmodEventSystem)
    {
        FMOD_VERIFY(fmodEventSystem->unload());
    }

    projectsMap.clear();
    toplevelGroups.clear();
}

void SoundSystem::Update(float32 timeElapsed)
{
    DAVA_PROFILER_CPU_SCOPE(ProfilerCPUMarkerName::SOUND_SYSTEM);

    if (fmodEventSystem)
    {
        FMOD_VERIFY(fmodEventSystem->update());
    }

    uint32 size = static_cast<uint32>(soundsToReleaseOnUpdate.size());
    if (size)
    {
        for (uint32 i = 0; i < size; i++)
            soundsToReleaseOnUpdate[i]->Release();
        soundsToReleaseOnUpdate.clear();
    }
}

void SoundSystem::ReleaseOnUpdate(SoundEvent* sound)
{
    soundsToReleaseOnUpdate.push_back(sound);
}

uint32 SoundSystem::GetMemoryUsageBytes() const
{
    uint32 memory = 0;

    if (fmodEventSystem)
    {
        FMOD_VERIFY(fmodEventSystem->getMemoryInfo(FMOD_MEMBITS_ALL, FMOD_EVENT_MEMBITS_ALL, &memory, 0));
    }

    return memory;
}

float32 SoundSystem::GetTotalCPUUsage() const
{
    float32 cpuTotal = 0.f;
    FMOD_VERIFY(fmodSystem->getCPUUsage(0, 0, 0, 0, &cpuTotal));

    return cpuTotal;
}

int32 SoundSystem::GetChannelsUsed() const
{
    int32 channels = -1;
    FMOD_VERIFY(fmodSystem->getChannelsPlaying(&channels));

    return channels;
}

int32 SoundSystem::GetChannelsMax() const
{
    int32 softChannels = -1;
    FMOD_VERIFY(fmodSystem->getSoftwareChannels(&softChannels));

    return softChannels;
}

void SoundSystem::Suspend()
{
#ifdef __DAVAENGINE_ANDROID__
    //SoundSystem should be suspended by FMODAudioDevice::stop() on JAVA layer.
    //It's called, but unfortunately it's doesn't work
    Mute(true);
#endif
}

void SoundSystem::Resume()
{
#ifdef __DAVAENGINE_IPHONE__
    FMOD_IPhone_RestoreAudioSession();
#endif
#ifdef __DAVAENGINE_ANDROID__
    Mute(false);
#endif
}

void SoundSystem::Mute(bool value)
{
    FMOD_VERIFY(masterChannelGroup->setMute(value));
    FMOD_VERIFY(masterEventChannelGroup->setMute(value));
}

void SoundSystem::SetCurrentLocale(const String& langID)
{
    if (fmodEventSystem)
    {
        FMOD_VERIFY(fmodEventSystem->setLanguage(langID.c_str()));
    }
}

String SoundSystem::GetCurrentLocale() const
{
    if (fmodEventSystem)
    {
        char lang[256] = {};
        FMOD_VERIFY(fmodEventSystem->getLanguage(lang));
        return String(lang);
    }

    return String();
}

void SoundSystem::SetListenerPosition(const Vector3& position)
{
    if (fmodEventSystem)
    {
        FMOD_VERIFY(fmodEventSystem->set3DListenerAttributes(0, reinterpret_cast<const FMOD_VECTOR*>(&position), 0, 0, 0));
    }
}

void SoundSystem::SetListenerOrientation(const Vector3& forward, const Vector3& left)
{
    if (fmodEventSystem)
    {
        Vector3 forwardNorm = forward;
        forwardNorm.Normalize();
        Vector3 upNorm = forwardNorm.CrossProduct(left);
        upNorm.Normalize();

        DVASSERT(forwardNorm.SquareLength() > EPSILON);
        DVASSERT(upNorm.SquareLength() > EPSILON);
        DVASSERT(left.SquareLength() > EPSILON);

        FMOD_VERIFY(fmodEventSystem->set3DListenerAttributes(0, 0, 0, reinterpret_cast<FMOD_VECTOR*>(&forwardNorm), reinterpret_cast<FMOD_VECTOR*>(&upNorm)));
    }
}

void SoundSystem::GetGroupEventsNamesRecursive(FMOD::EventGroup* group, String& currNamePath, Vector<String>& names)
{
    char* groupName = 0;
    FMOD_VERIFY(group->getInfo(0, &groupName));
    DVASSERT(groupName);
    String currPath = currNamePath + "/" + groupName;

    int32 eventsCount = 0;
    FMOD_VERIFY(group->getNumEvents(&eventsCount));
    names.reserve(names.size() + eventsCount);
    for (int32 i = 0; i < eventsCount; i++)
    {
        FMOD::Event* event = 0;
        FMOD_VERIFY(group->getEventByIndex(i, FMOD_EVENT_INFOONLY, &event));
        if (!event)
            continue;

        char* eventName = 0;
        FMOD_VERIFY(event->getInfo(0, &eventName, 0));
        DVASSERT(eventName);

        names.push_back(currPath + "/" + eventName);
    }

    int32 groupsCount = 0;
    FMOD_VERIFY(group->getNumGroups(&groupsCount));
    for (int32 i = 0; i < groupsCount; i++)
    {
        FMOD::EventGroup* childGroup = 0;
        FMOD_VERIFY(group->getGroupByIndex(i, false, &childGroup));
        if (childGroup)
            GetGroupEventsNamesRecursive(childGroup, currPath, names);
    }
}

void SoundSystem::GetAllEventsNames(Vector<String>& names)
{
    names.clear();

    if (fmodEventSystem == nullptr)
        return;

    int32 projectsCount = 0;
    FMOD_VERIFY(fmodEventSystem->getNumProjects(&projectsCount));
    for (int32 i = 0; i < projectsCount; i++)
    {
        FMOD::EventProject* project = 0;
        FMOD_VERIFY(fmodEventSystem->getProjectByIndex(i, &project));
        if (!project)
            continue;

        FMOD_EVENT_PROJECTINFO info;
        FMOD_VERIFY(project->getInfo(&info));
        String projectName(info.name);

        int32 groupsCount = 0;
        FMOD_VERIFY(project->getNumGroups(&groupsCount));
        for (int32 j = 0; j < groupsCount; j++)
        {
            FMOD::EventGroup* group = 0;
            FMOD_VERIFY(project->getGroupByIndex(j, false, &group));
            if (!group)
                continue;

            GetGroupEventsNamesRecursive(group, projectName, names);
        }
    }
}

void SoundSystem::PreloadFMODEventGroupData(const String& groupName)
{
    if (fmodEventSystem == nullptr)
        return;

    FMOD::EventGroup* eventGroup = nullptr;
    FMOD_VERIFY(fmodEventSystem->getGroup(groupName.c_str(), true, &eventGroup));
    if (eventGroup)
        FMOD_VERIFY(eventGroup->loadEventData(FMOD_EVENT_RESOURCE_STREAMS_AND_SAMPLES, FMOD_EVENT_NONBLOCKING));
}

void SoundSystem::ReleaseFMODEventGroupData(const String& groupName)
{
    if (fmodEventSystem == nullptr)
        return;

    FMOD::EventGroup* eventGroup = nullptr;
    FMOD_VERIFY(fmodEventSystem->getGroup(groupName.c_str(), false, &eventGroup));
    if (eventGroup)
        FMOD_VERIFY(eventGroup->freeEventData());
}

void SoundSystem::ReleaseAllEventWaveData()
{
    int32 topCount = static_cast<int32>(toplevelGroups.size());
    for (int32 i = 0; i < topCount; ++i)
        ReleaseFMODEventGroupData(toplevelGroups[i]);
}

void SoundSystem::SetAllGroupsVolume(float32 volume)
{
    LockGuard<Mutex> lock(soundGroupsMutex);

    for (size_t i = 0; i < soundGroups.size(); ++i)
    {
        SoundGroup& group = soundGroups[i];

        group.volume = volume;
        for (auto& x : group.events)
        {
            x->SetVolume(volume);
        }
    }
}

void SoundSystem::SetGroupVolume(const FastName& groupName, float32 volume)
{
    LockGuard<Mutex> lock(soundGroupsMutex);

    for (size_t i = 0; i < soundGroups.size(); ++i)
    {
        SoundGroup& group = soundGroups[i];
        if (group.name == groupName)
        {
            group.volume = volume;
            for (auto& x : group.events)
            {
                x->SetVolume(volume);
            }

            break;
        }
    }
}

float32 SoundSystem::GetGroupVolume(const FastName& groupName) const
{
    LockGuard<Mutex> lock(soundGroupsMutex);

    float32 ret = -1.f;
    for (size_t i = 0; i < soundGroups.size(); ++i)
    {
        SoundGroup const& group = soundGroups[i];
        if (group.name == groupName)
        {
            ret = group.volume;
            break;
        }
    }

    return ret;
}

void SoundSystem::SetAllGroupsSpeed(float32 speed)
{
    DVASSERT(speed >= 0.0f);

    LockGuard<Mutex> lock(soundGroupsMutex);

    for (size_t i = 0; i < soundGroups.size(); ++i)
    {
        SoundGroup& group = soundGroups[i];

        group.speed = speed;
        for (auto& x : group.events)
        {
            x->SetSpeed(speed);
        }
    }
}

void SoundSystem::SetGroupSpeed(const FastName& groupName, float32 speed)
{
    DVASSERT(speed >= 0.0f);

    LockGuard<Mutex> lock(soundGroupsMutex);

    for (size_t i = 0; i < soundGroups.size(); ++i)
    {
        SoundGroup& group = soundGroups[i];
        if (group.name == groupName)
        {
            group.speed = speed;
            for (auto& x : group.events)
            {
                x->SetSpeed(speed);
            }

            break;
        }
    }
}

float32 SoundSystem::GetGroupSpeed(const FastName& groupName) const
{
    LockGuard<Mutex> lock(soundGroupsMutex);

    float32 ret = -1.f;
    for (size_t i = 0; i < soundGroups.size(); ++i)
    {
        SoundGroup const& group = soundGroups[i];
        if (group.name == groupName)
        {
            ret = group.speed;
            break;
        }
    }

    return ret;
}

void SoundSystem::AddSoundEventToGroup(const FastName& groupName, SoundEvent* event)
{
    soundGroupsMutex.Lock();

    for (size_t i = 0; i < soundGroups.size(); ++i)
    {
        SoundGroup& group = soundGroups[i];
        if (group.name == groupName)
        {
            event->SetVolume(group.volume);
            group.events.push_back(event);

            soundGroupsMutex.Unlock();
            return;
        }
    }

    SoundGroup group;
    group.volume = 1.f;
    group.name = groupName;
    group.events.push_back(event);

    soundGroups.push_back(group);

    soundGroupsMutex.Unlock();
}

void SoundSystem::RemoveSoundEventFromGroups(SoundEvent* event)
{
    soundGroupsMutex.Lock();

    for (size_t i = 0; i < soundGroups.size();)
    {
        Vector<SoundEvent*>& events = soundGroups[i].events;
        for (size_t k = 0, eventsSize = events.size(); k < eventsSize; k++)
        {
            if (events[k] == event)
            {
                RemoveExchangingWithLast(events, k);
                break;
            }
        }

        if (events.empty())
        {
            RemoveExchangingWithLast(soundGroups, i);
        }
        else
        {
            ++i;
        }
    }

    soundGroupsMutex.Unlock();
}

#ifdef __DAVAENGINE_IPHONE__
bool SoundSystem::IsSystemMusicPlaying()
{
    bool ret = false;
    FMOD_IPhone_OtherAudioIsPlaying(&ret);
    return ret;
}

void SoundSystem::DuckSystemMusic(bool duck)
{
    FMOD_IPhone_DuckOtherAudio(duck);
}

#endif

FMOD_RESULT F_CALLBACK DAVA_FMOD_FILE_OPENCALLBACK(const char* name, int unicode, unsigned int* filesize, void** handle, void** userdata)
{
    File* file = File::Create(FilePath(name), File::OPEN | File::READ);
    if (!file)
        return FMOD_ERR_FILE_NOTFOUND;

    (*filesize) = static_cast<uint32>(file->GetSize());
    (*handle) = file;

    return FMOD_OK;
}

FMOD_RESULT F_CALLBACK DAVA_FMOD_FILE_READCALLBACK(void* handle, void* buffer, unsigned int sizebytes, unsigned int* bytesread, void* userdata)
{
    File* file = static_cast<File*>(handle);
    (*bytesread) = file->Read(buffer, sizebytes);

    return FMOD_OK;
}

FMOD_RESULT F_CALLBACK DAVA_FMOD_FILE_SEEKCALLBACK(void* handle, unsigned int pos, void* userdata)
{
    File* file = static_cast<File*>(handle);
    file->Seek(pos, File::SEEK_FROM_START);

    return FMOD_OK;
}

FMOD_RESULT F_CALLBACK DAVA_FMOD_FILE_CLOSECALLBACK(void* handle, void* userdata)
{
    File* file = static_cast<File*>(handle);
    SafeRelease(file);

    return FMOD_OK;
}
};

#endif //DAVA_FMOD
