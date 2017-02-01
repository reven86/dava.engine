#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "configparser.h"
#include "updatedialog.h"
#include "buttonswidget.h"
#include "selfupdater.h"
#include "defines.h"
#include "filemanager.h"
#include "preferencesdialog.h"
#include "configdownloader.h"
#include "errormessenger.h"
#include "branchesListModel.h"
#include "branchesFilterModel.h"
#include "BAManagerClient.h"
#include "appscommandssender.h"
#include "configrefresher.h"

#include <QSet>
#include <QQueue>
#include <QInputDialog>
#include <QMessageBox>
#include <QMenu>
#include <QLabel>
#include <QVariant>
#include <QComboBox>
#include <QShortcut>

static const QString stateKey = "mainWindow_state";
static const QString geometryKey = "mainWindow_geometry";

class BranchListComparator
{
public:
    bool operator()(const QString& left, const QString& right) const
    {
        int leftValue, rightValue;
        if (sscanf(left.toStdString().c_str(), "%d", &leftValue) > 0 &&
            sscanf(right.toStdString().c_str(), "%d", &rightValue) > 0 &&
            leftValue != rightValue)
        {
            return leftValue > rightValue;
        }

        return left < right;
    }
};

//expected format of input string: 0.8_2015-02-14_11.20.12_0000,
//where 0.8 - DAVA version, 2015-02-14 - build date, 11.20.12 - build time and 0000 - build version
//all blocks can be modified or empty
bool VersionListComparator(const AppVersion& leftVer, const AppVersion& rightVer)
{
    const QString& left = leftVer.id;
    const QString& right = rightVer.id;
    if (left == right)
    {
        return leftVer.buildNum < rightVer.buildNum;
    }
    if (leftVer.isToolSet != rightVer.isToolSet)
    {
        //value with toolset == false must be less
        return leftVer.isToolSet == false;
    }
    QStringList leftList = left.split('_', QString::SkipEmptyParts);
    QStringList rightList = right.split('_', QString::SkipEmptyParts);

    int minSize = qMin(leftList.size(), rightList.size());
    for (int i = 0; i < minSize; ++i)
    {
        const QString& leftSubStr = leftList.at(i);
        const QString& rightSubStr = rightList.at(i);
        QStringList leftSubList = leftSubStr.split('.', QString::SkipEmptyParts);
        QStringList rightSubList = rightSubStr.split('.', QString::SkipEmptyParts);
        int subMinSize = qMin(leftSubList.size(), rightSubList.size());
        for (int subStrIndex = 0; subStrIndex < subMinSize; ++subStrIndex)
        {
            bool leftOk;
            bool rightOk;
            const QString& leftSubSubStr = leftSubList.at(subStrIndex);
            const QString& rightSubSubStr = rightSubList.at(subStrIndex);
            qlonglong leftVal = leftSubSubStr.toLongLong(&leftOk);
            qlonglong rightVal = rightSubSubStr.toLongLong(&rightOk);
            if (leftOk && rightOk)
            {
                if (leftVal != rightVal)
                {
                    return leftVal < rightVal;
                }
            }
            else //date format or other
            {
                if (leftSubSubStr != rightSubSubStr)
                {
                    return leftSubSubStr < rightSubSubStr;
                }
            }
        }
        //if version lists are equal - checking for extra subversion
        if (leftSubList.size() != rightSubList.size())
        {
            return leftSubList.size() < rightSubList.size();
        }
    }
    return false; // string are equal
};

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->action_updateConfiguration->setShortcuts(QList<QKeySequence>() << QKeySequence("F5") << QKeySequence("Ctrl+R"));
    ui->tableWidget->setStyleSheet(TABLE_STYLESHEET);

    setWindowTitle(QString("DAVA Launcher %1").arg(LAUNCHER_VER));

    connect(ui->textBrowser, &QTextBrowser::anchorClicked, this, &MainWindow::OnlinkClicked);
    connect(ui->action_updateConfiguration, &QAction::triggered, this, &MainWindow::Refresh);
    connect(ui->action_downloadAll, &QAction::triggered, this, &MainWindow::OnInstallAll);
    connect(ui->action_removeAll, &QAction::triggered, this, &MainWindow::OnRemoveAll);
    connect(ui->listView, &QListView::clicked, this, &MainWindow::OnListItemClicked);
    connect(ui->tableWidget, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(OnCellDoubleClicked(QModelIndex)));

    connect(ui->actionPreferences, &QAction::triggered, this, &MainWindow::OpenPreferencesEditor);

    appManager = new ApplicationManager(this);
    connect(appManager, &ApplicationManager::BranchChanged, this, &MainWindow::ShowTable);

    newsDownloader = new FileDownloader(this);
    configDownloader = new ConfigDownloader(appManager, this);
    baManagerClient = new BAManagerClient(appManager, this);
    configRefresher = new ConfigRefresher(this);

    connect(configRefresher, &ConfigRefresher::RefreshConfig, this, &MainWindow::Refresh);

    //create secret shortcut
    //it will be used to get commands manually for testing reasons
    QShortcut* shortCut = new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_G), this);
    shortCut->setContext(Qt::ApplicationShortcut);
    connect(shortCut, &QShortcut::activated, baManagerClient, &BAManagerClient::AskForCommands);

    connect(newsDownloader, &FileDownloader::Finished, this, &MainWindow::NewsDownloadFinished);
    listModel = new BranchesListModel(appManager, this);
    filterModel = new BranchesFilterModel(this);
    filterModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
    connect(ui->lineEdit_search, &QLineEdit::textChanged, filterModel, &QSortFilterProxyModel::setFilterFixedString);
    filterModel->setSourceModel(listModel);
    ui->listView->setModel(filterModel);

    //if run this method directly qApp->exec() will be called twice
    QMetaObject::invokeMethod(this, "Refresh", Qt::QueuedConnection);

    QSettings settings;
    restoreGeometry(settings.value(geometryKey).toByteArray());
    restoreState(settings.value(stateKey).toByteArray());

    FileManager* fileManager = appManager->GetFileManager();
    ::LoadPreferences(fileManager, configDownloader, baManagerClient, configRefresher);
}

