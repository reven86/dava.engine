#pragma once

#include "AssetCache/ServerNetProxy.h"
#include "AssetCache/ClientNetProxy.h"

#include "AssetCacheHttpServer.h"
#include "ServerLogics.h"
#include "ApplicationSettings.h"
#include "SharedDataRequester.h"

#include <atomic>
#include <QObject>

class QTimer;

class ServerCore : public QObject,
                   public DAVA::AssetCache::ClientNetProxyListener,
                   public CacheDBOwner,
                   public AssetCacheHttpServerListener
{
    Q_OBJECT

    static const DAVA::uint32 UPDATE_INTERVAL_MS = 1;
    static const DAVA::uint32 CONNECT_TIMEOUT_SEC = 1;
    static const DAVA::uint32 CONNECT_REATTEMPT_WAIT_SEC = 5;
    static const DAVA::uint32 SHARED_UPDATE_INTERVAL_SEC = 1; // 5 * 60;

public:
    enum class State
    {
        STARTED,
        STOPPED
    };
    enum class RemoteState
    {
        STARTED,
        STOPPED,
        CONNECTING,
        VERIFYING,
        WAITING_REATTEMPT
    };

    ServerCore();
    ~ServerCore() override;

    ApplicationSettings& Settings();

    void Start();
    void Stop();

    State GetState() const;
    RemoteState GetRemoteState() const;

    void SetApplicationPath(const DAVA::String& path);

    void ClearStorage();
    void GetStorageSpaceUsage(DAVA::uint64& occupied, DAVA::uint64& overall) const;

    // ClientNetProxyListener
    void OnClientProxyStateChanged() override;
    void OnServerStatusReceived() override;
    void OnIncorrectPacketReceived(DAVA::AssetCache::IncorrectPacketType) override;

    // CacheDBOwner
    void OnStorageSizeChanged(DAVA::uint64 occupied, DAVA::uint64 overall) override;

    // AssetCacheHttpServerListener
    void OnStatusRequested(ClientID clientId) override;

signals:
    void ServerStateChanged(const ServerCore* serverCore) const;
    void StorageSizeChanged(DAVA::uint64 occupied, DAVA::uint64 overall) const;

public slots:
    void OnSettingsUpdated(const ApplicationSettings* settings);

private slots:
    void OnTimerUpdate();
    void OnConnectTimeout();
    void OnReattemptTimer();
    void OnSharedDataUpdateTimer();

private:
    void StartListening();
    void StopListening();

    bool ConnectRemote();
    bool VerifyRemote();
    void DisconnectRemote();
    void ReconnectRemoteLater();
    void UseNextRemote();
    void ResetRemotesList();

private:
    AssetCacheHttpServer httpServer;
    DAVA::AssetCache::ServerNetProxy serverProxy;
    DAVA::AssetCache::ClientNetProxy clientProxy;
    CacheDB dataBase;

    ServerLogics serverLogics;
    ApplicationSettings settings;

    std::atomic<State> state;
    std::atomic<RemoteState> remoteState;

    DAVA::String appPath;

    DAVA::Vector<RemoteServerParams> remoteServers;
    DAVA::int32 remoteServerIndex = -1;
    RemoteServerParams currentRemoteServer;

    SharedDataRequester sharedDataRequester;

    QTimer* updateTimer = nullptr;
    QTimer* connectTimer = nullptr;
    QTimer* reconnectWaitTimer = nullptr;
    QTimer* sharedDataUpdateTimer = nullptr;
};

inline ServerCore::State ServerCore::GetState() const
{
    return state;
}

inline ServerCore::RemoteState ServerCore::GetRemoteState() const
{
    return remoteState;
}

inline ApplicationSettings& ServerCore::Settings()
{
    return settings;
}
