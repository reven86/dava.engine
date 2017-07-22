#include "Core/Tasks/DownloadTask.h"

DownloadTask::DownloadTask(ApplicationManager* appManager, const QString& description_, const std::vector<QUrl>& urls_)
    : BaseTask(appManager)
    , description(description_)
    , urls(urls_)
{
}

DownloadTask::DownloadTask(ApplicationManager* appManager, const QString& description_, const QUrl url)
    : BaseTask(appManager)
    , description(description_)
    , urls(1, url)
{
}

BaseTask::eTaskType DownloadTask::GetTaskType() const
{
    return DOWNLOAD_TASK;
}

QString DownloadTask::GetDescription() const
{
    return description;
}

const std::vector<QByteArray>& DownloadTask::GetLoadedData() const
{
    return loadedData;
}

void DownloadTask::AddLoadedData(const QByteArray& data)
{
    loadedData.push_back(data);
}

const std::vector<QUrl>& DownloadTask::GetUrls() const
{
    return urls;
}

bool DownloadTask::IsCancelled() const
{
    return isCancelled;
}

void DownloadTask::SetCancelled(bool cancelled)
{
    isCancelled = cancelled;
}
