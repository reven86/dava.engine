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


//#include "DataStorage/DataStorage.h"
#include "DataStorageSteam.h"

#include "FileSystem/KeyedArchive.h"
#include "FileSystem/DynamicMemoryFile.h"


namespace DAVA
{

#if defined(USE_STEAM)

IDataStorage* DataStorage::Create()
{
    return new DataStorageSteam();
}

DataStorageSteam::DataStorageSteam()
    : values(new KeyedArchive)
{


   
    remoteStorage = SteamRemoteStorage();
}

ScopedPtr<KeyedArchive> DataStorageSteam::ReadArchFromStorage() const
{
    ScopedPtr<KeyedArchive> dataArchive(nullptr);

    ScopedPtr<DynamicMemoryFile> dataFile(nullptr);

    if (!remoteStorage->FileExists(storageFileName.c_str()))
    {
        return dataArchive;
    }

    int32 cubFileSize = remoteStorage->GetFileSize(storageFileName.c_str());
    if (0 >= cubFileSize)
    {
        return dataArchive;
    }

    uint8 * buffer = new uint8[cubFileSize];

    int32 cubRead = remoteStorage->FileRead(storageFileName.c_str(), buffer, cubFileSize);

    dataFile = DynamicMemoryFile::Create(buffer, cubFileSize, File::CREATE | File::WRITE | File::READ);

    dataFile->Seek(0, File::SEEK_FROM_START);

    dataArchive = new KeyedArchive();
    bool isLoaded = dataArchive->Load(dataFile);
    DVASSERT_MSG(isLoaded, "Wrong SteamArchive Format.");

    return dataArchive;
}

void DataStorageSteam::WriteArchiveToStorage(const ScopedPtr<KeyedArchive> arch) const
{
    ScopedPtr<DynamicMemoryFile> dataFile(DynamicMemoryFile::Create(File::CREATE | File::WRITE));
    arch->Save(dataFile);

    bool isWritten = remoteStorage->FileWrite(storageFileName.c_str(), dataFile->GetData(), dataFile->GetSize());

    if (!isWritten)
    {
        Logger::Error("Can't write to Cloud Steam Storage.");
    }
}

String DataStorageSteam::GetStringValue(const String& key)
{
    const String value = values->GetString(key);
    return value;
}

int64 DataStorageSteam::GetLongValue(const String& key)
{
    int64 value = values->GetInt64(key);
    return value;
}

void DataStorageSteam::SetStringValue(const String& key, const String& value)
{
    values->SetString(key, value);
    isValuesChanged = true;
};

void DataStorageSteam::SetLongValue(const String& key, int64 value)
{
    values->SetInt64(key, value);
    isValuesChanged = true;
}

void DataStorageSteam::RemoveEntry(const String& key)
{
    values->DeleteKey(key);
    isValuesChanged = true;
}

void DataStorageSteam::Clear()
{
    values->DeleteAllKeys();
    WriteArchiveToStorage(values);
    isValuesChanged = false;
}

void DataStorageSteam::Push()
{
    auto remoteArch = ReadArchFromStorage();
    auto remoteMap = remoteArch->GetArchieveData();

    // iterate over remote keys and merge new keys into local archive
    // use local values for same keys
    for (auto pair : remoteMap)
    {
        const String remoteKey = pair.first;
        VariantType * remoteValue = remoteArch->GetVariant(remoteKey);
        VariantType::eVariantType remoteValueType = VariantType::TYPE_NONE;
        if (nullptr != remoteValue)
        {
            remoteValueType = remoteValue->GetType();
        }

        VariantType * localValue = values->GetVariant(remoteKey);
        VariantType::eVariantType localValueType = VariantType::TYPE_NONE;
        if (nullptr != localValue)
        {
            localValueType = localValue->GetType();
        }

        // has no value
        if (nullptr != remoteValue && !isValuesChanged)
        {
            values->SetVariant(remoteKey, *remoteValue);
        }     
    }

    WriteArchiveToStorage(values); 
    isValuesChanged = false;
}

#endif

}