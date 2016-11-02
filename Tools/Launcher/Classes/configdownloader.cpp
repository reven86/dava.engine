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

    //init URLS with default;
    for (int i = 0; i < URLTypesCount; ++i)
    {
        eURLType type = static_cast<eURLType>(i);
        urls[type] = GetDefaultURL(type);
    }
}

ConfigDownloader::~ConfigDownloader()
{
    SafeDelete(ui);
}

int ConfigDownloader::exec()
{
    aborted = false;
    appManager->GetRemoteConfig()->Clear();

    //QMap gives you a value when you use range-based for
    for (const QString& str : urls)
    {
        requests << networkManager->get(QNetworkRequest(QUrl(str)));
    }
    return QDialog::exec();
}

QString ConfigDownloader::GetDefaultURL(eURLType type) const
{
    switch (type)
    {
    case LauncherInfoURL:
        return "http://ba-manager.wargaming.net/panel/modules/jsonAPI/lite.php?source=launcher";
    case StringsURL:
        return "http://ba-manager.wargaming.net/panel/modules/jsonAPI/lite.php?source=seo_list";
    case FavoritesURL:
        return "http://ba-manager.wargaming.net/panel/modules/jsonAPI/lite.php?source=branches&filter=os:" + platformString;
    case AllBuildsURL:
        return "http://ba-manager.wargaming.net/panel/modules/jsonAPI/lite.php?source=builds&filter=os:" + platformString;
    default:
        Q_ASSERT(false && "unacceptable request");
        return QString();
    }
}

QString ConfigDownloader::GetURL(eURLType type) const
{
    return urls[type];
}

void ConfigDownloader::SetURL(eURLType type, QString url)
{
    urls[type] = url;
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
        appManager->SaveLocalConfig();
        accept();
    }
}

void ConfigDownloader::OnCancelClicked()
{
    aborted = true;
    for (QNetworkReply* networkReply : requests)
    {
        networkReply->abort();
    }
    reject();
}
