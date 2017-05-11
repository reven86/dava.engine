#pragma once

#include <QNetworkAccessManager>
#include <QNetworkReply>

class FileDownloader : public QObject
{
    Q_OBJECT

public:
    explicit FileDownloader(QObject* parent = nullptr);

signals:
    void Finished(QByteArray downloadedData, QList<QPair<QByteArray, QByteArray>> rawHeaderList, int errorCode, QString errorDescr);

public slots:
    void Download(const QUrl& url);
    void Cancel();

private slots:
    void DownloadFinished();

private:
    QNetworkAccessManager* networkManager = nullptr;
    QNetworkReply* currentDownload = nullptr;
};
