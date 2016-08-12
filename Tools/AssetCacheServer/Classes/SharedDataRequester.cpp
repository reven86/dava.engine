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

void SharedDataRequester::RequestSharedData(ServerID ownID)
{
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
    }

    QString s = QString("http://ba-manager.wargaming.net/panel/modules/jsonAPI/acs/api.php?cmd=getPools");
    getPoolsRequest = networkManager->get(QNetworkRequest(QUrl(s)));
    connect(getPoolsRequest, &QNetworkReply::finished, this, &SharedDataRequester::OnGetPoolsFinished);

    s = QString("http://ba-manager.wargaming.net/panel/modules/jsonAPI/acs/api.php?cmd=getShared&key=%1").arg(ownID ? QString::number(ownID) : "NULL");
    getServersRequest = networkManager->get(QNetworkRequest(QUrl(s)));
    connect(getPoolsRequest, &QNetworkReply::finished, this, &SharedDataRequester::OnGetServersFinished);
}

void SharedDataRequester::OnGetPoolsFinished()
{
    DVASSERT(getPoolsRequest != nullptr);
    DVASSERT(getServersRequest != nullptr);

    getPoolsRequest->deleteLater();
    QNetworkReply::NetworkError error = getPoolsRequest->error();
    if (error != QNetworkReply::NoError)
    {
        DAVA::Logger::Error("Can't get pools list: %s", getPoolsRequest->errorString().toStdString().c_str());
        getServersRequest->abort();
        getServersRequest->deleteLater();
        getServersRequest = nullptr;
    }

    pools = SharedDataParser::ParsePoolsReply(getPoolsRequest->readAll());
    getPoolsRequest = nullptr;
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
    DAVA::List<SharedServerParams> servers = SharedDataParser::ParseServersReply(getServersRequest->readAll());
    getServersRequest = nullptr;

    emit SharedDataReceived(pools, servers);
}

void SharedDataRequester::OnAddServerFinished()
{
}

void SharedDataRequester::OnRemoveServerFinished()
{
}

void SharedDataRequester::Replied(QNetworkReply* reply)
{
    //     reply->deleteLater();
    //     QNetworkReply::NetworkError error = reply->error();
    //
    //     if (error != QNetworkReply::NoError)
    //     {
    //         //aborted = true;
    //         DAVA::String err = reply->errorString().toStdString();
    //         //ErrorMessenger::ShowErrorMessage(ErrorMessenger::ERROR_NETWORK, error, reply->errorString());
    //         return;
    //     }
}
