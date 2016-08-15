#include "Version.h"

#include "AssetCacheServerWindow.h"
#include "ui_AssetCacheServerWidget.h"

#include "AssetCache/AssetCacheConstants.h"

#include "ApplicationSettings.h"
#include "RemoteServerWidget.h"

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

#if defined(__DAVAENGINE_WINDOWS__)
#include <QSettings>
#elif defined(__DAVAENGINE_MACOS__)
#include <QXmlStreamReader>
#endif

using namespace DAVA;

namespace
{
const String DEFAULT_REMOTE_IP = AssetCache::GetLocalHost();
const uint16 DEFAULT_REMOTE_PORT = AssetCache::ASSET_SERVER_PORT;
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
    connect(ui->portSpinBox, SIGNAL(valueChanged(int)), this, SLOT(OnPortChanged(int)));
    connect(ui->autoStartCheckBox, SIGNAL(toggled(bool)), this, SLOT(OnAutoStartToggled(bool)));
    connect(ui->restartCheckBox, SIGNAL(toggled(bool)), this, SLOT(OnRestartToggled(bool)));
    connect(ui->advancedLabel, &QLabel::linkActivated, this, &AssetCacheServerWindow::OnAdvancedLinkActivated);

    connect(ui->addNewServerButton, &QPushButton::clicked, this, &AssetCacheServerWindow::OnAddServerClicked);

    connect(ui->applyButton, &QPushButton::clicked, this, &AssetCacheServerWindow::OnApplyButtonClicked);
    connect(ui->closeButton, &QPushButton::clicked, this, &AssetCacheServerWindow::OnCloseButtonClicked);

    serversBoxLayout = new QVBoxLayout();
    ui->scrollAreaWidgetContents->setLayout(serversBoxLayout);
    ui->scrollAreaWidgetContents->layout()->addItem(new QSpacerItem(0, 0, QSizePolicy::Fixed, QSizePolicy::Expanding));

    ShowAdvancedSettings(showAdvanced);
    CreateTrayMenu();

    ChangeSettingsState(NOT_EDITED);

    connect(&serverCore, &ServerCore::ServerStateChanged, this, &AssetCacheServerWindow::OnServerStateChanged);
    connect(&serverCore, &ServerCore::StorageSizeChanged, this, &AssetCacheServerWindow::UpdateUsageProgressbar);

    LoadSettings();
    SetupLaunchOnStartup(ui->systemStartupCheckBox->isChecked(), ui->restartCheckBox->isChecked());

    OnServerStateChanged(&serverCore);

    uint64 occupied, overall;
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

    FilePath plist("~/Library/LaunchAgents/AssetCacheServer.plist");
    FileSystem::Instance()->DeleteFile(plist);

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

        ScopedPtr<File> file(File::PureCreate(plist, File::CREATE | File::WRITE));
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
        VerifyData();
    }
}

void AssetCacheServerWindow::OnFolderTextChanged()
{
    ui->cacheFolderLineEdit->setFocus();
    VerifyData();
}

void AssetCacheServerWindow::OnCacheSizeChanged(double)
{
    VerifyData();
}

void AssetCacheServerWindow::OnNumberOfFilesChanged(int)
{
    VerifyData();
}

void AssetCacheServerWindow::OnAutoSaveTimeoutChanged(int)
{
    VerifyData();
}

void AssetCacheServerWindow::OnPortChanged(int)
{
    VerifyData();
}

void AssetCacheServerWindow::OnAutoStartToggled(bool)
{
    VerifyData();
}

void AssetCacheServerWindow::OnSystemStartupToggled(bool checked)
{
    ui->restartCheckBox->setEnabled(checked);
    VerifyData();
}

void AssetCacheServerWindow::OnRestartToggled(bool)
{
    VerifyData();
}

void AssetCacheServerWindow::OnAdvancedLinkActivated(const QString& link)
{
    ShowAdvancedSettings(!showAdvanced);
}

void AssetCacheServerWindow::ShowAdvancedSettings(bool show)
{
    ui->emptyLabel->setVisible(show);

    ui->numberOfFilesLabel->setVisible(show);
    ui->numberOfFilesLabel2->setVisible(show);
    ui->numberOfFilesSpinBox->setVisible(show);

    ui->autoSaveLabel->setVisible(show);
    ui->autoSaveLabel2->setVisible(show);
    ui->autoSaveTimeoutSpinBox->setVisible(show);

    ui->portLabel->setVisible(show);
    ui->portLabel2->setVisible(show);
    ui->portSpinBox->setVisible(show);

    ui->autoStartLabel->setVisible(show);
    ui->autoStartLabel2->setVisible(show);
    ui->autoStartCheckBox->setVisible(show);
    
#if defined(__DAVAENGINE_MACOS__) // restart functionality supported on macos platform only
    ui->restartLabel->setVisible(show);
    ui->restartLabel2->setVisible(show);
    ui->restartLabel3->setVisible(show);
    ui->restartCheckBox->setVisible(show);
#else
    ui->restartLabel->setVisible(false);
    ui->restartLabel2->setVisible(false);
    ui->restartLabel3->setVisible(false);
    ui->restartCheckBox->setVisible(false);
#endif

    ui->advancedLabel->setText(
    QString("<html><head/><body><p><a href=\"adv\"><span style=\" text - decoration: underline; color:#0000ff; \">%1 advanced settings</span></a></p></body></html>")
    .arg(show ? "Hide" : "Show"));
    showAdvanced = show;
}

