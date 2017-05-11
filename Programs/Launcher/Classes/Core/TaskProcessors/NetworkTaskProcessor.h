#pragma once

#include "Core/TaskProcessors/BaseTaskProcessor.h"

#include <QObject>
#include <QList>
#include <QNetworkAccessManager>

#include <memory>

class DownloadTask;
class QString;
class QNetworkAccessManager;
class QNetworkReply;

class NetworkTaskProcessor final : public QObject, public BaseTaskProcessor
{
    Q_OBJECT

public:
    NetworkTaskProcessor();
    ~NetworkTaskProcessor() override;

    void AddTask(std::unique_ptr<BaseTask>&& task, ReceiverNotifier notifier) override;
    void Terminate() override;

private slots:
    void OnDownloadFinished(QNetworkReply* reply);
    void OnAccessibleChanged(QNetworkAccessManager::NetworkAccessibility accessible);
    void OnDownloadProgress(qint64 bytes, qint64 total);

private:
    void StartNextTask();
    void OnNetworkError();

    struct TaskParams
    {
        TaskParams(std::unique_ptr<BaseTask>&& task, ReceiverNotifier notifier);

        std::unique_ptr<DownloadTask> task;
        ReceiverNotifier notifier;

        QList<QNetworkReply*> requests;
    };
    QNetworkAccessManager* networkAccessManager;
    std::list<std::unique_ptr<TaskParams>> tasks;
    std::unique_ptr<TaskParams> currentTask;
};
