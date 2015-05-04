#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSystemTrayIcon>

#include "RemoteAssetCacheServer.h"

class QMenu;

namespace Ui
{
    class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

signals:
    void FolderChanged(QString &);
    void FolderSizeChanged(qreal);
    void FilesCountChanged(quint32);
    void NewServerAdded(ServerData);
    void ServerRemoved(ServerData);
    void ServersChanged(QVector<ServerData>);

protected:
    void closeEvent(QCloseEvent *e) override;

private slots:
    void OnAddNewServerWidget();
    void OnRemoveServerWidget();
    void OnSelectFolder();
    void CheckEnableClearButton();
    void OnTrayIconActivated(QSystemTrayIcon::ActivationReason reason);
    void OnServerParametersChanged();
    void OnOpenAction();

    void OnSaveButtonClicked();
    void OnCancelButtonClicked();

    void SetFolder(QString &folderPath);
    void SetFolderSize(qreal folderSize);
    void SetFilesCount(quint32 filesCounts);
    void AddServers(QVector<ServerData> &newServers);
    void AddServer(ServerData newServer);

private:
    void CreateTrayIconActions();
    void ShowTrayIcon();
    void ReadSettings();
    void WriteSettings();

private:
    Ui::MainWindow *ui;
    QSystemTrayIcon *trayIcon;
    QAction *openAction;
    QMenu *trayActionsMenu;

    QList<RemoteAssetCacheServer *> servers;
};

#endif // MAINWINDOW_H
