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
    explicit ConfigDownloader(ApplicationManager* manager, QWidget* parent = 0);
    ~ConfigDownloader();

    int exec() override;

private slots:
    void DownloadFinished(QNetworkReply* reply);

private:
    Ui::ConfigDownloader* ui = nullptr;

    ApplicationManager* appManager = nullptr;
    QNetworkAccessManager* networkManager = nullptr;
    QList<QNetworkReply*> requests;
};

#endif // CONFIGDOWNLOADER_H
