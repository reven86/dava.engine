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
    
CachedFiles::CachedFiles(const CachedFiles & right) DAVA_NOEXCEPT
	: files(right.files)
	, filesSize(right.filesSize)
	, filesAreLoaded(right.filesAreLoaded)
{
}
    
CachedFiles::CachedFiles(CachedFiles &&right) DAVA_NOEXCEPT
    : files(std::move(right.files))
    , filesSize(right.filesSize)
    , filesAreLoaded(right.filesAreLoaded)
{
    right.filesSize = 0;
    right.filesAreLoaded = false;
}
    
    
CachedFiles::~CachedFiles()
{
    if(filesAreLoaded)
    {
        UnloadFiles();
    }
    else
    {
#if defined (__DAVAENGINE_DEBUG__)
        for(auto & f : files)
        {
			DVASSERT(IsDataLoaded(f.second) == false);
        }
#endif// (__DAVAENGINE_DEBUG__)
    }
    
    files.clear();
    filesSize = 0;
    filesAreLoaded = false;
}
    
void CachedFiles::AddFile(const FilePath &path)
{
    DVASSERT(files.count(path) == 0);

	FilesData fileData = std::make_shared<Vector<uint8> >();
    if(filesAreLoaded)
    {
        fileData = LoadFile(path);
        filesSize += fileData.get()->size();
    }
    else
    {
        uint32 sz = 0;
        FileSystem::Instance()->GetFileSize(path, sz);
        if (sz > 0)
        {
            filesSize += sz;
        }
    }

    files[path] = fileData;
}

    
void CachedFiles::Serialize(KeyedArchive * archieve, bool serializeData) const
{
    DVASSERT(nullptr != archieve);

    archieve->SetUInt64("files_size", filesSize);

    auto count = files.size();
    archieve->SetUInt32("files_count", count);
    
    int32 index = 0;
    for(auto & f : files)
    {
        archieve->SetString(Format("path_%d", index), f.first.GetStringValue());

		if (IsDataLoaded(f.second) && serializeData)
        {
            auto & fileData = f.second;
            archieve->SetByteArray(Format("file_%d", index), fileData.get()->data(), fileData.get()->size());
        }
        
        ++index;
    }
}
    

void CachedFiles::Deserialize(KeyedArchive * archieve)
{
    DVASSERT(nullptr != archieve);
    DVASSERT(files.empty());
    DVASSERT(filesAreLoaded == false);
    
    filesSize = archieve->GetUInt64("files_size");
    
    auto count = archieve->GetUInt32("files_count");
    for(uint32 i = 0; i < count; ++i)
    {
        FilePath path = archieve->GetString(Format("path_%d", i));
		FilesData fileData = std::make_shared<Vector<uint8> >();

        auto key = Format("file_%d", i);
        auto size = archieve->GetByteArraySize(key);
        if(size)
        {
            filesAreLoaded = true;

			fileData.get()->resize(size);
			Memcpy(fileData.get()->data(), archieve->GetByteArray(key), size);
        }
        
        files[path] = fileData;
    }
}


bool CachedFiles::operator == (const CachedFiles &right) const
{
    return (files == right.files) && (filesAreLoaded == right.filesAreLoaded) && (filesSize == right.filesSize);
}

CachedFiles & CachedFiles::operator=(const CachedFiles &right)
{
    if (this != &right)
    {
        if (filesAreLoaded)
            UnloadFiles();

        filesAreLoaded = right.filesAreLoaded;
        filesSize = right.filesSize;

        files = right.files;
    }

    return (*this);
}

CachedFiles & CachedFiles::operator=(CachedFiles &&right)
{
    if (this != &right)
    {
        files = std::move(right.files);

        filesAreLoaded = right.filesAreLoaded;
        filesSize = right.filesSize;
        
        right.filesSize = 0;
        right.filesAreLoaded = false;
    }

    return (*this);
}

    
uint64 CachedFiles::GetFilesSize() const
{
    DVASSERT((files.empty() && filesSize == 0) || (!files.empty() && filesSize > 0));
    return filesSize;
}

void CachedFiles::LoadFiles()
{
    DVASSERT(filesAreLoaded == false);
    
    filesAreLoaded = true;
    
    for(auto & f : files)
    {
        DVASSERT(IsDataLoaded(f.second) == false);
        f.second = LoadFile(f.first);
    }
}

void CachedFiles::UnloadFiles()
{
    DVASSERT(filesAreLoaded == true);

    filesAreLoaded = false;
    for(auto & f : files)
    {
		DVASSERT(IsDataLoaded(f.second) == true);
		f.second.reset();
    }
}
    
void CachedFiles::Save(const FilePath & folder) const
{
    DVASSERT(folder.IsDirectoryPathname());
    
    FileSystem::Instance()->CreateDirectory(folder, true);
    
    for(auto & f : files)
    {
        if(IsDataLoaded(f.second) == false)
        {
            Logger::Warning("[CachedFiles::%s] File(%s) not loaded", __FUNCTION__, f.first.GetStringValue().c_str());
            continue;
        }
        
        auto savedPath = folder + f.first.GetFilename();
        
        ScopedPtr<File> file(File::Create(savedPath, File::CREATE | File::WRITE));
        if(file)
        {
            auto written = file->Write(f.second.get()->data(), f.second.get()->size());
			DVVERIFY(written == f.second.get()->size());
        }
        else
        {
            Logger::Error("[CachedFiles::%s] Cannot create file %s", __FUNCTION__, savedPath.GetStringValue().c_str());
        }
    }
}
    
CachedFiles::FilesData CachedFiles::LoadFile(const FilePath & pathname)
{
	FilesData fileData = std::make_shared<Vector<uint8>>();

	ScopedPtr<File> file(File::Create(pathname, File::OPEN | File::READ));
    if(file)
    {
        auto dataSize = file->GetSize();
		fileData.get()->resize(dataSize);
        
		auto read = file->Read(fileData.get()->data(), dataSize);
        DVVERIFY(read == dataSize);
    }
    else
    {
        Logger::Error("[CachedFiles::%s] Cannot read file %s", __FUNCTION__, pathname.GetStringValue().c_str());
    }
    
    return fileData;
}

CachedFiles CachedFiles::Copy(const FilePath & folder) const
{
    DVASSERT(folder.IsDirectoryPathname());
    
    CachedFiles copyFiles;
    
    copyFiles.filesAreLoaded = filesAreLoaded;
    copyFiles.filesSize = filesSize;

    for(auto & f: files)
    {
        auto path = folder + f.first.GetFilename();
        copyFiles.files[path] = f.second;
    }
    
    return copyFiles;
}

} // end of namespace AssetCache
} // end of namespace DAVA

