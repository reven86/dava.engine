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

#ifdef SOUND_STUB 

#include "Sound/SoundSystem.h"
#include "Sound/FMODSoundEventStub.h"
#include "FileSystem/FileList.h"
#include "Scene3D/Entity.h"
#include "Scene3D/Systems/QualitySettingsSystem.h"
#include "FileSystem/YamlParser.h"



#define MAX_SOUND_CHANNELS 64

namespace DAVA
{


static const FastName SEREALIZE_EVENTTYPE_EVENTFILE("eventFromFile");
static const FastName SEREALIZE_EVENTTYPE_EVENTSYSTEM("eventFromSystem");

SoundSystem::SoundSystem()
{

}

SoundSystem::~SoundSystem()
{
//	FMOD_VERIFY(fmodEventSystem->release());
}

void SoundSystem::InitFromQualitySettings()
{
    QualitySettingsSystem * qSystem = QualitySettingsSystem::Instance();
    FilePath sfxConfig = qSystem->GetSFXQualityConfigPath(qSystem->GetCurSFXQuality());
    if(!sfxConfig.IsEmpty())
    {
        ParseSFXConfig(sfxConfig);
    }
    else
    {
        Logger::Warning("[SoundSystem] No default quality SFX config!");
    }
}

SoundEvent * SoundSystem::CreateSoundEventByID(const FastName & eventName, const FastName & groupName)
{
    SoundEvent * event = new STUBSoundEvent();//new FMODSoundEvent(eventName);
    //AddSoundEventToGroup(groupName, event);
    
    return event;
}

SoundEvent * SoundSystem::CreateSoundEventFromFile(const FilePath & fileName, const FastName & groupName, uint32 flags /* = SOUND_EVENT_DEFAULT */, int32 priority /* = 128 */)
{
    SoundEvent * event = 0;
    
#ifdef __DAVAENGINE_IPHONE__
    if((flags & SoundEvent::SOUND_EVENT_CREATE_STREAM) && !(flags & SoundEvent::SOUND_EVENT_CREATE_3D))
    {
        MusicIOSSoundEvent * musicEvent = MusicIOSSoundEvent::CreateMusicEvent(fileName);
        if(musicEvent && (flags & SoundEvent::SOUND_EVENT_CREATE_LOOP))
            musicEvent->SetLoopCount(-1);
        
        event = musicEvent;
    }
#endif //__DAVAENGINE_IPHONE__
    
    if(!event)
    {
        event = new STUBSoundEvent();//FMODFileSoundEvent::CreateWithFlags(fileName, flags, priority);
    }
    
    if(event)
    {
       // AddSoundEventToGroup(groupName, event);
    }
    
    return event;
}

void SoundSystem::SerializeEvent(const SoundEvent * sEvent, KeyedArchive *toArchive)
{
/*    if(IsPointerToExactClass<FMODFileSoundEvent>(sEvent))
    {
        FMODFileSoundEvent * sound = (FMODFileSoundEvent *)sEvent;
        toArchive->SetFastName("EventType", SEREALIZE_EVENTTYPE_EVENTFILE);

        toArchive->SetUInt32("flags", sound->flags);
        toArchive->SetInt32("priority", sound->priority);
        toArchive->SetString("filePath", sound->fileName.GetFrameworkPath());
    }
    else if(IsPointerToExactClass<FMODSoundEvent>(sEvent))
    {
        FMODSoundEvent * sound = (FMODSoundEvent *)sEvent;
        toArchive->SetFastName("EventType", SEREALIZE_EVENTTYPE_EVENTSYSTEM);

        toArchive->SetFastName("eventName", sound->eventName);
    }
#ifdef __DAVAENGINE_IPHONE__
    else if(IsPointerToExactClass<MusicIOSSoundEvent>(sEvent))
    {
        MusicIOSSoundEvent * musicEvent = (MusicIOSSoundEvent *)sEvent;
        toArchive->SetFastName("EventType", SEREALIZE_EVENTTYPE_EVENTFILE);
        
        uint32 flags = SoundEvent::SOUND_EVENT_CREATE_STREAM;
        if(musicEvent->GetLoopCount() == -1)
            flags |= SoundEvent::SOUND_EVENT_CREATE_LOOP;
        
        toArchive->SetUInt32("flags", flags);
        toArchive->SetString("filePath", musicEvent->GetEventName());
    }
#endif //__DAVAENGINE_IPHONE__

    FastName groupName;
    bool groupWasFound = false;
    Vector<SoundGroup>::iterator it = soundGroups.begin();
    Vector<SoundGroup>::iterator itEnd = soundGroups.end();
    for(;it != itEnd; ++it)
    {
        Vector<SoundEvent *> & events = it->events;
        Vector<SoundEvent *>::const_iterator itEv = events.begin();
        Vector<SoundEvent *>::const_iterator itEvEnd = events.end();
        for(;itEv != itEvEnd; ++itEv)
        {
            if((*itEv) == sEvent)
            {
                groupName = it->name;
                groupWasFound = true;
                break;
            }
        }
        if(groupWasFound)
            break;
    }
    if(groupWasFound)
    {
        toArchive->SetFastName("groupName", groupName);
    }*/
}

SoundEvent * SoundSystem::DeserializeEvent(KeyedArchive *archive)
{
    DVASSERT(archive);

    FastName eventType = archive->GetFastName("EventType");
    FastName groupName = archive->GetFastName("groupName", FastName(""));

    if(eventType == SEREALIZE_EVENTTYPE_EVENTFILE)
    {
        uint32 flags = archive->GetUInt32("flags");
        int32 priority = archive->GetInt32("priority");
        FilePath path(archive->GetString("filePath"));

        return CreateSoundEventFromFile(path, groupName, flags, priority);
    }
    else if(eventType == SEREALIZE_EVENTTYPE_EVENTSYSTEM)
    {
        FastName eventName = archive->GetFastName("eventName");

        return CreateSoundEventByID(eventName, groupName);
    }

    return 0;
}

void SoundSystem::ParseSFXConfig(const FilePath & configPath)
{
    YamlParser* parser = YamlParser::Create(configPath);
    if(parser)
    {    
        YamlNode* rootNode = parser->GetRootNode();
        const YamlNode * fmodfevsNode = rootNode->Get("fmod_projects");
        if(fmodfevsNode)
        {
            int32 fevCount = fmodfevsNode->GetCount();
            for(int32 i = 0; i < fevCount; ++i)
            {
               // LoadFEV(FilePath(fmodfevsNode->Get(i)->AsString()));
            }
        }
    }
    SafeRelease(parser);
}
/*
void SoundSystem::LoadFEV(const FilePath & filePath)
{

}

void SoundSystem::UnloadFEV(const FilePath & filePath)
{

}

void SoundSystem::UnloadFMODProjects()
{

}
*/
void SoundSystem::Update(float32 timeElapsed)
{

}

void SoundSystem::Suspend()
{
}
/*
void SoundSystem::ReleaseOnUpdate(SoundEvent * sound)
{
}

uint32 SoundSystem::GetMemoryUsageBytes() const
{
    uint32 memory = 0;
    
   
    return memory;
}

float32 SoundSystem::GetTotalCPUUsage() const
{
    float32 cpuTotal = 0.f;

    return cpuTotal;
}

int32 SoundSystem::GetChannelsUsed() const
{
    int32 channels = -1;
    return channels;
}

int32 SoundSystem::GetChannelsMax() const
{
    int32 softChannels = -1;

    return softChannels;
}
*/
void SoundSystem::Resume()
{
#ifdef __DAVAENGINE_IPHONE__
    FMOD_IPhone_RestoreAudioSession();
#endif
}

void SoundSystem::SetCurrentLocale(const String & langID)
{
   
}

void SoundSystem::SetListenerPosition(const Vector3 & position)
{
    
}
void SoundSystem::SetListenerOrientation(const Vector3 & forward, const Vector3 & left)
{

}
/*
void SoundSystem::GetGroupEventsNamesRecursive(FMOD::EventGroup * group, String & currNamePath, Vector<String> & names)
{

}

void SoundSystem::GetAllEventsNames(Vector<String> & names)
{
 
}

void SoundSystem::PreloadFMODEventGroupData(const String & groupName)
{
  
}
    
void SoundSystem::ReleaseFMODEventGroupData(const String & groupName)
{

}
    
void SoundSystem::ReleaseAllEventWaveData()
{
 
}
    */
void SoundSystem::SetGroupVolume(const FastName & groupName, float32 volume)
{
  
}

float32 SoundSystem::GetGroupVolume(const FastName & groupName)
{

    return -1.f;
}
/*
void SoundSystem::AddSoundEventToGroup(const FastName & groupName, SoundEvent * event)
{

}
    
void SoundSystem::RemoveSoundEventFromGroups(SoundEvent * event)
{

}
*/
/*
FMOD_RESULT F_CALLBACK DAVA_FMOD_FILE_OPENCALLBACK(const char * name, int unicode, unsigned int * filesize, void ** handle, void ** userdata)
{
    File * file = File::Create(FilePath(name), File::OPEN | File::READ);
    if(!file)
        return FMOD_ERR_FILE_NOTFOUND;

    (*filesize) = file->GetSize();
    (*handle) = file;

    return FMOD_OK;
}

FMOD_RESULT F_CALLBACK DAVA_FMOD_FILE_READCALLBACK(void * handle, void * buffer, unsigned int sizebytes, unsigned int * bytesread, void * userdata)
{
    File * file = (File*)handle;
    (*bytesread) = file->Read(buffer, sizebytes);

    return FMOD_OK;
}

FMOD_RESULT F_CALLBACK DAVA_FMOD_FILE_SEEKCALLBACK(void * handle, unsigned int pos, void * userdata)
{
    File * file = (File*)handle;
    file->Seek(pos, File::SEEK_FROM_START);

    return FMOD_OK;
}

FMOD_RESULT F_CALLBACK DAVA_FMOD_FILE_CLOSECALLBACK(void * handle, void * userdata)
{
    File * file = (File*)handle;
    SafeRelease(file);

    return FMOD_OK;
}
*/
};

#endif 