#include "AssetCache/DoubleMD5Key.h"
#include "FileSystem/KeyedArchive.h"
#include "Debug/DVAssert.h"

namespace DAVA
{
namespace AssetCache
{
String KeyToString(const DoubleMD5Key& key)
{
    static const DAVA::uint32 bufferSize = HASH_SIZE * 2;
    Array<DAVA::char8, bufferSize + 1> buffer; // +1 is for MD5::HashToChar for \0

    const uint32 keySize = static_cast<uint32>(key.size());
    const uint32 bufSize = static_cast<uint32>(buffer.size());
    MD5::HashToChar(key.data(), keySize, buffer.data(), bufSize);

    return String(buffer.data(), bufferSize);
}

void StringToKey(const String& string, DoubleMD5Key& key)
{
    DVASSERT(string.length() == HASH_SIZE * 2);

    const uint32 stringSize = static_cast<uint32>(string.size());
    const uint32 keySize = static_cast<uint32>(key.size());
    MD5::CharToHash(string.data(), stringSize, key.data(), keySize);
}

void SerializeKey(const DoubleMD5Key& key, KeyedArchive* archieve)
{
    const uint32 keySize = static_cast<uint32>(key.size());
    archieve->SetByteArray("keyData", key.data(), keySize);
}

void DeserializeKey(DoubleMD5Key& key, const KeyedArchive* archieve)
{
    auto size = archieve->GetByteArraySize("keyData");
    DVASSERT(size == HASH_SIZE);

    Memcpy(key.data(), archieve->GetByteArray("keyData"), size);
}

void SetPrimaryKey(DoubleMD5Key& key, const MD5::MD5Digest& digest)
{
    Memcpy(key.data(), digest.digest.data(), digest.digest.size());
}

void SetSecondaryKey(DoubleMD5Key& key, const MD5::MD5Digest& digest)
{
    Memcpy(key.data() + MD5::MD5Digest::DIGEST_SIZE, digest.digest.data(), digest.digest.size());
}

} // end of namespace AssetCache
} // end of namespace DAVA
