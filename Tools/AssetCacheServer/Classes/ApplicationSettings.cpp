#include "ApplicationSettings.h"

#include "FileSystem/FileSystem.h"
#include "FileSystem/KeyedArchive.h"

using namespace DAVA;

RemoteServerParams::RemoteServerParams(String _ip, uint16 _port, bool _enabled)
    : ip(_ip)
    , port(_port)
    , enabled(_enabled)
{
}

bool RemoteServerParams::IsEmpty() const
{
    return ip.empty();
}

bool RemoteServerParams::operator==(const RemoteServerParams& right) const
{
    return (ip == right.ip) && (port == right.port);
}

bool RemoteServerParams::EquivalentTo(const DAVA::Net::Endpoint& right) const
{
    return (ip == right.Address().ToString()) && (port == right.Port());
}

bool RemoteServerParams::operator<(const RemoteServerParams& right) const
{
    if (ip == right.ip)
    {
        return port < right.port;
    }
    return ip < right.ip;
}

const String ApplicationSettings::DEFAULT_FOLDER = "~doc:/AssetServer/AssetCacheStorage";
const float64 ApplicationSettings::DEFAULT_CACHE_SIZE_GB = 5.0;

void ApplicationSettings::Save() const
{
    static FilePath path("~doc:/AssetServer/ACS_settings.dat");

    FileSystem::Instance()->CreateDirectory(path.GetDirectory(), true);

    ScopedPtr<File> file(File::Create(path, File::CREATE | File::WRITE));
    if (!file)
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
    if (file)
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

void ApplicationSettings::Serialize(DAVA::KeyedArchive* archive) const
{
    DVASSERT(nullptr != archive);

    archive->SetString("FolderPath", folder.GetStringValue());
    archive->SetFloat64("FolderSize", cacheSizeGb);
    archive->SetUInt32("NumberOfFiles", filesCount);
    archive->SetUInt32("AutoSaveTimeout", autoSaveTimeoutMin);
    archive->SetUInt32("Port", listenPort);
    archive->SetUInt32("HttpPort", listenHttpPort);
    archive->SetBool("AutoStart", autoStart);
    archive->SetBool("SystemStartup", launchOnSystemStartup);
    archive->SetBool("Restart", restartOnCrash);

    uint32 size = static_cast<uint32>(remoteServers.size());
    archive->SetUInt32("ServersSize", size);

    uint32 index = 0;
    for (auto& sd : remoteServers)
    {
        archive->SetString(Format("Server_%d_ip", index), sd.ip);
        archive->SetUInt32(Format("Server_%d_port", index), sd.port);
        archive->SetBool(Format("Server_%d_enabled", index), sd.enabled);
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
    listenPort = archive->GetUInt32("Port", DEFAULT_PORT);
    listenHttpPort = archive->GetUInt32("HttpPort", DEFAULT_HTTP_PORT);
    autoStart = archive->GetBool("AutoStart", DEFAULT_AUTO_START);
    launchOnSystemStartup = archive->GetBool("SystemStartup", DEFAULT_LAUNCH_ON_SYSTEM_STARTUP);
    restartOnCrash = archive->GetBool("Restart", DEFAULT_RESTART_ON_CRASH);

    auto count = archive->GetUInt32("ServersSize");
    for (uint32 i = 0; i < count; ++i)
    {
        RemoteServerParams sd;
        sd.ip = archive->GetString(Format("Server_%d_ip", i));
        sd.port = archive->GetUInt32(Format("Server_%d_port", i));
        sd.enabled = archive->GetBool(Format("Server_%d_enabled", i), false);

        remoteServers.push_back(sd);
    }
}

const FilePath& ApplicationSettings::GetFolder() const
{
    return folder;
}

void ApplicationSettings::SetFolder(const FilePath& _folder)
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

const uint16 ApplicationSettings::GetHttpPort() const
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

const List<RemoteServerParams>& ApplicationSettings::GetServers() const
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
