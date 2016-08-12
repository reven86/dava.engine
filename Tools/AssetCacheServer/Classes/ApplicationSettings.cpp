#include "ApplicationSettings.h"

#include "FileSystem/FileSystem.h"
#include "FileSystem/KeyedArchive.h"

RemoteServerParams::RemoteServerParams(DAVA::String _ip, DAVA::uint16 _port, bool _enabled)
    : ip(_ip)
    , port(_port)
    , enabled(_enabled)
{
}

void RemoteServerParams::SetEmpty()
{
    ip.clear();
}

bool RemoteServerParams::IsEmpty() const
{
    return ip.empty();
}

bool RemoteServerParams::operator==(const RemoteServerParams& right) const
{
    return (ip == right.ip) && (port == right.port);
}

bool RemoteServerParams::operator<(const RemoteServerParams& right) const
{
    if (ip == right.ip)
    {
        return port < right.port;
    }
    return ip < right.ip;
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
const bool ApplicationSettings::DEFAULT_SHARED_FOR_OTHERS = false;

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
    archive->SetUInt32("Port", listenPort);
    archive->SetUInt32("HttpPort", listenHttpPort);
    archive->SetBool("AutoStart", autoStart);
    archive->SetBool("SystemStartup", launchOnSystemStartup);
    archive->SetBool("Restart", restartOnCrash);

    DAVA::uint32 size = static_cast<DAVA::uint32>(customServers.size());
    archive->SetUInt32("ServersSize", size);

    DAVA::uint32 index = 0;
    for (auto& pool : customServers)
    {
        archive->SetString(DAVA::Format("Server_%u_ip", index), pool.ip);
        archive->SetUInt32(DAVA::Format("Server_%u_port", index), pool.port);
        archive->SetBool(DAVA::Format("Server_%u_enabled", index), pool.enabled);
        ++index;
    }

    DAVA::uint32 poolsSize = static_cast<DAVA::uint32>(sharedPools.size());
    archive->SetUInt32("PoolsSize", poolsSize);

    DAVA::uint32 poolIndex = 0;
    for (auto& poolEntry : sharedPools)
    {
        const SharedPool& pool = poolEntry.second;
        archive->SetUInt32(DAVA::Format("Pool_%u_ID", poolIndex), pool.poolID);
        archive->SetString(DAVA::Format("Pool_%u_name", poolIndex), pool.poolName);
        archive->SetString(DAVA::Format("Pool_%u_description", poolIndex), pool.poolDescription);
        archive->SetBool(DAVA::Format("Pool_%u_enabled", poolIndex), pool.enabled);

        DAVA::uint32 serversSize = static_cast<DAVA::uint32>(pool.servers.size());
        archive->SetUInt32(DAVA::Format("Pool_%u_ServersSize", poolIndex), serversSize);

        DAVA::uint32 serverIndex = 0;
        for (auto& serverEntry : pool.servers)
        {
            const SharedServer& server = serverEntry.second;
            archive->SetUInt32(DAVA::Format("Pool_%u_Server_%u_ID", poolIndex, serverIndex), server.serverID);
            archive->SetString(DAVA::Format("Pool_%u_Server_%u_name", poolIndex, serverIndex), server.serverName);
            archive->SetString(DAVA::Format("Pool_%u_Server_%u_ip", poolIndex, serverIndex), server.remoteParams.ip);
            archive->SetUInt32(DAVA::Format("Pool_%u_Server_%u_port", poolIndex, serverIndex), server.remoteParams.port);
            archive->SetBool(DAVA::Format("Pool_%u_Server_%u_enabled", poolIndex, serverIndex), server.remoteParams.enabled);
            ++serverIndex;
        }

        ++poolIndex;
    }
}

