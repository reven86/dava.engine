#ifndef CONFIGDOWNLOADER_H
#define CONFIGDOWNLOADER_H

#include <QDialog>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QFile>
#include "filedownloader.h"

namespace Ui
{
class ConfigDownloader;
}

class ApplicationManager;
class ConfigDownloader : public QDialog
{
    Q_OBJECT

public:
    explicit ConfigDownloader(ApplicationManager* manager, QNetworkAccessManager* accessManager, QWidget* parent = 0);
    ~ConfigDownloader();

    virtual int exec();

private slots:
    void DownloadFinished(QByteArray downloadedData, QList<QPair<QByteArray, QByteArray>> rawHeaderList, int errorCode, QString errorDescr);

private:
    Ui::ConfigDownloader* ui;

    FileDownloader* downloader;

    ApplicationManager* appManager;
};

#endif // CONFIGDOWNLOADER_H
