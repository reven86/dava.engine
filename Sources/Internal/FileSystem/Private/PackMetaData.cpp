#include "FileSystem/Private/PackMetaData.h"
#include "Compression/LZ4Compressor.h"
#include "Base/Exception.h"

#include <sqlite_modern_cpp.h>

namespace DAVA
{
PackMetaData::PackMetaData(const void* ptr, std::size_t size)
{
    Deserialize(ptr, size);
}

PackMetaData::PackMetaData(const FilePath& metaDb)
{
    // extract tables from sqlite DB
    sqlite::database db(metaDb.GetAbsolutePathname());

    size_t numIndexes = 0;

    db << "SELECT count(*) FROM files"
    >> [&](int64 countIndexes)
    {
        DVASSERT(countIndexes > 0);
        numIndexes = static_cast<size_t>(countIndexes);
    };

    tableFiles.reserve(numIndexes);

    db << "SELECT path, pack_index FROM files"
    >> [&](std::string, int packIndex)
    {
        tableFiles.push_back(packIndex);
    };

    size_t numPacks = 0;

    db << "SELECT count(*) FROM packs"
    >> [&](int64 countPacks)
    {
        DVASSERT(countPacks > 0);
        numPacks = static_cast<size_t>(countPacks);
    };

    tablePacks.reserve(numPacks);

    db << "SELECT name, dependency FROM packs"
    >> [&](std::string name, std::string dependency)
    {
        tablePacks.push_back(std::tuple<String, String>(name, dependency));
    };

    // debug check that max index of fileIndex exist in packIndex
    auto it = max_element(begin(tableFiles), end(tableFiles));
    uint32 maxIndex = *it;
    if (maxIndex >= tablePacks.size())
    {
        DAVA_THROW(Exception, "read metadata error - too big index bad meta");
    }
}

uint32 PackMetaData::GetPackIndexForFile(const uint32 fileIndex) const
{
    return tableFiles.at(fileIndex);
}

void PackMetaData::GetPackInfo(const uint32 packIndex, String& packName, String& dependencies) const
{
    const auto& tuple = tablePacks.at(packIndex);
    packName = std::get<0>(tuple);
    dependencies = std::get<1>(tuple);
}

Vector<uint8> PackMetaData::Serialize() const
{
    DVASSERT(tablePacks.size() > 0);
    DVASSERT(tableFiles.size() > 0);

    std::stringstream ss;

    size_t sizePackData = 0;
    for (const auto& tuple : tablePacks)
    {
        const String& packName = std::get<0>(tuple);
        const String& depend = std::get<1>(tuple);
        ss << packName << ' ' << depend << '\n';
    }

    String bytes = ss.str();
    Vector<uint8> v(begin(bytes), end(bytes));
    Vector<uint8> compBytes;

    LZ4Compressor compressor;

    if (!compressor.Compress(v, compBytes))
    {
        DAVA_THROW(Exception, "can't compress pack table");
    }

    // 4b header - "meta"
    // 4b num_files
    // num_files b
    // 4b - uncompressed_size
    // 4b - compressed_size
    // compressed_size b

    size_t numBytes = sizeof(uint32) * 4 + tableFiles.size() * sizeof(uint32)
    + sizePackData;

    ScopedPtr<DynamicMemoryFile> file(DynamicMemoryFile::Create(File::READ | File::WRITE));
    if (4 != file->Write("meta", 4))
    {
        DAVA_THROW(Exception, "write meta header failed");
    }

    uint32 numFiles = static_cast<uint32>(tableFiles.size());
    if (4 != file->Write(&numFiles, sizeof(numFiles)))
    {
        DAVA_THROW(Exception, "write num_files failed");
    }

    uint32 sizeOfFilesMetaIndexes = static_cast<uint32>(tableFiles.size() * 4);
    if (sizeOfFilesMetaIndexes != file->Write(&tableFiles[0], sizeOfFilesMetaIndexes))
    {
        DAVA_THROW(Exception, "write meta file indexes failed");
    }

    uint32 uncompressedSize = static_cast<uint32>(bytes.size());

    if (4 != file->Write(&uncompressedSize, sizeof(uncompressedSize)))
    {
        DAVA_THROW(Exception, "write uncompressedSize failed");
    }

    uint32 compressedSize = static_cast<uint32>(compBytes.size());

    if (4 != file->Write(&compressedSize, sizeof(compressedSize)))
    {
        DAVA_THROW(Exception, "write compressedSize failed");
    }

    if (4 != file->Write(&compBytes[0], compressedSize))
    {
        DAVA_THROW(Exception, "write compressedSize failed");
    }

    // write num of files
    Vector<uint8> result = file->GetDataVector();
    return result;
}

void PackMetaData::Deserialize(const void* ptr, size_t size)
{
    DVASSERT(ptr != nullptr);
    DVASSERT(size >= 16);

    ScopedPtr<DynamicMemoryFile> file(DynamicMemoryFile::Create(reinterpret_cast<const uint8*>(ptr), static_cast<int32>(size), File::READ | File::WRITE));

    // 4b header - "meta"
    // 4b num_files
    // num_files b
    // 4b - uncompressed_size
    // 4b - compressed_size
    // compressed_size b
    std::array<char, 4> header;
    if (4 != file->Read(&header[0], 4) ||
        header != std::array<char, 4>{ 'm', 'e', 't', 'a' })
    {
        DAVA_THROW(Exception, "read metadata error - not meta");
    }
    uint32 numFiles = 0;
    if (4 != file->Read(&numFiles, 4))
    {
        DAVA_THROW(Exception, "read metadata error - no numFiles");
    }
    tableFiles.resize(numFiles);

    const uint32 numFilesBytes = numFiles * 4;
    if (numFilesBytes != file->Read(&tableFiles[0], numFilesBytes))
    {
        DAVA_THROW(Exception, "read metadata error - no tableFiles");
    }

    uint32 uncompressedSize = 0;
    if (4 != file->Read(&uncompressedSize, 4))
    {
        DAVA_THROW(Exception, "read metadata error - no uncompressedSize");
    }
    uint32 compressedSize = 0;
    if (4 != file->Read(&compressedSize, 4))
    {
        DAVA_THROW(Exception, "read metadata error - no compressedSize");
    }

    DVASSERT(16 + numFilesBytes + compressedSize == size);

    Vector<uint8> compressedBuf(compressedSize);

    if (compressedSize != file->Read(&compressedBuf[0], compressedSize))
    {
        DAVA_THROW(Exception, "read metadata error - no compressedBuf");
    }

    DVASSERT(uncompressedSize >= compressedSize);

    Vector<uint8> uncompressedBuf(uncompressedSize);

    LZ4Compressor compressor;
    if (!compressor.Decompress(compressedBuf, uncompressedBuf))
    {
        DAVA_THROW(Exception, "read metadata error - can't decompress");
    }

    const char* startBuf = reinterpret_cast<const char*>(&uncompressedBuf[0]);
    const char* endBuf = reinterpret_cast<const char*>(&uncompressedBuf[uncompressedSize]);
    const String str(startBuf, endBuf);

    // now parse decompressed packs data line by line (%s %s\n) format
    std::stringstream ss(str);
    String packName;
    String packDependency;
    for (; !ss.eof();)
    {
        ss >> packName >> packDependency;
        tablePacks.push_back(std::tuple<String, String>(packName, packDependency));
    }

    // debug check that max index of fileIndex exist in packIndex
    auto it = std::max_element(begin(tableFiles), end(tableFiles));
    uint32 maxIndex = *it;
    if (maxIndex >= tablePacks.size())
    {
        DAVA_THROW(Exception, "read metadata error - too big index bad meta");
    }
}

} // end namespace DAVA
