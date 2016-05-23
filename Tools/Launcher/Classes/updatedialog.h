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

namespace Ui
{
class UpdateDialog;
}

class ApplicationManager;

struct UpdateTask
{
    UpdateTask(const QString& branch, const QString& app, const AppVersion& _version, bool _isSelfUpdate = false, bool _isRemove = false)
        : branchID(branch)
        , appID(app)
        , version(_version)
        , isSelfUpdate(_isSelfUpdate)
        , isRemoveBranch(_isRemove)
    {
    }

    QString branchID;
    QString appID;
    AppVersion version;
    bool isSelfUpdate;
    bool isRemoveBranch;
};

class UpdateDialog : public QDialog
{
    Q_OBJECT

public:
    explicit UpdateDialog(const QQueue<UpdateTask>& taskQueue, ApplicationManager* _appManager, QNetworkAccessManager* accessManager, QWidget* parent = 0);
    ~UpdateDialog();

    void UpdateLastLogValue(const QString& log);
    void BreakLog();

signals:
    void AppInstalled(const QString& branchID, const QString& appID, const AppVersion& version);

public slots:
    void OnCancelClicked();

private slots:
    void NetworkError(QNetworkReply::NetworkError code);
    void DownloadProgress(qint64 bytesReceived, qint64 bytesTotal);
    void DownloadFinished();
    void DownloadReadyRead();
    void StartNextTask();

private:
    bool ListArchive(const QString& archivePath, ZipUtils::CompressedFilesAndSizes& files);
    bool TestArchive(const QString& archivePath, const ZipUtils::CompressedFilesAndSizes& files);
    bool UnpackArchive(const QString& archivePath, const QString& outDir, const ZipUtils::CompressedFilesAndSizes& files);
    void UpdateButton();

    void AddTopLogValue(const QString& log);
    void AddLogValue(const QString& log);
    void CompleteLog();

    std::unique_ptr<Ui::UpdateDialog> ui;

    QNetworkAccessManager* networkManager = nullptr;
    QNetworkReply* currentDownload = nullptr;

    int tasksCount = 0;

    QFile outputFile;
    QQueue<UpdateTask> tasks;

    QTreeWidgetItem* currentLogItem = nullptr;
    QTreeWidgetItem* currentTopLogItem = nullptr;

    int lastErrorCode = QNetworkReply::NoError;
    QString lastErrorDesrc;

    ApplicationManager* appManager = nullptr;
};

#endif // UPDATEDIALOG_H
