#include "FileSystem/Private/PackArchive.h"
#include "Compression/ZipCompressor.h"
#include "Compression/LZ4Compressor.h"
#include "FileSystem/FileSystem.h"
#include "Utils/CRC32.h"

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

    uint64 size = file->GetSize();
    if (size < sizeof(packFile.footer))
    {
        throw std::runtime_error("file size less then pack footer: " + fileName);
    }

    if (!file->Seek(size - sizeof(packFile.footer), File::SEEK_FROM_START))
    {
        throw std::runtime_error("can't seek to footer in file: " + fileName);
    }

    auto& footerBlock = packFile.footer;
    uint32 numBytesRead = file->Read(&footerBlock, sizeof(footerBlock));
    if (numBytesRead != sizeof(footerBlock))
    {
        throw std::runtime_error("can't read footer from packfile: " + fileName);
    }

    uint32 crc32footer = CRC32::ForBuffer(reinterpret_cast<char*>(&packFile.footer.info), sizeof(packFile.footer.info));
    if (crc32footer != packFile.footer.infoCrc32)
    {
        throw std::runtime_error("crc32 not match in footer for: " + fileName);
    }

    if (footerBlock.info.packArchiveMarker != FileMarker)
    {
        throw std::runtime_error("incorrect marker in pack file: " + fileName);
    }

    uint64 startFilesTableBlock = size - (sizeof(packFile.footer) + packFile.footer.info.filesTableSize);

    Vector<char> tmpBuffer;
    tmpBuffer.resize(packFile.footer.info.filesTableSize);

    if (!file->Seek(startFilesTableBlock, File::SEEK_FROM_START))
    {
        throw std::runtime_error("can't seek to filesTable block in file: " + fileName);
    }

    numBytesRead = file->Read(tmpBuffer.data(), packFile.footer.info.filesTableSize);
    if (numBytesRead != packFile.footer.info.filesTableSize)
    {
        throw std::runtime_error("can't read filesTable block from file: " + fileName);
    }

    uint32 crc32filesTable = CRC32::ForBuffer(tmpBuffer.data(), packFile.footer.info.filesTableSize);
    if (crc32filesTable != packFile.footer.info.filesTableCrc32)
    {
        throw std::runtime_error("crc32 not match in filesTable in file: " + fileName);
    }

    Vector<uint8>& compressedNamesBuffer = packFile.filesTable.names.compressedNames;
    compressedNamesBuffer.resize(packFile.footer.info.namesSizeCompressed, '\0');

    uint32 sizeOfFilesData = packFile.footer.info.numFiles * sizeof(FileTableEntry);
    const char* startOfCompressedNames = &tmpBuffer[sizeOfFilesData];

    packFile.filesTable.names.compressedNames.resize(packFile.footer.info.namesSizeCompressed);

    std::copy_n(startOfCompressedNames, packFile.footer.info.namesSizeCompressed, reinterpret_cast<char*>(packFile.filesTable.names.compressedNames.data()));

    Vector<uint8> originalNamesBuffer;
    originalNamesBuffer.resize(packFile.footer.info.namesSizeOriginal);
    if (!LZ4HCCompressor().Decompress(compressedNamesBuffer, originalNamesBuffer))
    {
        throw std::runtime_error("can't uncompress file names");
    }

    String fileNames(begin(originalNamesBuffer), end(originalNamesBuffer));

    Vector<FileTableEntry>& fileTable = packFile.filesTable.data.files;
    fileTable.resize(footerBlock.info.numFiles);

    FileTableEntry* startFilesData = reinterpret_cast<FileTableEntry*>(tmpBuffer.data());

    std::copy_n(startFilesData, footerBlock.info.numFiles, fileTable.data());

    filesInfo.reserve(footerBlock.info.numFiles);

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
                      info.originalSize = fileEntry.originalSize;
                      info.compressedSize = fileEntry.compressedSize;
                      info.hash = fileEntry.compressedCrc32;
                      info.compressionType = fileEntry.type;

                      filesInfo.push_back(info);

                      fileNameIndex = fileNames.find('\0', fileNameIndex + 1);
                      ++fileNameIndex;
                  });
}

const Vector<ResourceArchive::FileInfo>& PackArchive::GetFilesInfo() const
{
    return filesInfo;
}

const ResourceArchive::FileInfo* PackArchive::GetFileInfo(const String& relativeFilePath) const
{
    auto it = mapFileData.find(relativeFilePath);

    if (it != mapFileData.end())
    {
        // find out index of FileInfo*
        const PackFormat::FileTableEntry* currentFile = it->second;
        const PackFormat::FileTableEntry* start = packFile.filesTable.data.files.data();
        ptrdiff_t index = std::distance(start, currentFile);
        return &filesInfo.at(static_cast<uint32>(index));
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
    output.resize(fileEntry.originalSize);

    bool isOk = file->Seek(fileEntry.startPosition, File::SEEK_FROM_START);
    if (!isOk)
    {
        Logger::Error("can't load file: %s course: can't find start file position in pack file", relativeFilePath.c_str());
        return false;
    }

    switch (fileEntry.type)
    {
    case Compressor::Type::None:
    {
        uint32 readOk = file->Read(output.data(), fileEntry.originalSize);
        if (readOk != fileEntry.originalSize)
        {
            Logger::Error("can't load file: %s course: can't read uncompressed content", relativeFilePath.c_str());
            return false;
        }
    }
    break;
    case Compressor::Type::Lz4:
    case Compressor::Type::Lz4HC:
    {
        Vector<uint8> packedBuf(fileEntry.compressedSize);

        uint32 readOk = file->Read(packedBuf.data(), fileEntry.compressedSize);
        if (readOk != fileEntry.compressedSize)
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
        Vector<uint8> packedBuf(fileEntry.compressedSize);

        uint32 readOk = file->Read(packedBuf.data(), fileEntry.compressedSize);
        if (readOk != fileEntry.compressedSize)
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
