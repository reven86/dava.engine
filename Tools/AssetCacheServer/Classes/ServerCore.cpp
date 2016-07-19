#include "ServerCore.h"
#include "Platform/DeviceInfo.h"
#include "Utils/Utils.h"

#include <QTimer>

ServerCore::ServerCore()
    : dataBase(*this)
    , state(State::STOPPED)
    , remoteState(RemoteState::STOPPED)
{
    QObject::connect(&settings, &ApplicationSettings::SettingsUpdated, this, &ServerCore::OnSettingsUpdated);

    serverProxy.SetDelegate(&serverLogics);
    clientProxy.AddListener(&serverLogics);
    clientProxy.AddListener(this);

    DAVA::String serverName = DAVA::WStringToString(DAVA::DeviceInfo::GetName());
    serverLogics.Init(&serverProxy, serverName, &clientProxy, &dataBase);

    updateTimer = new QTimer(this);
    QObject::connect(updateTimer, &QTimer::timeout, this, &ServerCore::OnTimerUpdate);

    connectTimer = new QTimer(this);
    connectTimer->setInterval(CONNECT_TIMEOUT_SEC * 1000);
    connectTimer->setSingleShot(true);
    QObject::connect(connectTimer, &QTimer::timeout, this, &ServerCore::OnConnectTimeout);

    reconnectWaitTimer = new QTimer(this);
    reconnectWaitTimer->setInterval(CONNECT_REATTEMPT_WAIT_SEC * 1000);
    reconnectWaitTimer->setSingleShot(true);
    QObject::connect(reconnectWaitTimer, &QTimer::timeout, this, &ServerCore::OnReattemptTimer);

    settings.Load();
}

ServerCore::~ServerCore()
{
    Stop();
}

void ServerCore::Start()
{
    if (state != State::STARTED)
    {
        StartListening();

        remoteServerData = settings.GetCurrentServer();
        ConnectRemote();

        updateTimer->start(UPDATE_INTERVAL_MS);

        state = State::STARTED;
        emit ServerStateChanged(this);
    }
}

void ServerCore::Stop()
{
    if (state != State::STOPPED)
    {
        updateTimer->stop();

        StopListening();
        DisconnectRemote();

        emit ServerStateChanged(this);
    }
}

void ServerCore::StartListening()
{
    DVASSERT(state == State::STOPPED);

    serverProxy.Listen(settings.GetPort());
    state = State::STARTED;
}

void ServerCore::StopListening()
{
    state = State::STOPPED;
    serverProxy.Disconnect();
}

bool ServerCore::ConnectRemote()
{
    DVASSERT(remoteState == RemoteState::STOPPED || remoteState == RemoteState::WAITING_REATTEMPT)

    if (!remoteServerData.ip.empty())
    {
        bool created = clientProxy.Connect(remoteServerData.ip, remoteServerData.port);
        if (created)
        {
            connectTimer->start();
            remoteState = RemoteState::CONNECTING;
            return true;
        }
    }

    return false;
}

bool ServerCore::VerifyRemote()
{
    if (clientProxy.RequestServerStatus())
    {
        connectTimer->start();
        remoteState = RemoteState::VERIFYING;
        return true;
    }

    return false;
}

void ServerCore::DisconnectRemote()
{
    connectTimer->stop();
    reconnectWaitTimer->stop();
    remoteState = RemoteState::STOPPED;
    clientProxy.Disconnect();
}

void ServerCore::ReconnectRemoteLater()
{
    DisconnectRemote();
    remoteState = RemoteState::WAITING_REATTEMPT;
    reconnectWaitTimer->start();
}

void ServerCore::OnTimerUpdate()
{
    serverLogics.Update();

    auto netSystem = DAVA::Net::NetCore::Instance();
    if (netSystem)
    {
        netSystem->Poll();
    }
}

void ServerCore::OnConnectTimeout()
{
    ReconnectRemoteLater();
    emit ServerStateChanged(this);
}

void ServerCore::OnReattemptTimer()
{
    ConnectRemote();
    emit ServerStateChanged(this);
}

void ServerCore::OnClientProxyStateChanged()
{
    DVASSERT(remoteState != RemoteState::STOPPED);

    if (clientProxy.ChannelIsOpened())
    {
        DVASSERT(remoteState == RemoteState::CONNECTING);
        connectTimer->stop();
        reconnectWaitTimer->stop();
        VerifyRemote();
    }
    else
    {
        connectTimer->start();
        reconnectWaitTimer->stop();
        remoteState = RemoteState::CONNECTING;
    }

    emit ServerStateChanged(this);
}

void ServerCore::OnServerStatusReceived()
{
    DVASSERT(remoteState == RemoteState::VERIFYING);

    connectTimer->stop();
    reconnectWaitTimer->stop();
    remoteState = RemoteState::STARTED;

    emit ServerStateChanged(this);
}

void ServerCore::OnIncorrectPacketReceived(DAVA::AssetCache::IncorrectPacketType)
{
    DVASSERT(remoteState != RemoteState::STOPPED);
    ReconnectRemoteLater();
    emit ServerStateChanged(this);
}

void ServerCore::OnSettingsUpdated(const ApplicationSettings* _settings)
{
    DVASSERT(&settings == _settings);

    auto& folder = settings.GetFolder();
    auto sizeGb = settings.GetCacheSizeGb();
    auto count = settings.GetFilesCount();
    auto autoSaveTimeoutMin = settings.GetAutoSaveTimeoutMin();
    const auto remoteServer = settings.GetCurrentServer();

    bool needServerRestart = false;
    bool needClientRestart = false;

    if (state == State::STARTED)
    { // disconnect network if settings changed
        if (serverProxy.GetListenPort() != settings.GetPort())
        {
            needServerRestart = true;
            StopListening();
        }

        if ((remoteState != RemoteState::STOPPED && !(remoteServerData == remoteServer)) ||
            (remoteState == RemoteState::STOPPED && !remoteServer.ip.empty()))
        {
            needClientRestart = true;
            DisconnectRemote();
        }
    }

    //updated DB settings
    if (sizeGb && !folder.IsEmpty())
    {
        dataBase.UpdateSettings(folder, sizeGb * 1024 * 1024 * 1024, count, autoSaveTimeoutMin * 60 * 1000);
    }
    else
    {
        DAVA::Logger::Warning("[ServerCore::%s] Empty settings", __FUNCTION__);
    }

    // restart network connections after changing of settings
    if (needServerRestart)
    {
        StartListening();
    }

    if (state == State::STARTED && needClientRestart)
    {
        remoteServerData = remoteServer;
        ConnectRemote();
    }

    if (needServerRestart || needClientRestart)
    {
        emit ServerStateChanged(this);
        return;
    }
}

void ServerCore::ClearStorage()
{
    dataBase.ClearStorage();
}

void ServerCore::GetStorageSpaceUsage(DAVA::uint64& occupied, DAVA::uint64& overall) const
{
    occupied = dataBase.GetOccupiedSize();
    overall = dataBase.GetStorageSize();
}

void ServerCore::OnStorageSizeChanged(DAVA::uint64 occupied, DAVA::uint64 overall)
{
    emit StorageSizeChanged(occupied, overall);
}
