#include "Version.h"

#include "UI/AssetCacheServerWindow.h"
#include "ui_AssetCacheServerWidget.h"
#include "UI/SharedPoolWidget.h"
#include "UI/SharedServerWidget.h"

#include "AssetCache/AssetCacheConstants.h"

#include "ApplicationSettings.h"
#include "CustomServerWidget.h"

#include "FileSystem/KeyedArchive.h"
#include "FileSystem/FileSystem.h"
#include "FileSystem/File.h"
#include "Logger/Logger.h"
#include "Job/JobManager.h"
#include "QtTools/FileDialog/FileDialog.h"

#include <QFileDialog>
#include <QMenu>
#include <QCloseEvent>
#include <QSettings>
#include <QCheckBox>
#include <QDoubleSpinBox>
#include <QSpinBox>
#include <QLabel>
#include <QScrollBar>

#if defined(__DAVAENGINE_WINDOWS__)
#include <QSettings>
#elif defined(__DAVAENGINE_MACOS__)
#include <QXmlStreamReader>
#endif

namespace
{
const DAVA::String DEFAULT_REMOTE_IP = DAVA::AssetCache::GetLocalHost();
const DAVA::uint16 DEFAULT_REMOTE_PORT = DAVA::AssetCache::ASSET_SERVER_PORT;
}

AssetCacheServerWindow::AssetCacheServerWindow(ServerCore& core, QWidget* parent)
    : QMainWindow(parent)
    , ui(new Ui::AssetCacheServerWidget)
    , serverCore(core)
{
    setWindowFlags((windowFlags() | Qt::CustomizeWindowHint) & ~Qt::WindowFullscreenButtonHint);

    ui->setupUi(this);

    // hint to force labels have same size (for better outlook)
    ui->numberOfFilesLabel->setFixedSize(ui->numberOfFilesLabel->sizeHint());
    // the same for spin boxes
    ui->cacheSizeSpinBox->setFixedSize(ui->cacheSizeSpinBox->sizeHint());

    setWindowTitle(QString("Asset Cache Server | %1").arg(APPLICATION_BUILD_VERSION));

    connect(ui->cacheFolderLineEdit, &QLineEdit::textChanged, this, &AssetCacheServerWindow::OnFolderTextChanged);
    connect(ui->selectFolderButton, &QPushButton::clicked, this, &AssetCacheServerWindow::OnFolderSelection);
    connect(ui->systemStartupCheckBox, SIGNAL(toggled(bool)), this, SLOT(OnSystemStartupToggled(bool)));
    connect(ui->cacheSizeSpinBox, SIGNAL(valueChanged(double)), this, SLOT(OnCacheSizeChanged(double)));
    connect(ui->clearButton, &QPushButton::clicked, this, &AssetCacheServerWindow::OnClearButtonClicked);

    connect(ui->numberOfFilesSpinBox, SIGNAL(valueChanged(int)), this, SLOT(OnNumberOfFilesChanged(int)));
    connect(ui->autoSaveTimeoutSpinBox, SIGNAL(valueChanged(int)), this, SLOT(OnAutoSaveTimeoutChanged(int)));
    connect(ui->autoStartCheckBox, SIGNAL(toggled(bool)), this, SLOT(OnAutoStartToggled(bool)));
    connect(ui->restartCheckBox, SIGNAL(toggled(bool)), this, SLOT(OnRestartToggled(bool)));
    connect(ui->addNewServerButton, &QPushButton::clicked, this, &AssetCacheServerWindow::OnAddServerClicked);
    connect(ui->poolComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(OnPoolChanged(int)));
    connect(ui->serverNameEdit, &QLineEdit::textChanged, this, &AssetCacheServerWindow::OnServerNameTextChanged);

    connect(ui->applyButton, &QPushButton::clicked, this, &AssetCacheServerWindow::OnApplyButtonClicked);
    connect(ui->closeButton, &QPushButton::clicked, this, &AssetCacheServerWindow::OnCloseButtonClicked);

    connect(ui->settingsListWidget, &QListWidget::currentRowChanged, this, &AssetCacheServerWindow::ChangeSettingsPage);
    ui->settingsListWidget->setCurrentRow(0);

    serversLayout = new QVBoxLayout(this);
    customServersLayout = new QVBoxLayout(this);
    sharedServersLayout = new QVBoxLayout(this);
    serversLayout->addLayout(sharedServersLayout);
    serversLayout->addLayout(customServersLayout);
    ui->scrollAreaWidgetContents->setLayout(serversLayout);
    ui->scrollAreaWidgetContents->layout()->addItem(new QSpacerItem(0, 0, QSizePolicy::Fixed, QSizePolicy::Expanding));

    QObject::connect(ui->scrollArea->verticalScrollBar(), SIGNAL(rangeChanged(int, int)), this, SLOT(OnServersAreaRangeChanged(int, int)));

    CreateTrayMenu();

    ChangeSettingsState(NOT_EDITED);

    connect(&serverCore, &ServerCore::ServerStateChanged, this, &AssetCacheServerWindow::OnServerStateChanged);
    connect(&serverCore, &ServerCore::StorageSizeChanged, this, &AssetCacheServerWindow::UpdateUsageProgressbar);
    connect(&serverCore.Settings(), &ApplicationSettings::SettingsUpdated, this, &AssetCacheServerWindow::ReconstructSharedServersList);

    LoadSettings();
    SetupLaunchOnStartup(ui->systemStartupCheckBox->isChecked(), ui->restartCheckBox->isChecked());

    OnServerStateChanged(&serverCore);

    DAVA::uint64 occupied, overall;
    serverCore.GetStorageSpaceUsage(occupied, overall);
    UpdateUsageProgressbar(occupied, overall);
}

