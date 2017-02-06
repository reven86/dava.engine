#include "Tools/AssetCache/CacheItemKey.h"

#include <FileSystem/KeyedArchive.h>
#include <Debug/DVAssert.h>

namespace DAVA
{
namespace AssetCache
{
String CacheItemKey::ToString() const
{
    static const DAVA::uint32 bufferSize = HASH_SIZE * 2;
    Array<DAVA::char8, bufferSize + 1> buffer; // +1 is for MD5::HashToChar for \0

    const uint32 keySize = static_cast<uint32>(size());
    const uint32 bufSize = static_cast<uint32>(buffer.size());
    MD5::HashToChar(data(), keySize, buffer.data(), bufSize);

    return String(buffer.data(), bufferSize);
}

void CacheItemKey::FromString(const String& string)
{
    DVASSERT(string.length() == HASH_SIZE * 2);

    const uint32 stringSize = static_cast<uint32>(string.size());
    const uint32 keySize = static_cast<uint32>(size());
    MD5::CharToHash(string.data(), stringSize, data(), keySize);
}

void CacheItemKey::Serialize(KeyedArchive* archive) const
{
    const int32 keySize = static_cast<int32>(size());
    archive->SetByteArray("keyData", data(), keySize);
}

void CacheItemKey::Deserialize(const KeyedArchive* archive)
{
    int32 size = archive->GetByteArraySize("keyData");
    DVASSERT(size == HASH_SIZE);

    Memcpy(data(), archive->GetByteArray("keyData"), size);
}

void CacheItemKey::SetPrimaryKey(const MD5::MD5Digest& digest)
{
    Memcpy(data(), digest.digest.data(), digest.digest.size());
}

void CacheItemKey::SetSecondaryKey(const MD5::MD5Digest& digest)
{
    Memcpy(data() + MD5::MD5Digest::DIGEST_SIZE, digest.digest.data(), digest.digest.size());
}

} // end of namespace AssetCache
} // end of namespace DAVA
