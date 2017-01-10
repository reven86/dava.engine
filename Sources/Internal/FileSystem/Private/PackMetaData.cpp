#include "FileSystem/Private/PackMetaData.h"
#include "FileSystem/FilePath.h"
#include "FileSystem/DynamicMemoryFile.h"
#include "Compression/LZ4Compressor.h"
#include "Base/Exception.h"
#include "Debug/DVAssert.h"

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

    String bytes;

    {
        std::stringstream ss;

        size_t sizePackData = 0;
        for (const auto& tuple : tablePacks)
        {
            const String& packName = std::get<0>(tuple);
            const String& depend = std::get<1>(tuple);
            ss << packName << ' ' << depend << '\n';
        }

        bytes = ss.str();
    }

    Vector<uint8> v(begin(bytes), end(bytes));
    Vector<uint8> compBytes;

    LZ4Compressor compressor;

    if (!compressor.Compress(v, compBytes))
    {
        DAVA_THROW(Exception, "can't compress pack table");
    }

    // (4b) header - "meta"
    // (4b) num_files
    // (4*num_files) meta_indexes
    // (4b) - uncompressed_size
    // (4b) - compressed_size
    // (compressed_size b) packs_and_dependencies_compressed
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

    uint32 sizeOfFilesMetaIndexes = static_cast<uint32>(tableFiles.size() * sizeof(uint32));
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

    if (compressedSize != file->Write(&compBytes[0], compressedSize))
    {
        DAVA_THROW(Exception, "write compressedSize failed");
    }

    // write num of files
    return file->GetDataVector();
}

struct membuf : std::streambuf
{
    membuf(const void* ptr, size_t size)
    {
        char* begin = const_cast<char*>(static_cast<const char*>(ptr));
        char* end = const_cast<char*>(begin + size);
        setg(begin, begin, end);
    }
};

void PackMetaData::Deserialize(const void* ptr, size_t size)
{
    DVASSERT(ptr != nullptr);
    DVASSERT(size >= 16);

    using namespace std;

    membuf buf(ptr, size);

    istream file(&buf);

    // (4b) header - "meta"
    // (4b) num_files
    // (4*num_files) meta_indexes
    // (4b) - uncompressed_size
    // (4b) - compressed_size
    // (compressed_size b) packs_and_dependencies_compressed
    array<char, 4> header;
    file.read(&header[0], 4);
    if (header != array<char, 4>{ 'm', 'e', 't', 'a' })
    {
        DAVA_THROW(Exception, "read metadata error - not meta");
    }
    uint32_t numFiles = 0;
    file.read(reinterpret_cast<char*>(&numFiles), 4);
    if (!file)
    {
        DAVA_THROW(Exception, "read metadata error - no numFiles");
    }
    tableFiles.resize(numFiles);

    const uint32_t numFilesBytes = numFiles * 4;
    file.read(reinterpret_cast<char*>(&tableFiles[0]), numFilesBytes);
    if (!file)
    {
        DAVA_THROW(Exception, "read metadata error - no tableFiles");
    }

    uint32_t uncompressedSize = 0;
    file.read(reinterpret_cast<char*>(&uncompressedSize), 4);
    if (!file)
    {
        DAVA_THROW(Exception, "read metadata error - no uncompressedSize");
    }
    uint32_t compressedSize = 0;
    file.read(reinterpret_cast<char*>(&compressedSize), 4);
    if (!file)
    {
        DAVA_THROW(Exception, "read metadata error - no compressedSize");
    }

    DVASSERT(16 + numFilesBytes + compressedSize == size);

    vector<uint8_t> compressedBuf(compressedSize);

    file.read(reinterpret_cast<char*>(&compressedBuf[0]), compressedSize);
    if (!file)
    {
        DAVA_THROW(Exception, "read metadata error - no compressedBuf");
    }

    DVASSERT(uncompressedSize >= compressedSize);

    vector<uint8_t> uncompressedBuf(uncompressedSize);

    if (!LZ4Compressor().Decompress(compressedBuf, uncompressedBuf))
    {
        DAVA_THROW(Exception, "read metadata error - can't decompress");
    }

    const char* startBuf = reinterpret_cast<const char*>(&uncompressedBuf[0]);

    membuf outBuf(startBuf, uncompressedSize);
    istream ss(&outBuf);

    // now parse decompressed packs data line by line (%s %s\n) format
    for (string line, packName, packDependency; getline(ss, line);)
    {
        auto first_space = line.find(' ');
        if (first_space == string::npos)
        {
            DAVA_THROW(Exception, "can't parse packs and dependencies");
        }
        packName = line.substr(0, first_space);
        packDependency = line.substr(first_space + 1);
        tablePacks.push_back(tuple<String, String>(packName, packDependency));
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
