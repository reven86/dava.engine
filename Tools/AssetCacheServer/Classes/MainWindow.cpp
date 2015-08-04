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


#include "MainWindow.h"
#include "ui_MainWindow.h"

#include "AssetCache/AssetCacheConstants.h"

#include "ApplicationSettings.h"
#include "RemoteAssetCacheServer.h"

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

namespace {
    String DEFAULT_REMOTE_IP = "127.0.0.1";
    uint16 DEFAULT_REMOTE_PORT = DAVA::AssetCache::ASSET_SERVER_PORT;
}

MainWindow::MainWindow(ServerCore& core, QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , serverCore(core)
{
    ui->setupUi(this);

    connect(ui->cacheFolderLineEdit, &QLineEdit::textChanged, this, &MainWindow::OnFolderTextChanged);
    connect(ui->selectFolderButton, &QPushButton::clicked, this, &MainWindow::OnFolderSelected);
    connect(ui->clearDirectoryButton, &QPushButton::clicked, ui->cacheFolderLineEdit, &QLineEdit::clear);
    connect(ui->cacheSizeSpinBox, SIGNAL(valueChanged(double)), this, SLOT(OnCacheSizeChanged(double)));
    connect(ui->numberOfFilesSpinBox, SIGNAL(valueChanged(int)), this, SLOT(OnNumberOfFilesChanged(int)));
    connect(ui->autoSaveTimeoutSpinBox, SIGNAL(valueChanged(int)), this, SLOT(OnAutoSaveTimeoutChanged(int)));
    connect(ui->portSpinBox, SIGNAL(valueChanged(int)), this, SLOT(OnPortChanged(int)));
    connect(ui->autoStartCheckBox, SIGNAL(stateChanged(int)), this, SLOT(OnAutoStartChanged(int)));

    connect(ui->addNewServerButton, &QPushButton::clicked, this, &MainWindow::OnRemoteServerAdded);

    connect(ui->applyButton, &QPushButton::clicked, this, &MainWindow::OnApplyButtonClicked);
    connect(ui->closeButton, &QPushButton::clicked, this, &MainWindow::OnCloseButtonClicked);

    serversBoxLayout = new QVBoxLayout();
    ui->scrollAreaWidgetContents->setLayout(serversBoxLayout);
    ui->scrollAreaWidgetContents->layout()->addItem(new QSpacerItem(0, 0, QSizePolicy::Fixed, QSizePolicy::Expanding));

    CreateTrayIcon();

    ChangeSettingsState(NOT_EDITED);

    connect(&serverCore, &ServerCore::ServerStateChanged, this, &MainWindow::OnServerStateChanged);
    LoadSettings();
    OnServerStateChanged(&serverCore);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::CreateTrayIcon()
{
    startAction = new QAction("Start server", this);
    connect(startAction, &QAction::triggered, this, &MainWindow::OnStartAction);

    stopAction = new QAction("Stop server", this);
    connect(stopAction, &QAction::triggered, this, &MainWindow::OnStopAction);

    QAction *editAction = new QAction("Edit settings", this);
    connect(editAction, &QAction::triggered, this, &MainWindow::OnEditAction);

    QAction *quitAction = new QAction("Quit", this);
    connect(quitAction, &QAction::triggered, qApp, QApplication::quit);

    QMenu *trayActionsMenu = new QMenu(this);
    trayActionsMenu->addAction(startAction);
    trayActionsMenu->addAction(stopAction);
    trayActionsMenu->addSeparator();
    trayActionsMenu->addAction(editAction);
    trayActionsMenu->addSeparator();
    trayActionsMenu->addAction(quitAction);

    QIcon windowIcon(":/icon/TrayIcon.png");
    setWindowIcon(windowIcon);

    greenTrayIcon.reset(new QIcon(":/icon/TrayIcon_green.png"));
    redTrayIcon.reset(new QIcon(":/icon/TrayIcon_red.png"));

    trayIcon = new QSystemTrayIcon(this);
    trayIcon->setIcon(*redTrayIcon);
    trayIcon->setToolTip("Asset Cache Server");
    trayIcon->setContextMenu(trayActionsMenu);
    trayIcon->show();

    connect(trayIcon, &QSystemTrayIcon::activated, this, &MainWindow::OnTrayIconActivated);
}

void MainWindow::closeEvent(QCloseEvent *e)
{
    hide();
    e->ignore();

    if (settingsState != NOT_EDITED)
    {
        LoadSettings();
    }
}

void MainWindow::ChangeSettingsState(SettingsState newState)
{
    settingsState = newState;
    ui->applyButton->setEnabled(settingsState == VERIFIED);
}

void MainWindow::OnTrayIconActivated(QSystemTrayIcon::ActivationReason reason)
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

void MainWindow::OnFolderSelected()
{
    QString directory = FileDialog::getExistingDirectory(this, "Choose directory", QDir::currentPath(),
                                                         QFileDialog::ShowDirsOnly);
    ui->cacheFolderLineEdit->setText(directory);
    
    VerifyData();
}

void MainWindow::OnFolderTextChanged()
{
    ui->clearDirectoryButton->setEnabled(!ui->cacheFolderLineEdit->text().isEmpty());
    ui->cacheFolderLineEdit->setFocus();
    VerifyData();
}

void MainWindow::OnCacheSizeChanged(double)
{
    VerifyData();
}

void MainWindow::OnNumberOfFilesChanged(int)
{
    VerifyData();
}

void MainWindow::OnAutoSaveTimeoutChanged(int)
{
    VerifyData();
}

void MainWindow::OnPortChanged(int)
{
    VerifyData();
}

void MainWindow::OnAutoStartChanged(int)
{
    VerifyData();
}

void MainWindow::OnRemoteServerAdded()
{
    AddRemoteServer(ServerData(DEFAULT_REMOTE_IP, DEFAULT_REMOTE_PORT));
    VerifyData();
}

void MainWindow::OnRemoteServerRemoved()
{
    RemoteAssetCacheServer *w = qobject_cast<RemoteAssetCacheServer *>(sender());
    remoteServers.removeOne(w);

    w->deleteLater();
    VerifyData();
}

void MainWindow::OnRemoteServerEdited()
{
    VerifyData();
}

void MainWindow::OnEditAction()
{
    show();
    raise();
}

void MainWindow::OnStartAction()
{
    serverCore.Start();
}

void MainWindow::OnStopAction()
{
    serverCore.Stop();
}

void MainWindow::AddRemoteServer(const ServerData & newServer)
{
    RemoteAssetCacheServer *server = new RemoteAssetCacheServer(newServer, this);
    remoteServers << server;

    connect(server, &RemoteAssetCacheServer::RemoveLater, this, &MainWindow::OnRemoteServerRemoved);
    connect(server, &RemoteAssetCacheServer::ParametersChanged, this, &MainWindow::OnRemoteServerEdited);

    serversBoxLayout->insertWidget(serversBoxLayout->count() - 1, server);

    VerifyData();
}

void MainWindow::RemoveServers()
{
    while (!remoteServers.empty())
    {
        remoteServers.front()->deleteLater();
        remoteServers.pop_front();
    }
}

void MainWindow::VerifyData()
{
    for (auto &server : remoteServers)
    {
        if (!server->IsCorrectData())
        {
            ChangeSettingsState(NOT_VERIFIED);
            return;
        }
    }

    ChangeSettingsState(VERIFIED);
}

void MainWindow::OnApplyButtonClicked()
{
    SaveSettings();
}

void MainWindow::OnCloseButtonClicked()
{
    hide();

    if (settingsState != NOT_EDITED)
    {
        LoadSettings();
    }
}

void MainWindow::SaveSettings()
{
    serverCore.Settings().SetFolder(ui->cacheFolderLineEdit->text().toStdString());
    serverCore.Settings().SetCacheSizeGb(ui->cacheSizeSpinBox->value());
    serverCore.Settings().SetFilesCount(ui->numberOfFilesSpinBox->value());
    serverCore.Settings().SetAutoSaveTimeoutMin(ui->autoSaveTimeoutSpinBox->value());
    serverCore.Settings().SetPort(ui->portSpinBox->value());
    serverCore.Settings().SetAutoStart(ui->autoStartCheckBox->isChecked());

    serverCore.Settings().ResetServers();
    for (auto &server : remoteServers)
    {
        serverCore.Settings().AddServer(server->GetServerData());
    }

    serverCore.Settings().Save();
    ChangeSettingsState(NOT_EDITED);
}

void MainWindow::LoadSettings()
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
    for (auto& sd: servers)
    {
        AddRemoteServer(sd);
    }
    
    blockSignals(blocked);

    ChangeSettingsState(NOT_EDITED);
}

void MainWindow::OnServerStateChanged(const ServerCore* server)
{
    DVASSERT(&serverCore == server);

    switch(serverCore.GetState())
    {
    case ServerCore::State::STARTED:
    {
        trayIcon->setIcon(*greenTrayIcon);
        startAction->setDisabled(true);
        stopAction->setEnabled(true);
        break;
    }
    case ServerCore::State::STOPPED:
    {
        trayIcon->setIcon(*redTrayIcon);
        startAction->setEnabled(true);
        stopAction->setDisabled(true);
        break;
    }
    }
}
