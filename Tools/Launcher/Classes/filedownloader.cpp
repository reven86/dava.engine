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

    connect(currentDownload, SIGNAL(finished()), this, SLOT(DownloadFinished()));
    connect(currentDownload, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(NetworkError(QNetworkReply::NetworkError)));
}

void FileDownloader::Cancel()
{
    if (currentDownload)
    {
        currentDownload->abort();
    }
}

void FileDownloader::NetworkError(QNetworkReply::NetworkError code)
{
    lastErrorCode = code;
    lastErrorDesc = currentDownload->errorString();
}

void FileDownloader::DownloadFinished()
{
    if (lastErrorCode)
    {
        emit Finished(QByteArray(), QList<QPair<QByteArray, QByteArray>>(), lastErrorCode, lastErrorDesc);
    }
    else if (currentDownload)
    {
        emit Finished(currentDownload->readAll(), currentDownload->rawHeaderPairs(), lastErrorCode, lastErrorDesc);

        currentDownload->deleteLater();
        currentDownload = 0;
    }
}