AssetCacheServerWindow::~AssetCacheServerWindow()
{
    if (trayIcon)
    {
        trayIcon->hide();
    }
    delete ui;
}

void AssetCacheServerWindow::CreateTrayMenu()
{
    startAction = new QAction("Start server", this);
    connect(startAction, &QAction::triggered, this, &AssetCacheServerWindow::OnStartAction);

    stopAction = new QAction("Stop server", this);
    connect(stopAction, &QAction::triggered, this, &AssetCacheServerWindow::OnStopAction);

    QAction* editAction = new QAction("Edit settings", this);
    connect(editAction, &QAction::triggered, this, &AssetCacheServerWindow::OnEditAction);

    QAction* quitAction = new QAction("Quit", this);
    connect(quitAction, &QAction::triggered, qApp, QApplication::quit);

    QMenu* trayActionsMenu = new QMenu(this);
    trayActionsMenu->addAction(startAction);
    trayActionsMenu->addAction(stopAction);
    trayActionsMenu->addSeparator();
    trayActionsMenu->addAction(editAction);
    trayActionsMenu->addSeparator();
    trayActionsMenu->addAction(quitAction);

    QIcon windowIcon(":/icon/TrayIcon.png");
    setWindowIcon(windowIcon);

    greenGreenTrayIcon.reset(new QIcon(":/icon/TrayIcon_green_green.png"));
    greenGrayTrayIcon.reset(new QIcon(":/icon/TrayIcon_green_gray.png"));
    greenRedTrayIcon.reset(new QIcon(":/icon/TrayIcon_green_red.png"));
    redGrayTrayIcon.reset(new QIcon(":/icon/TrayIcon_red_gray.png"));

    trayIcon = new QSystemTrayIcon(this);
    trayIcon->setIcon(*redGrayTrayIcon);
    trayIcon->setToolTip("Asset Cache Server");
    trayIcon->setContextMenu(trayActionsMenu);
    trayIcon->show();

    connect(trayIcon, &QSystemTrayIcon::activated, this, &AssetCacheServerWindow::OnTrayIconActivated);
}

void AssetCacheServerWindow::ChangeSettingsPage(int newRow)
{
    ui->stackedWidget->setCurrentIndex(newRow);
}

void AssetCacheServerWindow::closeEvent(QCloseEvent* e)
{
    hide();
    e->ignore();

    if (settingsState != NOT_EDITED)
    {
        LoadSettings();
    }
}

