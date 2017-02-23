#include "configdownloader.h"
#include "ui_configdownloader.h"
#include "filemanager.h"
#include "errormessenger.h"
#include "applicationmanager.h"

#include "QtHelpers/ProcessHelper.h"

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

    //init class member with default value
    serverHostName = "http://ba-manager.wargaming.net";
}

ConfigDownloader::~ConfigDownloader()
{
    SafeDelete(ui);
}

int ConfigDownloader::exec()
{
    aborted = false;
    appManager->GetRemoteConfig()->Clear();
    QUrl launcherUrl;
    if (IsTestAPIUsed())
    {
        launcherUrl = QUrl(GetServerHostName() + GetURL(LauncherTestInfoURL));
    }
    else
    {
        launcherUrl = QUrl(GetServerHostName() + GetURL(LauncherInfoURL));
    }
    requests << networkManager->get(QNetworkRequest(launcherUrl));

    for (int i = StringsURL; i < URLTypesCount; ++i)
    {
        eURLType type = static_cast<eURLType>(i);
        QUrl url(GetServerHostName() + GetURL(type));
        requests << networkManager->get(QNetworkRequest(url));
    }
    return QDialog::exec();
}

QString ConfigDownloader::GetURL(eURLType type) const
{
    switch (type)
    {
    case LauncherInfoURL:
        return "/panel/modules/jsonAPI/launcher/lite.php?source=launcher";
    case LauncherTestInfoURL:
        return "/panel/modules/jsonAPI/launcher/lite4test.php?source=launcher";
    case StringsURL:
        return "/panel/modules/jsonAPI/launcher/lite.php?source=seo_list";
    case FavoritesURL:
        return "/panel/modules/jsonAPI/launcher/lite.php?source=branches&filter=os:" + platformString;
    case AllBuildsCurrentPlatformURL:
        return "/panel/modules/jsonAPI/launcher/lite.php?source=builds&filter=os:" + platformString;
    case AllBuildsAndroidURL:
        return "/panel/modules/jsonAPI/launcher/lite.php?source=builds&filter=os:android";
    case AllBuildsIOSURL:
        return "/panel/modules/jsonAPI/launcher/lite.php?source=builds&filter=os:ios";
    case AllBuildsUWPURL:
        return "/panel/modules/jsonAPI/launcher/lite.php?source=builds&filter=os:uwp";
    default:
        Q_ASSERT(false && "unacceptable request");
        return QString();
    }
}

QString ConfigDownloader::GetServerHostName() const
{
    return serverHostName;
}

bool ConfigDownloader::IsTestAPIUsed() const
{
    return useTestAPI;
}

void ConfigDownloader::SetUseTestAPI(bool use)
{
    useTestAPI = use;
}

void ConfigDownloader::SetServerHostName(const QString& url)
{
    serverHostName = url;
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
        appManager->GetLocalConfig()->UpdateApplicationsNames();
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
