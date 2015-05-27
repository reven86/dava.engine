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
{
    settings = new ApplicationSettings();

    QObject::connect(settings, &ApplicationSettings::SettingsUpdated, this, &ServerCore::OnSettingsUpdated);
    
    settings->Load();
    
    server.SetDelegate(&logics);
}

ServerCore::~ServerCore()
{
    settings->Save();
    delete settings;
    
    server.Disconnect();
}


void ServerCore::Start()
{
    server.Listen(DAVA::AssetCache::ASSET_SERVER_PORT);
    logics.Init(&server, &dataBase);
    
    QTimer::singleShot(UPDATE_TIMEOUT, this, &ServerCore::UpdateByTimer);
}

void ServerCore::Update()
{
    auto netSystem = DAVA::Net::NetCore::Instance();
    if(netSystem)
    {
        netSystem->Poll();
    }
}

void ServerCore::UpdateByTimer()
{
    Update();
    QTimer::singleShot(UPDATE_TIMEOUT, this, &ServerCore::UpdateByTimer);
}


void ServerCore::OnSettingsUpdated(const ApplicationSettings *_settings)
{
    if(settings == _settings)
    {
        auto & folder = settings->GetFolder();
        auto size = settings->GetCacheSize();
        auto count = settings->GetFilesCount();
        
        if(size && !folder.IsEmpty())
        {
            dataBase.UpdateSettings(folder, size, count);
        }
        else
        {
            DAVA::Logger::Warning("[ServerCore::%s] Empty settings", __FUNCTION__);
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

