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

#ifndef DATA_STORAGE_STEAM_H
#define DATA_STORAGE_STEAM_H

#if defined(__DAVAENGINE_STEAM__)

#include "DataStorage/DataStorage.h"

#include "Utils/Utils.h"

#include "steam/steam_api.h"

namespace DAVA
{

#if defined(__DAVAENGINE_STEAM__)

class DynamicMemoryFile;
class DataStorageSteam : public IDataStorage
{
private:
    const String storageFileName = "CloudArchive";
public:
    DataStorageSteam();
    String GetStringValue(const String& key) override;
    int64 GetLongValue(const String& key) override;
    void SetStringValue(const String& key, const String& value) override;
    void SetLongValue(const String& key, int64 value) override;
    void RemoveEntry(const String& key) override;
    void Clear() override;
    void Push() override;

private:
    ScopedPtr<KeyedArchive> ReadArchFromStorage() const;
    void WriteArchiveToStorage(const ScopedPtr<KeyedArchive> arch) const;

    ISteamRemoteStorage * remoteStorage = nullptr;
    ScopedPtr<KeyedArchive> values;
    bool isValuesChanged = false;
};

#endif //__DAVAENGINE_STEAM__
}

#endif

#endif