MainWindow::~MainWindow()
{
    QSettings settings;
    settings.setValue(geometryKey, saveGeometry());
    settings.setValue(stateKey, saveState());

    FileManager* fileManager = appManager->GetFileManager();
    ::SavePreferences(fileManager, configDownloader, baManagerClient, configRefresher);
    SafeDelete(ui);
}

void MainWindow::OnlinkClicked(QUrl url)
{
    QDesktopServices::openUrl(url);
}

void MainWindow::OnCellDoubleClicked(QModelIndex index)
{
    if (index.column() != COLUMN_BUTTONS)
        OnRun(index.row());
}

void MainWindow::OnRun(int rowNumber)
{
    QString appID, insVersionID, avVersionID;
    GetTableApplicationIDs(rowNumber, appID, insVersionID, avVersionID);
    if (!appID.isEmpty() && !insVersionID.isEmpty())
        appManager->RunApplication(selectedBranchID, appID, insVersionID);
}

void MainWindow::OnInstallAll()
{
    QQueue<UpdateTask> tasks;
    ConfigParser* remoteConfig = appManager->GetRemoteConfig();
    ConfigParser* localConfig = appManager->GetLocalConfig();
    for (int i = 0, count = ui->tableWidget->rowCount(); i < count; ++i)
    {
        QString appID, insVersionID, avVersionID;
        GetTableApplicationIDs(i, appID, insVersionID, avVersionID);
        AppVersion* currentVersion = localConfig->GetAppVersion(selectedBranchID, appID, insVersionID);
        AppVersion* newVersion = remoteConfig->GetAppVersion(selectedBranchID, appID, avVersionID);
        Application* remoteApplication = remoteConfig->GetApplication(selectedBranchID, appID);
        if (newVersion != nullptr && remoteApplication != nullptr)
        {
            tasks.push_back(UpdateTask(selectedBranchID, appID, currentVersion, *newVersion));
        }
    }

    if (!tasks.isEmpty())
    {
        ShowUpdateDialog(tasks);
    }
    ShowTable(selectedBranchID);
}

