#include "defines.h"
#include "filedownloader.h"
#include <QObject>

FileDownloader::FileDownloader(QObject* parent)
    : QObject(parent)
    , networkManager(new QNetworkAccessManager(this))
{
}

void FileDownloader::Download(const QUrl& url)
{
    Cancel();

    currentDownload = networkManager->get(QNetworkRequest(url));

    connect(currentDownload, &QNetworkReply::finished, this, &FileDownloader::DownloadFinished);
}

void FileDownloader::Cancel()
{
    if (currentDownload != nullptr)
    {
        currentDownload->abort();
        currentDownload = nullptr;
    }
}

void FileDownloader::DownloadFinished()
{
    QNetworkReply* download = qobject_cast<QNetworkReply*>(sender());
    download->deleteLater();

    if (download->error() != QNetworkReply::NoError)
    {
        emit Finished(QByteArray(), QList<QPair<QByteArray, QByteArray>>(), download->error(), download->errorString());
    }
    else
    {
        emit Finished(download->readAll(), download->rawHeaderPairs(), download->error(), download->errorString());
    }
    //clear current download state
    currentDownload = nullptr;
}
