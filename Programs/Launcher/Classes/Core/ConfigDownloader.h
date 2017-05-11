#pragma once

#include <array>
#include <memory>

#include <QString>

class BaseTask;
class ApplicationManager;

class ConfigDownloader
{
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

    ConfigDownloader(ApplicationManager* manager);

    QString GetURL(eURLType type) const;
    QString GetServerHostName() const;
    void SetServerHostName(const QString& url);

    bool IsTestAPIUsed() const;
    void SetUseTestAPI(bool use);

    std::unique_ptr<BaseTask> CreateTask() const;

private:
    ApplicationManager* appManager = nullptr;
    bool useTestAPI = false;

    std::array<QString, URLTypesCount> urls;
    QString serverHostName;
};
