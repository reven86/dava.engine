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
    setModal(true);
    ui->setupUi(this);
    connect(networkManager, &QNetworkAccessManager::finished, this, &ConfigDownloader::DownloadFinished);
    connect(ui->cancelButton, &QPushButton::clicked, this, &ConfigDownloader::OnCancelClicked);
}

ConfigDownloader::~ConfigDownloader()
{
    SafeDelete(ui);
}

int ConfigDownloader::exec()
{
    aborted = false;
    appManager->GetRemoteConfig()->Clear();
    QStringList urls = QStringList() << "http://ba-manager.wargaming.net/panel/modules/jsonAPI/lite.php?source=launcher" //version, url, news
                                     << "http://ba-manager.wargaming.net/panel/modules/jsonAPI/lite.php?source=seo_list" //stirngs
                                     << "http://ba-manager.wargaming.net/panel/modules/jsonAPI/lite.php?source=branches&filter=os:" + platformString // favorites
                                     << "http://ba-manager.wargaming.net/panel/modules/jsonAPI/lite.php?source=builds&filter=os:" + platformString //all builds
    ;
    for (const QString& str : urls)
    {
        requests << networkManager->get(QNetworkRequest(QUrl(str)));
    }
    return QDialog::exec();
}

void ConfigDownloader::DownloadFinished(QNetworkReply* reply)
{
    requests.removeOne(reply);
    reply->deleteLater();
    if (aborted)
    {
        return;
    }
    QNetworkReply::NetworkError error = reply->error();

    if (error != QNetworkReply::NoError)
    {
        aborted = true;
        ErrorMessenger::ShowErrorMessage(ErrorMessenger::ERROR_NETWORK, error, reply->errorString());
        reject();
        return;
    }
    else
    {
        appManager->ParseRemoteConfigData(reply->readAll());
    }

    if (requests.isEmpty())
    {
        appManager->GetRemoteConfig()->UpdateApplicationsNames();
        appManager->localConfig.SaveToFile(appManager->localConfigFilePath);
        accept();
    }
}

void ConfigDownloader::OnCancelClicked()
{
    for (QNetworkReply* networkReply : requests)
    {
        networkReply->abort();
    }
}