void AssetCacheServerWindow::OnAddServerClicked()
{
    AddRemoteServer(ServerData(DEFAULT_REMOTE_IP, DEFAULT_REMOTE_PORT, false));
    VerifyData();
}

void AssetCacheServerWindow::OnRemoteServerRemoved()
{
    RemoteServerWidget* remoteServer = qobject_cast<RemoteServerWidget*>(sender());
    remoteServers.remove(remoteServer);

    remoteServer->deleteLater();
    VerifyData();
}

void AssetCacheServerWindow::OnRemoteServerEdited()
{
    VerifyData();
}

void AssetCacheServerWindow::OnRemoteServerChecked(bool checked)
{
    if (checked)
    {
        RemoteServerWidget* checkedServer = qobject_cast<RemoteServerWidget*>(sender());
        for (auto& nextServer : remoteServers)
        {
            if (nextServer->IsChecked() && nextServer != checkedServer)
            {
                nextServer->SetChecked(false);
                break;
            }
        }
    }

    VerifyData();
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

void AssetCacheServerWindow::AddRemoteServer(const ServerData& newServer)
{
    RemoteServerWidget* server = new RemoteServerWidget(newServer, this);
    remoteServers.push_back(server);

    connect(server, &RemoteServerWidget::RemoveLater, this, &AssetCacheServerWindow::OnRemoteServerRemoved);
    connect(server, &RemoteServerWidget::ParametersChanged, this, &AssetCacheServerWindow::OnRemoteServerEdited);
    connect(server, SIGNAL(ServerChecked(bool)), this, SLOT(OnRemoteServerChecked(bool)));

    serversBoxLayout->insertWidget(serversBoxLayout->count() - 1, server);

    VerifyData();
}

void AssetCacheServerWindow::RemoveServers()
{
    while (!remoteServers.empty())
    {
        remoteServers.front()->deleteLater();
        remoteServers.pop_front();
    }
}

void AssetCacheServerWindow::VerifyData()
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
    serverCore.Settings().SetPort(ui->portSpinBox->value());
    serverCore.Settings().SetAutoStart(ui->autoStartCheckBox->isChecked());
    serverCore.Settings().SetLaunchOnSystemStartup(ui->systemStartupCheckBox->isChecked());
    serverCore.Settings().SetRestartOnCrash(ui->restartCheckBox->isChecked());

    serverCore.Settings().ResetServers();
    for (auto& server : remoteServers)
    {
        serverCore.Settings().AddServer(server->GetServerData());
    }

    serverCore.Settings().Save();
    ChangeSettingsState(NOT_EDITED);
}

void AssetCacheServerWindow::LoadSettings()
{
    bool blocked = blockSignals(true);

    ui->cacheFolderLineEdit->setText(serverCore.Settings().GetFolder().GetAbsolutePathname().c_str());
    ui->cacheSizeSpinBox->setValue(serverCore.Settings().GetCacheSizeGb());
    ui->numberOfFilesSpinBox->setValue(serverCore.Settings().GetFilesCount());
    ui->autoSaveTimeoutSpinBox->setValue(serverCore.Settings().GetAutoSaveTimeoutMin());
    ui->portSpinBox->setValue(serverCore.Settings().GetPort());
    ui->autoStartCheckBox->setChecked(serverCore.Settings().IsAutoStart());
    ui->systemStartupCheckBox->setChecked(serverCore.Settings().IsLaunchOnSystemStartup());
    ui->restartCheckBox->setEnabled(serverCore.Settings().IsLaunchOnSystemStartup());
    ui->restartCheckBox->setChecked(serverCore.Settings().IsRestartOnCrash());

    RemoveServers();
    auto& servers = serverCore.Settings().GetServers();
    for (auto& sd : servers)
    {
        AddRemoteServer(sd);
    }

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

void AssetCacheServerWindow::UpdateUsageProgressbar(uint64 occupied, uint64 overall)
{
    float64 p = overall ? (100. / static_cast<float64>(overall)) : 0;
    int val = static_cast<int>(p * static_cast<float64>(occupied));
    ui->occupiedSizeBar->setRange(0, 100);
    ui->occupiedSizeBar->setValue(val);
}