void AssetCacheServerWindow::ChangeSettingsState(SettingsState newState)
{
    settingsState = newState;
    ui->applyButton->setEnabled(settingsState == EDITED);
}

void AssetCacheServerWindow::OnFirstLaunch()
{
    show();
}

void AssetCacheServerWindow::SetupLaunchOnStartup(bool toLaunchOnStartup, bool toRestartOnCrash)
{
#if defined(__DAVAENGINE_WINDOWS__)

    QSettings settings("HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Run", QSettings::NativeFormat);
    if (toLaunchOnStartup)
    {
        settings.setValue("AssetCacheServer", QDir::toNativeSeparators(QCoreApplication::applicationFilePath()));
    }
    else
    {
        settings.remove("AssetCacheServer");
    }
    
#elif defined(__DAVAENGINE_MACOS__)

    DAVA::FilePath plist("~/Library/LaunchAgents/AssetCacheServer.plist");
    DAVA::FileSystem::Instance()->DeleteFile(plist);

    if (toLaunchOnStartup)
    {
        QByteArray buffer;
        buffer.reserve(1024);
        QXmlStreamWriter xml(&buffer);

        xml.writeStartDocument();
        xml.writeDTD("<!DOCTYPE plist PUBLIC \"-//Apple Computer//DTD PLIST 1.0//EN\" \"http://www.apple.com/DTDs/PropertyList-1.0.dtd\">");
        xml.writeStartElement("plist");
        xml.writeAttribute("version", "1.0");

        xml.writeStartElement("dict");
        xml.writeTextElement("key", "Label");
        xml.writeTextElement("string", "com.davaconsulting.assetcacheserver");
        xml.writeTextElement("key", "ProgramArguments");
        xml.writeStartElement("array");
        xml.writeTextElement("string", QDir::toNativeSeparators(QCoreApplication::applicationFilePath()));
        xml.writeEndElement();
        xml.writeTextElement("key", "RunAtLoad");
        xml.writeStartElement("true");
        xml.writeEndElement();

        xml.writeTextElement("key", "KeepAlive");
        if (toRestartOnCrash)
        {
            xml.writeStartElement("dict");
            xml.writeTextElement("key", "SuccessfulExit");
            xml.writeStartElement("false");
            xml.writeEndElement();
            xml.writeEndElement();
        }
        else
        {
            xml.writeStartElement("false");
            xml.writeEndElement();
        }

        xml.writeEndElement();

        xml.writeEndElement();
        xml.writeEndDocument();

        DAVA::ScopedPtr<DAVA::File> file(DAVA::File::PureCreate(plist, DAVA::File::CREATE | DAVA::File::WRITE));
        DVASSERT(file);
        file->Write(buffer.data(), buffer.size());
    }
    
#endif
}

void AssetCacheServerWindow::OnTrayIconActivated(QSystemTrayIcon::ActivationReason reason)
{
    switch (reason)
    {
    case QSystemTrayIcon::DoubleClick:
    {
        showNormal();
        activateWindow();
        raise();
        break;
    }
    default:
        break;
    }
}

void AssetCacheServerWindow::OnFolderSelection()
{
    QString startPath = ui->cacheFolderLineEdit->text();
    if (startPath.isEmpty())
    {
        startPath = QDir::currentPath();
    }

    QString directory = FileDialog::getExistingDirectory(this, "Choose directory", startPath,
                                                         QFileDialog::ShowDirsOnly);

    if (!directory.isEmpty())
    {
        ui->cacheFolderLineEdit->setText(directory);
        VerifyChanges();
    }
}

void AssetCacheServerWindow::OnFolderTextChanged()
{
    ui->cacheFolderLineEdit->setFocus();
    VerifyChanges();
}

void AssetCacheServerWindow::OnCacheSizeChanged(double)
{
    VerifyChanges();
}

void AssetCacheServerWindow::OnNumberOfFilesChanged(int)
{
    VerifyChanges();
}

