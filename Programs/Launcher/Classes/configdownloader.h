#pragma once

#include "filedownloader.h"

#include <QDialog>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QFile>

#include <array>

namespace Ui
{
class ConfigDownloader;
}

class ApplicationManager;
class ConfigDownloader : public QDialog
{
    Q_OBJECT

public:
    //word URL added to resolve name conflict
    //enum must be started with zero to make loop through it
    enum eURLType
    {
        LauncherInfoURL = 0,
        LauncherTestInfoURL,
        StringsURL,
        FavoritesURL,
        AllBuildsCurrentPlatformURL,
        AllBuildsAndroidURL,
        AllBuildsIOSURL,
        AllBuildsUWPURL,
        URLTypesCount
    };

    explicit ConfigDownloader(ApplicationManager* manager, QWidget* parent = 0);
    ~ConfigDownloader();

    int exec() override;

    QString GetURL(eURLType type) const;
    QString GetServerHostName() const;

    bool IsTestAPIUsed() const;
    void SetUseTestAPI(bool use);

public slots:
    void SetServerHostName(const QString& url);

private slots:
    void DownloadFinished(QNetworkReply* reply);
    void OnCancelClicked();

private:
    Ui::ConfigDownloader* ui = nullptr;

    ApplicationManager* appManager = nullptr;
    QNetworkAccessManager* networkManager = nullptr;
    QList<QNetworkReply*> requests;
    bool aborted = false;
    bool useTestAPI = false;

    std::array<QString, URLTypesCount> urls;
    QString serverHostName;
};
