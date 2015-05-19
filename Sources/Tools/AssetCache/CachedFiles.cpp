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



#include "AssetCache/CachedFiles.h"
#include "Base/Data.h"
#include "FileSystem/KeyedArchive.h"
#include "FileSystem/DynamicMemoryFile.h"
#include "FileSystem/File.h"
#include "FileSystem/FileList.h"
#include "Debug/DVAssert.h"

namespace DAVA
{
    
namespace AssetCache
{
    
CachedFiles::CachedFiles()
    : filesAreLoaded(false)
{
}

CachedFiles::CachedFiles(const CachedFiles & right)
{
    filesAreLoaded = right.filesAreLoaded;
    
    files = right.files;
    for(auto & f: files)
    {
        SafeRetain(f.second);
    }
}
    
CachedFiles::~CachedFiles()
{
    UnloadFiles();
    files.clear();
}
    
void CachedFiles::AddFile(const FilePath &path)
{
    DVASSERT(files.count(path) == 0);

    Data *fileData = nullptr;
    if(filesAreLoaded)
    {
        auto file = File::Create(path, File::OPEN | File::READ);
        if(file)
        {
            auto dataSize = file->GetSize();
            fileData = new Data(dataSize);
            
            auto read = file->Read(fileData->GetPtr(), dataSize);
            DVVERIFY(read == dataSize);
            
            file->Release();
        }
    }

    files[path] = fileData;
}

    
void CachedFiles::Serialize(KeyedArchive * archieve) const
{
    DVASSERT(nullptr != archieve);
    
    auto count = files.size();
    archieve->SetUInt32("files_count", count);
    
    int32 index = 0;
    for(auto & f : files)
    {
        archieve->SetString(Format("path_%d", index), f.first.GetStringValue());

        if(f.second)
        {
            auto fileData = f.second;
            archieve->SetByteArray(Format("file_%d", index), fileData->GetPtr(), fileData->GetSize());
        }
        
        ++index;
    }
}
    

void CachedFiles::Deserialize(KeyedArchive * archieve)
{
    DVASSERT(nullptr != archieve);
    
    UnloadFiles();
    files.clear();
    
    auto count = archieve->GetUInt32("files_count");
    for(uint32 i = 0; i < count; ++i)
    {
        FilePath path = archieve->GetString(Format("path_%d", i));
        Data *fileData = nullptr;
        
        auto key = Format("file_%d", i);
        auto size = archieve->GetByteArraySize(key);
        if(size)
        {
            filesAreLoaded = true;

            fileData = new Data(size);
            Memcpy(fileData->GetPtr(), archieve->GetByteArray(key), size);
        }
        
        files[path] = fileData;
    }
}


bool CachedFiles::operator == (const CachedFiles &right) const
{
    return (files == right.files) && (filesAreLoaded == right.filesAreLoaded);
}

CachedFiles & CachedFiles::operator=(const CachedFiles &right)
{
    UnloadFiles();
    
    filesAreLoaded = right.filesAreLoaded;
    files = right.files;
    for(auto & f: files)
    {
        SafeRetain(f.second);
    }

    return (*this);
}

    
uint64 CachedFiles::GetFilesSize() const
{
    uint64 fileSize = 0;
    for(auto & f: files)
    {
        if(f.second)
        {
            fileSize += f.second->GetSize();
        }
        else
        {
            Logger::Warning("[CachedFiles::%s] File(%s) not loaded ", __FUNCTION__, f.first.GetStringValue().c_str());
        }
    }
    
    return fileSize;
}

void CachedFiles::LoadFiles()
{
    filesAreLoaded = true;
    
    for(auto & f : files)
    {
        DVASSERT(f.second == nullptr);
        
        auto file = File::Create(f.first, File::OPEN | File::READ);
        if(file)
        {
            auto dataSize = file->GetSize();
            f.second = new Data(dataSize);
            
            auto read = file->Read(f.second->GetPtr(), dataSize);
            DVVERIFY(read == dataSize);
            
            file->Release();
        }
    }
}

void CachedFiles::UnloadFiles()
{
    filesAreLoaded = false;
    for(auto & f : files)
    {
        SafeRelease(f.second);
    }
}
    

    
}; // end of namespace AssetCache
}; // end of namespace DAVA

