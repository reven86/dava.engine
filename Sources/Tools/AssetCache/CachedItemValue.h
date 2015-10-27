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

#ifndef __DAVAENGINE_ASSET_CACHE_CACHED_ITEM_VALUE_H__
#define __DAVAENGINE_ASSET_CACHE_CACHED_ITEM_VALUE_H__

#include "Base/BaseTypes.h"
#include "Base/Data.h"
#include "FileSystem/FilePath.h"
#include "Utils/MD5.h"

namespace DAVA
{
class KeyedArchive;
class File;
class VariantBuffer;

namespace AssetCache
{
class CachedItemValue final
{
    using ValueData = std::shared_ptr<Vector<uint8>>;
    using ValueDataContainer = Map<String, ValueData>;

public:
    CachedItemValue() = default;
    ~CachedItemValue();

    CachedItemValue(const CachedItemValue& right);
    CachedItemValue(CachedItemValue&& right);

    CachedItemValue& operator=(const CachedItemValue& right);
    CachedItemValue& operator=(CachedItemValue&& right);

    void Add(const String& name, ValueData data);

    bool IsEmpty() const;
    bool IsFetched() const;
    uint64 GetSize() const;

    void Serialize(KeyedArchive* archieve, bool serializeData) const;
    void Deserialize(KeyedArchive* archieve);

    bool Serialize(File* file) const;
    bool Deserialize(File* file);

    bool operator==(const CachedItemValue& right) const;

    bool Fetch(const FilePath& folder);
    void Free();

    void Export(const FilePath& folder) const;

private:
    ValueData LoadFile(const FilePath& pathname);

    bool IsDataLoaded(const ValueData& data) const;

private:
    ValueDataContainer dataContainer;

    uint64 size = 0;
    bool isFetched = false;
};

inline bool CachedItemValue::IsEmpty() const
{
    return dataContainer.empty();
}

inline bool CachedItemValue::IsFetched() const
{
    return isFetched;
}

inline bool CachedItemValue::IsDataLoaded(const CachedItemValue::ValueData& data) const
{
    return (data.get() != nullptr && !data.get()->empty());
}

inline uint64 CachedItemValue::GetSize() const
{
    DVASSERT((dataContainer.empty() && size == 0) || (!dataContainer.empty() && size > 0));
    return size;
}

} // end of namespace AssetCache
} // end of namespace DAVA

#endif // __DAVAENGINE_ASSET_CACHE_CACHED_ITEM_VALUE_H__
