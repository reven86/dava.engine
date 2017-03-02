#include "FileSystem/Private/PackMetaData.h"
#include "FileSystem/FilePath.h"
#include "FileSystem/DynamicMemoryFile.h"
#include "Compression/LZ4Compressor.h"
#include "Base/Exception.h"
#include "Debug/DVAssert.h"
#include "Utils/Utils.h"
#include "Utils/StringFormat.h"
#include "Logger/Logger.h"

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

    packIndexes.reserve(numIndexes);

    db << "SELECT path, pack_index FROM files"
    >> [&](std::string, int packIndex)
    {
        packIndexes.push_back(packIndex);
    };

    size_t numPacks = 0;

    db << "SELECT count(*) FROM packs"
    >> [&](int64 countPacks)
    {
        DVASSERT(countPacks > 0);
        numPacks = static_cast<size_t>(countPacks);
    };

    packDependencies.reserve(numPacks);

    db << "SELECT name, dependency FROM packs"
    >> [&](std::string name, std::string dependency)
    {
        packDependencies.push_back(PackInfo{ name, dependency });
    };

    // debug check that max index of fileIndex exist in packIndex
    auto it = max_element(begin(packIndexes), end(packIndexes));
    uint32 maxIndex = *it;
    if (maxIndex >= packDependencies.size())
    {
        DAVA_THROW(Exception, "read metadata error - too big index bad meta");
    }
}

Vector<String> PackMetaData::GetDependencyNames(const String& requestedPackName) const
{
    using namespace DAVA;
    Vector<String> requestNames;

    const PackInfo& packInfo = GetPackInfo(requestedPackName);
    const String& dependencies = packInfo.packDependencies;
    const String delimiter(", ");

    Split(dependencies, delimiter, requestNames);

    // convert every name from string representation of index to packName
    for (String& pack : requestNames)
    {
        try
        {
            unsigned long i = stoul(pack);
            uint32 index = static_cast<uint32>(i);
            const PackInfo& info = GetPackInfo(index);
            pack = info.packName;
        }
        catch (std::exception& ex)
        {
            String str = Format("bad dependency index for pack: %s, index value: %s, error: %s.",
                                packInfo.packName.c_str(), pack.c_str(), ex.what());
            Logger::Error("%s", str.c_str());
            DAVA_THROW(Exception, str);
        }
    }

    return requestNames;
}

Vector<uint32> PackMetaData::GetFileIndexes(const String& requestedPackName) const
{
    Vector<uint32> result;

    for (const PackInfo& t : packDependencies)
    {
        const String& packName = t.packName;
        if (packName == requestedPackName)
        {
            ptrdiff_t packIndex = std::distance(&packDependencies[0], &t);
            uint32 pIndex = static_cast<uint32>(packIndex);

            size_t numFilesInThisPack = std::count(begin(packIndexes), end(packIndexes), pIndex);
            result.reserve(numFilesInThisPack);

            for (const auto& index : packIndexes)
            {
                if (index == pIndex)
                {
                    ptrdiff_t fileIndex = std::distance(&packIndexes[0], &index);
                    uint32 fIndex = static_cast<uint32>(fileIndex);
                    result.push_back(fIndex);
                }
            }
            break;
        }
    }
    return result;
}

uint32 PackMetaData::GetPackIndexForFile(const uint32 fileIndex) const
{
    return packIndexes.at(fileIndex);
}

const PackMetaData::PackInfo& PackMetaData::GetPackInfo(const uint32 packIndex) const
{
    return packDependencies.at(packIndex);
}

const PackMetaData::PackInfo& PackMetaData::GetPackInfo(const String& packName) const
{
    for (const auto& packInfo : packDependencies)
    {
        if (packInfo.packName == packName)
        {
            return packInfo;
        }
    }
    DAVA_THROW(Exception, "no such packName: " + packName);
}

Vector<uint8> PackMetaData::Serialize() const
{
    DVASSERT(packDependencies.size() > 0);
    DVASSERT(packIndexes.size() > 0);

    Vector<uint8> compBytes;
    uint32 uncompressedSize = 0;
    {
        String bytes;
        std::stringstream ss;

        size_t sizePackData = 0;
        for (const PackInfo& tuple : packDependencies)
        {
            const String& packName = tuple.packName;
            const String& depend = tuple.packDependencies;
            ss << packName << ' ' << depend << '\n';
        }

        bytes = ss.str();

        uncompressedSize = static_cast<uint32>(bytes.size());

        Vector<uint8> v(begin(bytes), end(bytes));

        LZ4Compressor compressor;

        if (!compressor.Compress(v, compBytes))
        {
            DAVA_THROW(Exception, "can't compress pack table");
        }
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

    uint32 numFiles = static_cast<uint32>(packIndexes.size());
    if (4 != file->Write(&numFiles, sizeof(numFiles)))
    {
        DAVA_THROW(Exception, "write num_files failed");
    }

    uint32 sizeOfFilesMetaIndexes = static_cast<uint32>(packIndexes.size() * sizeof(uint32));
    if (sizeOfFilesMetaIndexes != file->Write(&packIndexes[0], sizeOfFilesMetaIndexes))
    {
        DAVA_THROW(Exception, "write meta file indexes failed");
    }

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
    packIndexes.resize(numFiles);

    const uint32_t numFilesBytes = numFiles * 4;
    file.read(reinterpret_cast<char*>(&packIndexes[0]), numFilesBytes);
    if (!file)
    {
        DAVA_THROW(Exception, "read metadata error - no packIndexes");
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
        packDependencies.push_back(PackInfo{ packName, packDependency });
    }

    // debug check that max index of fileIndex exist in packIndex
    auto it = std::max_element(begin(packIndexes), end(packIndexes));
    uint32 maxIndex = *it;
    if (maxIndex >= packDependencies.size())
    {
        DAVA_THROW(Exception, "read metadata error - too big index bad meta");
    }
}

} // end namespace DAVA
