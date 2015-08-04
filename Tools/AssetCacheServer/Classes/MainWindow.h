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


#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <memory>
#include <QMainWindow>
#include <QSystemTrayIcon>

#include "ServerCore.h"

class QMenu;
class QVBoxLayout;

class RemoteAssetCacheServer;
class ApplicationSettings;
struct ServerData;

namespace Ui
{
    class MainWindow;
}


class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(ServerCore& core, QWidget *parent = nullptr);
    ~MainWindow() override;

protected:
    void closeEvent(QCloseEvent *e) override;

    void LoadSettings();
    void SaveSettings();

private slots:
    void OnTrayIconActivated(QSystemTrayIcon::ActivationReason reason);

    void OnEditAction();
    void OnStartAction();
    void OnStopAction();
    
    void OnFolderSelected();
    void OnFolderTextChanged();
    void OnCacheSizeChanged(double);
    void OnNumberOfFilesChanged(int);
    void OnAutoSaveTimeoutChanged(int);
    void OnPortChanged(int);
    void OnAutoStartChanged(int);

    void OnRemoteServerAdded();
    void OnRemoteServerRemoved();
    void OnRemoteServerEdited();

    void OnApplyButtonClicked();
    void OnCloseButtonClicked();

    void OnServerStateChanged(const ServerCore*);

private:
    void CreateTrayIcon();

    void AddRemoteServer(const ServerData & newServer);
    void RemoveServers();

    void VerifyData();

    enum SettingsState
    {
        NOT_EDITED, NOT_VERIFIED, VERIFIED
    };

    void ChangeSettingsState(SettingsState newState);

private:
    Ui::MainWindow *ui;
    QAction *startAction;
    QAction *stopAction;
    QSystemTrayIcon* trayIcon;
    std::unique_ptr<QIcon> greenTrayIcon;
    std::unique_ptr<QIcon> redTrayIcon;

    QVBoxLayout *serversBoxLayout;
    QList<RemoteAssetCacheServer *> remoteServers;

    ServerCore& serverCore;

    SettingsState settingsState = NOT_EDITED;
};

#endif // MAINWINDOW_H
