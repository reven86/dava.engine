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



#include "AssetCache/CachedItemValue.h"
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
    
CachedItemValue::CachedItemValue(const CachedItemValue & right) DAVA_NOEXCEPT
	: dataContainer(right.dataContainer)
	, size(right.size)
	, isFetched(right.isFetched)
{
}
    
CachedItemValue::CachedItemValue(CachedItemValue &&right) DAVA_NOEXCEPT
	: dataContainer(std::move(right.dataContainer))
    , size(right.size)
	, isFetched(right.isFetched)
{
    right.size = 0;
    right.isFetched = false;
}
    
    
CachedItemValue::~CachedItemValue()
{
    if(isFetched)
    {
        Free();
    }
    else
    {
#if defined (__DAVAENGINE_DEBUG__)
		for (auto & data : dataContainer)
        {
			DVASSERT(IsDataLoaded(data.second) == false);
        }
#endif// (__DAVAENGINE_DEBUG__)
    }
    
    dataContainer.clear();
    size = 0;
    isFetched = false;
}
    
void CachedItemValue::AddFile(const FilePath &path)
{
    DVASSERT(dataContainer.count(path.GetFilename()) == 0);

	ValueData data = std::make_shared<Vector<uint8> >();
    if(isFetched)
    {
        data = LoadFile(path);
        size += data.get()->size();
    }
    else
    {
        uint32 sz = 0;
        FileSystem::Instance()->GetFileSize(path, sz);
        if (sz > 0)
        {
            size += sz;
        }
    }

	dataContainer[path.GetFilename()] = data;
}

    
void CachedItemValue::Serialize(KeyedArchive * archieve, bool serializeData) const
{
    DVASSERT(nullptr != archieve);

    archieve->SetUInt64("size", size);

    auto count = dataContainer.size();
    archieve->SetUInt32("data_count", count);
    
    int32 index = 0;
    for(auto & dc : dataContainer)
    {
        archieve->SetString(Format("name_%d", index), dc.first);

		if (IsDataLoaded(dc.second) && serializeData)
        {
            auto & data = dc.second;
            archieve->SetByteArray(Format("data_%d", index), data.get()->data(), data.get()->size());
        }
        
        ++index;
    }
}
    

void CachedItemValue::Deserialize(KeyedArchive * archieve)
{
    DVASSERT(nullptr != archieve);
	DVASSERT(dataContainer.empty());
    DVASSERT(isFetched == false);
    
    size = archieve->GetUInt64("size");
    
    auto count = archieve->GetUInt32("data_count");
    for(uint32 i = 0; i < count; ++i)
    {
        String name = archieve->GetString(Format("name_%d", i));
		ValueData data = std::make_shared<Vector<uint8> >();

        auto key = Format("data_%d", i);
        auto size = archieve->GetByteArraySize(key);
        if(size)
        {
            isFetched = true;

			data.get()->resize(size);
			Memcpy(data.get()->data(), archieve->GetByteArray(key), size);
        }
        
		dataContainer[name] = data;
    }
}


bool CachedItemValue::operator == (const CachedItemValue &right) const
{
	return (dataContainer == right.dataContainer) && (isFetched == right.isFetched) && (size == right.size);
}

CachedItemValue & CachedItemValue::operator=(const CachedItemValue &right)
{
    if (this != &right)
    {
        if (isFetched)
            Free();

        isFetched = right.isFetched;
        size = right.size;

		dataContainer = right.dataContainer;
    }

    return (*this);
}

CachedItemValue & CachedItemValue::operator=(CachedItemValue &&right)
{
    if (this != &right)
    {
		dataContainer = std::move(right.dataContainer);

        isFetched = right.isFetched;
        size = right.size;
        
        right.size = 0;
        right.isFetched = false;
    }

    return (*this);
}

    

void CachedItemValue::Fetch(const FilePath & folder)
{
	DVASSERT(folder.IsDirectoryPathname());
	DVASSERT(isFetched == false);
    
    isFetched = true;
	for (auto & dc : dataContainer)
    {
        DVASSERT(IsDataLoaded(dc.second) == false);
		dc.second = LoadFile(folder + dc.first);
    }
}

void CachedItemValue::Free()
{
    DVASSERT(isFetched == true);

    isFetched = false;
	for (auto & dc : dataContainer)
    {
		DVASSERT(IsDataLoaded(dc.second) == true);
		dc.second.reset();
    }
}
    
void CachedItemValue::Export(const FilePath & folder) const
{
    DVASSERT(folder.IsDirectoryPathname());
    
    FileSystem::Instance()->CreateDirectory(folder, true);
    
	for (auto & dc : dataContainer)
    {
        if(IsDataLoaded(dc.second) == false)
        {
            Logger::Warning("[CachedItemValue::%s] File(%s) not loaded", __FUNCTION__, dc.first.c_str());
            continue;
        }
        
        auto savedPath = folder + dc.first;
        
        ScopedPtr<File> file(File::Create(savedPath, File::CREATE | File::WRITE));
        if(file)
        {
			const ValueData &data = dc.second;

            auto written = file->Write(data.get()->data(), data.get()->size());
			DVVERIFY(written == data.get()->size());
        }
        else
        {
            Logger::Error("[CachedItemValue::%s] Cannot create file %s", __FUNCTION__, savedPath.GetStringValue().c_str());
        }
    }
}
    
CachedItemValue::ValueData CachedItemValue::LoadFile(const FilePath & pathname)
{
	ValueData data = std::make_shared<Vector<uint8>>();

	ScopedPtr<File> file(File::Create(pathname, File::OPEN | File::READ));
    if(file)
    {
        auto dataSize = file->GetSize();
		data.get()->resize(dataSize);
        
		auto read = file->Read(data.get()->data(), dataSize);
        DVVERIFY(read == dataSize);
    }
    else
    {
        Logger::Error("[CachedItemValue::%s] Cannot read file %s", __FUNCTION__, pathname.GetStringValue().c_str());
    }
    
    return data;
}

} // end of namespace AssetCache
} // end of namespace DAVA

