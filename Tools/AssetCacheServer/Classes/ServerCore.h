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