void AssetCacheServerWindow::OnAutoSaveTimeoutChanged(int)
{
    VerifyChanges();
}

void AssetCacheServerWindow::OnPortChanged(int)
{
    VerifyChanges();
}

void AssetCacheServerWindow::OnHttpPortChanged(int)
{
    VerifyChanges();
}

void AssetCacheServerWindow::OnAutoStartToggled(bool)
{
    VerifyChanges();
}

void AssetCacheServerWindow::OnServerNameTextChanged()
{
    VerifyChanges();
}

void AssetCacheServerWindow::OnPoolChanged(int)
{
    VerifyChanges();
}

void AssetCacheServerWindow::OnSystemStartupToggled(bool checked)
{
    ui->restartCheckBox->setEnabled(checked);
    VerifyChanges();
}

void AssetCacheServerWindow::OnRestartToggled(bool)
{
    VerifyChanges();
}

void AssetCacheServerWindow::OnServersAreaRangeChanged(int min, int max)
{
    if (customServerManuallyAdded)
    {
        customServerManuallyAdded = false;
        ui->scrollArea->verticalScrollBar()->setValue(max);
    }
}

void AssetCacheServerWindow::OnAddServerClicked()
{
    AddCustomServer(RemoteServerParams(DEFAULT_REMOTE_IP, DEFAULT_REMOTE_PORT, false));
    customServerManuallyAdded = true;
    VerifyChanges();
}

void AssetCacheServerWindow::OnCustomServerRemoved()
{
    CustomServerWidget* serverWidget = qobject_cast<CustomServerWidget*>(sender());
    customServersWidgets.remove(serverWidget);

    serverWidget->deleteLater();
    VerifyChanges();
}

void AssetCacheServerWindow::OnRemoteServerEdited()
{
    VerifyChanges();
}

void AssetCacheServerWindow::OnSharedPoolChecked(bool checked)
{
    if (checked)
    {
        SharedPoolWidget* poolWidget = qobject_cast<SharedPoolWidget*>(sender());
        ClearAllChecks();
        poolWidget->SetChecked(true);
    }

    VerifyChanges();
}

void AssetCacheServerWindow::OnSharedServerChecked(bool checked)
{
    if (checked)
    {
        SharedServerWidget* serverWidget = qobject_cast<SharedServerWidget*>(sender());
        ClearAllChecks();
        serverWidget->SetChecked(true);
    }

    VerifyChanges();
}

void AssetCacheServerWindow::OnCustomServerChecked(bool checked)
{
    if (checked)
    {
        CustomServerWidget* serverWidget = qobject_cast<CustomServerWidget*>(sender());
        ClearAllChecks();
        serverWidget->SetChecked(true);
    }

    VerifyChanges();
}

void AssetCacheServerWindow::ClearAllChecks()
{
    for_each(sharedPoolsWidgets.begin(), sharedPoolsWidgets.end(), [](SharedPoolWidget* w) { w->SetChecked(false); });
    for_each(sharedServersWidgets.begin(), sharedServersWidgets.end(), [](SharedServerWidget* w) { w->SetChecked(false); });
    for_each(customServersWidgets.begin(), customServersWidgets.end(), [](CustomServerWidget* w) { w->SetChecked(false); });
}

AssetCacheServerWindow::CheckedRemote::CheckedRemote(const SharedPoolWidget* w)
    : type(AssetCacheServerWindow::CheckedRemote::POOL)
    , poolID(w->GetPoolID())
{
}

AssetCacheServerWindow::CheckedRemote::CheckedRemote(const SharedServerWidget* w)
    : type(AssetCacheServerWindow::CheckedRemote::POOL_SERVER)
    , poolID(w->GetPoolID())
    , serverID(w->GetServerID())
{
}

AssetCacheServerWindow::CheckedRemote::CheckedRemote(const CustomServerWidget* w)
    : type(AssetCacheServerWindow::CheckedRemote::CUSTOM_SERVER)
{
}

