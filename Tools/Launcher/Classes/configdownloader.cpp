#include "configdownloader.h"
#include "ui_configdownloader.h"
#include "filemanager.h"
#include "processhelper.h"
#include "errormessenger.h"
#include "applicationmanager.h"
#include <QProcess>

ConfigDownloader::ConfigDownloader(ApplicationManager* manager, QWidget* parent)
    : QDialog(parent, Qt::WindowTitleHint | Qt::CustomizeWindowHint)
    , ui(new Ui::ConfigDownloader)
    , appManager(manager)
    , networkManager(new QNetworkAccessManager(this))
{
    ui->setupUi(this);
    connect(networkManager, &QNetworkAccessManager::finished, this, &ConfigDownloader::DownloadFinished);
    requests << "http://ba-manager.wargaming.net/panel/modules/json_lite.php?source=builds"
    // << "http://ba-manager.wargaming.net/panel/modules/json_lite.php?source=branches"
    // << "http://ba-manager.wargaming.net/panel/modules/json_lite.php?source=launcher"
    ;
}

ConfigDownloader::~ConfigDownloader()
{
    SafeDelete(ui);
}

int ConfigDownloader::exec()
{
    for (const QString& str : requests)
    {
        networkManager->get(QNetworkRequest(QUrl(str)));
    }
    return QDialog::exec();
}

void ConfigDownloader::DownloadFinished(QNetworkReply* reply)
{
    QNetworkReply::NetworkError error = reply->error();

    if (error != QNetworkReply::NoError && error != QNetworkReply::OperationCanceledError)
    {
        ErrorMessenger::ShowErrorMessage(ErrorMessenger::ERROR_NETWORK, errorCode, errorDescr);
        reject();
    }
    else
    {
        qDebug() << reply->readAll();
        //appManager->ParseRemoteConfigData(reply->readAll());
    }
    requests.pop_back();
    if (requests.isEmpty())
    {
        accept();
    }
}
