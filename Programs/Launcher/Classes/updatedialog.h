#ifndef UPDATEDIALOG_H
#define UPDATEDIALOG_H

#include "configparser.h"
#include "ziputils.h"

#include <QDialog>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QQueue>
#include <QFile>
#include <QTreeWidgetItem>
#include <memory>

namespace Ui
{
class UpdateDialog;
}

class ApplicationManager;
class FileManager;

struct UpdateTask
{
    UpdateTask(const QString& branch, const QString& app, const AppVersion* currentVersion_, const AppVersion& newVersion_, bool isSelfUpdate_ = false, bool isRemove_ = false)
        : branchID(branch)
        , appID(app)
        , newVersion(newVersion_)
        , currentVersion(currentVersion_)
        , isSelfUpdate(isSelfUpdate_)
        , isRemoveBranch(isRemove_)
    {
    }

    QString branchID;
    QString appID;
    AppVersion newVersion;
    const AppVersion* currentVersion;
    bool isSelfUpdate;
    bool isRemoveBranch;
};

class UpdateDialog : public QDialog
{
    Q_OBJECT

public:
    explicit UpdateDialog(const QQueue<UpdateTask>& taskQueue, ApplicationManager* _appManager, QWidget* parent = 0);
    ~UpdateDialog();

    void UpdateLastLogValue(const QString& log);
    void BreakLog();

public slots:
    void OnCancelClicked();

private slots:
    void DownloadProgress(qint64 bytesReceived, qint64 bytesTotal);
    void DownloadFinished();
    void OnNetworkAccessibleChanged(QNetworkAccessManager::NetworkAccessibility accessible);
    void StartNextTask();

private:
    bool ListArchive(const QString& archivePath, ZipUtils::CompressedFilesAndSizes& files);
    bool UnpackArchive(const QString& archivePath, const QString& outDir, const ZipUtils::CompressedFilesAndSizes& files);
    void UpdateButton();

    void AddTopLogValue(const QString& log);
    void AddLogValue(const QString& log);
    void CompleteLog();

    std::unique_ptr<Ui::UpdateDialog> ui;

    QNetworkAccessManager* networkManager = nullptr;
    QNetworkReply* currentDownload = nullptr;

    int tasksCount = 0;

    QQueue<UpdateTask> tasks;

    QTreeWidgetItem* currentLogItem = nullptr;
    QTreeWidgetItem* currentTopLogItem = nullptr;
    ApplicationManager* appManager = nullptr;
};

#endif // UPDATEDIALOG_H
