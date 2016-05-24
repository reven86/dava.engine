#include "AssetCache/ServerCacheEntry.h"

#include "FileSystem/KeyedArchive.h"

#include "Platform/SystemTimer.h"
#include "Debug/DVAssert.h"

namespace DAVA
{
namespace AssetCache
{
ServerCacheEntry::ServerCacheEntry()
{
}

ServerCacheEntry::ServerCacheEntry(const CachedItemValue& _value)
    : value(_value)
{
}

ServerCacheEntry::ServerCacheEntry(ServerCacheEntry&& right)
    : value(std::move(right.value))
    , accessID(right.accessID)
{
}

ServerCacheEntry& ServerCacheEntry::operator=(ServerCacheEntry&& right)
{
    if (this != &right)
    {
        value = std::move(right.value);
        accessID = right.accessID;
    }

    return (*this);
}

bool ServerCacheEntry::operator==(const ServerCacheEntry& right) const
{
    return (accessID == right.accessID) && (value == right.value);
}

void ServerCacheEntry::Serialize(KeyedArchive* archieve) const
{
    DVASSERT(nullptr != archieve);

    archieve->SetUInt64("accessID", accessID);

    ScopedPtr<KeyedArchive> valueArchieve(new KeyedArchive());
    value.Serialize(valueArchieve, false);
    archieve->SetArchive("value", valueArchieve);
}

void ServerCacheEntry::Deserialize(KeyedArchive* archieve)
{
    DVASSERT(nullptr != archieve);

    accessID = archieve->GetUInt64("accessID");

    KeyedArchive* valueArchieve = archieve->GetArchive("value");
    DVASSERT(valueArchieve);
    value.Deserialize(valueArchieve);
}

bool ServerCacheEntry::Fetch(const FilePath& folder)
{
    return value.Fetch(folder);
}

void ServerCacheEntry::Free()
{
    value.Free();
}

}; // end of namespace AssetCache
}; // end of namespace DAVA