AssetCacheServerWindow::CheckedRemote AssetCacheServerWindow::GetCheckedRemote() const
{
    for (const SharedPoolWidget* serverWidget : sharedPoolsWidgets)
    {
        if (serverWidget->IsChecked())
            return CheckedRemote(serverWidget);
    }

    for (const SharedServerWidget* serverWidget : sharedServersWidgets)
    {
        if (serverWidget->IsChecked())
            return CheckedRemote(serverWidget);
    }

    for (const CustomServerWidget* serverWidget : customServersWidgets)
    {
        if (serverWidget->IsChecked())
            return CheckedRemote(serverWidget);
    }

    return CheckedRemote();
}

void AssetCacheServerWindow::OnEditAction()
{
    show();
    raise();
}

void AssetCacheServerWindow::OnStartAction()
{
    serverCore.Start();
}

void AssetCacheServerWindow::OnStopAction()
{
    serverCore.Stop();
}

void AssetCacheServerWindow::SelectedRemoteSetText()
{
    EnabledRemote enabledRemote = serverCore.Settings().GetEnabledRemote();

    switch (enabledRemote.type)
    {
    case EnabledRemote::POOL:
    {
        ui->selectedRemoteLineEdit->setText(enabledRemote.pool->poolName.c_str());
        break;
    }
    case EnabledRemote::POOL_SERVER:
    {
        const DAVA::Map<PoolID, SharedPool>& pools = serverCore.Settings().GetSharedPools();
        const auto& found = pools.find(enabledRemote.server->poolID);
        DVASSERT(found != pools.end());
        const SharedPool& pool = found->second;
        ui->selectedRemoteLineEdit->setText(DAVA::Format("%s / %s", pool.poolName.c_str(), enabledRemote.server->serverName.c_str()).c_str());
        break;
    }
    case EnabledRemote::CUSTOM_SERVER:
    {
        ui->selectedRemoteLineEdit->setText(DAVA::Format("%s:%u", enabledRemote.customServer->ip.c_str(), enabledRemote.customServer->port).c_str());
        break;
    }
    case EnabledRemote::NONE:
    default:
    {
        ui->selectedRemoteLineEdit->setText("none");
        break;
    }
    }
}

void AssetCacheServerWindow::ReconstructServersList()
{
    ReconstructSharedServersList();
    ReconstructCustomServersList();
}

void AssetCacheServerWindow::ReconstructSharedServersList()
{
    RemoveSharedServers();

    while (ui->poolComboBox->count() > 0)
    {
        ui->poolComboBox->removeItem(0);
    }
    comboBoxIDs.clear();

    const DAVA::Map<PoolID, SharedPool>& pools = serverCore.Settings().GetSharedPools();
    comboBoxIDs.reserve(pools.size());

    for (auto& poolEntry : pools)
    {
        const SharedPool& pool = poolEntry.second;
        AddSharedPool(pool);

        ui->poolComboBox->addItem(pool.poolID == 0 ? "none" : pool.poolName.c_str());
        comboBoxIDs.push_back(pool.poolID);
    }
}

void AssetCacheServerWindow::RemoveSharedServers()
{
    while (!sharedPoolsWidgets.empty())
    {
        DAVA::Logger::Debug("remove pool widget");
        sharedServersLayout->removeWidget(sharedPoolsWidgets.front());
        sharedPoolsWidgets.front()->deleteLater();
        sharedPoolsWidgets.pop_front();
    }

    while (!sharedServersWidgets.empty())
    {
        DAVA::Logger::Debug("remove server widget");
        sharedServersLayout->removeWidget(sharedServersWidgets.front());
        sharedServersWidgets.front()->deleteLater();
        sharedServersWidgets.pop_front();
    }
}

