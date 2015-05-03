#ifndef REMOTEASSETCACHSERVER_H
#define REMOTEASSETCACHSERVER_H

#include <QWidget>

struct ServerData
{
    ServerData();
    ServerData(QString newIp, quint16 newPort);

    QString ip;
    quint16 port;
};

namespace Ui
{
    class RemoteAssetCacheServer;
}

class RemoteAssetCacheServer : public QWidget
{
    Q_OBJECT

public:
    explicit RemoteAssetCacheServer(QWidget *parent = nullptr);
    explicit RemoteAssetCacheServer(ServerData &newServer, QWidget *parent = nullptr);
    ~RemoteAssetCacheServer() override;

    ServerData GetServerData() const;

signals:
    void ParametersChanged();
    void RemoveLater();

private slots:
    void OnParanetersChanged();

private:
    Ui::RemoteAssetCacheServer *ui;
};

#endif // REMOTEASSETCACHSERVER_H
