#pragma once

#include "Core/Tasks/BaseTask.h"

#include <QString>
#include <QUrl>
#include <QByteArray>

class DownloadTask final : public BaseTask
{
public:
    DownloadTask(ApplicationManager* appManager, const QString& description, const std::vector<QUrl>& urls);
    DownloadTask(ApplicationManager* appManager, const QString& description, const QUrl url);

    QString GetDescription() const override;
    eTaskType GetTaskType() const override;

    //change it to std::vector<std::pair<QIODevice*, QUrl>> later
    const std::vector<QByteArray>& GetLoadedData() const;
    void AddLoadedData(const QByteArray& data);

    const std::vector<QUrl>& GetUrls() const;

protected:
    QString description;
    std::vector<QUrl> urls;
    std::vector<QByteArray> loadedData;
};
