#include "Core/ConfigDownloader.h"
#include "Core/ApplicationManager.h"

#include "Core/Tasks/UpdateConfigTask.h"

#include "Utils/ErrorMessenger.h"

ConfigDownloader::ConfigDownloader(ApplicationManager* appManager_)
    : appManager(appManager_)
    , serverHostName("http://ba-manager.wargaming.net")
{
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

std::unique_ptr<BaseTask> ConfigDownloader::CreateTask() const
{
    QVector<QUrl> urls;
    if (IsTestAPIUsed())
    {
        urls.push_back(GetServerHostName() + GetURL(LauncherTestInfoURL));
    }
    else
    {
        urls.push_back(GetServerHostName() + GetURL(LauncherInfoURL));
    }

    for (int i = StringsURL; i < URLTypesCount; ++i)
    {
        eURLType type = static_cast<eURLType>(i);
        QUrl url(GetServerHostName() + GetURL(type));
        urls.push_back(url);
    }
    return std::unique_ptr<BaseTask>(new UpdateConfigTask(appManager, urls));
}

void ConfigDownloader::SetServerHostName(const QString& url)
{
    serverHostName = url;
}