void ApplicationSettings::Deserialize(DAVA::KeyedArchive* archive)
{
    DVASSERT(nullptr != archive);

    DVASSERT(customServers.empty());

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
    for (DAVA::uint32 poolIndex = 0; poolIndex < count; ++poolIndex)
    {
        RemoteServerParams sd;
        sd.ip = archive->GetString(DAVA::Format("Server_%d_ip", poolIndex));
        sd.port = archive->GetUInt32(DAVA::Format("Server_%d_port", poolIndex));
        sd.enabled = archive->GetBool(DAVA::Format("Server_%d_enabled", poolIndex), false);

        customServers.push_back(sd);
    }

    DAVA::uint32 poolsSize = archive->GetUInt32("PoolsSize");

    for (DAVA::uint32 poolIndex = 0; poolIndex < poolsSize; ++poolIndex)
    {
        DAVA::uint32 poolID = archive->GetUInt32(DAVA::Format("Pool_%u_ID", poolIndex));
        SharedPool& pool = sharedPools[poolID];
        pool.poolID = poolID;
        pool.poolName = archive->GetString(DAVA::Format("Pool_%u_name", poolIndex));
        pool.poolDescription = archive->GetString(DAVA::Format("Pool_%u_description", poolIndex));
        pool.enabled = archive->GetBool(DAVA::Format("Pool_%u_enabled", poolIndex));

        DAVA::uint32 serversSize = archive->GetUInt32(DAVA::Format("Pool_%u_ServersSize", poolIndex));

        for (DAVA::uint32 serverIndex = 0; serverIndex < serversSize; ++serverIndex)
        {
            DAVA::uint32 serverID = archive->GetUInt32(DAVA::Format("Pool_%u_Server_%u_ID", poolIndex, serverIndex));
            SharedServer& server = pool.servers[serverID];
            server.serverID = serverID;
            server.poolID = poolID;
            server.serverName = archive->GetString(DAVA::Format("Pool_%u_Server_%u_name", poolIndex, serverIndex));
            server.remoteParams.ip = archive->GetString(DAVA::Format("Pool_%u_Server_%u_ip", poolIndex, serverIndex));
            server.remoteParams.port = archive->GetUInt32(DAVA::Format("Pool_%u_Server_%u_port", poolIndex, serverIndex));
            server.remoteParams.enabled = archive->GetBool(DAVA::Format("Pool_%u_Server_%u_enabled", poolIndex, serverIndex));
        }
    }

    //     SharedPool& pool1 = sharedPools[1];
    //     pool1.enabled = true;
    //     pool1.poolID = 1;
    //     pool1.poolName = "dava team";
    //     pool1.poolDescription = "used by dava dev & QA";
    //
    //     SharedServer& server1 = pool1.servers[1];
    //     server1.poolID = 1;
    //     server1.serverID = 1;
    //     server1.serverName = "Stas";
    //     server1.remoteParams.ip = "127.0.0.1";
    //     server1.remoteParams.port = 44236;
    //
    //     SharedServer& server2 = pool1.servers[2];
    //     server2.poolID = 1;
    //     server2.serverID = 2;
    //     server2.serverName = "Stas mac";
    //     server2.remoteParams.ip = "127.0.0.1";
    //     server2.remoteParams.port = 44234;
    //
    //     SharedPool& pool7 = sharedPools[7];
    //     pool7.enabled = false;
    //     pool7.poolID = 7;
    //     pool7.poolName = "blitz team";
    //     pool7.poolDescription = "used by blitz dev & QA";
    //
    //     SharedServer& server71 = pool7.servers[71];
    //     server71.poolID = 7;
    //     server71.serverID = 71;
    //     server71.serverName = "M";
    //     server71.remoteParams.ip = "127.0.0.1";
    //
    //     SharedPool& pool0 = sharedPools[0];
    //     pool0.enabled = false;
    //     pool0.poolID = 0;
    //     pool0.poolName = "void team";
    //     pool0.poolDescription = "bla bla";
    //
    //     SharedServer& server99 = pool0.servers[99];
    //     server99.poolID = 0;
    //     server99.serverID = 99;
    //     server99.serverName = "Free server";
    //     server99.remoteParams.ip = "127.0.0.9";
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

bool ApplicationSettings::IsSharedForOthers() const
{
    return sharedForOthers;
}

void ApplicationSettings::SetSharedForOthers(bool val)
{
    sharedForOthers = val;
}

ServerID ApplicationSettings::OwnID() const
{
    return ownID;
}

void ApplicationSettings::SetOwnID(ServerID val)
{
    ownID = val;
}

const DAVA::Map<PoolID, SharedPool>& ApplicationSettings::GetSharedPools() const
{
    return sharedPools;
}

void ApplicationSettings::ClearCustomServers()
{
    customServers.clear();
}

const DAVA::List<RemoteServerParams>& ApplicationSettings::GetCustomServers() const
{
    return customServers;
}

// void ApplicationSettings::AddSharedPool(const SharedPool& pool)
// {
//     auto insertedResult = sharedPools.emplace(pool.poolID, pool);
//     DVASSERT(insertedResult.second == true);
// }
//
// void ApplicationSettings::AddSharedServer(const SharedServer& server)
// {
//     auto poolIter = sharedPools.find(server.poolID);
//     DVASSERT(poolIter != sharedPools.end());
//     SharedPool& pool = poolIter->second;
//
//     auto insertedResult = pool.servers.emplace(server.serverID, server);
//     DVASSERT(insertedResult.second == true);
// }
//

void ApplicationSettings::AddCustomServer(const RemoteServerParams& server)
{
    customServers.push_back(server);
}

//
// void ApplicationSettings::RemoveSharedPool(const SharedPool& pool)
// {
//     size_t erasedCount = sharedPools.erase(pool.poolID);
//     DVASSERT(erasedCount == 1);
// }
//
// void ApplicationSettings::RemoveSharedServer(const SharedServer& server)
// {
//     auto poolIter = sharedPools.find(server.poolID);
//     DVASSERT(poolIter != sharedPools.end());
//     SharedPool& pool = poolIter->second;
//
//     size_t erasedCount = pool.servers.erase(server.serverID);
//     DVASSERT(erasedCount == 1);
// }

void ApplicationSettings::UpdateSharedPools(const DAVA::List<SharedPoolParams>& pools, const DAVA::List<SharedServerParams>& servers)
{
    DAVA::Map<PoolID, SharedPool> updatedPools;

    updatedPools[0];
    for (const SharedPoolParams& pool : pools)
    {
        SharedPool& updatedPool = updatedPools[pool.poolID];
        updatedPool.poolID = pool.poolID;
        updatedPool.poolName = pool.name;
        updatedPool.poolDescription = pool.description;
    }

    for (const SharedServerParams& server : servers)
    {
        auto& poolIter = updatedPools.find(server.poolID);
        if (poolIter != updatedPools.end())
        {
            SharedServer& updatedServer = poolIter->second.servers[server.serverID];
            updatedServer.serverID = server.serverID;
            updatedServer.poolID = server.poolID;
            updatedServer.remoteParams.ip = server.ip;
            updatedServer.remoteParams.port = server.port;
        }
        else
        {
            DAVA::Logger::Error("Can't find pool with id %u referenced by server id %u", server.poolID, server.serverID);
        }
    }

    EnabledRemote currentEnabledRemote = GetEnabledRemote();
    if (currentEnabledRemote.type == EnabledRemote::POOL)
    {
        auto& poolIter = updatedPools.find(currentEnabledRemote.pool->poolID);
        if (poolIter != updatedPools.end())
        {
            SharedPool& pool = poolIter->second;
            pool.enabled = true;
        }
    }
    else if (currentEnabledRemote.type == EnabledRemote::POOL_SERVER)
    {
        auto& poolIter = updatedPools.find(currentEnabledRemote.server->poolID);
        if (poolIter != updatedPools.end())
        {
            SharedPool& pool = poolIter->second;
            auto& serverIter = pool.servers.find(currentEnabledRemote.server->serverID);
            if (serverIter != pool.servers.end())
            {
                SharedServer& server = serverIter->second;
                server.remoteParams.enabled = true;
            }
        }
    }

    if (updatedPools[0].servers.empty())
    {
        updatedPools.erase(0);
    }

    sharedPools.swap(updatedPools);
    emit SettingsUpdated(this);
}

void ApplicationSettings::RemoveCustomServer(const RemoteServerParams& server)
{
    customServers.remove(server);
}

EnabledRemote ApplicationSettings::GetEnabledRemote()
{
    for (auto& poolIter : sharedPools)
    {
        SharedPool& pool = poolIter.second;
        if (pool.enabled)
            return EnabledRemote(&pool);

        for (auto& serverIter : pool.servers)
        {
            SharedServer& server = serverIter.second;
            if (server.remoteParams.enabled)
                return EnabledRemote(&server);
        }
    }

    for (RemoteServerParams& customServer : customServers)
    {
        if (customServer.enabled)
            return EnabledRemote(&customServer);
    }

    return EnabledRemote();
}

DAVA::List<RemoteServerParams> ApplicationSettings::GetEnabledRemoteServers()
{
    DAVA::List<RemoteServerParams> enabledRemotesParams;

    EnabledRemote enabledRemote = GetEnabledRemote();
    switch (enabledRemote.type)
    {
    case EnabledRemote::POOL:
    {
        for (auto serverIter : enabledRemote.pool->servers)
        {
            SharedServer& server = serverIter.second;
            enabledRemotesParams.push_back(server.remoteParams);
        }
        break;
    }
    case EnabledRemote::POOL_SERVER:
    {
        enabledRemotesParams.push_back(enabledRemote.server->remoteParams);
        break;
    }
    case EnabledRemote::CUSTOM_SERVER:
    {
        enabledRemotesParams.push_back(*(enabledRemote.customServer));
        break;
    }
    case EnabledRemote::NONE:
    default:
        break;
    }

    return enabledRemotesParams;
}

void ApplicationSettings::DisableRemote()
{
    EnabledRemote enabledRemote = GetEnabledRemote();
    switch (enabledRemote.type)
    {
    case EnabledRemote::POOL:
        enabledRemote.pool->enabled = false;
        break;
    case EnabledRemote::POOL_SERVER:
        enabledRemote.server->remoteParams.enabled = false;
        break;
    case EnabledRemote::CUSTOM_SERVER:
        enabledRemote.customServer->enabled = false;
        break;
    default:
        break;
    }
}

void ApplicationSettings::EnableSharedPool(PoolID poolID)
{
    DVASSERT(GetEnabledRemote().type == EnabledRemote::NONE);

    auto& pairFound = sharedPools.find(poolID);
    if (pairFound != sharedPools.end())
    {
        SharedPool& pool = pairFound->second;
        pool.enabled = true;
    }
    else
    {
        DVASSERT_MSG(false, DAVA::Format("Can't find pool with id %u", poolID).c_str());
    }
}

void ApplicationSettings::EnableSharedServer(PoolID poolID, ServerID serverID)
{
    DVASSERT(GetEnabledRemote().type == EnabledRemote::NONE);

    auto& pairFound = sharedPools.find(poolID);
    if (pairFound != sharedPools.end())
    {
        SharedPool& pool = pairFound->second;
        auto& serverPairFound = pool.servers.find(serverID);
        if (serverPairFound != pool.servers.end())
        {
            SharedServer& server = serverPairFound->second;
            server.remoteParams.enabled = true;
        }
        else
        {
            DVASSERT_MSG(false, DAVA::Format("Can't find server with id %u inside of pool %u", serverID, poolID).c_str());
        }
    }
    else
    {
        DVASSERT_MSG(false, DAVA::Format("Can't find pool with id %u", poolID).c_str());
    }
}
