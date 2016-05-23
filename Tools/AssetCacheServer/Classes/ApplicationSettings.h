#ifndef __APPLICATION_SETTINGS_H__
#define __APPLICATION_SETTINGS_H__

#include "AssetCache/AssetCacheConstants.h"
#include "FileSystem/FilePath.h"
#include "Network/Base/Endpoint.h"
#include <QObject>

namespace DAVA
{
class KeyedArchive;
};

using namespace DAVA;

struct ServerData
{
    ServerData() = default;
    ServerData(String _ip, uint16 _port, bool _enabled);

    bool IsEmpty() const;
    bool EquivalentTo(const DAVA::Net::Endpoint& right) const;

    bool operator==(const ServerData& right) const;
    bool operator<(const ServerData& right) const;

    String ip = "";
    uint16 port = AssetCache::ASSET_SERVER_PORT;
    bool enabled = false;
};

//TODO: we need one code for settings in different projects
//need to use introspection instead of hardcoding of values

class ApplicationSettings : public QObject
{
    Q_OBJECT

private:
    static const String DEFAULT_FOLDER;
    static const float64 DEFAULT_CACHE_SIZE_GB;
    static const uint32 DEFAULT_FILES_COUNT = 5;
    static const uint32 DEFAULT_AUTO_SAVE_TIMEOUT_MIN = 1;
    static const uint16 DEFAULT_PORT = DAVA::AssetCache::ASSET_SERVER_PORT;
    static const bool DEFAULT_AUTO_START = true;
    static const bool DEFAULT_LAUNCH_ON_SYSTEM_STARTUP = true;

public:
    void Save() const;
    void Load();

    bool IsFirstLaunch() const;

    const FilePath& GetFolder() const;
    void SetFolder(const FilePath& folder);

    const float64 GetCacheSizeGb() const;
    void SetCacheSizeGb(const float64 size);

    const uint32 GetFilesCount() const;
    void SetFilesCount(const uint32 count);

    const uint64 GetAutoSaveTimeoutMin() const;
    void SetAutoSaveTimeoutMin(const uint64 timeout);

    const uint16 GetPort() const;
    void SetPort(const uint16 port);

    const bool IsAutoStart() const;
    void SetAutoStart(bool);

    const bool IsLaunchOnSystemStartup() const;
    void SetLaunchOnSystemStartup(bool);

    const List<ServerData>& GetServers() const;
    void ResetServers();
    void AddServer(const ServerData& server);
    void RemoveServer(const ServerData& server);

    ServerData GetCurrentServer() const;

signals:
    void SettingsUpdated(const ApplicationSettings* settings) const;

private:
    void Serialize(DAVA::KeyedArchive* archieve) const;
    void Deserialize(DAVA::KeyedArchive* archieve);

public:
    FilePath folder = DEFAULT_FOLDER;
    float64 cacheSizeGb = DEFAULT_CACHE_SIZE_GB;
    uint32 filesCount = DEFAULT_FILES_COUNT;
    uint32 autoSaveTimeoutMin = DEFAULT_AUTO_SAVE_TIMEOUT_MIN;
    uint16 listenPort = DEFAULT_PORT;
    bool autoStart = DEFAULT_AUTO_START;
    bool launchOnSystemStartup = DEFAULT_LAUNCH_ON_SYSTEM_STARTUP;
    List<ServerData> remoteServers;

    bool isFirstLaunch = true;
};

inline bool ApplicationSettings::IsFirstLaunch() const
{
    return isFirstLaunch;
}

#endif // __APPLICATION_SETTINGS_H__
