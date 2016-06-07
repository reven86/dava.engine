#include "pack_archive.h"

#include <algorithm>
#include <cstring>

#include "lz4.h"

bool LZ4CompressorDecompress(const std::vector<uint8_t>& in,
                             std::vector<uint8_t>& out)
{
    int32_t decompressResult =
    LZ4_decompress_fast(reinterpret_cast<const char*>(in.data()),
                        reinterpret_cast<char*>(out.data()),
                        static_cast<uint32_t>(out.size()));
    if (decompressResult < 0)
    {
        return false;
    }
    return true;
}

PackArchive::PackArchive(const std::string& archiveName)
{
    using namespace PackFormat;

    arcName = archiveName;
    std::string fileName = archiveName;

    file.open(fileName, std::ios_base::binary);
    if (!file)
    {
        throw std::runtime_error("can't Open file: " + fileName);
    }
    auto& headerBlock = packFile.header;
    file.read(reinterpret_cast<char*>(&headerBlock), sizeof(headerBlock));
    if (!file)
    {
        throw std::runtime_error("can't read header from packfile: " + fileName);
    }
    if (headerBlock.marker != FileMarker)
    {
        throw std::runtime_error("incorrect marker in pack file: " + fileName);
    }
    if (headerBlock.numFiles == 0)
    {
        throw std::runtime_error("can't load packfile no files inside");
    }
    if (headerBlock.startNamesBlockPosition != file.tellg())
    {
        throw std::runtime_error(
        "error in header of packfile start position for file names incorrect");
    }
    std::string& fileNames = packFile.names.sortedNamesLz4hc;
    fileNames.resize(headerBlock.namesBlockSizeCompressedLZ4HC, '\0');

    file.read(&fileNames[0], static_cast<uint32_t>(fileNames.size()));
    if (!file)
    {
        throw std::runtime_error("can't read file names from packfile");
    }

    std::vector<uint8_t> compressedNames(begin(fileNames), end(fileNames));
    std::vector<uint8_t> originalNames;
    originalNames.resize(packFile.header.namesBlockSizeOriginal);
    if (!LZ4CompressorDecompress(compressedNames, originalNames))
    {
        throw std::runtime_error("can't uncompress file names");
    }
    fileNames.assign(begin(originalNames), end(originalNames));

    if (headerBlock.startFilesDataBlockPosition != file.tellg())
    {
        throw std::runtime_error(
        "can't load packfile incorrect start position of files data");
    }

    std::vector<FileTableEntry>& fileTable = packFile.filesData.files;
    fileTable.resize(headerBlock.numFiles);

    if (headerBlock.filesTableBlockSize / headerBlock.numFiles !=
        sizeof(FileTableEntry))
    {
        throw std::runtime_error(
        "can't load packfile bad originalSize of file table");
    }

    file.read(reinterpret_cast<char*>(&fileTable[0]),
              headerBlock.filesTableBlockSize);
    if (!file)
    {
        throw std::runtime_error("can't read files table from packfile");
    }

    if (headerBlock.startPackedFilesBlockPosition != file.tellg())
    {
        throw std::runtime_error(
        "can't read packfile, incorrect start position of compressed content");
    }

    filesInfoSortedByName.reserve(headerBlock.numFiles);

    size_t numFiles =
    std::count_if(begin(fileNames), end(fileNames), [](const char& ch) {
        return '\0' == ch;
    });
    if (numFiles != fileTable.size())
    {
        throw std::runtime_error("number of file names not match with table");
    }

    // now fill support structures for fast search by filename
    size_t fileNameIndex{ 0 };

    std::for_each(begin(fileTable),
                  end(fileTable),
                  [&](FileTableEntry& fileEntry) {
                      const char* fileNameLoc = &fileNames[fileNameIndex];
                      mapFileData.emplace(fileNameLoc, &fileEntry);

                      FileInfo info;

                      info.relativeFilePath = fileNameLoc;
                      info.originalSize = fileEntry.original;
                      info.compressedSize = fileEntry.compressed;
                      info.compressionType = fileEntry.packType;

                      filesInfoSortedByName.push_back(info);

                      fileNameIndex = fileNames.find('\0', fileNameIndex + 1);
                      ++fileNameIndex;
                  });

    bool result = std::is_sorted(begin(filesInfoSortedByName),
                                 end(filesInfoSortedByName),
                                 [](const FileInfo& first,
                                    const FileInfo& second) {
                                     return std::strcmp(first.relativeFilePath,
                                                        second.relativeFilePath)
                                     <= 0;
                                 });
    if (!result)
    {
        throw std::runtime_error("relative file names in archive not sorted!");
    }
}

const std::vector<FileInfo>& PackArchive::GetFilesInfo() const
{
    return filesInfoSortedByName;
}

const FileInfo* PackArchive::GetFileInfo(const std::string& relativeFilePath) const
{
    const auto it = std::lower_bound(begin(filesInfoSortedByName),
                                     end(filesInfoSortedByName),
                                     relativeFilePath,
                                     [relativeFilePath](const FileInfo& info,
                                                        const std::string& name) {
                                         return info.relativeFilePath < name;
                                     });
    if (it != filesInfoSortedByName.end())
    {
        return &(*it);
    }
    return nullptr;
}

bool PackArchive::HasFile(const std::string& relativeFilePath) const
{
    auto iterator = mapFileData.find(relativeFilePath);
    return iterator != mapFileData.end();
}

bool PackArchive::LoadFile(const std::string& relativeFilePath,
                           std::vector<uint8_t>& output)
{
    using namespace PackFormat;

    if (!HasFile(relativeFilePath))
    {
        return false;
    }

    const FileTableEntry& fileEntry = *mapFileData.find(relativeFilePath)->second;
    output.resize(fileEntry.original);

    file.seekg(fileEntry.startPositionInPackedFilesBlock, std::ios_base::beg);
    if (!file)
    {
        //Logger::Error("can't load file: %s course: can't find start file position in pack file", relativeFilePath.c_str());
        return false;
    }

    switch (fileEntry.packType)
    {
    case 0: // None
    {
        file.read(reinterpret_cast<char*>(output.data()), fileEntry.original);
        if (!file)
        {
            //Logger::Error("can't load file: %s course: can't read uncompressed content", relativeFilePath.c_str());
            return false;
        }
    }
    break;
    case 1: // Lz4
    case 2: // Lz4HC
    {
        std::vector<uint8_t> packedBuf(fileEntry.compressed);

        file.read(reinterpret_cast<char*>(packedBuf.data()),
                  fileEntry.compressed);
        if (!file)
        {
            //Logger::Error("can't load file: %s course: can't read compressed content", relativeFilePath.c_str());
            return false;
        }

        if (!LZ4CompressorDecompress(packedBuf, output))
        {
            //Logger::Error("can't load file: %s  course: decompress error", relativeFilePath.c_str());
            return false;
        }
    }
    break;
    case 3:
    {
        std::vector<uint8_t> packedBuf(fileEntry.compressed);

        file.read(reinterpret_cast<char*>(packedBuf.data()),
                  fileEntry.compressed);
        if (!file)
        {
            //Logger::Error("can't load file: %s course: can't read compressed content", relativeFilePath.c_str());
            return false;
        }

        //if (!ZipCompressor().Decompress(packedBuf, output))
        {
            //Logger::Error("can't load file: %s  course: decompress error", relativeFilePath.c_str());
            return false;
        }
    }
    break;
    } // end switch
    return true;
}
