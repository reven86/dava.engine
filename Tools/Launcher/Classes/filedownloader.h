#ifndef FILEDOWNLOADER_H
#define FILEDOWNLOADER_H

#include <QNetworkAccessManager>
#include <QNetworkReply>

class FileDownloader : public QObject
{
    Q_OBJECT

public:
    explicit FileDownloader(QNetworkAccessManager* accessManager);
    ~FileDownloader();

signals:
    void Finished(QByteArray downloadedData, QList<QPair<QByteArray, QByteArray>> rawHeaderList, int errorCode, QString errorDescr);

public slots:
    void Download(QUrl url);
    void Cancel();

private slots:
    void NetworkError(QNetworkReply::NetworkError code);
    void DownloadFinished();

private:
    QNetworkAccessManager* networkManager;
    QNetworkReply* currentDownload;

    int lastErrorCode;
    QString lastErrorDesc;
};

#endif // FILEDOWNLOADER_H