void MainWindow::OnRemoveAll()
{
    QMessageBox msbox(QMessageBox::Information, tr("Remove all installed applications"),
                      tr("Are you sure you want to remove all installed applications in branch %1?").arg(selectedBranchID),
                      QMessageBox::Yes | QMessageBox::No);
    if (msbox.exec() == QMessageBox::No)
    {
        return;
    }

    for (int i = 0, count = ui->tableWidget->rowCount(); i < count; ++i)
    {
        QString appID, insVersionID, avVersionID;
        GetTableApplicationIDs(i, appID, insVersionID, avVersionID);
        if (!appID.isEmpty() && !insVersionID.isEmpty())
        {
            appManager->RemoveApplication(selectedBranchID, appID, true, false);
        }
    }
    ShowTable(selectedBranchID);
}

void MainWindow::OnInstall(int rowNumber)
{
    QString appID, insVersionID, avVersionID;
    GetTableApplicationIDs(rowNumber, appID, insVersionID, avVersionID);

    AppVersion* newVersion = appManager->GetRemoteConfig()->GetAppVersion(selectedBranchID, appID, avVersionID);
    Application* remoteApplication = appManager->GetRemoteConfig()->GetApplication(selectedBranchID, appID);
    if (newVersion == nullptr || remoteApplication == nullptr)
    {
        ErrorMessenger::LogMessage(QtCriticalMsg, "can not found remote application or version OnInstall");
        return;
    }
    AppVersion* currentVersion = appManager->GetLocalConfig()->GetAppVersion(selectedBranchID, appID, insVersionID);
    QQueue<UpdateTask> tasks;
    tasks.push_back(UpdateTask(selectedBranchID, appID, currentVersion, *newVersion));

    ShowUpdateDialog(tasks);

    ShowTable(selectedBranchID);
}

void MainWindow::OnRemove(int rowNumber)
{
    QString appID, insVersionID, avVersionID;
    GetTableApplicationIDs(rowNumber, appID, insVersionID, avVersionID);
    if (!appID.isEmpty() && !insVersionID.isEmpty())
    {
        QMessageBox msbox(QMessageBox::Information, "Remove application",
                          QString("Are you sure you want to remove %1?").arg(appManager->GetString(appID)),
                          QMessageBox::Yes | QMessageBox::No);
        int result = msbox.exec();

        if (result == QMessageBox::Yes && appManager->RemoveApplication(selectedBranchID, appID, true, false))
            RefreshApps();
    }
}

void MainWindow::Refresh()
{
    ui->action_updateConfiguration->setEnabled(false);
    FileManager* fileManager = appManager->GetFileManager();
    FileManager::DeleteDirectory(fileManager->GetTempDirectory());

    if (configDownloader->exec() == QDialog::Accepted)
    {
        CheckUpdates();
    }

    RefreshApps();
    ui->action_updateConfiguration->setEnabled(true);
}

void MainWindow::CheckUpdates()
{
    QQueue<UpdateTask> tasks;
    appManager->CheckUpdates(tasks);
    ShowUpdateDialog(tasks);
}

void MainWindow::RefreshApps()
{
    RefreshBranchesList();

    if (appManager->ShouldShowNews())
        selectedBranchID = CONFIG_LAUNCHER_WEBPAGE_KEY;
    else
        ui->listView->setCurrentIndex(selectedListItem);

    ShowTable(selectedBranchID);
}

void MainWindow::OnListItemClicked(QModelIndex qindex)
{
    QVariant var = ui->listView->model()->data(qindex, BranchesListModel::DAVA_WIDGET_ROLE);
    QString dataRole = var.toString();
    if (!dataRole.isEmpty())
    {
        selectedBranchID = dataRole;
        selectedListItem = ui->listView->currentIndex();
        ShowTable(selectedBranchID);
    }
}

void MainWindow::ShowWebpage()
{
    ui->stackedWidget->setCurrentIndex(0);

    const QString& webpageUrl = appManager->GetLocalConfig()->GetWebpageURL();
    newsDownloader->Download(QUrl(webpageUrl));

    appManager->NewsShowed();
}

void MainWindow::NewsDownloadFinished(QByteArray downloadedData, QList<QPair<QByteArray, QByteArray>> rawHeaderList, int errorCode, QString errorDescr)
{
    if (errorCode != QNetworkReply::NoError && errorCode != QNetworkReply::OperationCanceledError)
    {
        ui->textBrowser->setHtml(tr("Network error: %1").arg(errorDescr));
    }
    else
    {
        ui->textBrowser->setHtml(QString(downloadedData));
    }
}

