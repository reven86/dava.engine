#pragma once

#include "applicationmanager.h"
#include "buttonswidget.h"
#include "filedownloader.h"

#include <QMainWindow>
#include <QListWidgetItem>
#include <QtGui>
#include <QSet>
#include <QDebug>

class BranchesListModel;
class QSortFilterProxyModel;
class FileManager;
class ConfigDownloader;
class BAManagerClient;
class ConfigRefresher;

namespace Ui
{
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = 0);
    ~MainWindow();

private slots:
    void Refresh();
    void OnListItemClicked(QModelIndex);

    void OnRun(int rowNumber);
    void OnInstall(int rowNumber);
    void OnRemove(int rowNumber);

    void OnInstallAll();
    void OnRemoveAll();

    void OnCellDoubleClicked(QModelIndex index);

    void OnlinkClicked(QUrl url);

    void NewsDownloadFinished(QByteArray downloadedData, QList<QPair<QByteArray, QByteArray>> rawHeaderList, int errorCode, QString errorDescr);

    void OpenPreferencesEditor();

    void ShowTable(const QString& branchID);

private:
    void CheckUpdates();
    void RefreshApps();

    void ShowWebpage();
    void ShowUpdateDialog(QQueue<UpdateTask>& tasks);

    void RefreshBranchesList();
    void UpdateButtonsState(int rowNumber, ButtonsWidget::ButtonsState state);

    void GetTableApplicationIDs(int rowNumber, QString& appID, QString& installedVersionID, QString& avalibleVersionID);

    QWidget* CreateAppNameTableItem(const QString& stringID, int rowNum);
    QWidget* CreateAppInstalledTableItem(const QString& stringID, int rowNum);
    QWidget* CreateAppAvalibleTableItem(Application* app, int rowNum);

    Ui::MainWindow* ui = nullptr;
    ApplicationManager* appManager = nullptr;
    BAManagerClient* baManagerClient = nullptr;
    ConfigRefresher* configRefresher = nullptr;

    FileDownloader* newsDownloader = nullptr;
    ConfigDownloader* configDownloader = nullptr;
    QPersistentModelIndex selectedListItem;
    QString selectedBranchID;

    QFont tableFont;
    BranchesListModel* listModel = nullptr;
    QSortFilterProxyModel* filterModel = nullptr;
};