void AssetCacheServerWindow::AddSharedPool(const SharedPool& pool)
{
    if (pool.poolID == 0)
    {
        //sharedServersLayout->addWidget(new QLabel("Shared servers without pools:", this));
    }
    else
    {
        SharedPoolWidget* poolWidget = new SharedPoolWidget(pool, this);
        connect(poolWidget, SIGNAL(PoolChecked(bool)), this, SLOT(OnSharedPoolChecked(bool)));
        sharedServersLayout->addWidget(poolWidget);
        sharedPoolsWidgets.push_back(poolWidget);
    }

    for (auto serverEntry : pool.servers)
    {
        SharedServer& server = serverEntry.second;
        DAVA::Logger::Debug("adding server %u '%s' widget", server.serverID, server.serverName.c_str());
        SharedServerWidget* serverWidget = new SharedServerWidget(server, this);
        connect(serverWidget, SIGNAL(ServerChecked(bool)), this, SLOT(OnSharedServerChecked(bool)));
        sharedServersLayout->addWidget(serverWidget);
        sharedServersWidgets.push_back(serverWidget);
    }

    //     QFrame* horizontalLine = new QFrame(this);
    //     horizontalLine->setFrameShape(QFrame::HLine);
    //     sharedServersLayout->addSpacing(10);
    //     sharedServersLayout->addWidget(horizontalLine);
    //     sharedServersLayout->addSpacing(10);
}

void AssetCacheServerWindow::ReconstructCustomServersList()
{
    RemoveCustomServers();

    auto& servers = serverCore.Settings().GetCustomServers();
    for (const RemoteServerParams& sd : servers)
    {
        AddCustomServer(sd);
    }
}

void AssetCacheServerWindow::RemoveCustomServers()
{
    while (!customServersWidgets.empty())
    {
        customServersWidgets.front()->deleteLater();
        customServersWidgets.pop_front();
    }
}

void AssetCacheServerWindow::AddCustomServer(const RemoteServerParams& newServer)
{
    CustomServerWidget* serverWidget = new CustomServerWidget(newServer, this);
    customServersWidgets.push_back(serverWidget);

    connect(serverWidget, &CustomServerWidget::RemoveLater, this, &AssetCacheServerWindow::OnCustomServerRemoved);
    connect(serverWidget, &CustomServerWidget::ParametersChanged, this, &AssetCacheServerWindow::OnRemoteServerEdited);
    connect(serverWidget, SIGNAL(ServerChecked(bool)), this, SLOT(OnCustomServerChecked(bool)));

    customServersLayout->addWidget(serverWidget);

    VerifyChanges();
}

void AssetCacheServerWindow::VerifyChanges()
{
    SettingsState newState = NOT_EDITED;
    if (ui->cacheFolderLineEdit->text().isEmpty())
    {
        newState = EDITED_NOT_CORRECT;
    }
    else
    {
        newState = EDITED;
    }

    ChangeSettingsState(newState);
}

void AssetCacheServerWindow::OnClearButtonClicked()
{
    serverCore.ClearStorage();
}

void AssetCacheServerWindow::OnApplyButtonClicked()
{
    bool toLaunchOnStartup = ui->systemStartupCheckBox->isChecked();
    bool toRestartOnCrash = ui->restartCheckBox->isChecked();
    if (serverCore.Settings().IsLaunchOnSystemStartup() != toLaunchOnStartup)
    {
        SetupLaunchOnStartup(toLaunchOnStartup, toRestartOnCrash);
    }

    SaveSettings();
}

void AssetCacheServerWindow::OnCloseButtonClicked()
{
    hide();

    if (settingsState != NOT_EDITED)
    {
        LoadSettings();
    }
}