void MainWindow::OpenPreferencesEditor()
{
    FileManager* fileManager = appManager->GetFileManager();
    PreferencesDialog::ShowPreferencesDialog(fileManager, configDownloader, configRefresher, this);
}

void MainWindow::ShowTable(const QString& branchID)
{
    if (branchID == CONFIG_LAUNCHER_WEBPAGE_KEY || branchID.isEmpty())
    {
        ui->action_removeAll->setEnabled(false);
        ui->action_downloadAll->setEnabled(false);
        ShowWebpage();
        return;
    }

    ui->stackedWidget->setCurrentIndex(1);
    ui->tableWidget->setRowCount(0);

    ConfigParser* localConfig = appManager->GetLocalConfig();
    ConfigParser* remoteConfig = appManager->GetRemoteConfig();

    QVector<int> states;
    if (remoteConfig)
    {
        Branch* remoteBranch = remoteConfig->GetBranch(branchID);
        ui->action_downloadAll->setEnabled(remoteBranch != nullptr && remoteBranch->GetAppCount() != 0);
        if (remoteBranch)
        {
            int appCount = remoteBranch->GetAppCount();
            ui->tableWidget->setRowCount(appCount);
            for (int i = 0; i < appCount; ++i)
            {
                int state = 0;
                QString avalibleVersion;
                Application* remoteApp = remoteBranch->GetApplication(i);

                QWidget* item = CreateAppNameTableItem(remoteApp->id, i);
                item->setMinimumWidth(120);
                ui->tableWidget->setCellWidget(i, COLUMN_APP_NAME, item);

                ui->tableWidget->setCellWidget(i, COLUMN_APP_AVAL, CreateAppAvalibleTableItem(remoteApp, i));

                int versCount = remoteApp->GetVerionsCount();
                if (versCount == 1)
                    avalibleVersion = remoteApp->GetVersion(0)->id;
                else
                    state |= ButtonsWidget::BUTTONS_STATE_AVALIBLE;

                if (localConfig)
                {
                    Application* localApp = localConfig->GetApplication(branchID, remoteApp->id);
                    if (localApp)
                    {
                        ui->tableWidget->setCellWidget(i, COLUMN_APP_INS, CreateAppInstalledTableItem(localApp->GetVersion(0)->id, i));

                        state |= ButtonsWidget::BUTTONS_STATE_INSTALLED;
                        if (avalibleVersion != localApp->GetVersion(0)->id)
                            state |= ButtonsWidget::BUTTONS_STATE_AVALIBLE;
                    }
                    else
                    {
                        state |= ButtonsWidget::BUTTONS_STATE_AVALIBLE;
                    }
                }

                states.push_back(state);
            }
        }
        Branch* localBranch = localConfig->GetBranch(branchID);
        if (localBranch)
        {
            int appCount = localBranch->GetAppCount();
            for (int i = 0; i < appCount; ++i)
            {
                Application* localApp = localBranch->GetApplication(i);
                if (remoteBranch != nullptr && remoteBranch->GetApplication(localApp->id) != nullptr)
                {
                    continue;
                }
                int rowCount = ui->tableWidget->rowCount();
                ui->tableWidget->setRowCount(rowCount + 1);
                ui->tableWidget->setCellWidget(rowCount, COLUMN_APP_NAME, CreateAppNameTableItem(localApp->id, i));
                ui->tableWidget->setCellWidget(rowCount, COLUMN_APP_INS, CreateAppInstalledTableItem(localApp->GetVersion(0)->id, rowCount));

                states.push_back(ButtonsWidget::BUTTONS_STATE_INSTALLED);
            }
        }
    }
    else if (localConfig)
    {
        Branch* localBranch = localConfig->GetBranch(branchID);
        if (localBranch)
        {
            int appCount = localBranch->GetAppCount();
            ui->tableWidget->setRowCount(appCount);
            for (int i = 0; i < appCount; ++i)
            {
                Application* localApp = localBranch->GetApplication(i);
                ui->tableWidget->setCellWidget(i, COLUMN_APP_NAME, CreateAppNameTableItem(localApp->id, i));
                ui->tableWidget->setCellWidget(i, COLUMN_APP_INS, CreateAppInstalledTableItem(localApp->GetVersion(0)->id, i));

                states.push_back(ButtonsWidget::BUTTONS_STATE_INSTALLED);
            }
        }
    }
    ui->action_removeAll->setEnabled(ui->tableWidget->rowCount() != 0);

    for (int i = 0; i < ui->tableWidget->rowCount(); ++i)
    {
        ButtonsWidget* butts = new ButtonsWidget(i, this);
        butts->SetButtonsState(states[i]);
        ui->tableWidget->setCellWidget(i, COLUMN_BUTTONS, butts);
    }

    ui->tableWidget->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    QHeaderView* hHeader = ui->tableWidget->horizontalHeader();
    hHeader->setSectionResizeMode(COLUMN_APP_NAME, QHeaderView::ResizeToContents);
    hHeader->setSectionResizeMode(COLUMN_APP_INS, QHeaderView::Stretch);
    hHeader->setSectionResizeMode(COLUMN_APP_AVAL, QHeaderView::Stretch);
    hHeader->setSectionResizeMode(COLUMN_BUTTONS, QHeaderView::ResizeToContents);
}

