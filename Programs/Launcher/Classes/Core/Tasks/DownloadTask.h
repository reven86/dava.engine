#pragma once

#include "Core/Tasks/BaseTask.h"

#include <QString>
#include <QUrl>
#include <QVector>
#include <QByteArray>

class DownloadTask final : public BaseTask
{
public:
    DownloadTask(ApplicationManager* appManager, const QString& description, const QVector<QUrl>& urls);
    DownloadTask(ApplicationManager* appManager, const QString& description, const QUrl url);

    QString GetDescription() const override;

    //change it to QVector<std::pair<QIODevice*, QUrl>> later
    const QVector<QByteArray>& GetLoadedData() const;
    void AddLoadedData(const QByteArray& data);

    const QVector<QUrl>& GetUrls() const;

protected:
    QString description;
    QVector<QUrl> urls;
    QVector<QByteArray> loadedData;
};
