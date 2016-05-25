#include "configdownloader.h"
#include "ui_configdownloader.h"
#include "filemanager.h"
#include "processhelper.h"
#include "errormessanger.h"
#include "applicationmanager.h"
#include <QProcess>

ConfigDownloader::ConfigDownloader(ApplicationManager* manager, QNetworkAccessManager* accessManager, QWidget* parent)
    :
    QDialog(parent, Qt::WindowTitleHint | Qt::CustomizeWindowHint)
    ,
    ui(new Ui::ConfigDownloader)
    ,
    downloader(0)
    ,
    appManager(manager)
{
    ui->setupUi(this);

    downloader = new FileDownloader(accessManager);
    connect(ui->cancelButton, SIGNAL(clicked()), downloader, SLOT(Cancel()));

    connect(downloader, SIGNAL(Finished(QByteArray, QList<QPair<QByteArray, QByteArray>>, int, QString)),
            this, SLOT(DownloadFinished(QByteArray, QList<QPair<QByteArray, QByteArray>>, int, QString)));
}

ConfigDownloader::~ConfigDownloader()
{
    SafeDelete(ui);
    SafeDelete(downloader);
}

int ConfigDownloader::exec()
{
    downloader->Download(QUrl(appManager->localConfig->GetRemoteConfigURL()));

    return QDialog::exec();
}

void ConfigDownloader::DownloadFinished(QByteArray downloadedData, QList<QPair<QByteArray, QByteArray>> rawHeaderList, int errorCode, QString errorDescr)
{
    if (errorCode)
    {
        if (errorCode != QNetworkReply::OperationCanceledError)
        {
            ErrorMessanger::Instance()->ShowErrorMessage(ErrorMessanger::ERROR_NETWORK, errorCode, errorDescr);
        }
        reject();
    }
    else
    {
        const QByteArray contentTypeConst("Content-Type");
        for (QList<QPair<QByteArray, QByteArray>>::ConstIterator it = rawHeaderList.begin(); it != rawHeaderList.end(); ++it)
        {
            if ((*it).first == contentTypeConst && (*it).second.left(9) != QByteArray("text/html"))
            {
                appManager->ParseRemoteConfigData(downloadedData);
            }
        }

        accept();
    }
}
