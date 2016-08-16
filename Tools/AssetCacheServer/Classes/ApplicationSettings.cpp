#include "ApplicationSettings.h"

#include "FileSystem/FileSystem.h"
#include "FileSystem/KeyedArchive.h"

RemoteServerParams::RemoteServerParams(DAVA::String _ip, bool _enabled)
    : ip(_ip)
    , enabled(_enabled)
{
}

bool RemoteServerParams::IsEmpty() const
{
    return ip.empty();
}

bool RemoteServerParams::operator==(const RemoteServerParams& right) const
{
    return (ip == right.ip);
}

bool RemoteServerParams::EquivalentTo(const DAVA::Net::Endpoint& right) const
{
    return (ip == right.Address().ToString());
}

const DAVA::String ApplicationSettings::DEFAULT_FOLDER = "~doc:/AssetServer/AssetCacheStorage";
const DAVA::float64 ApplicationSettings::DEFAULT_CACHE_SIZE_GB = 5.0;
const DAVA::uint32 ApplicationSettings::DEFAULT_FILES_COUNT = 5;
const DAVA::uint32 ApplicationSettings::DEFAULT_AUTO_SAVE_TIMEOUT_MIN = 1;
const DAVA::uint16 ApplicationSettings::DEFAULT_PORT = DAVA::AssetCache::ASSET_SERVER_PORT;
const DAVA::uint16 ApplicationSettings::DEFAULT_HTTP_PORT = DAVA::AssetCache::ASSET_SERVER_HTTP_PORT;
const bool ApplicationSettings::DEFAULT_AUTO_START = true;
const bool ApplicationSettings::DEFAULT_LAUNCH_ON_SYSTEM_STARTUP = true;
const bool ApplicationSettings::DEFAULT_RESTART_ON_CRASH = false;

void ApplicationSettings::Save() const
{
    static DAVA::FilePath path("~doc:/AssetServer/ACS_settings.dat");

    DAVA::FileSystem::Instance()->CreateDirectory(path.GetDirectory(), true);

    DAVA::ScopedPtr<DAVA::File> file(DAVA::File::Create(path, DAVA::File::CREATE | DAVA::File::WRITE));
    if (!file)
    {
        DAVA::Logger::Error("[ApplicationSettings::%s] Cannot create file %s", __FUNCTION__, path.GetStringValue().c_str());
        return;
    }

    DAVA::ScopedPtr<DAVA::KeyedArchive> archive(new DAVA::KeyedArchive());
    Serialize(archive);
    archive->Save(file);

    emit SettingsUpdated(this);
}

void ApplicationSettings::Load()
{
    static DAVA::FilePath path("~doc:/AssetServer/ACS_settings.dat");

    DAVA::ScopedPtr<DAVA::File> file(DAVA::File::Create(path, DAVA::File::OPEN | DAVA::File::READ));
    if (file)
    {
        isFirstLaunch = false;
        DAVA::ScopedPtr<DAVA::KeyedArchive> archive(new DAVA::KeyedArchive());
        archive->Load(file);
        Deserialize(archive);
    }
    else
    {
        isFirstLaunch = true;
    }

    emit SettingsUpdated(this);
}

void ApplicationSettings::Serialize(DAVA::KeyedArchive* archive) const
{
    DVASSERT(nullptr != archive);

    archive->SetString("FolderPath", folder.GetStringValue());
    archive->SetFloat64("FolderSize", cacheSizeGb);
    archive->SetUInt32("NumberOfFiles", filesCount);
    archive->SetUInt32("AutoSaveTimeout", autoSaveTimeoutMin);
    archive->SetBool("AutoStart", autoStart);
    archive->SetBool("SystemStartup", launchOnSystemStartup);
    archive->SetBool("Restart", restartOnCrash);

    DAVA::uint32 size = static_cast<DAVA::uint32>(remoteServers.size());
    archive->SetUInt32("ServersSize", size);

    DAVA::uint32 index = 0;
    for (auto& sd : remoteServers)
    {
        archive->SetString(DAVA::Format("Server_%d_ip", index), sd.ip);
        archive->SetBool(DAVA::Format("Server_%d_enabled", index), sd.enabled);
        ++index;
    }
}

