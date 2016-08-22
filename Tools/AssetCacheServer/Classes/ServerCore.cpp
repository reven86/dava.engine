#include "ServerCore.h"
#include "Platform/DeviceInfo.h"
#include "Utils/Utils.h"

#include <QTimer>

ServerCore::ServerCore()
    : httpServer(DAVA::Net::NetCore::Instance()->Loop())
    , dataBase(*this)
    , state(State::STOPPED)
    , remoteState(RemoteState::STOPPED)
{
    QObject::connect(&settings, &ApplicationSettings::SettingsUpdated, this, &ServerCore::OnSettingsUpdated);

    serverProxy.SetListener(&serverLogics);
    clientProxy.AddListener(&serverLogics);
    clientProxy.AddListener(this);
    httpServer.SetListener(this);

    DAVA::String serverName = DAVA::WStringToString(DAVA::DeviceInfo::GetName());
    serverLogics.Init(&serverProxy, serverName, &clientProxy, &dataBase);

    updateTimer = new QTimer(this);
    QObject::connect(updateTimer, &QTimer::timeout, this, &ServerCore::OnRefreshTimer);

    connectTimer = new QTimer(this);
    connectTimer->setInterval(CONNECT_TIMEOUT_SEC * 1000);
    connectTimer->setSingleShot(true);
    QObject::connect(connectTimer, &QTimer::timeout, this, &ServerCore::OnConnectTimeout);

    reconnectWaitTimer = new QTimer(this);
    reconnectWaitTimer->setInterval(CONNECT_REATTEMPT_WAIT_SEC * 1000);
    reconnectWaitTimer->setSingleShot(true);
    QObject::connect(reconnectWaitTimer, &QTimer::timeout, this, &ServerCore::OnReattemptTimer);

    sharedDataUpdateTimer = new QTimer(this);
    sharedDataUpdateTimer->setInterval(SHARED_UPDATE_INTERVAL_SEC * 1000);
    sharedDataUpdateTimer->setSingleShot(false);
    sharedDataUpdateTimer->start();
    QObject::connect(sharedDataUpdateTimer, &QTimer::timeout, this, &ServerCore::OnSharedDataUpdateTimer);
    QObject::connect(&sharedDataRequester, &SharedDataRequester::SharedDataReceived, this, &ServerCore::OnSharedDataReceived);
    QObject::connect(&sharedDataRequester, &SharedDataRequester::ServerShared, this, &ServerCore::OnServerShared);
    QObject::connect(&sharedDataRequester, &SharedDataRequester::ServerUnshared, this, &ServerCore::OnServerUnshared);

    settings.Load();

    ResetRemotesList();
    UseNextRemote();
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
    httpServer.Start(settings.GetHttpPort());
    state = State::STARTED;
}

void ServerCore::StopListening()
{
    state = State::STOPPED;
    serverProxy.Disconnect();
    httpServer.Stop();
}

bool ServerCore::ConnectRemote()
{
    DVASSERT(remoteState == RemoteState::STOPPED || remoteState == RemoteState::WAITING_REATTEMPT)

    if (!currentRemoteServer.IsEmpty())
    {
        bool created = clientProxy.Connect(currentRemoteServer.ip, DAVA::AssetCache::ASSET_SERVER_PORT);
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
    UseNextRemote();
    remoteState = RemoteState::WAITING_REATTEMPT;
    reconnectWaitTimer->start();
}

void ServerCore::UseNextRemote()
{
    if (remoteServerIndex == -1)
    {
        if (remoteServers.empty() == false)
        {
            remoteServerIndex = 0;
            currentRemoteServer = remoteServers.front();
        }
        else
        {
            currentRemoteServer.SetEmpty();
        }
    }
    else
    {
        DVASSERT(remoteServerIndex < remoteServers.size());
        if (++remoteServerIndex == remoteServers.size())
            remoteServerIndex = 0;
        currentRemoteServer = remoteServers[remoteServerIndex];
    }
}

void ServerCore::ResetRemotesList()
{
    DAVA::List<RemoteServerParams> remotes = settings.GetEnabledRemoteServers();
    remoteServers.assign(std::make_move_iterator(std::begin(remotes)), std::make_move_iterator(std::end(remotes)));
    remoteServerIndex = -1;
}

void ServerCore::OnRefreshTimer()
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

void ServerCore::OnSharedDataUpdateTimer()
{
    sharedDataRequester.RequestSharedData(settings.GetOwnID());
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

    bool remoteChanged = false;

    ResetRemotesList();
    auto found = std::find(remoteServers.begin(), remoteServers.end(), currentRemoteServer);
    if (found == remoteServers.end())
    {
        remoteChanged = true;
        UseNextRemote();
    }

    bool needServerRestart = false;
    bool needClientRestart = false;

    if (state == State::STARTED)
    { // disconnect network if settings changed
        if (serverProxy.GetListenPort() != settings.GetPort() || httpServer.GetListenPort() != settings.GetHttpPort())
        {
            needServerRestart = true;
            StopListening();
        }

        if ((remoteState != RemoteState::STOPPED && remoteChanged) ||
            (remoteState == RemoteState::STOPPED && !currentRemoteServer.IsEmpty()))
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
        ConnectRemote();
    }

    if (needServerRestart || needClientRestart)
    {
        emit ServerStateChanged(this);
        return;
    }
}

void ServerCore::InitiateShareRequest(PoolID poolID, const DAVA::String& serverName)
{
    SharedServerParams serverParams;
    serverParams.poolID = poolID;
    serverParams.name = serverName;
    serverParams.port = settings.GetPort();
    sharedDataRequester.AddSharedServer(serverParams, appPath);
}

void ServerCore::InitiateUnshareRequest()
{
    sharedDataRequester.RemoveSharedServer(settings.GetOwnID());
}

void ServerCore::OnServerShared(PoolID poolID, ServerID serverID, const DAVA::String& serverName)
{
    settings.SetOwnPoolID(poolID);
    settings.SetOwnID(serverID);
    settings.SetOwnName(serverName);
    settings.SetSharedForOthers(true);
    emit ServerShared();
}

void ServerCore::OnServerUnshared()
{
    settings.SetOwnID(0);
    settings.SetSharedForOthers(false);
    emit ServerUnshared();
}

void ServerCore::OnSharedDataReceived(const DAVA::List<SharedPoolParams>& pools, const DAVA::List<SharedServerParams>& servers)
{
    settings.UpdateSharedPools(pools, servers);
    emit SharedDataUpdated();
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

void ServerCore::OnStatusRequested(ClientID clientId)
{
    AssetServerStatus status;
    status.started = true;
    status.assetServerPath = appPath;
    httpServer.SendStatus(clientId, status);
}

void ServerCore::SetApplicationPath(const DAVA::String& path)
{
    appPath = path;
}
