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

    std::string fileName = archiveName;

    file.open(fileName, std::ios_base::binary | std::ios_base::ate);
    if (!file)
    {
        throw std::runtime_error("can't Open file: " + fileName);
    }

    uint64_t size = file.tellg();
    if (size < sizeof(packFile.footer))
    {
        throw std::runtime_error("file size less then pack footer: " + fileName);
    }

    if (!file.seekg(size - sizeof(packFile.footer), std::ios_base::beg))
    {
        throw std::runtime_error("can't seek to footer in file: " + fileName);
    }

    auto& footerBlock = packFile.footer;
    file.read(reinterpret_cast<char*>(&footerBlock), sizeof(footerBlock));
    if (!file)
    {
        throw std::runtime_error("can't read footer from packfile: " + fileName);
    }

    // skip count crc32

    if (footerBlock.info.packArchiveMarker != FileMarker)
    {
        throw std::runtime_error("incorrect marker in pack file: " + fileName);
    }

    uint64_t startFilesTableBlock = size - (sizeof(packFile.footer) + packFile.footer.info.filesTableSize);

    std::vector<char> tmpBuffer;
    tmpBuffer.resize(packFile.footer.info.filesTableSize);

    if (!file.seekg(startFilesTableBlock, std::ios_base::beg))
    {
        throw std::runtime_error("can't seek to filesTable block in file: " + fileName);
    }

    file.read(tmpBuffer.data(), packFile.footer.info.filesTableSize);
    if (!file)
    {
        throw std::runtime_error("can't read filesTable block from file: " + fileName);
    }

    std::vector<uint8_t>& compressedNamesBuffer = packFile.filesTable.names.compressedNames;
    compressedNamesBuffer.resize(packFile.footer.info.namesSizeCompressed, '\0');

    uint32_t sizeOfFilesData = packFile.footer.info.numFiles * sizeof(FileTableEntry);
    const char* startOfCompressedNames = &tmpBuffer[sizeOfFilesData];

    packFile.filesTable.names.compressedNames.resize(packFile.footer.info.namesSizeCompressed);

    std::copy_n(startOfCompressedNames, packFile.footer.info.namesSizeCompressed, reinterpret_cast<char*>(packFile.filesTable.names.compressedNames.data()));

    std::vector<uint8_t> originalNamesBuffer;
    originalNamesBuffer.resize(packFile.footer.info.namesSizeOriginal);
    if (!LZ4CompressorDecompress(compressedNamesBuffer, originalNamesBuffer))
    {
        throw std::runtime_error("can't uncompress file names");
    }

    std::string fileNames(begin(originalNamesBuffer), end(originalNamesBuffer));

    std::vector<FileTableEntry>& fileTable = packFile.filesTable.data.files;
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

                      FileInfo info;

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

const std::vector<FileInfo>& PackArchive::GetFilesInfo() const
{
    return filesInfo;
}

const FileInfo* PackArchive::GetFileInfo(const std::string& relativeFilePath) const
{
    auto it = mapFileData.find(relativeFilePath);

    if (it != mapFileData.end())
    {
        // find out index of FileInfo*
        const PackFormat::FileTableEntry* currentFile = it->second;
        const PackFormat::FileTableEntry* start = packFile.filesTable.data.files.data();
        ptrdiff_t index = std::distance(start, currentFile);
        return &filesInfo.at(static_cast<uint32_t>(index));
    }
    return nullptr;
}

bool PackArchive::HasFile(const std::string& relativeFilePath) const
{
    auto iterator = mapFileData.find(relativeFilePath);
    return iterator != mapFileData.end();
}

bool PackArchive::LoadFile(const std::string& relativeFilePath, std::vector<uint8_t>& output)
{
    using namespace PackFormat;

    if (!HasFile(relativeFilePath))
    {
        return false;
    }

    const FileTableEntry& fileEntry = *mapFileData.find(relativeFilePath)->second;
    output.resize(fileEntry.originalSize);

    file.seekg(fileEntry.startPosition, std::ios_base::beg);
    if (!file)
    {
        return false;
    }

    switch (fileEntry.type)
    {
    case 0:
    {
        file.read(reinterpret_cast<char*>(output.data()), fileEntry.originalSize);
        if (!file)
        {
            return false;
        }
    }
    break;
    case 1: // Compressor::Type::Lz4:
    case 2: // Compressor::Type::Lz4HC:
    {
        std::vector<uint8_t> packedBuf(fileEntry.compressedSize);

        file.read(reinterpret_cast<char*>(packedBuf.data()), fileEntry.compressedSize);
        if (!file)
        {
            return false;
        }

        if (!LZ4CompressorDecompress(packedBuf, output))
        {
            return false;
        }
    }
    break;
    case 3: // Compressor::Type::RFC1951:
    {
        std::vector<uint8_t> packedBuf(fileEntry.compressedSize);

        file.read(reinterpret_cast<char*>(packedBuf.data()), fileEntry.compressedSize);
        if (!file)
        {
            return false;
        }

        //TODO if (!ZipCompressor().Decompress(packedBuf, output))

        return false;
    }
    break;
    } // end switch
    return true;
}
