#include "FileSystem/Private/PackArchive.h"
#include "Compression/ZipCompressor.h"
#include "Compression/LZ4Compressor.h"
#include "FileSystem/FileSystem.h"

namespace DAVA
{
PackArchive::PackArchive(const FilePath& archiveName)
{
    using namespace PackFormat;

    String fileName = archiveName.GetAbsolutePathname();

    file.Set(File::Create(fileName, File::OPEN | File::READ));
    if (!file)
    {
        throw std::runtime_error("can't Open file: " + fileName);
    }
    auto& headerBlock = packFile.header;
    uint32 numBytesRead = file->Read(&headerBlock, sizeof(headerBlock));
    if (numBytesRead != sizeof(headerBlock))
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
    if (headerBlock.startNamesBlockPosition != file->GetPos())
    {
        throw std::runtime_error("error in header of packfile start position for file names incorrect");
    }

    Vector<uint8>& compressedNamesBuffer = packFile.names.sortedNamesLz4hc;
    compressedNamesBuffer.resize(headerBlock.namesBlockSizeCompressedLZ4HC, '\0');

    numBytesRead = file->Read(compressedNamesBuffer.data(), static_cast<uint32>(compressedNamesBuffer.size()));
    if (numBytesRead != compressedNamesBuffer.size())
    {
        throw std::runtime_error("Can't read file names from packfile");
    }

    Vector<uint8> originalNamesBuffer;
    originalNamesBuffer.resize(packFile.header.namesBlockSizeOriginal);
    if (!LZ4HCCompressor().Decompress(compressedNamesBuffer, originalNamesBuffer))
    {
        throw std::runtime_error("can't uncompress file names");
    }

    String fileNames(begin(originalNamesBuffer), end(originalNamesBuffer));

    if (headerBlock.startFilesDataBlockPosition != file->GetPos())
    {
        throw std::runtime_error("can't load packfile incorrect start position of files data");
    }

    Vector<FileTableEntry>& fileTable = packFile.filesData.files;
    fileTable.resize(headerBlock.numFiles);

    if (headerBlock.filesTableBlockSize / headerBlock.numFiles !=
        sizeof(FileTableEntry))
    {
        throw std::runtime_error("can't load packfile bad originalSize of file table");
    }

    numBytesRead = file->Read(reinterpret_cast<char*>(&fileTable[0]),
                              headerBlock.filesTableBlockSize);
    if (numBytesRead != headerBlock.filesTableBlockSize)
    {
        throw std::runtime_error("can't read files table from packfile");
    }

    if (headerBlock.startPackedFilesBlockPosition != file->GetPos())
    {
        throw std::runtime_error("can't read packfile, incorrect start position of compressed content");
    }

    filesInfoSortedByName.reserve(headerBlock.numFiles);

    size_t numFiles =
    std::count_if(begin(fileNames), end(fileNames), [](const char& ch)
                  {
                      return '\0' == ch;
                  });
    if (numFiles != fileTable.size())
    {
        throw std::runtime_error("number of file names not match with table");
    }

    // now fill support structures for fast search by filename
    size_t fileNameIndex{ 0 };

    std::for_each(begin(fileTable), end(fileTable), [&](FileTableEntry& fileEntry)
                  {
                      const char* fileNameLoc = &fileNames[fileNameIndex];
                      mapFileData.emplace(fileNameLoc, &fileEntry);

                      ResourceArchive::FileInfo info;

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
                                 [](const ResourceArchive::FileInfo& first,
                                    const ResourceArchive::FileInfo& second)
                                 {
                                     return std::strcmp(first.relativeFilePath.c_str(), second.relativeFilePath.c_str()) < 0;
                                 });
    if (!result)
    {
        throw std::runtime_error("relative file names in archive not sorted!");
    }
}

const Vector<ResourceArchive::FileInfo>& PackArchive::GetFilesInfo() const
{
    return filesInfoSortedByName;
}

const ResourceArchive::FileInfo* PackArchive::GetFileInfo(const String& relativeFilePath) const
{
    const auto it = std::lower_bound(begin(filesInfoSortedByName), end(filesInfoSortedByName), relativeFilePath,
                                     [relativeFilePath](const ResourceArchive::FileInfo& info,
                                                        const String& name)
                                     {
                                         return info.relativeFilePath < name;
                                     });
    if (it != filesInfoSortedByName.end())
    {
        return &(*it);
    }
    return nullptr;
}

bool PackArchive::HasFile(const String& relativeFilePath) const
{
    auto iterator = mapFileData.find(relativeFilePath);
    return iterator != mapFileData.end();
}

bool PackArchive::LoadFile(const String& relativeFilePath, Vector<uint8>& output) const
{
    using namespace PackFormat;

    if (!HasFile(relativeFilePath))
    {
        Logger::Error("can't load file: %s course: not found\n",
                      relativeFilePath.c_str());
        return false;
    }

    const FileTableEntry& fileEntry = *mapFileData.find(relativeFilePath)->second;
    output.resize(fileEntry.original);

    bool isOk = file->Seek(fileEntry.startPositionInPackedFilesBlock, File::SEEK_FROM_START);
    if (!isOk)
    {
        Logger::Error("can't load file: %s course: can't find start file position in pack file", relativeFilePath.c_str());
        return false;
    }

    switch (fileEntry.packType)
    {
    case Compressor::Type::None:
    {
        uint32 readOk = file->Read(output.data(), fileEntry.original);
        if (readOk != fileEntry.original)
        {
            Logger::Error("can't load file: %s course: can't read uncompressed content", relativeFilePath.c_str());
            return false;
        }
    }
    break;
    case Compressor::Type::Lz4:
    case Compressor::Type::Lz4HC:
    {
        Vector<uint8> packedBuf(fileEntry.compressed);

        uint32 readOk = file->Read(packedBuf.data(), fileEntry.compressed);
        if (readOk != fileEntry.compressed)
        {
            Logger::Error("can't load file: %s course: can't read compressed content", relativeFilePath.c_str());
            return false;
        }

        if (!LZ4Compressor().Decompress(packedBuf, output))
        {
            Logger::Error("can't load file: %s  course: decompress error", relativeFilePath.c_str());
            return false;
        }
    }
    break;
    case Compressor::Type::RFC1951:
    {
        Vector<uint8> packedBuf(fileEntry.compressed);

        uint32 readOk = file->Read(packedBuf.data(), fileEntry.compressed);
        if (readOk != fileEntry.compressed)
        {
            Logger::Error("can't load file: %s course: can't read compressed content", relativeFilePath.c_str());
            return false;
        }

        if (!ZipCompressor().Decompress(packedBuf, output))
        {
            Logger::Error("can't load file: %s  course: decompress error", relativeFilePath.c_str());
            return false;
        }
    }
    break;
    } // end switch
    return true;
}

} // end namespace DAVA
