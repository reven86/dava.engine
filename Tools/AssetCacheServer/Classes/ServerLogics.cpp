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


#include "ServerLogics.h"

#include "Concurrency/LockGuard.h"

void ServerLogics::Init(DAVA::AssetCache::Server *_server, DAVA::AssetCache::Client *_client, DAVA::AssetCache::CacheDB *_dataBase)
{
    server = _server;
    client = _client;
    
    dataBase = _dataBase;
}

void ServerLogics::OnAddToCache(DAVA::TCPChannel *tcpChannel, const DAVA::AssetCache::CacheItemKey &key, const DAVA::AssetCache::CachedFiles &files)
{
    if((nullptr != server) && (nullptr != tcpChannel))
    {
        dataBase->Insert(key, files);
        server->FilesAddedToCache(tcpChannel, key, true);

        ServerTask task;
        task.key = key;
        task.files = files;
        task.request = DAVA::AssetCache::PACKET_ADD_FILES_REQUEST;
        
        {   //add task for lazy sending of files;
            DAVA::LockGuard<DAVA::Mutex> lock(taskMutex);
            serverTasks.push_back(task);
        }
    }
}

void ServerLogics::OnRequestedFromCache(DAVA::TCPChannel *tcpChannel, const DAVA::AssetCache::CacheItemKey &key)
{
    if((nullptr != server) && (nullptr != dataBase) && (nullptr != tcpChannel))
    {
        auto entry = dataBase->Get(key);
        if(nullptr != entry)
        {   // Found in db.
            server->SendFiles(tcpChannel, key, entry->GetFiles());
        }
        else if(client->IsConnected())
        {   // Not found in db. Ask from remote cache.
            client->GetFromCache(key);
            
            RequestDescription description;
            description.request = DAVA::AssetCache::PACKET_GET_FILES_REQUEST;
            description.key = key;
            description.clientChannel = tcpChannel;
            
            DAVA::LockGuard<DAVA::Mutex> lock(requestMutex);
            waitedRequests.push_back(description);
        }
        else
        {   // Not found in db. Remote server isn't connected.
            server->SendFiles(tcpChannel, key, DAVA::AssetCache::CachedFiles());
        }
    }
}

void ServerLogics::OnWarmingUp(DAVA::TCPChannel *tcpChannel, const DAVA::AssetCache::CacheItemKey &key)
{
    if(nullptr != dataBase)
    {
        dataBase->InvalidateAccessToken(key);
    }
}

void ServerLogics::OnChannelClosed(DAVA::TCPChannel *tcpChannel, const DAVA::char8* message)
{
    if(waitedRequests.size())
    {
        DAVA::LockGuard<DAVA::Mutex> lock(requestMutex);
        auto iter = std::find_if(waitedRequests.begin(), waitedRequests.end(), [&tcpChannel](const RequestDescription& description) -> bool
                                 {
                                     return (description.clientChannel == tcpChannel);
                                 });
        
        if(iter != waitedRequests.end())
        {
            waitedRequests.erase(iter);
        }
        else
        {
            DVASSERT(false && "cannot found description for closed channel")
        }
    }
}


void ServerLogics::OnReceivedFromCache(const DAVA::AssetCache::CacheItemKey &key, const DAVA::AssetCache::CachedFiles &files)
{
    if(nullptr != dataBase)
    {
        dataBase->Insert(key, files);
    }

    if((nullptr != server) && waitedRequests.size())
    {
        RequestDescription description;
        
        {   //find description for key
            DAVA::LockGuard<DAVA::Mutex> lock(requestMutex);
            auto iter = std::find_if(waitedRequests.begin(), waitedRequests.end(), [&key](const RequestDescription& description) -> bool
                                     {
                                         return (description.key == key) && (description.request == DAVA::AssetCache::PACKET_GET_FILES_REQUEST);
                                     });
            
            if(iter != waitedRequests.end())
            {
                description = *iter;
            }
            else
            {
                DVASSERT(false && "cannot found description for key")
            }
        }
        
        if(nullptr != description.clientChannel)
        {
            server->SendFiles(description.clientChannel, key, files);
        }
    }
}

void ServerLogics::ProcessServerTasks()
{
    if(!serverTasks.empty() && client && client->IsConnected())
    {
        const ServerTask & task = serverTasks.front();
        client->AddToCache(task.key, task.files);
        
        DAVA::LockGuard<DAVA::Mutex> lock(taskMutex);
        serverTasks.pop_front();
    }
}

void ServerLogics::Update()
{
    ProcessServerTasks();
    
    if(dataBase)
    {
        dataBase->Update();
    }
}

