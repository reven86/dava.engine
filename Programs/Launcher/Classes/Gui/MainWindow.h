#pragma once

#include "Gui/ButtonsWidget.h"
#include "Core/Receiver.h"

#include <QMainWindow>
#include <QListWidgetItem>
#include <QtGui>
#include <QSet>
#include <QDebug>

class ApplicationManager;
class BranchesListModel;
class QSortFilterProxyModel;

struct Application;

namespace Ui
{
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(ApplicationManager* appManager, QWidget* parent = 0);
    ~MainWindow();

    void RefreshApps();

    void ShowTable(const QString& branchID);

    const Receiver& GetReceiver() const;

signals:
    void RefreshClicked();
    void RunClicked(int row);
    void DownloadClicked(int row);
    void RemoveClicked(int row);

    void ShowPreferences();
    void CancelClicked();

private slots:
    void OnRun(int rowNumber);
    void OnInstall(int rowNumber);
    void OnRemove(int rowNumber);

    void OnListItemClicked(QModelIndex);

    void OnCellDoubleClicked(QModelIndex index);

    void OnlinkClicked(QUrl url);

private:
    void RefreshBranchesList();
    void UpdateButtonsState(int rowNumber, ButtonsWidget::ButtonsState state);
    void OnConnectedChanged(bool connected);

    void GetTableApplicationIDs(int rowNumber, QString& appID, QString& installedVersionID, QString& avalibleVersionID);

    QWidget* CreateAppNameTableItem(const QString& stringID, int rowNum);
    QWidget* CreateAppInstalledTableItem(const QString& stringID, int rowNum);
    QWidget* CreateAppAvalibleTableItem(Application* app, int rowNum);

    void OnTaskStarted(const BaseTask* task);
    void OnTaskProgress(const BaseTask* task, quint32 progress);
    void OnTaskFinished(const BaseTask* task);

    void OnNewsLoaded(const BaseTask* task);

    Ui::MainWindow* ui = nullptr;

    QString selectedBranchID;

    QFont tableFont;
    BranchesListModel* listModel = nullptr;
    QSortFilterProxyModel* filterModel = nullptr;
    ApplicationManager* appManager = nullptr;

    Receiver receiver;
};