void MainWindow::ShowUpdateDialog(QQueue<UpdateTask>& tasks)
{
    if (tasks.isEmpty())
    {
        return;
    }
    //disable config refresher while update something
    QSignalBlocker blocker(configRefresher);
    //self-update
    if (tasks.front().isSelfUpdate)
    {
        FileManager* fileManager = appManager->GetFileManager();
        SelfUpdater updater(fileManager, tasks.front().newVersion.url, this);
        updater.setWindowModality(Qt::ApplicationModal);
        updater.exec();
        if (updater.result() != QDialog::Rejected)
        {
            return;
        }
        tasks.dequeue();
    }
    if (!tasks.isEmpty())
    {
        //application update
        UpdateDialog dialog(tasks, appManager, this);
        dialog.exec();
    }
}

void MainWindow::RefreshBranchesList()
{
    ConfigParser* localConfig = appManager->GetLocalConfig();
    ConfigParser* remoteConfig = appManager->GetRemoteConfig();
    listModel->ClearItems();

    if (!localConfig->GetWebpageURL().isEmpty())
    {
        listModel->AddItem(CONFIG_LAUNCHER_WEBPAGE_KEY, BranchesListModel::LIST_ITEM_NEWS);
        listModel->AddItem("", BranchesListModel::LIST_ITEM_SEPARATOR);
    }

    QStringList favs;
    QSet<QString> branchIDs;
    if (localConfig)
    {
        localConfig->MergeBranchesIDs(branchIDs);
        favs = localConfig->GetFavorites();
    }
    if (remoteConfig)
    {
        remoteConfig->MergeBranchesIDs(branchIDs);
        favs = remoteConfig->GetFavorites();
    }

    QList<QString> branchesList = branchIDs.toList();
    qSort(branchesList.begin(), branchesList.end(), BranchListComparator());

    int branchesCount = branchesList.size();

    //Add favorites branches
    bool hasFavorite = false;
    for (int i = 0; i < favs.size(); ++i)
    {
        const QString& branchID = favs[i];
        if (branchesList.contains(branchID))
        {
            listModel->AddItem(branchID, BranchesListModel::LIST_ITEM_FAVORITES);
            hasFavorite = true;
        }
    }
    if (hasFavorite)
        listModel->AddItem("", BranchesListModel::LIST_ITEM_SEPARATOR);

    //Add Others
    for (int i = 0; i < branchesCount; i++)
    {
        const QString& branchID = branchesList[i];
        if (!favs.contains(branchID))
            listModel->AddItem(branchID, BranchesListModel::LIST_ITEM_BRANCH);
    }
}

