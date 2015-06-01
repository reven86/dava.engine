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



#include "AssetCache/Test/AssetCacheTest.h"
#include "AssetCache/AssetCache.h"

#include "FileSystem/DynamicMemoryFile.h"
#include "FileSystem/FileList.h"
#include "Platform/SystemTimer.h"
#include "Platform/Thread.h"


#include "Network/Base/Endpoint.h"



namespace DAVA
{
    
namespace AssetCache
{
 
class AssetClientAddRequest: public ClientDelegate
{
public:
    Client client;
    bool testFinished = false;

public:

    AssetClientAddRequest(const String & hash, const FilePath & pathname)
    {
        client.SetDelegate(this);
        
        Connect();
        
        CacheItemKey key(hash);
        CachedFiles files;
        
        files.AddFile(pathname);
        files.LoadFiles();
        files.InvalidateFileSize();
        
        auto added = client.AddToCache(key, files);
        Logger::FrameworkDebug("[AssetClientAddRequest::%s] Add request sent with result %d", __FUNCTION__, added);
        
        auto startTime = SystemTimer::Instance()->AbsoluteMS();
        while(testFinished == false)
        {
            Thread::Sleep(10);
        }
        auto deltaTime = SystemTimer::Instance()->AbsoluteMS() - startTime;
        Logger::FrameworkDebug("[AssetClientAddRequest::%s] Wait of adding time = %lld", __FUNCTION__, deltaTime);
    }
    
    ~AssetClientAddRequest()
    {
        client.Disconnect();
    }
    
    bool Connect()
    {
        auto startTime = SystemTimer::Instance()->AbsoluteMS();
        bool connected = client.Connect("127.0.0.1", AssetCache::ASSET_SERVER_PORT);
        Logger::FrameworkDebug("[AssetClientAddRequest::%s] Connect request sent with result %d", __FUNCTION__, connected);
        if(connected)
        {
            while(client.IsConnected() == false)
            {
                Thread::Sleep(10);
            }
        }

        auto deltaTime = SystemTimer::Instance()->AbsoluteMS() - startTime;
        Logger::FrameworkDebug("[AssetClientAddRequest::%s] Connection time = %lld", __FUNCTION__, deltaTime);
        return connected;
    }
    
    
    void OnAddedToCache(const CacheItemKey &key, bool added) override
    {
        Logger::FrameworkDebug("[AssetClientAddRequest::%s] files added = %d", __FUNCTION__, added);
        testFinished = true;
    }
};
    
    
   
//=============================
class AssetClientGetRequest: public ClientDelegate
{
public:
    Client client;
    bool testFinished = false;
    
    FilePath folder;
    
public:
    
    AssetClientGetRequest(const String & hash, const FilePath & _folder) : folder(_folder)
    {
        client.SetDelegate(this);
        
        Connect();
        
        CacheItemKey key(hash);
        
        auto get = client.GetFromCache(key);
        Logger::Debug("[AssetClientGetRequest::%s] Get request sent with result %d", __FUNCTION__, get);
        
        auto startTime = SystemTimer::Instance()->AbsoluteMS();
        while(testFinished == false)
        {
            Thread::Sleep(10);
        }
        auto deltaTime = SystemTimer::Instance()->AbsoluteMS() - startTime;
        Logger::Debug("[AssetClientGetRequest::%s] Wait of adding time = %lld", __FUNCTION__, deltaTime);
        
    }
    
    ~AssetClientGetRequest()
    {
        client.Disconnect();
    }
    
    bool Connect()
    {
        auto startTime = SystemTimer::Instance()->AbsoluteMS();
        bool connected = client.Connect("127.0.0.1", AssetCache::ASSET_SERVER_PORT);
        Logger::Debug("[AssetClientGetRequest::%s] Connect request sent with result %d", __FUNCTION__, connected);
        if(connected)
        {
            while(client.IsConnected() == false)
            {
                Thread::Sleep(10);
            }
        }
        
        auto deltaTime = SystemTimer::Instance()->AbsoluteMS() - startTime;
        Logger::Debug("[AssetClientGetRequest::%s] Connection time = %lld", __FUNCTION__, deltaTime);
        return connected;
    }
    
    void OnReceivedFromCache(const CacheItemKey &key, const CachedFiles &files) override
    {
        Logger::Debug("[AssetClientGetRequest::%s]", __FUNCTION__);
        testFinished = true;
        
        if(files.IsEmtpy())
        {
            Logger::Debug("[AssetClientGetRequest::%s] there are no files in cache", __FUNCTION__);
        }
        else
        {
            files.Save(folder);
        }
    }
};

    
///=================================

void AssetClientTestAdd()
{
    auto count = 3;
    
    for(decltype(count) i = 0; i < count; ++i)
    {
        AssetClientAddRequest test(String(64, '0' + i), Format("/Users/victorkleschenko/Downloads/log_copy%d.log", i));
    }
}

    
void AssetClientTestGet()
{
    auto count = 3;
    
    for(decltype(count) i = 0; i < count; ++i)
    {
        AssetClientGetRequest test0(String(64, '0' + i), Format("/Users/victorkleschenko/Downloads/Test/%d/", i));
    }
}
    
    
    


}; // end of namespace AssetCache
}; // end of namespace DAVA

