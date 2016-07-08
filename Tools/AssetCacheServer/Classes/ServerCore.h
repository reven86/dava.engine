#ifndef __SERVER_CORE_H__
#define __SERVER_CORE_H__

#include "AssetCache/AssetCache.h"
#include "AssetCache/ClientNetProxy.h"
#include "ServerLogics.h"
#include "ApplicationSettings.h"

#include <atomic>

#include <QObject>

class QTimer;

class ServerCore : public QObject,
                   public DAVA::AssetCache::ClientNetProxyListener
{
    Q_OBJECT

    static const uint32 UPDATE_INTERVAL_MS = 1;
    static const uint32 CONNECT_TIMEOUT_SEC = 1;
    static const uint32 CONNECT_REATTEMPT_WAIT_SEC = 5;

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
        WAITING_REATTEMPT
    };

    ServerCore();
    ~ServerCore() override;

    ApplicationSettings& Settings();

    void Start();
    void Stop();

    State GetState() const;
    RemoteState GetRemoteState() const;

    // ClientNetProxyListener
    virtual void OnAssetClientStateChanged() override;

signals:
    void ServerStateChanged(const ServerCore* serverCore) const;

public slots:
    void OnSettingsUpdated(const ApplicationSettings* settings);

private slots:
    void OnTimerUpdate();
    void OnConnectTimeout();
    void OnReattemptTimer();

private:
    void StartListening();
    void StopListening();

    bool ConnectRemote();
    void DisconnectRemote();

private:
    DAVA::AssetCache::ServerNetProxy server;
    DAVA::AssetCache::ClientNetProxy client;
    DAVA::AssetCache::CacheDB dataBase;

    ServerLogics serverLogics;
    ApplicationSettings settings;

    std::atomic<State> state;
    std::atomic<RemoteState> remoteState;

    ServerData remoteServerData;

    QTimer* updateTimer;
    QTimer* connectTimer;
    QTimer* reattemptWaitTimer;
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

#endif // __SERVER_CORE_H__
