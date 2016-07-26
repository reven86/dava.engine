#pragma once

#include "AssetCache/AssetCacheConstants.h"
#include "FileSystem/FilePath.h"
#include "Network/Base/Endpoint.h"
#include <QObject>

namespace DAVA
{
class KeyedArchive;
};

struct RemoteServerParams
{
    RemoteServerParams() = default;
    RemoteServerParams(DAVA::String _ip, DAVA::uint16 _port, bool _enabled);

    bool IsEmpty() const;
    bool EquivalentTo(const DAVA::Net::Endpoint& right) const;

    bool operator==(const RemoteServerParams& right) const;
    bool operator<(const RemoteServerParams& right) const;

    DAVA::String ip = "";
    DAVA::uint16 port = DAVA::AssetCache::ASSET_SERVER_PORT;
    bool enabled = false;
};

//TODO: we need one code for settings in different projects
//need to use introspection instead of hardcoding of values

class ApplicationSettings : public QObject
{
    Q_OBJECT

private:
    static const DAVA::String DEFAULT_FOLDER;
    static const DAVA::float64 DEFAULT_CACHE_SIZE_GB;
    static const DAVA::uint32 DEFAULT_FILES_COUNT = 5;
    static const DAVA::uint32 DEFAULT_AUTO_SAVE_TIMEOUT_MIN = 1;
    static const DAVA::uint16 DEFAULT_PORT = DAVA::AssetCache::ASSET_SERVER_PORT;
    static const DAVA::uint16 DEFAULT_HTTP_PORT = DAVA::AssetCache::ASSET_SERVER_HTTP_PORT;
    static const bool DEFAULT_AUTO_START = true;
    static const bool DEFAULT_LAUNCH_ON_SYSTEM_STARTUP = true;
    static const bool DEFAULT_RESTART_ON_CRASH = false;

public:
    void Save() const;
    void Load();

    bool IsFirstLaunch() const;

    const DAVA::FilePath& GetFolder() const;
    void SetFolder(const DAVA::FilePath& folder);

    const DAVA::float64 GetCacheSizeGb() const;
    void SetCacheSizeGb(const DAVA::float64 size);

    const DAVA::uint32 GetFilesCount() const;
    void SetFilesCount(const DAVA::uint32 count);

    const DAVA::uint64 GetAutoSaveTimeoutMin() const;
    void SetAutoSaveTimeoutMin(const DAVA::uint64 timeout);

    const DAVA::uint16 GetPort() const;
    void SetPort(const DAVA::uint16 port);

    const DAVA::uint16 GetHttpPort() const;
    void SetHttpPort(const DAVA::uint16 port);

    const bool IsAutoStart() const;
    void SetAutoStart(bool);

    const bool IsLaunchOnSystemStartup() const;
    void SetLaunchOnSystemStartup(bool);

    const bool IsRestartOnCrash() const;
    void SetRestartOnCrash(bool);

    const DAVA::List<RemoteServerParams>& GetServers() const;
    void ResetServers();
    void AddServer(const RemoteServerParams& server);
    void RemoveServer(const RemoteServerParams& server);

    RemoteServerParams GetCurrentServer() const;

signals:
    void SettingsUpdated(const ApplicationSettings* settings) const;

private:
    void Serialize(DAVA::KeyedArchive* archieve) const;
    void Deserialize(DAVA::KeyedArchive* archieve);

public:
    DAVA::FilePath folder = DEFAULT_FOLDER;
    DAVA::float64 cacheSizeGb = DEFAULT_CACHE_SIZE_GB;
    DAVA::uint32 filesCount = DEFAULT_FILES_COUNT;
    DAVA::uint32 autoSaveTimeoutMin = DEFAULT_AUTO_SAVE_TIMEOUT_MIN;
    DAVA::uint16 listenPort = DEFAULT_PORT;
    DAVA::uint16 listenHttpPort = DEFAULT_HTTP_PORT;
    bool autoStart = DEFAULT_AUTO_START;
    bool launchOnSystemStartup = DEFAULT_LAUNCH_ON_SYSTEM_STARTUP;
    bool restartOnCrash = DEFAULT_RESTART_ON_CRASH;
    DAVA::List<RemoteServerParams> remoteServers;

    bool isFirstLaunch = true;
};

inline bool ApplicationSettings::IsFirstLaunch() const
{
    return isFirstLaunch;
}