void ApplicationSettings::Deserialize(DAVA::KeyedArchive* archive)
{
    DVASSERT(nullptr != archive);

    DVASSERT(remoteServers.empty());

    folder = archive->GetString("FolderPath", DEFAULT_FOLDER);
    cacheSizeGb = archive->GetFloat64("FolderSize", DEFAULT_CACHE_SIZE_GB);
    filesCount = archive->GetUInt32("NumberOfFiles", DEFAULT_FILES_COUNT);
    autoSaveTimeoutMin = archive->GetUInt32("AutoSaveTimeout", DEFAULT_AUTO_SAVE_TIMEOUT_MIN);
    autoStart = archive->GetBool("AutoStart", DEFAULT_AUTO_START);
    launchOnSystemStartup = archive->GetBool("SystemStartup", DEFAULT_LAUNCH_ON_SYSTEM_STARTUP);
    restartOnCrash = archive->GetBool("Restart", DEFAULT_RESTART_ON_CRASH);

    auto count = archive->GetUInt32("ServersSize");
    for (DAVA::uint32 i = 0; i < count; ++i)
    {
        RemoteServerParams sd;
        sd.ip = archive->GetString(DAVA::Format("Server_%d_ip", i));
        sd.enabled = archive->GetBool(DAVA::Format("Server_%d_enabled", i), false);

        remoteServers.push_back(sd);
    }
}

const DAVA::FilePath& ApplicationSettings::GetFolder() const
{
    return folder;
}

void ApplicationSettings::SetFolder(const DAVA::FilePath& _folder)
{
    folder = _folder;
}

const DAVA::float64 ApplicationSettings::GetCacheSizeGb() const
{
    return cacheSizeGb;
}

void ApplicationSettings::SetCacheSizeGb(const DAVA::float64 size)
{
    cacheSizeGb = size;
}

const DAVA::uint32 ApplicationSettings::GetFilesCount() const
{
    return filesCount;
}

void ApplicationSettings::SetFilesCount(const DAVA::uint32 count)
{
    filesCount = count;
}

const DAVA::uint64 ApplicationSettings::GetAutoSaveTimeoutMin() const
{
    return autoSaveTimeoutMin;
}

void ApplicationSettings::SetAutoSaveTimeoutMin(const DAVA::uint64 timeout)
{
    autoSaveTimeoutMin = timeout;
}

const DAVA::uint16 ApplicationSettings::GetPort() const
{
    return listenPort;
}

void ApplicationSettings::SetPort(const DAVA::uint16 val)
{
    listenPort = val;
}

const DAVA::uint16 ApplicationSettings::GetHttpPort() const
{
    return listenHttpPort;
}

void ApplicationSettings::SetHttpPort(const DAVA::uint16 port)
{
    listenHttpPort = port;
}

const bool ApplicationSettings::IsAutoStart() const
{
    return autoStart;
}

void ApplicationSettings::SetAutoStart(bool val)
{
    autoStart = val;
}

const bool ApplicationSettings::IsLaunchOnSystemStartup() const
{
    return launchOnSystemStartup;
}

void ApplicationSettings::SetLaunchOnSystemStartup(bool val)
{
    launchOnSystemStartup = val;
}

const bool ApplicationSettings::IsRestartOnCrash() const
{
    return restartOnCrash;
}

void ApplicationSettings::SetRestartOnCrash(bool val)
{
    restartOnCrash = val;
}

const DAVA::List<RemoteServerParams>& ApplicationSettings::GetServers() const
{
    return remoteServers;
}

void ApplicationSettings::ResetServers()
{
    remoteServers.clear();
}

void ApplicationSettings::AddServer(const RemoteServerParams& server)
{
    remoteServers.push_back(server);
}

void ApplicationSettings::RemoveServer(const RemoteServerParams& server)
{
    remoteServers.remove(server);
}

RemoteServerParams ApplicationSettings::GetCurrentServer() const
{
    for (auto& server : remoteServers)
    {
        if (server.enabled)
        {
            return server;
        }
    }

    return RemoteServerParams();
}