void AssetCacheServerWindow::SaveSettings()
{
    serverCore.Settings().SetFolder(ui->cacheFolderLineEdit->text().toStdString());
    serverCore.Settings().SetCacheSizeGb(ui->cacheSizeSpinBox->value());
    serverCore.Settings().SetFilesCount(ui->numberOfFilesSpinBox->value());
    serverCore.Settings().SetAutoSaveTimeoutMin(ui->autoSaveTimeoutSpinBox->value());
    serverCore.Settings().SetAutoStart(ui->autoStartCheckBox->isChecked());
    serverCore.Settings().SetLaunchOnSystemStartup(ui->systemStartupCheckBox->isChecked());
    serverCore.Settings().SetRestartOnCrash(ui->restartCheckBox->isChecked());

    serverCore.Settings().DisableRemote();

    serverCore.Settings().ClearCustomServers();
    for (auto& server : customServersWidgets)
    {
        serverCore.Settings().AddCustomServer(server->GetServerData());
    }

    CheckedRemote checkedRemote = GetCheckedRemote();
    if (checkedRemote.type == CheckedRemote::POOL)
        serverCore.Settings().EnableSharedPool(checkedRemote.poolID);
    else if (checkedRemote.type == CheckedRemote::POOL_SERVER)
        serverCore.Settings().EnableSharedServer(checkedRemote.poolID, checkedRemote.serverID);

    serverCore.Settings().SetOwnPoolID(comboBoxIDs[ui->poolComboBox->currentIndex()]);
    serverCore.Settings().SetOwnName(ui->serverNameEdit->text().toStdString());
    serverCore.Settings().SetSharedForOthers(ui->shareCheckBox->isChecked());

    serverCore.Settings().Save();

    SelectedRemoteSetText();
    ChangeSettingsState(NOT_EDITED);
}

void AssetCacheServerWindow::LoadSettings()
{
    bool blocked = blockSignals(true);

    ui->cacheFolderLineEdit->setText(serverCore.Settings().GetFolder().GetAbsolutePathname().c_str());
    ui->cacheSizeSpinBox->setValue(serverCore.Settings().GetCacheSizeGb());
    ui->numberOfFilesSpinBox->setValue(serverCore.Settings().GetFilesCount());
    ui->autoSaveTimeoutSpinBox->setValue(serverCore.Settings().GetAutoSaveTimeoutMin());
    ui->autoStartCheckBox->setChecked(serverCore.Settings().IsAutoStart());
    ui->systemStartupCheckBox->setChecked(serverCore.Settings().IsLaunchOnSystemStartup());
    ui->restartCheckBox->setEnabled(serverCore.Settings().IsLaunchOnSystemStartup());
    ui->restartCheckBox->setChecked(serverCore.Settings().IsRestartOnCrash());

    ReconstructServersList();
    SelectedRemoteSetText();
    ui->serverNameEdit->setText(serverCore.Settings().GetOwnName().c_str());
    ui->shareCheckBox->setChecked(serverCore.Settings().IsSharedForOthers());

    blockSignals(blocked);

    ChangeSettingsState(NOT_EDITED);
}

void AssetCacheServerWindow::OnServerStateChanged(const ServerCore* server)
{
    DVASSERT(&serverCore == server);

    auto serverState = serverCore.GetState();
    auto remoteState = serverCore.GetRemoteState();

    switch (serverState)
    {
    case ServerCore::State::STARTED:
    {
        switch (remoteState)
        {
        case ServerCore::RemoteState::STARTED:
        {
            trayIcon->setIcon(*greenGreenTrayIcon);
            break;
        }
        case ServerCore::RemoteState::CONNECTING:
        case ServerCore::RemoteState::VERIFYING:
        case ServerCore::RemoteState::WAITING_REATTEMPT:
        {
            trayIcon->setIcon(*greenRedTrayIcon);
            break;
        }
        case ServerCore::RemoteState::STOPPED:
        {
            trayIcon->setIcon(*greenGrayTrayIcon);
            break;
        }
        default:
        {
            DVASSERT(false && "Unknown remote state");
        }
        }

        startAction->setDisabled(true);
        stopAction->setEnabled(true);
        break;
    }
    case ServerCore::State::STOPPED:
    {
        trayIcon->setIcon(*redGrayTrayIcon);
        startAction->setEnabled(true);
        stopAction->setDisabled(true);
        break;
    }
    }
}

void AssetCacheServerWindow::UpdateUsageProgressbar(DAVA::uint64 occupied, DAVA::uint64 overall)
{
    DAVA::float64 p = overall ? (100. / static_cast<DAVA::float64>(overall)) : 0;
    int val = static_cast<int>(p * static_cast<DAVA::float64>(occupied));
    ui->occupiedSizeBar->setRange(0, 100);
    ui->occupiedSizeBar->setValue(val);
}
