#include "FileSystem/Private/PackArchive.h"
#include "Compression/ZipCompressor.h"
#include "Compression/LZ4Compressor.h"
#include "FileSystem/FileSystem.h"
#include "Utils/CRC32.h"

namespace DAVA
{
void PackArchive::ExtractFileTableData(const PackFormat::PackFile::FooterBlock& footerBlock,
                                       const Vector<uint8>& tmpBuffer,
                                       String& fileNames,
                                       PackFormat::PackFile::FilesTableBlock& fileTableBlock)
{
    Vector<uint8>& compressedNamesBuffer = fileTableBlock.names.compressedNames;
    compressedNamesBuffer.resize(footerBlock.info.namesSizeCompressed, '\0');

    uint32 sizeOfFilesData = footerBlock.info.numFiles * sizeof(PackFormat::FileTableEntry);
    const char* startOfCompressedNames = reinterpret_cast<const char*>(&tmpBuffer[sizeOfFilesData]);

    fileTableBlock.names.compressedNames.resize(footerBlock.info.namesSizeCompressed);

    std::copy_n(startOfCompressedNames, footerBlock.info.namesSizeCompressed, reinterpret_cast<char*>(fileTableBlock.names.compressedNames.data()));

    Vector<uint8> originalNamesBuffer;
    originalNamesBuffer.resize(footerBlock.info.namesSizeOriginal);
    if (!LZ4HCCompressor().Decompress(compressedNamesBuffer, originalNamesBuffer))
    {
        throw std::runtime_error("can't uncompress file names");
    }

    fileNames = String(begin(originalNamesBuffer), end(originalNamesBuffer));

    Vector<PackFormat::FileTableEntry>& fileTable = fileTableBlock.data.files;
    fileTable.resize(footerBlock.info.numFiles);

    const PackFormat::FileTableEntry* startFilesData = reinterpret_cast<const PackFormat::FileTableEntry*>(tmpBuffer.data());

    std::copy_n(startFilesData, footerBlock.info.numFiles, fileTable.data());
}

void PackArchive::FillFilesInfo(const PackFormat::PackFile& packFile,
                                const String& fileNames,
                                UnorderedMap<String, const PackFormat::FileTableEntry*>& mapFileData,
                                Vector<ResourceArchive::FileInfo>& filesInfo)
{
    filesInfo.reserve(packFile.footer.info.numFiles);

    size_t numFiles =
    std::count_if(begin(fileNames), end(fileNames), [](const char& ch)
                  {
                      return '\0' == ch;
                  });

    const Vector<PackFormat::FileTableEntry>& fileTable = packFile.filesTable.data.files;

    if (numFiles != fileTable.size())
    {
        throw std::runtime_error("number of file names not match with table");
    }

    // now fill support structures for fast search by filename
    size_t fileNameIndex{ 0 };

    std::for_each(begin(fileTable), end(fileTable), [&](const PackFormat::FileTableEntry& fileEntry)
                  {
                      const char* fileNameLoc = &fileNames[fileNameIndex];
                      mapFileData.emplace(fileNameLoc, &fileEntry);

                      ResourceArchive::FileInfo info;

                      info.relativeFilePath = fileNameLoc;
                      info.originalSize = fileEntry.originalSize;
                      info.originalCrc32 = fileEntry.originalCrc32;
                      info.compressedSize = fileEntry.compressedSize;
                      info.compressedCrc32 = fileEntry.compressedCrc32;
                      info.compressionType = fileEntry.type;

                      filesInfo.push_back(info);

                      fileNameIndex = fileNames.find('\0', fileNameIndex + 1);
                      ++fileNameIndex;
                  });
}

PackArchive::PackArchive(const FilePath& archiveName_)
    : archiveName(archiveName_)
{
    using namespace PackFormat;

    ScopedPtr<File> file(File::Create(archiveName, File::OPEN | File::READ));
    String fileName = archiveName.GetAbsolutePathname();

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

    if (footerBlock.info.numFiles > 0)
    {
        uint64 startFilesTableBlock = size - (sizeof(packFile.footer) + packFile.footer.info.filesTableSize);

        Vector<uint8> tmpBuffer;
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

        String fileNames;

        ExtractFileTableData(footerBlock, tmpBuffer, fileNames, packFile.filesTable);

        FillFilesInfo(packFile, fileNames, mapFileData, filesInfo);
    }
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

    ScopedPtr<File> file(File::Create(archiveName, File::OPEN | File::READ));

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
