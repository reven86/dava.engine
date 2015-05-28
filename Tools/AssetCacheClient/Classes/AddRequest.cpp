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

#include "AddRequest.h"
#include "Constants.h"

#include "Platform/SystemTimer.h"


using namespace DAVA;


AddRequest::AddRequest()
    :   CacheRequest("add")
{
    options.AddOption("-f", VariantType(String("")), "Files list to send files to server");
}

int AddRequest::SendRequest()
{
    const String hash = options.GetOption("-h").AsString();
    const String filepath = options.GetOption("-f").AsString();
    
    AssetCache::CacheItemKey key(hash);
    AssetCache::CachedFiles files;
    files.AddFile(filepath);
    files.LoadFiles();
    
    auto requestSent = client.AddToCache(key, files);
    if (!requestSent)
    {
        Logger::Error("[AddRequest::%s] Cannot send files to server", __FUNCTION__);
        return AssetCacheClientConstants::EXIT_CANNOT_CONNECT;
    }
    
    return AssetCacheClientConstants::EXIT_OK;
}


int AddRequest::CheckOptionsInternal() const
{
    const String filepath = options.GetOption("-f").AsString();
    if(filepath.empty())
    {
        Logger::Error("[AddRequest::%s] Empty file list", __FUNCTION__);
        return AssetCacheClientConstants::EXIT_WRONG_COMMAND_LINE;
    }

    return AssetCacheClientConstants::EXIT_OK;
}


void AddRequest::OnAddedToCache(const AssetCache::CacheItemKey &key, bool added)
{
    requestResult.recieved = true;
    requestResult.succeed = added;
}

