#ifndef SELFUPDATER_H
#define SELFUPDATER_H

#include <QDialog>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QFile>
#include <memory>

namespace Ui
{
class SelfUpdater;
}

class SelfUpdater final : public QDialog
{
    Q_OBJECT

public:
    explicit SelfUpdater(const QString& arcUrl, QNetworkAccessManager* accessManager, QWidget* parent = 0);
    ~SelfUpdater() override;

signals:
    void StartUpdating();

private slots:
    void NetworkError(QNetworkReply::NetworkError code);
    void DownloadFinished();
    void OnStartUpdating();

private:
    std::unique_ptr<Ui::SelfUpdater> ui;
    QString archiveUrl;

    QNetworkAccessManager* networkManager = nullptr;
    QNetworkReply* currentDownload = nullptr;

    int lastErrorCode = QNetworkReply::NoError;
    QString lastErrorDesrc;
};

#endif // SELFUPDATER_H
