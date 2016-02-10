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

#include "AssetCache/AssetCacheConstants.h"
#include "Base/GlobalEnum.h"

namespace DAVA
{
namespace AssetCache
{
} // end of namespace AssetCache
} // end of namespace DAVA

ENUM_DECLARE(DAVA::AssetCache::ErrorCodes)
{
    ENUM_ADD_DESCR(DAVA::AssetCache::ERROR_OK, "OK");
    ENUM_ADD_DESCR(DAVA::AssetCache::ERROR_CODE_NOT_INITIALIZED, "ERROR_CODE_NOT_INITIALIZED");
    ENUM_ADD_DESCR(DAVA::AssetCache::ERROR_WRONG_COMMAND_LINE, "WRONG_COMMAND_LINE");
    ENUM_ADD_DESCR(DAVA::AssetCache::ERROR_WRONG_IP, "WRONG_IP");
    ENUM_ADD_DESCR(DAVA::AssetCache::ERROR_OPERATION_TIMEOUT, "ERROR_OPERATION_TIMEOUT");
    ENUM_ADD_DESCR(DAVA::AssetCache::ERROR_CANNOT_CONNECT, "CANNOT_CONNECT");
    ENUM_ADD_DESCR(DAVA::AssetCache::ERROR_SERVER_ERROR, "SERVER_ERROR");
    ENUM_ADD_DESCR(DAVA::AssetCache::ERROR_NOT_FOUND_ON_SERVER, "NOT_FOUND_ON_SERVER");
    ENUM_ADD_DESCR(DAVA::AssetCache::ERROR_READ_FILES, "READ_FILES");
    ENUM_ADD_DESCR(DAVA::AssetCache::ERROR_ADDRESS_RESOLVER_FAILED, "ADDRESS_RESOLVER_FAILED");
    ENUM_ADD_DESCR(DAVA::AssetCache::ERROR_CANNOT_SEND_REQUEST_ADD, "CANNOT_SEND_REQUEST_ADD");
    ENUM_ADD_DESCR(DAVA::AssetCache::ERROR_CANNOT_SEND_REQUEST_GET, "CANNOT_SEND_REQUEST_GET");
    ENUM_ADD_DESCR(DAVA::AssetCache::ERROR_CORRUPTED_DATA, "ERROR_CORRUPTED_DATA");
}
