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


#ifndef __ASSETCACHESERVER_WINDOW_H__
#define __ASSETCACHESERVER_WINDOW_H__

#include <memory>
#include <QMainWindow>
#include <QSystemTrayIcon>

#include "ServerCore.h"

class QMenu;
class QVBoxLayout;

class RemoteServerWidget;
class ApplicationSettings;
struct ServerData;

namespace Ui
{
    class AssetCacheServerWidget;
}


class AssetCacheServerWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit AssetCacheServerWindow(ServerCore& core, QWidget *parent = nullptr);
    ~AssetCacheServerWindow() override;

protected:
    void closeEvent(QCloseEvent *e) override;

    void LoadSettings();
    void SaveSettings();

private slots:
    void OnTrayIconActivated(QSystemTrayIcon::ActivationReason reason);

    void OnEditAction();
    void OnStartAction();
    void OnStopAction();
    
    void OnFolderSelection();
    void OnFolderTextChanged();
    void OnCacheSizeChanged(double);
    void OnNumberOfFilesChanged(int);
    void OnAutoSaveTimeoutChanged(int);
    void OnPortChanged(int);
    void OnAutoStartChanged(int);

    void OnRemoteServerAdded();
    void OnRemoteServerRemoved();
    void OnRemoteServerEdited();
    void OnRemoteServerChecked(bool);

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
        NOT_EDITED, EDITED
    };

    void ChangeSettingsState(SettingsState newState);

private:
    Ui::AssetCacheServerWidget *ui = nullptr;
    QAction *startAction = nullptr;
    QAction *stopAction = nullptr;
    QSystemTrayIcon* trayIcon = nullptr;
    std::unique_ptr<QIcon> greenGreenTrayIcon;
    std::unique_ptr<QIcon> greenGrayTrayIcon;
    std::unique_ptr<QIcon> greenRedTrayIcon;
    std::unique_ptr<QIcon> redGrayTrayIcon;

    QVBoxLayout *serversBoxLayout = nullptr;
    List<RemoteServerWidget*> remoteServers;

    ServerCore &serverCore;

    SettingsState settingsState = NOT_EDITED;
};

#endif // __ASSETCACHESERVER_WINDOW_H__
