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


#include "ServerCore.h"

#include <QTimer>

ServerCore::ServerCore()
    : serverIsRunning(false)
{
    settings = new ApplicationSettings();

    QObject::connect(settings, &ApplicationSettings::SettingsUpdated, this, &ServerCore::OnSettingsUpdated);
    
    settings->Load();
    
    server.SetDelegate(&serverLogics);
    client.SetDelegate(&serverLogics);
}

ServerCore::~ServerCore()
{
    Stop();
    
    settings->Save();
    delete settings;
}


void ServerCore::Start()
{
    server.Listen(settings->GetPort());
    serverLogics.Init(&server, &client, &dataBase);

    const auto remoteServer = settings->GetCurrentServer();
    if(!remoteServer.ip.empty())
    {
        client.Connect(remoteServer.ip, remoteServer.port);
    }
    
    serverIsRunning = true;
    
    QTimer::singleShot(UPDATE_TIMEOUT, this, &ServerCore::UpdateByTimer);
}

void ServerCore::Stop()
{
    if(serverIsRunning)
    {
        serverIsRunning = false;
        
        client.Disconnect();
        server.Disconnect();
    }
}



void ServerCore::Update()
{
    serverLogics.Update();
    
    auto netSystem = DAVA::Net::NetCore::Instance();
    if(netSystem)
    {
        netSystem->Poll();
    }
}

void ServerCore::UpdateByTimer()
{
    Update();
    
    if(serverIsRunning)
    {
        QTimer::singleShot(UPDATE_TIMEOUT, this, &ServerCore::UpdateByTimer);
    }
}


void ServerCore::OnSettingsUpdated(const ApplicationSettings *_settings)
{
    if(settings == _settings)
    {
        const auto & folder = settings->GetFolder();
        const auto sizeGb = settings->GetCacheSizeGb();
        const auto count = settings->GetFilesCount();
        const auto autoSaveTimeout = settings->GetAutoSaveTimeoutMs();
        const auto remoteServer = settings->GetCurrentServer();
        
        bool needServerRestart = false; //TODO: why we need different places for disconnect and listen???
        bool needClientRestart = true;

        if(serverIsRunning)
        {   // disconnect network if settings changed
            if (server.IsConnected() && server.GetListenPort() != settings->GetPort())
            {
                needServerRestart = true;
                server.Disconnect();
            }
            
            auto clientConnection = client.GetConnection();
            if(client.IsConnected() && clientConnection)
            {
                const auto & endpoint = clientConnection->GetEndpoint();
                if(remoteServer == endpoint)
                {
                    needClientRestart = false;
                }
                else
                {
                    client.Disconnect();
                }
            }
        }
        
        {   //updated DB settings
            if(sizeGb && !folder.IsEmpty())
            {
                dataBase.UpdateSettings(folder, sizeGb * 1024 * 1024 * 1024, count, autoSaveTimeout);
            }
            else
            {
                DAVA::Logger::Warning("[ServerCore::%s] Empty settings", __FUNCTION__);
            }
        }

        if(serverIsRunning)
        {   // restart network connections after changing of settings
            if (needServerRestart)
            {
                server.Listen(settings->GetPort());
            }
            
            if(needClientRestart && !remoteServer.ip.empty())
            {
                client.Connect(remoteServer.ip, remoteServer.port);
            }
        }
    }
    else
    {
        DAVA::Logger::Error("[ServerCore::%s] Wrong settings", __FUNCTION__);
    }
}


ApplicationSettings * ServerCore::GetSettings() const
{
    return settings;
}

