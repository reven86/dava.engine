#ifndef __ASSETCACHESERVER_WINDOW_H__
#define __ASSETCACHESERVER_WINDOW_H__

#include <memory>
#include <QMainWindow>
#include <QSystemTrayIcon>

#include "ServerCore.h"

class QMenu;
class QVBoxLayout;

class RemoteServerWidget;
class ApplicationSettings;
struct ServerData;

namespace Ui
{
class AssetCacheServerWidget;
}

class AssetCacheServerWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit AssetCacheServerWindow(ServerCore& core, QWidget* parent = nullptr);
    ~AssetCacheServerWindow() override;
    void OnFirstLaunch();

protected:
    void closeEvent(QCloseEvent* e) override;

    void LoadSettings();
    void SaveSettings();

private slots:
    void OnTrayIconActivated(QSystemTrayIcon::ActivationReason reason);

    void OnEditAction();
    void OnStartAction();
    void OnStopAction();

    void OnFolderSelection();
    void OnFolderTextChanged();
    void OnCacheSizeChanged(double);
    void OnNumberOfFilesChanged(int);
    void OnAutoSaveTimeoutChanged(int);
    void OnPortChanged(int);
    void OnAutoStartChanged(int);
    void OnSystemStartupChanged(int);

    void OnRemoteServerAdded();
    void OnRemoteServerRemoved();
    void OnRemoteServerEdited();
    void OnRemoteServerChecked(bool);

    void OnApplyButtonClicked();
    void OnCloseButtonClicked();

    void OnServerStateChanged(const ServerCore*);

private:
    void CreateTrayIcon();

    void AddRemoteServer(const ServerData& newServer);
    void RemoveServers();

    void SetupLaunchOnStartup(bool toLaunchOnStartup);

    void VerifyData();

    enum SettingsState
    {
        NOT_EDITED,
        EDITED,
        EDITED_NOT_CORRECT
    };

    void ChangeSettingsState(SettingsState newState);

private:
    Ui::AssetCacheServerWidget* ui = nullptr;
    QAction* startAction = nullptr;
    QAction* stopAction = nullptr;
    QSystemTrayIcon* trayIcon = nullptr;
    std::unique_ptr<QIcon> greenGreenTrayIcon;
    std::unique_ptr<QIcon> greenGrayTrayIcon;
    std::unique_ptr<QIcon> greenRedTrayIcon;
    std::unique_ptr<QIcon> redGrayTrayIcon;

    QVBoxLayout* serversBoxLayout = nullptr;
    List<RemoteServerWidget*> remoteServers;

    ServerCore& serverCore;

    SettingsState settingsState = NOT_EDITED;
};

#endif // __ASSETCACHESERVER_WINDOW_H__
