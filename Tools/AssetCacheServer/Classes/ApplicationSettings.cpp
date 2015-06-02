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


#include "ApplicationSettings.h"

#include "FileSystem/FileSystem.h"
#include "FileSystem/KeyedArchive.h"


void ApplicationSettings::Save() const
{
    static DAVA::FilePath path("~doc:/AssetServer/ACS_settings.dat");

    DAVA::FileSystem::Instance()->CreateDirectory(path.GetDirectory(), true);

    DAVA::ScopedPtr<DAVA::File> file(DAVA::File::Create(path, DAVA::File::CREATE | DAVA::File::WRITE));
    if(static_cast<DAVA::File *>(file) ==nullptr)
    {
        DAVA::Logger::Error("[ApplicationSettings::%s] Cannot create file %s", __FUNCTION__, path.GetStringValue().c_str());
        return;
    }
    
    DAVA::ScopedPtr<DAVA::KeyedArchive> archieve(new DAVA::KeyedArchive());
    Serialize(archieve);
    archieve->Save(file);
    
    emit SettingsUpdated(this);
}

void ApplicationSettings::Load()
{
    static DAVA::FilePath path("~doc:/AssetServer/ACS_settings.dat");
    
    DAVA::ScopedPtr<DAVA::File> file(DAVA::File::Create(path, DAVA::File::OPEN | DAVA::File::READ));
    if(static_cast<DAVA::File *>(file) ==nullptr)
    {
        DAVA::Logger::Error("[ApplicationSettings::%s] Cannot open file %s", __FUNCTION__, path.GetStringValue().c_str());
        return;
    }
    
    DAVA::ScopedPtr<DAVA::KeyedArchive> archieve(new DAVA::KeyedArchive());
    archieve->Load(static_cast<DAVA::File *>(file));
    Deserialize(archieve);

    emit SettingsUpdated(this);
}

void ApplicationSettings::Serialize(DAVA::KeyedArchive * archieve) const
{
    DVASSERT(nullptr != archieve);
    
    archieve->SetString("FolderPath", folder.GetStringValue());
    archieve->SetFloat64("FolderSize", cacheSize);
    archieve->SetUInt32("NumberOfFiles", filesCount);
    
    auto size = servers.size();
    archieve->SetUInt32("ServersSize", size);

    DAVA::uint32 index = 0;
    for(auto & sd: servers)
    {
        archieve->SetString(DAVA::Format("Server_%d_ip", index), sd.ip);
        ++index;
    }
}

void ApplicationSettings::Deserialize(DAVA::KeyedArchive * archieve)
{
    DVASSERT(nullptr != archieve);
    
    DVASSERT(servers.size() == 0);
    
    folder = archieve->GetString("FolderPath");
    cacheSize = archieve->GetFloat64("FolderSize");
    filesCount = archieve->GetUInt32("NumberOfFiles");

    auto count = archieve->GetUInt32("ServersSize");
    for(DAVA::uint32 i = 0; i < count; ++i)
    {
        ServerData sd;
        sd.ip = archieve->GetString(DAVA::Format("Server_%d_ip", i));

        servers.push_back(sd);
    }
}


const DAVA::FilePath & ApplicationSettings::GetFolder() const
{
    return folder;
}

void ApplicationSettings::SetFolder(const DAVA::FilePath & _folder)
{
    folder = _folder;
}

const DAVA::float64 ApplicationSettings::GetCacheSize() const
{
    return cacheSize;
}

void ApplicationSettings::SetCacheSize(const DAVA::float64 size)
{
    cacheSize = size;
}

const DAVA::uint32 ApplicationSettings::GetFilesCount() const
{
    return filesCount;
}

void ApplicationSettings::SetFilesCount(const DAVA::uint32 count)
{
    filesCount = count;
}

const DAVA::uint64 ApplicationSettings::GetAutoSaveTimeout() const
{
    return autoSaveTimeout;
}

void ApplicationSettings::SetAutoSaveTimeout(const DAVA::uint64 timeout)
{
    autoSaveTimeout = timeout;
}


const DAVA::List<ServerData> & ApplicationSettings::GetServers() const
{
    return servers;
}

void ApplicationSettings::ResetServers()
{
    servers.clear();
}

void ApplicationSettings::AddServer(const ServerData & server)
{
    servers.push_back(server);
}

void ApplicationSettings::RemoveServer(const ServerData & server)
{
    servers.remove(server);
}


