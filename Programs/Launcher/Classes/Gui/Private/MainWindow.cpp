#include "ui_mainwindow.h"
#include "Gui/MainWindow.h"
#include "Gui/ButtonsWidget.h"
#include "Gui/Models/BranchesListModel.h"
#include "Gui/Models/BranchesFilterModel.h"

#include "Data/ConfigParser.h"

#include "Core/ApplicationManager.h"
#include "Core/Tasks/DownloadTask.h"

#include "Utils/ErrorMessenger.h"

#include "defines.h"

#include <QSet>
#include <QQueue>
#include <QInputDialog>
#include <QMenu>
#include <QLabel>
#include <QVariant>
#include <QComboBox>

namespace MainWindowDetails
{
const QString stateKey = "mainWindow_state";
const QString geometryKey = "mainWindow_geometry";
const QString selectedBranchKey = "mainWindow_selectedBranch";
}

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

MainWindow::MainWindow(ApplicationManager* appManager_, QWidget* parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , appManager(appManager_)
{
    ui->setupUi(this);
    ui->action_updateConfiguration->setShortcuts(QList<QKeySequence>() << QKeySequence("F5") << QKeySequence("Ctrl+R"));
    connect(ui->action_updateConfiguration, &QAction::triggered, this, &MainWindow::RefreshClicked);

    connect(ui->actionPreferences, &QAction::triggered, this, &MainWindow::ShowPreferences);
    connect(ui->pushButton_cancel, &QPushButton::clicked, this, &MainWindow::CancelClicked);

    ui->tableWidget->setStyleSheet(TABLE_STYLESHEET);
    ui->tableWidget->setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContents);

    setWindowTitle(QString("DAVA Launcher %1").arg(LAUNCHER_VER));

    connect(ui->textBrowser, &QTextBrowser::anchorClicked, this, &MainWindow::OnlinkClicked);
    connect(ui->listView, &QListView::clicked, this, &MainWindow::OnListItemClicked);
    connect(ui->tableWidget, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(OnCellDoubleClicked(QModelIndex)));

    listModel = new BranchesListModel(appManager, this);
    filterModel = new BranchesFilterModel(this);
    filterModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
    connect(ui->lineEdit_search, &QLineEdit::textChanged, filterModel, &QSortFilterProxyModel::setFilterFixedString);
    filterModel->setSourceModel(listModel);
    ui->listView->setModel(filterModel);

    using namespace std::placeholders;
    receiver.onStarted = std::bind(&MainWindow::OnTaskStarted, this, _1);
    receiver.onProgress = std::bind(&MainWindow::OnTaskProgress, this, _1, _2);
    receiver.onFinished = std::bind(&MainWindow::OnTaskFinished, this, _1);

    QSettings settings;
    restoreGeometry(settings.value(MainWindowDetails::geometryKey).toByteArray());
    restoreState(settings.value(MainWindowDetails::stateKey).toByteArray());
    selectedBranchID = settings.value(MainWindowDetails::selectedBranchKey).toString();

    OnConnectedChanged(false);
}