QWidget* MainWindow::CreateAppNameTableItem(const QString& stringID, int rowNum)
{
    QString string = appManager->GetString(stringID);
    QLabel* item = new QLabel(string);

    item->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(item, &QLabel::customContextMenuRequested, [this, item, rowNum](const QPoint& pos) {
        QString appID, insVersionID, avVersionID;
        GetTableApplicationIDs(rowNum, appID, insVersionID, avVersionID);

        AppVersion* version = appManager->GetRemoteConfig()->GetAppVersion(selectedBranchID, appID, avVersionID);
        if (version == nullptr)
        {
            return;
        }
        QMenu menu(this);
        QAction* copyURLAction = menu.addAction("Copy " + appID + " URL");
        QAction* selectedAction = menu.exec(ui->tableWidget->viewport()->mapToGlobal(pos) + item->pos());
        if (selectedAction == copyURLAction)
            QApplication::clipboard()->setText(version->url);
    });

    item->setProperty(DAVA_CUSTOM_PROPERTY_NAME, stringID);
    item->setFont(tableFont);

    return item;
}

QWidget* MainWindow::CreateAppInstalledTableItem(const QString& stringID, int rowNum)
{
    QLabel* item = new QLabel(appManager->GetString(stringID));
    item->setFont(tableFont);
    item->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    item->setContextMenuPolicy(Qt::CustomContextMenu);
    item->setProperty(DAVA_CUSTOM_PROPERTY_NAME, stringID);
    connect(item, &QLabel::customContextMenuRequested, [this, item, rowNum](const QPoint& pos) {
        QMenu menu(this);
        QAction* copyAction = menu.addAction("Copy version");
        QString actionText =
#if defined(Q_OS_WIN)
        "Show in explorer";
#elif defined(Q_OS_MAC)
        "Show in finder";
#endif //platform
        QAction* showInFinderAction = menu.addAction(actionText);
        QAction* resultAction = menu.exec(ui->tableWidget->viewport()->mapToGlobal(pos) + item->pos());

        if (resultAction == copyAction)
        {
            QApplication::clipboard()->setText(item->property(DAVA_CUSTOM_PROPERTY_NAME).toString());
        }
        else if (resultAction == showInFinderAction)
        {
            QString appID, insVersionID, avVersionID;
            GetTableApplicationIDs(rowNum, appID, insVersionID, avVersionID);
            appManager->ShowApplicataionInExplorer(selectedBranchID, appID);
        }
    });

    return item;
}

QWidget* MainWindow::CreateAppAvalibleTableItem(Application* app, int rowNum)
{
    int versCount = app->GetVerionsCount();
    if (versCount == 1)
    {
        return CreateAppInstalledTableItem(app->GetVersion(0)->id, rowNum);
    }
    else
    {
        qSort(app->versions.begin(), app->versions.end(), VersionListComparator);

        QComboBox* comboBox = new QComboBox();
        for (int j = versCount - 1; j >= 0; --j)
        {
            comboBox->addItem(app->versions[j].id);
        }
        comboBox->view()->setTextElideMode(Qt::ElideLeft);
        comboBox->setFont(tableFont);
        comboBox->setFocusPolicy(Qt::NoFocus);
        comboBox->setCurrentIndex(0);

#ifdef Q_OS_MAC
        comboBox->setMaximumHeight(26);
#endif

        return comboBox;
    }
}

void MainWindow::GetTableApplicationIDs(int rowNumber, QString& appID,
                                        QString& installedVersionID, QString& avalibleVersionID)
{
    QWidget* cell = 0;

    cell = ui->tableWidget->cellWidget(rowNumber, COLUMN_APP_NAME);
    if (cell)
        appID = cell->property(DAVA_CUSTOM_PROPERTY_NAME).toString();

    cell = ui->tableWidget->cellWidget(rowNumber, COLUMN_APP_INS);
    if (cell)
        installedVersionID = cell->property(DAVA_CUSTOM_PROPERTY_NAME).toString();

    cell = ui->tableWidget->cellWidget(rowNumber, COLUMN_APP_AVAL);
    QComboBox* cBox = dynamic_cast<QComboBox*>(cell);
    if (cBox)
        avalibleVersionID = cBox->currentText();
    else if (cell)
        avalibleVersionID = cell->property(DAVA_CUSTOM_PROPERTY_NAME).toString();
}

void MainWindow::UpdateButtonsState(int rowNumber, ButtonsWidget::ButtonsState state)
{
    ButtonsWidget* buttons = dynamic_cast<ButtonsWidget*>(ui->tableWidget->cellWidget(rowNumber, COLUMN_BUTTONS));
    if (buttons)
        buttons->SetButtonsState(state);
}
