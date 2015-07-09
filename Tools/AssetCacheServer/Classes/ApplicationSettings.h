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


#ifndef __APPLICATION_SETTINGS_H__
#define __APPLICATION_SETTINGS_H__

#include "AssetCache/AssetCacheConstants.h"
#include "FileSystem/FilePath.h"
#include <QObject>

namespace DAVA 
{
    class KeyedArchive;
};

using namespace DAVA;

struct ServerData
{
    ServerData() = default;
    ServerData(String _ip, uint16 _port) : ip(_ip), port(_port) {};
    
    bool operator == (const ServerData & right) const
    {
        return (ip == right.ip) && (port == right.port);
    }

    bool operator < (const ServerData & right) const
    {
        if(ip == right.ip)
        {
            return port < right.port;
        }
        return ip < right.ip;
    }
    
    String ip = "127.0.0.1";
    uint16 port = AssetCache::ASSET_SERVER_PORT;
};


//TODO: we need one code for settings in different projects
//need to use introspection instead of hardcoding of values

class ApplicationSettings: public QObject
{
    Q_OBJECT
    
public:
    
    void Save() const;
    void Load();

    void SetDefaultSettings();

    bool IsFirstLaunch() const;

    const FilePath & GetFolder() const;
    void SetFolder(const FilePath & folder);

    const float64 GetCacheSizeGb() const;
    void SetCacheSizeGb(const float64 size);

    const uint32 GetFilesCount() const;
    void SetFilesCount(const uint32 count);

    const uint64 GetAutoSaveTimeoutMs() const;
    void SetAutoSaveTimeoutMs(const uint64 timeout);

    const uint16 GetPort() const;
    void SetPort(const uint16 port);

    const List<ServerData> & GetServers() const;
    void ResetServers();
    void AddServer(const ServerData & server);
    void RemoveServer(const ServerData & server);
    
signals:
    void SettingsUpdated(const ApplicationSettings * settings) const;
    
private:

    void Serialize(DAVA::KeyedArchive * archieve) const;
    void Deserialize(DAVA::KeyedArchive * archieve);
    
    
public:

    FilePath folder;
    float64 cacheSizeGb;
    uint32 filesCount;
    uint64 autoSaveTimeoutMs = 1 * 60 * 1000;
    uint16 listenPort;
    List<ServerData> remoteServers;

    bool isFirstLaunch = true;
};

inline bool ApplicationSettings::IsFirstLaunch() const
{
    return isFirstLaunch;
}

#endif // __APPLICATION_SETTINGS_H__