MainWindow::~MainWindow()
{
    QSettings settings;
    settings.setValue(MainWindowDetails::geometryKey, saveGeometry());
    settings.setValue(MainWindowDetails::stateKey, saveState());
    settings.setValue(MainWindowDetails::selectedBranchKey, selectedBranchID);
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

const Receiver& MainWindow::GetReceiver() const
{
    return receiver;
}

void MainWindow::OnRun(int rowNumber)
{
    QString appID, insVersionID, avVersionID;
    GetTableApplicationIDs(rowNumber, appID, insVersionID, avVersionID);
    if (!appID.isEmpty() && !insVersionID.isEmpty())
        appManager->RunApplication(selectedBranchID, appID, insVersionID);
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
    InstallApplicationParams params;
    params.branch = selectedBranchID;
    params.app = appID;
    params.newVersion = *newVersion;
    appManager->InstallApplication(params);
}

void MainWindow::OnRemove(int rowNumber)
{
    QString appID, insVersionID, avVersionID;
    GetTableApplicationIDs(rowNumber, appID, insVersionID, avVersionID);
    if (!appID.isEmpty() && !insVersionID.isEmpty())
    {
        appManager->RemoveApplication(selectedBranchID, appID);
        RefreshApps();
    }
}

void MainWindow::RefreshApps()
{
    RefreshBranchesList();

    QAbstractItemModel* model = ui->listView->model();
    QModelIndex first = model->index(0, 0);
    QModelIndexList foundIndexes = model->match(first, BranchesListModel::DAVA_WIDGET_ROLE, selectedBranchID, 1, Qt::MatchExactly);
    if (foundIndexes.isEmpty() == false)
    {
        ui->listView->setCurrentIndex(foundIndexes.first());
    }
    ShowTable(selectedBranchID);
}

void MainWindow::OnListItemClicked(QModelIndex qindex)
{
    QVariant var = ui->listView->model()->data(qindex, BranchesListModel::DAVA_WIDGET_ROLE);
    QString dataRole = var.toString();
    if (!dataRole.isEmpty())
    {
        selectedBranchID = dataRole;
        ShowTable(selectedBranchID);
    }
}

void MainWindow::OnNewsLoaded(const BaseTask* task)
{
    QString error = task->GetError();
    if (error.isEmpty())
    {
        Q_ASSERT(task->GetTaskType() == BaseTask::DOWNLOAD_TASK);
        const DownloadTask* downloadTask = static_cast<const DownloadTask*>(task);
        Q_ASSERT(downloadTask->GetLoadedData().empty() == false);
        ui->textBrowser->setHtml(downloadTask->GetLoadedData().front());
    }
    else
    {
        ui->textBrowser->setText(QObject::tr("Failed to load news: %1").arg(error));
    }
}

void MainWindow::ShowTable(QString branchID)
{
    ConfigParser* localConfig = appManager->GetLocalConfig();
    ConfigParser* remoteConfig = appManager->GetRemoteConfig();

    if (branchID.isEmpty() || branchID == CONFIG_LAUNCHER_WEBPAGE_KEY)
    {
        if (localConfig->GetWebpageURL().isEmpty() == false)
        {
            ui->stackedWidget->setCurrentIndex(0);
            QString description = QObject::tr("Loading news");
            ui->textBrowser->setText(description);
            Receiver tmpReceiver;
            tmpReceiver.onFinished = std::bind(&MainWindow::OnNewsLoaded, this, std::placeholders::_1);
            std::unique_ptr<BaseTask> task = appManager->CreateTask<DownloadTask>(description, localConfig->GetWebpageURL());
            appManager->AddTaskWithCustomReceivers(std::move(task), { tmpReceiver });
            return;
        }
        else
        {
            Branch* firstAvailableBranch = localConfig->GetBranch(0);
            if (firstAvailableBranch != nullptr)
            {
                branchID = firstAvailableBranch->id;
            }
        }
    }

    ui->stackedWidget->setCurrentIndex(1);
    ui->tableWidget->setRowCount(0);

    QVector<int> states;
    if (remoteConfig)
    {
        Branch* remoteBranch = remoteConfig->GetBranch(branchID);
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

    ui->tableWidget->setMaximumHeight(ui->tableWidget->sizeHint().height());
}

void MainWindow::OnConnectedChanged(bool connected)
{
    QString text = QString("BAManager is ") + (connected ? "connected" : "disconnected");
    ui->label_BAManagerStatusText->setText(text);
    QString iconPath = QString(":/Icons/") + (connected ? "green" : "red") + ".png";
    ui->label_BAManagerStatusIcon->setPixmap(QPixmap(iconPath));
}

void MainWindow::OnTaskProgress(const BaseTask* /*task*/, quint32 progress)
{
    ui->progressBar->setValue(progress);
}

void MainWindow::AddText(const QString& text, const QColor& color)
{
    QColor textColor = ui->textEdit_launcherStatus->textColor();
    ui->textEdit_launcherStatus->setTextColor(color);
    ui->textEdit_launcherStatus->append(QTime::currentTime().toString() + " : " + text);
    ui->textEdit_launcherStatus->setTextColor(textColor);
}

void MainWindow::OnTaskStarted(const BaseTask* task)
{
    AddText(task->GetDescription());

    BaseTask::eTaskType type = task->GetTaskType();
    if (type == BaseTask::DOWNLOAD_TASK || type == BaseTask::ZIP_TASK)
    {
        ui->stackedWidget_status->setCurrentIndex(0);
        ui->progressBar->setFormat(task->GetDescription() + " %p%");
        ui->progressBar->setValue(0);
    }
}

void MainWindow::OnTaskFinished(const BaseTask* task)
{
    const QString& error = task->GetError();
    if (error.isEmpty() == false)
    {
        AddText(error, "#aa0000");
    }
    BaseTask::eTaskType type = task->GetTaskType();
    if (type == BaseTask::DOWNLOAD_TASK)
    {
        OnConnectedChanged(error.isEmpty());
    }
    if (type == BaseTask::DOWNLOAD_TASK || type == BaseTask::ZIP_TASK)
    {
        ui->stackedWidget_status->setCurrentIndex(1);
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
        {
            QApplication::clipboard()->setText(version->url);
        }
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
