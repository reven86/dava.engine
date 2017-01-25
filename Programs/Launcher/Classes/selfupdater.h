#pragma once

#include <QDialog>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QFile>
#include <memory>

class FileManager;

namespace Ui
{
class SelfUpdater;
}

class SelfUpdater final : public QDialog
{
    Q_OBJECT

public:
    explicit SelfUpdater(FileManager* fileManager, const QString& arcUrl, QWidget* parent = nullptr);
    ~SelfUpdater() override;

private slots:
    void NetworkError(QNetworkReply::NetworkError code);
    void DownloadFinished();
    void DownloadProgress(qint64 bytesReceived, qint64 bytesTotal);

private:
    enum UpdateError
    {
        NO_ERRORS,
        ARCHIVE_ERROR,
        MOVE_FILES_ERROR,
        INFO_FILE_ERROR
    };
    UpdateError ProcessLauncherUpdate();
    QString ErrorString(UpdateError err) const;

    std::unique_ptr<Ui::SelfUpdater> ui;
    QString archiveUrl;

    QNetworkAccessManager* networkManager = nullptr;
    QNetworkReply* currentDownload = nullptr;

    int lastErrorCode = QNetworkReply::NoError;
    QString lastErrorDesrc;
    FileManager* fileManager = nullptr;
};
