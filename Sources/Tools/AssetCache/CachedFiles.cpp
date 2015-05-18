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

    if(filesAreLoaded)
    {
        files[path] = File::Create(path, File::OPEN | File::READ);
    }
    else
    {
        files[path] = nullptr;
    }
}

    
void CachedFiles::Serialize(KeyedArchive * archieve) const
{
    DVASSERT(nullptr != archieve);
    
    auto count = files.size();
    archieve->SetUInt32("files_count", count);
    
    int32 index = 0;
    Vector<uint8> fileData;
    for(auto & f : files)
    {
        archieve->SetString(Format("path_%d", index++), f.first.GetStringValue());

        if(f.second)
        {
            auto file = f.second;
            
            auto fSize = file->GetSize();
            if(fSize > fileData.size())
            {
                fileData.resize(fSize);
            }
            
            auto read = file->Read(fileData.data(), fSize);
            DVVERIFY(read == fSize);
            
            archieve->SetByteArray(Format("file_%d", index++), fileData.data(), read);
        }
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
        File *file = nullptr;
        
        auto key = Format("file_%d", i);
        auto size = archieve->GetByteArraySize(key);
        if(size)
        {
            filesAreLoaded = true;
            
            auto data = archieve->GetByteArray(key);
            file = DynamicMemoryFile::Create(data, size, File::OPEN | File::WRITE);
        }
        
        files[path] = file;
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
        f.second = File::Create(f.first, File::OPEN | File::READ);
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
    
const Map<FilePath, File *> & CachedFiles::GetFiles() const
{
    return files;
}

    
}; // end of namespace AssetCache
}; // end of namespace DAVA

