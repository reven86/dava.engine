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
CachedItemValue::CachedItemValue(const CachedItemValue& right)
    : dataContainer(right.dataContainer)
    , size(right.size)
    , isFetched(right.isFetched)
{
}

CachedItemValue::CachedItemValue(CachedItemValue&& right)
    : dataContainer(std::move(right.dataContainer))
    , size(right.size)
    , isFetched(right.isFetched)
{
    right.size = 0;
    right.isFetched = false;
}

CachedItemValue::~CachedItemValue()
{
    if (isFetched)
    {
        Free();
    }
    else
    {
        DVASSERT(std::all_of(dataContainer.cbegin(), dataContainer.cend(), [this](const ValueDataContainer::value_type& data) {
            return IsDataLoaded(data.second) == false;
        }));
    }

    dataContainer.clear();
    size = 0;
    isFetched = false;
}

void CachedItemValue::Add(const String& name, ValueData data)
{
    DVASSERT(dataContainer.count(name) == 0);
    DVASSERT(IsDataLoaded(data));

    dataContainer[name] = data;
    size += data.get()->size();

    isFetched = true;
}

void CachedItemValue::Serialize(KeyedArchive* archieve, bool serializeData) const
{
    DVASSERT(nullptr != archieve);

    archieve->SetUInt64("size", size);

    auto count = dataContainer.size();
    archieve->SetUInt32("data_count", count);

    int32 index = 0;
    for (const auto& dc : dataContainer)
    {
        archieve->SetString(Format("name_%d", index), dc.first);

        if (IsDataLoaded(dc.second) && serializeData)
        {
            auto& data = dc.second;
            archieve->SetByteArray(Format("data_%d", index), data.get()->data(), data.get()->size());
        }

        ++index;
    }
}

void CachedItemValue::Deserialize(KeyedArchive* archieve)
{
    DVASSERT(nullptr != archieve);
    DVASSERT(dataContainer.empty());
    DVASSERT(isFetched == false);

    size = archieve->GetUInt64("size");

    auto count = archieve->GetUInt32("data_count");
    for (uint32 i = 0; i < count; ++i)
    {
        String name = archieve->GetString(Format("name_%d", i));
        ValueData data = std::make_shared<Vector<uint8>>();

        auto key = Format("data_%d", i);
        auto size = archieve->GetByteArraySize(key);
        if (size > 0)
        {
            isFetched = true;

            data.get()->resize(size);
            Memcpy(data.get()->data(), archieve->GetByteArray(key), size);
        }

        dataContainer[name] = data;
    }
}

bool CachedItemValue::Serialize(File* buffer) const
{
    DVASSERT(buffer);

    if (buffer->Write(&size) != sizeof(size))
        return false;

    uint64 count = dataContainer.size();
    if (buffer->Write(&count) != sizeof(count))
        return false;

    for (const auto& entry : dataContainer)
    {
        if (buffer->WriteString(entry.first) == false)
            return false;

        uint32 dataSize = 0;
        const uint8* data = nullptr;

        if (IsDataLoaded(entry.second))
        {
            data = entry.second->data();
            dataSize = entry.second->size();
        }

        if (buffer->Write(&dataSize) != sizeof(dataSize))
            return false;

        if (buffer->Write(data, dataSize) != dataSize)
            return false;
    }

    return true;
}

bool CachedItemValue::Deserialize(File* file)
{
    DVASSERT(file);
    DVASSERT(isFetched == false);
    DVASSERT(dataContainer.empty());

    if (file->Read(&size) != sizeof(size))
        return false;

    uint64 count = 0;
    if (file->Read(&count) != sizeof(count))
        return false;

    for (; count > 0; --count)
    {
        String name;
        if (!file->ReadString(name))
            return false;

        uint32 datasize = 0;
        ValueData data = std::make_shared<Vector<uint8>>();

        if (file->Read(&datasize) != sizeof(datasize))
            return false;

        if (datasize > 0)
        {
            isFetched = true;
            data->resize(datasize);
            if (file->Read(data->data(), datasize) != datasize)
                return false;
        }

        dataContainer[name] = data;
    }

    return true;
}

bool CachedItemValue::operator==(const CachedItemValue& right) const
{
    if ((isFetched == right.isFetched) && (size == right.size) && (dataContainer.size() == right.dataContainer.size()))
    {
        return std::equal(dataContainer.cbegin(), dataContainer.cend(), right.dataContainer.cbegin(),
                          [](const ValueDataContainer::value_type& left, const ValueDataContainer::value_type& right) -> bool {
                              return left.first == right.first;
                          });
    }

    return false;
}

CachedItemValue& CachedItemValue::operator=(const CachedItemValue& right)
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

CachedItemValue& CachedItemValue::operator=(CachedItemValue&& right)
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

void CachedItemValue::Fetch(const FilePath& folder)
{
    DVASSERT(folder.IsDirectoryPathname());
    DVASSERT(isFetched == false);

    isFetched = true;
    for (auto& dc : dataContainer)
    {
        DVASSERT(IsDataLoaded(dc.second) == false);
        dc.second = LoadFile(folder + dc.first);
    }
}

void CachedItemValue::Free()
{
    DVASSERT(isFetched == true);

    isFetched = false;
    for (auto& dc : dataContainer)
    {
        DVASSERT(IsDataLoaded(dc.second) == true);
        dc.second.reset();
    }
}

void CachedItemValue::Export(const FilePath& folder) const
{
    DVASSERT(folder.IsDirectoryPathname());

    FileSystem::Instance()->CreateDirectory(folder, true);

    for (const auto& dc : dataContainer)
    {
        if (IsDataLoaded(dc.second) == false)
        {
            Logger::Warning("[CachedItemValue::%s] File(%s) not loaded", __FUNCTION__, dc.first.c_str());
            continue;
        }

        auto savedPath = folder + dc.first;

        ScopedPtr<File> file(File::Create(savedPath, File::CREATE | File::WRITE));
        if (file)
        {
            const ValueData& data = dc.second;

            auto written = file->Write(data.get()->data(), data.get()->size());
            DVVERIFY(written == data.get()->size());
        }
        else
        {
            Logger::Error("[CachedItemValue::%s] Cannot create file %s", __FUNCTION__, savedPath.GetStringValue().c_str());
        }
    }
}

CachedItemValue::ValueData CachedItemValue::LoadFile(const FilePath& pathname)
{
    ValueData data = std::make_shared<Vector<uint8>>();

    ScopedPtr<File> file(File::Create(pathname, File::OPEN | File::READ));
    if (file)
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
