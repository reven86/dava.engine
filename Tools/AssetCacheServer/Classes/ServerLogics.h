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


#ifndef __SERVER_LOGICS_H__
#define __SERVER_LOGICS_H__

#include "AssetCache/AssetCache.h"

class ServerLogics: public DAVA::AssetCache::ServerDelegate, public DAVA::AssetCache::ClientListener
{
public:
    void Init(DAVA::AssetCache::Server *server, DAVA::AssetCache::Client *client, DAVA::AssetCache::CacheDB *dataBase);
    
    //ServerDelegate
    void OnAddToCache(DAVA::TCPChannel *tcpChannel, const DAVA::AssetCache::CacheItemKey &key, const DAVA::AssetCache::CachedFiles &files) override;
    void OnRequestedFromCache(DAVA::TCPChannel *tcpChannel, const DAVA::AssetCache::CacheItemKey &key) override;
    void OnWarmingUp(DAVA::TCPChannel *tcpChannel, const DAVA::AssetCache::CacheItemKey &key) override;
    void OnChannelClosed(DAVA::TCPChannel *tcpChannel, const DAVA::char8* message) override;
    
    //ClientListener
    void OnReceivedFromCache(const DAVA::AssetCache::CacheItemKey &key, const DAVA::AssetCache::CachedFiles &files) override;
    
    void Update();
    
private:
    
    void ProcessServerTasks();
    
private:
    
    DAVA::AssetCache::Server *server = nullptr;
    DAVA::AssetCache::Client *client = nullptr;
    DAVA::AssetCache::CacheDB *dataBase = nullptr;
    
    struct RequestDescription
    {
        RequestDescription(DAVA::TCPChannel *channel, const DAVA::AssetCache::CacheItemKey &_key, DAVA::AssetCache::ePacketID _request);
        
        DAVA::TCPChannel *clientChannel = nullptr;
        DAVA::AssetCache::CacheItemKey key;

        DAVA::AssetCache::ePacketID request = DAVA::AssetCache::PACKET_UNKNOWN;
    };
    
    DAVA::List<RequestDescription> waitedRequests;
    DAVA::Mutex requestMutex;
    
    struct ServerTask
    {
        ServerTask(const DAVA::AssetCache::CacheItemKey &_key, DAVA::AssetCache::ePacketID _request);
        ServerTask(const DAVA::AssetCache::CacheItemKey &_key, const DAVA::AssetCache::CachedFiles & _files, DAVA::AssetCache::ePacketID _request);
        
        DAVA::AssetCache::CacheItemKey key;
        DAVA::AssetCache::CachedFiles files;
        
        DAVA::AssetCache::ePacketID request = DAVA::AssetCache::PACKET_UNKNOWN;
    };

    DAVA::List<ServerTask> serverTasks;
    DAVA::Mutex taskMutex;
};




#endif // __SERVER_LOGICS_H__
