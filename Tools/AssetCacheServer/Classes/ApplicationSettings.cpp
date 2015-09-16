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

using namespace DAVA;


ServerData::ServerData(String _ip, uint16 _port, bool _enabled)
	: ip(_ip)
	, port(_port)
	, enabled(_enabled) 
{
}

bool ServerData::IsEmpty() const 
{
	return ip.empty(); 
}

bool ServerData::operator == (const ServerData & right) const
{
	return (ip == right.ip) && (port == right.port);
}

bool ServerData::EquivalentTo(const DAVA::Net::Endpoint & right) const
{
	return (ip == right.Address().ToString()) && (port == right.Port());
}

bool ServerData::operator < (const ServerData & right) const
{
	if (ip == right.ip)
	{
		return port < right.port;
	}
	return ip < right.ip;
}


const String ApplicationSettings::DEFAULT_FOLDER = "";
const float64 ApplicationSettings::DEFAULT_CACHE_SIZE_GB = 5.0;


void ApplicationSettings::Save() const
{
    static FilePath path("~doc:/AssetServer/ACS_settings.dat");

    FileSystem::Instance()->CreateDirectory(path.GetDirectory(), true);

    ScopedPtr<File> file(File::Create(path, File::CREATE | File::WRITE));
    if(!file)
    {
        Logger::Error("[ApplicationSettings::%s] Cannot create file %s", __FUNCTION__, path.GetStringValue().c_str());
        return;
    }
    
    DAVA::ScopedPtr<DAVA::KeyedArchive> archive(new DAVA::KeyedArchive());
    Serialize(archive);
    archive->Save(file);
    
    emit SettingsUpdated(this);
}

void ApplicationSettings::Load()
{
    static FilePath path("~doc:/AssetServer/ACS_settings.dat");
    
    ScopedPtr<File> file(File::Create(path, File::OPEN | File::READ));
    if(file)
    {
        isFirstLaunch = false;
        ScopedPtr<DAVA::KeyedArchive> archive(new DAVA::KeyedArchive());
        archive->Load(file);
        Deserialize(archive);
    }
    else
    {
        isFirstLaunch = true;
    }

    emit SettingsUpdated(this);
}

void ApplicationSettings::Serialize(DAVA::KeyedArchive * archive) const
{
    DVASSERT(nullptr != archive);
    
    archive->SetString("FolderPath", folder.GetStringValue());
    archive->SetFloat64("FolderSize", cacheSizeGb);
    archive->SetUInt32("NumberOfFiles", filesCount);
    archive->SetUInt32("AutoSaveTimeout", autoSaveTimeoutMin);
    archive->SetUInt32("Port", listenPort);
    archive->SetBool("AutoStart", autoStart);
    
    auto size = remoteServers.size();
    archive->SetUInt32("ServersSize", size);

    uint32 index = 0;
    for(auto & sd: remoteServers)
    {
        archive->SetString(Format("Server_%d_ip", index), sd.ip);
        archive->SetUInt32(Format("Server_%d_port", index), sd.port);
        archive->SetBool(Format("Server_%d_enabled", index), sd.enabled);
        ++index;
    }
}

void ApplicationSettings::Deserialize(DAVA::KeyedArchive * archive)
{
    DVASSERT(nullptr != archive);
    
    DVASSERT(remoteServers.size() == 0);
    
    folder = archive->GetString("FolderPath", DEFAULT_FOLDER);
    cacheSizeGb = archive->GetFloat64("FolderSize", DEFAULT_CACHE_SIZE_GB);
    filesCount = archive->GetUInt32("NumberOfFiles", DEFAULT_FILES_COUNT);
    autoSaveTimeoutMin = archive->GetUInt32("AutoSaveTimeout", DEFAULT_AUTO_SAVE_TIMEOUT_MIN);
    listenPort = archive->GetUInt32("Port", DEFAULT_PORT);
    autoStart = archive->GetBool("AutoStart", DEFAULT_AUTO_START);

    auto count = archive->GetUInt32("ServersSize");
    for(uint32 i = 0; i < count; ++i)
    {
        ServerData sd;
        sd.ip = archive->GetString(Format("Server_%d_ip", i));
        sd.port = archive->GetUInt32(Format("Server_%d_port", i));
        sd.enabled = archive->GetBool(Format("Server_%d_enabled", i), false);

        remoteServers.push_back(sd);
    }
}


const FilePath & ApplicationSettings::GetFolder() const
{
    return folder;
}

void ApplicationSettings::SetFolder(const FilePath & _folder)
{
    folder = _folder;
}

const float64 ApplicationSettings::GetCacheSizeGb() const
{
    return cacheSizeGb;
}

void ApplicationSettings::SetCacheSizeGb(const float64 size)
{
    cacheSizeGb = size;
}

const uint32 ApplicationSettings::GetFilesCount() const
{
    return filesCount;
}

void ApplicationSettings::SetFilesCount(const uint32 count)
{
    filesCount = count;
}

const uint64 ApplicationSettings::GetAutoSaveTimeoutMin() const
{
    return autoSaveTimeoutMin;
}

void ApplicationSettings::SetAutoSaveTimeoutMin(const uint64 timeout)
{
    autoSaveTimeoutMin = timeout;
}

const uint16 ApplicationSettings::GetPort() const
{
    return listenPort;
}

void ApplicationSettings::SetPort(const uint16 val)
{
    listenPort = val;
}

const bool ApplicationSettings::IsAutoStart() const
{
    return autoStart;
}

void ApplicationSettings::SetAutoStart(bool val)
{
    autoStart = val;
}

const List<ServerData> & ApplicationSettings::GetServers() const
{
    return remoteServers;
}

void ApplicationSettings::ResetServers()
{
    remoteServers.clear();
}

void ApplicationSettings::AddServer(const ServerData & server)
{
    remoteServers.push_back(server);
}

void ApplicationSettings::RemoveServer(const ServerData & server)
{
    remoteServers.remove(server);
}

ServerData ApplicationSettings::GetCurrentServer() const
{
    for (auto& server : remoteServers)
    {
        if (server.enabled)
        {
            return server;
        }
    }

    return ServerData();
}



