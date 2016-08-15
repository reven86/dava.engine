#include "SharedDataRequester.h"
#include "Debug/DVAssert.h"

#include <QProcess>
#include <QNetworkAccessManager>
#include <QNetworkReply>

SharedDataRequester::SharedDataRequester(QWidget* parent)
    : networkManager(new QNetworkAccessManager(this))
{
    connect(networkManager, &QNetworkAccessManager::finished, this, &SharedDataRequester::Replied);
}

void SharedDataRequester::RequestSharedData(ServerID ownID_)
{
    DAVA::Logger::Debug("Requesting shared data");
    ownID = ownID_;
    if (getPoolsRequest)
    {
        getPoolsRequest->abort();
        getPoolsRequest->deleteLater();
    }

    pools.clear();

    if (getServersRequest)
    {
        getServersRequest->abort();
        getServersRequest->deleteLater();
        getServersRequest = nullptr;
    }

    QString s = QString("http://ba-manager.wargaming.net/panel/modules/jsonAPI/acs/api.php?cmd=getPools");
    DAVA::Logger::Debug("Sending request: %s", s.toStdString().c_str());
    getPoolsRequest = networkManager->get(QNetworkRequest(QUrl(s)));
    connect(getPoolsRequest, &QNetworkReply::finished, this, &SharedDataRequester::OnGetPoolsFinished);
}

void SharedDataRequester::OnGetPoolsFinished()
{
    DVASSERT(getPoolsRequest != nullptr);

    getPoolsRequest->deleteLater();
    QNetworkReply::NetworkError error = getPoolsRequest->error();
    if (error != QNetworkReply::NoError)
    {
        DAVA::Logger::Error("Can't get pools list: %s", getPoolsRequest->errorString().toStdString().c_str());
        return;
    }

    DAVA::Logger::Debug("Parsing pools reply");
    pools = SharedDataParser::ParsePoolsReply(getPoolsRequest->readAll());
    DAVA::Logger::Debug("done, %u pools parsed", pools.size());
    getPoolsRequest = nullptr;

    QString s = QString("http://ba-manager.wargaming.net/panel/modules/jsonAPI/acs/api.php?cmd=getShared&key=%1").arg(ownID ? QString::number(ownID) : "NULL");

    DAVA::Logger::Debug("Sending request: %s", s.toStdString().c_str());
    getServersRequest = networkManager->get(QNetworkRequest(QUrl(s)));
    connect(getServersRequest, &QNetworkReply::finished, this, &SharedDataRequester::OnGetServersFinished);
}

void SharedDataRequester::OnGetServersFinished()
{
    DVASSERT(getServersRequest != nullptr);
    getServersRequest->deleteLater();

    QNetworkReply::NetworkError error = getServersRequest->error();
    if (error != QNetworkReply::NoError)
    {
        DAVA::Logger::Error("Can't get servers list: %s", getServersRequest->errorString().toStdString().c_str());
    }

    DAVA::Logger::Debug("Parsing servers reply");
    DAVA::List<SharedServerParams> servers = SharedDataParser::ParseServersReply(getServersRequest->readAll());
    DAVA::Logger::Debug("done, %u servers parsed", servers.size());
    getServersRequest = nullptr;

    emit SharedDataReceived(pools, servers);
}

void SharedDataRequester::AddSharedServer(SharedServerParams& serverParams)
{
    QString s = QString("http://ba-manager.wargaming.net/panel/modules/jsonAPI/acs/api.php?cmd=share&&port=%u&name=%s&poolKey=%u").arg(serverParams.port).arg(serverParams.name.c_str()).arg(serverParams.poolID);
    DAVA::Logger::Debug("Sending request: %s", s.toStdString().c_str());
    shareRequest = networkManager->get(QNetworkRequest(QUrl(s)));
    connect(shareRequest, &QNetworkReply::finished, this, &SharedDataRequester::OnAddServerFinished);
}

void SharedDataRequester::OnAddServerFinished()
{
    DVASSERT(shareRequest != nullptr);
    shareRequest->deleteLater();

    QNetworkReply::NetworkError error = shareRequest->error();
    if (error != QNetworkReply::NoError)
    {
        DAVA::Logger::Error("Can't add server: %s", getServersRequest->errorString().toStdString().c_str());
        return;
    }

    emit SharedIDReceived(SharedDataParser::ParseAddReply());
}

void SharedDataRequester::RemoveSharedServer(ServerID serverID)
{
    QString s = QString("http://ba-manager.wargaming.net/panel/modules/jsonAPI/acs/api.php?cmd=unshare&key=%u").arg(serverID);
    DAVA::Logger::Debug("Sending request: %s", s.toStdString().c_str());
    unshareRequest = networkManager->get(QNetworkRequest(QUrl(s)));
    connect(unshareRequest, &QNetworkReply::finished, this, &SharedDataRequester::OnRemoveServerFinished);
}

void SharedDataRequester::OnRemoveServerFinished()
{
    DVASSERT(unshareRequest != nullptr);
    unshareRequest->deleteLater();

    QNetworkReply::NetworkError error = unshareRequest->error();
    if (error != QNetworkReply::NoError)
    {
        DAVA::Logger::Error("Can't remove server");
        return;
    }

    emit UnshareReceived();
}
