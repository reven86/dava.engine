#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "applicationmanager.h"
#include "buttonswidget.h"
#include "filedownloader.h"
#include <QMainWindow>
#include <QListWidgetItem>
#include <QtGui>
#include <QSet>
#include <QDebug>

class ListModel;
class QSortFilterProxyModel;

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

public slots:
    void OnRefreshClicked();
    void OnListItemClicked(QModelIndex);
    void RefreshApps();

    void OnRun(int rowNumber);
    void OnInstall(int rowNumber);
    void OnRemove(int rowNumber);

    void OnCellClicked(const QPoint& pos);
    void OnCellDoubleClicked(QModelIndex index);

    void OnlinkClicked(QUrl url);

    void NewsDownloadFinished(QByteArray downloadedData, QList<QPair<QByteArray, QByteArray>> rawHeaderList, int errorCode, QString errorDescr);

private:
    void ShowWebpage();
    void ShowTable(const QString& branchID);
    void ShowUpdateDialog(QQueue<UpdateTask>& tasks);

    void RefreshBranchesList();
    void UpdateButtonsState(int rowNumber, ButtonsWidget::ButtonsState state);

    void GetTableApplicationIDs(int rowNumber, QString& appID, QString& installedVersionID, QString& avalibleVersionID);

    QWidget* CreateAppNameTableItem(const QString& stringID);
    QWidget* CreateAppInstalledTableItem(const QString& stringID);
    QWidget* CreateAppAvalibleTableItem(Application* app);

    Ui::MainWindow* ui = nullptr;
    ApplicationManager* appManager = nullptr;

    FileDownloader* newsDownloader = nullptr;

    QPersistentModelIndex selectedListItem;
    QString selectedBranchID;

    QFont tableFont;
    ListModel* listModel = nullptr;
    QSortFilterProxyModel* filterModel = nullptr;
};

#endif // MAINWINDOW_H
