#pragma once

#include <memory>
#include <QMainWindow>
#include <QSystemTrayIcon>

#include "ServerCore.h"

class QMenu;
class QVBoxLayout;

class SharedPoolWidget;
class SharedServerWidget;
class CustomServerWidget;
class ApplicationSettings;
struct RemoteServerParams;
struct SharedPool;

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

private slots:
    void OnTrayIconActivated(QSystemTrayIcon::ActivationReason reason);
    void ChangeSettingsPage(int newRow);

    void OnEditAction();
    void OnStartAction();
    void OnStopAction();

    void OnFolderSelection();
    void OnFolderTextChanged();
    void OnCacheSizeChanged(double);
    void OnNumberOfFilesChanged(int);
    void OnAutoSaveTimeoutChanged(int);
    void OnPortChanged(int);
    void OnHttpPortChanged(int);
    void OnAutoStartToggled(bool);
    void OnSystemStartupToggled(bool);
    void OnRestartToggled(bool);
    void OnAdvancedLinkActivated(const QString&);

    void OnServersAreaRangeChanged(int, int);
    void OnAddServerClicked();
    void OnCustomServerRemoved();
    void OnRemoteServerEdited();
    void OnCustomServerChecked(bool);
    void OnSharedPoolChecked(bool);
    void OnSharedServerChecked(bool);

    void OnClearButtonClicked();
    void OnApplyButtonClicked();
    void OnCloseButtonClicked();

    void OnServerStateChanged(const ServerCore*);
    void UpdateUsageProgressbar(DAVA::uint64, DAVA::uint64);

private:
    //     struct CheckedWidget
    //     {
    //         enum Type { POOL, POOL_SERVER, CUSTOM_SERVER, NONE } type = NONE;
    //         union
    //         {
    //             SharedPoolWidget* poolWidget = nullptr;
    //             SharedServerWidget* serverWidget;
    //             CustomServerWidget* customServerWidget;
    //         };
    //     };

    struct CheckedRemote
    {
        enum Type
        {
            POOL,
            POOL_SERVER,
            CUSTOM_SERVER,
            NONE
        } type = NONE;
        PoolID poolID = 0;
        ServerID serverID = 0;

        CheckedRemote() = default;
        CheckedRemote(const SharedPoolWidget* w);
        CheckedRemote(const SharedServerWidget* w);
        CheckedRemote(const CustomServerWidget* w);
    };

private:
    void CreateTrayMenu();

    void closeEvent(QCloseEvent* e) override; // QWidget

    void LoadSettings();
    void SaveSettings();

    void ReconstructServersList();
    void ReconstructSharedServersList();
    void ReconstructCustomServersList();

    void RemoveSharedServers();
    void AddSharedPool(const SharedPool& pool);

    void RemoveCustomServers();
    void AddCustomServer(const RemoteServerParams& newServer);

    void ClearAllChecks();
    CheckedRemote AssetCacheServerWindow::GetCheckedRemote() const;

    void SelectedRemoteSetText();

    void ShowAdvancedSettings(bool show);

    void SetupLaunchOnStartup(bool toLaunchOnStartup, bool toRestartOnCrash);

    void VerifyChanges();

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

    QVBoxLayout* customServersLayout = nullptr;
    QVBoxLayout* sharedServersLayout = nullptr;
    QVBoxLayout* serversLayout = nullptr;

    bool customServerManuallyAdded = false;

    DAVA::List<CustomServerWidget*> customServersWidgets;
    DAVA::List<SharedPoolWidget*> sharedPoolsWidgets;
    DAVA::List<SharedServerWidget*> sharedServersWidgets;

    //CheckedWidget checkedWidget;

    ServerCore& serverCore;

    SettingsState settingsState = NOT_EDITED;
    bool showAdvanced = false;
};
