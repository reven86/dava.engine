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
}

ConfigDownloader::~ConfigDownloader()
{
    SafeDelete(ui);
}

int ConfigDownloader::exec()
{
    appManager->GetRemoteConfig()->Clear();
    QStringList urls = QStringList() << "http://ba-manager.wargaming.net/panel/modules/json_lite.php?source=launcher" //version, url, news
                                     << "http://ba-manager.wargaming.net/panel/modules/json_lite.php?source=seo_list" //stirngs
                                     << "http://ba-manager.wargaming.net/panel/modules/json_lite.php?source=branches&filter=os:" + platformString // favorites
                                     << "http://ba-manager.wargaming.net/panel/modules/json_lite.php?source=builds&filter=os:" + platformString //all builds
    ;
    for (const QString& str : urls)
    {
        requests << networkManager->get(QNetworkRequest(QUrl(str)));
    }
    return QDialog::exec();
}

void ConfigDownloader::DownloadFinished(QNetworkReply* reply)
{
    reply->deleteLater();
    QNetworkReply::NetworkError error = reply->error();

    if (error != QNetworkReply::NoError && error != QNetworkReply::OperationCanceledError)
    {
        ErrorMessenger::ShowErrorMessage(ErrorMessenger::ERROR_NETWORK, error, reply->errorString());
        reject();
    }
    else
    {
        appManager->ParseRemoteConfigData(reply->readAll());
    }
    if (!requests.contains(reply))
    {
        ErrorMessenger::ShowErrorMessage(ErrorMessenger::ERROR_NETWORK, tr("internal error while downloading config"));
        reject();
        return;
    }
    requests.removeOne(reply);
    if (requests.isEmpty())
    {
        appManager->GetRemoteConfig()->UpdateApplicationsNames();
        appManager->localConfig.SaveToFile(appManager->localConfigFilePath);
        accept();
    }
}
