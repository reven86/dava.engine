/*==================================================================================
    Copyright (c) 2008, binaryzebra
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    * Neither the name of the binaryzebra nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE binaryzebra AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL binaryzebra BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/

#include "AssetCacheServerWindow.h"
#include "ui_AssetCacheServerWidget.h"

#include "AssetCache/AssetCacheConstants.h"

#include "ApplicationSettings.h"
#include "RemoteServerWidget.h"

#include "FileSystem/KeyedArchive.h"
#include "FileSystem/FileSystem.h"
#include "FileSystem/Logger.h"
#include "Job/JobManager.h"
#include "QtTools/FileDialog/FileDialog.h"

#include <QFileDialog>
#include <QMenu>
#include <QCloseEvent>
#include <QSettings>
#include <QCheckBox>
#include <QDoubleSpinBox>
#include <QSpinBox>

namespace
{
String DEFAULT_REMOTE_IP = "127.0.0.1";
uint16 DEFAULT_REMOTE_PORT = DAVA::AssetCache::ASSET_SERVER_PORT;
}

AssetCacheServerWindow::AssetCacheServerWindow(ServerCore& core, QWidget* parent)
    : QMainWindow(parent)
    , ui(new Ui::AssetCacheServerWidget)
    , serverCore(core)
{
    setWindowFlags((windowFlags() | Qt::CustomizeWindowHint) & ~Qt::WindowFullscreenButtonHint);

    ui->setupUi(this);

    connect(ui->cacheFolderLineEdit, &QLineEdit::textChanged, this, &AssetCacheServerWindow::OnFolderTextChanged);
    connect(ui->selectFolderButton, &QPushButton::clicked, this, &AssetCacheServerWindow::OnFolderSelection);
    connect(ui->clearDirectoryButton, &QPushButton::clicked, ui->cacheFolderLineEdit, &QLineEdit::clear);
    connect(ui->cacheSizeSpinBox, SIGNAL(valueChanged(double)), this, SLOT(OnCacheSizeChanged(double)));
    connect(ui->numberOfFilesSpinBox, SIGNAL(valueChanged(int)), this, SLOT(OnNumberOfFilesChanged(int)));
    connect(ui->autoSaveTimeoutSpinBox, SIGNAL(valueChanged(int)), this, SLOT(OnAutoSaveTimeoutChanged(int)));
    connect(ui->portSpinBox, SIGNAL(valueChanged(int)), this, SLOT(OnPortChanged(int)));
    connect(ui->autoStartCheckBox, SIGNAL(stateChanged(int)), this, SLOT(OnAutoStartChanged(int)));

    connect(ui->addNewServerButton, &QPushButton::clicked, this, &AssetCacheServerWindow::OnRemoteServerAdded);

    connect(ui->applyButton, &QPushButton::clicked, this, &AssetCacheServerWindow::OnApplyButtonClicked);
    connect(ui->closeButton, &QPushButton::clicked, this, &AssetCacheServerWindow::OnCloseButtonClicked);

    serversBoxLayout = new QVBoxLayout();
    ui->scrollAreaWidgetContents->setLayout(serversBoxLayout);
    ui->scrollAreaWidgetContents->layout()->addItem(new QSpacerItem(0, 0, QSizePolicy::Fixed, QSizePolicy::Expanding));

    CreateTrayIcon();

    ChangeSettingsState(NOT_EDITED);

    connect(&serverCore, &ServerCore::ServerStateChanged, this, &AssetCacheServerWindow::OnServerStateChanged);
    LoadSettings();
    OnServerStateChanged(&serverCore);
}

AssetCacheServerWindow::~AssetCacheServerWindow()
{
    if (trayIcon)
    {
        trayIcon->hide();
    }
    delete ui;
}

void AssetCacheServerWindow::CreateTrayIcon()
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
    ui->clearDirectoryButton->setEnabled(!ui->cacheFolderLineEdit->text().isEmpty());
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

void AssetCacheServerWindow::OnAutoStartChanged(int)
{
    VerifyData();
}

void AssetCacheServerWindow::OnRemoteServerAdded()
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
    ChangeSettingsState(EDITED);
}

void AssetCacheServerWindow::OnApplyButtonClicked()
{
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
