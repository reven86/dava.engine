#pragma once

#include "ziputils.h"
#include "configparser.h"

#include <QObject>
#include <QString>
#include <QQueue>
#include <QNetworkAccessManager>

#include <functional>

struct SilentUpdateTask
{
    using CallBack = std::function<void(bool success, const QString& message)>;
    SilentUpdateTask(const QString& branch, const QString& app, const AppVersion* currentVersion_, const AppVersion& newVersion_, CallBack onFinished_)
        : branchID(branch)
        , appID(app)
        , newVersion(newVersion_)
        , currentVersion(currentVersion_)
        , onFinished(onFinished_)
    {
    }

    QString branchID;
    QString appID;
    AppVersion newVersion;
    const AppVersion* currentVersion = nullptr;
    CallBack onFinished;
};

class ApplicationManager;
class QNetworkReply;

class SilentUpdater : public QObject
{
    Q_OBJECT
public:
    SilentUpdater(ApplicationManager* appManager, QObject* parent);
    ~SilentUpdater();
    void AddTask(SilentUpdateTask&& task);

private slots:
    void OnDownloadFinished(QNetworkReply* reply);
    void OnNetworkAccessibleChanged(QNetworkAccessManager::NetworkAccessibility accessible);

private:
    //RAII structure to start next task at the end of a scope
    struct TaskStarter;
    void StartNextTask();
    bool ListArchive(const QString& archivePath, ZipUtils::CompressedFilesAndSizes& files);
    bool UnpackArchive(const QString& archivePath, const QString& outDir, const ZipUtils::CompressedFilesAndSizes& files);

    QNetworkAccessManager* networkManager = nullptr;
    //we hold this variable to avoid Qt bug https://bugreports.qt.io/browse/QTBUG-57390
    //When network becomes to incaccessible state we dont receive signal downloadFinished.
    QNetworkReply* currentReply = nullptr;
    ApplicationManager* appManager = nullptr;
    QQueue<SilentUpdateTask> tasks;
    bool canStartNextTask = true;
};
