/*==================================================================================
    Copyright (c) 2008, binaryzebra
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    * Neither the name of the binaryzebra nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE binaryzebra AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL binaryzebra BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/

#include "FileSystem/FileSystem.h"
#include "FileSystem/File.h"
#include "FileSystem/ResourceArchive.h"

#include <lz4/lz4.h>
#include <lz4/lz4hc.h>

namespace
{
const std::array<char, 4> PackFileMarker = {'P', 'A', 'C', 'K'};
const ::DAVA::uint32 PackFileMagic = 20150817;

struct PackFileStructure
{
    struct FileHeaderBlock
    {
        std::array<char, 4> resPackMarker;
        ::DAVA::uint32 fileNamesLength;
        ::DAVA::uint32 numFiles;
        ::DAVA::uint32 sizeOfAllFilesData;
        ::DAVA::uint32 startPositionFileNames;
        ::DAVA::uint32 startPositionFilesData;
        ::DAVA::uint32 startPositionCompressedContent;
        ::DAVA::uint32 magic;
    } headerBlock;

    struct StringsFileNamesBlock
    {
        ::DAVA::String fileNames;
    } fileNamesBlock;

    struct FileTableBlock
    {
        struct FileData
        {
            ::DAVA::uint32 fileStartPosition;
            ::DAVA::uint32 compressedSize;
            ::DAVA::uint32 originalSize;
            ::DAVA::ResourceArchive::CompressionType compressionType;
            std::array<char, 16> reservedMetadata;
        };
        static_assert(sizeof(FileData) == 32, "fix file table size");
        std::vector<FileData> fileTable;
    } fileTableBlock;

    struct RawCompressedFilesBlock
    {
        std::unique_ptr<uint8_t[]> rawCompressedFilesContent;
    } notUsedReadDirectlyFromFile;
};

using FileTableEntry = PackFileStructure::FileTableBlock::FileData;

static_assert(sizeof(PackFileStructure::FileHeaderBlock) == 32,
              "fix compiler padding");

static_assert(sizeof(FileTableEntry) == 32, "fix compiler padding");
}  // end of anonymous namespace

namespace DAVA
{
class ResourceArchiveImpl
{
public:
    explicit ResourceArchiveImpl() = default;

    bool Open(const std::string& file_name)
    {
        bool result = false;
        file.Set(File::Create(file_name, File::OPEN));
        if (!file)
        {
            throw std::runtime_error("can't Open file: " + file_name);
        }
        auto& header_block = packFile.headerBlock;
        uint32 isOk = file->Read(&header_block, sizeof(header_block));
        if (isOk <= 0)
        {
            throw std::runtime_error("can't read header from packfile");
        }
        if (header_block.resPackMarker != PackFileMarker)
        {
            throw std::runtime_error("incorrect marker in pack file: " +
                                     file_name);
        }
        if (PackFileMagic != header_block.magic)
        {
            throw std::runtime_error("can't read pack file incorrect magic: " +
                                     std::to_string(header_block.magic));
        }
        if (header_block.numFiles == 0)
        {
            throw std::runtime_error("can't load packfile no files inside");
        }
        if (header_block.startPositionFileNames != file->GetPos())
        {
            throw std::runtime_error(
                "error in header of packfile start position for file names "
                "incorrect");
        }
        String& fileNames = packFile.fileNamesBlock.fileNames;
        fileNames.resize(header_block.fileNamesLength, '\0');

        isOk = file->Read(&fileNames[0], fileNames.size());
        if (isOk <= 0)
        {
            throw std::runtime_error("can't read file names from packfile");
        }

        if (header_block.startPositionFilesData != file->GetPos())
        {
            throw std::runtime_error(
                "can't load packfile incorrect start position of files data");
        }

        Vector<FileTableEntry>& fileTable = packFile.fileTableBlock.fileTable;
        fileTable.resize(header_block.numFiles);

        if (header_block.sizeOfAllFilesData / header_block.numFiles !=
            sizeof(FileTableEntry))
        {
            throw std::runtime_error(
                "can't load packfile bad originalSize of file table");
        }

        isOk = file->Read(reinterpret_cast<char*>(&fileTable[0]),
                          header_block.sizeOfAllFilesData);
        if (isOk <= 0)
        {
            throw std::runtime_error("can't read files table from packfile");
        }

        if (header_block.startPositionCompressedContent != file->GetPos())
        {
            throw std::runtime_error(
                "can't read packfile, incorrect start position of compressed "
                "content");
        }

        filesInfoSortedByName.reserve(header_block.numFiles);

        // now fill support structures for quik search by filename
        std::size_t fileNameIndex{0};

        std::for_each(
            std::begin(fileTable), std::end(fileTable),
            [&](FileTableEntry& fileEntry)
            {
                const char* fileName = &fileNames[fileNameIndex];
                mapFileData.emplace(fileName, &fileEntry);

                filesInfoSortedByName.push_back(
                    ResourceArchive::FileInfo{fileName,
                                              fileEntry.originalSize,
                                              fileEntry.compressedSize,
                                              fileEntry.compressionType});

                fileNameIndex = fileNames.find('\0', fileNameIndex + 1);
                if (fileNameIndex == std::string::npos)
                {
                    throw std::runtime_error(
                        "can't load packfile, error during parsing filenames");
                }
                ++fileNameIndex;
            });

        result = std::is_sorted(filesInfoSortedByName.begin(),
                                filesInfoSortedByName.end(),
                                [](const ResourceArchive::FileInfo& first,
                                   const ResourceArchive::FileInfo& second)
                                {
                                    return first.name < second.name;
                                });
        if (!result)
        {
            throw std::runtime_error("file names in pakfile not sorted!");
        }
        else
        {
            result = true;
        }
        return result;
    }

    const std::vector<ResourceArchive::FileInfo>& GetFileList() const
    {
        return filesInfoSortedByName;
    }

    bool IsFileExist(const std::string& file_name,
                     std::size_t* uncompressed_size) const
    {
        auto iterator = mapFileData.find(file_name);
        bool found = iterator != mapFileData.end();
        if (found && uncompressed_size)
        {
            *uncompressed_size = iterator->second->originalSize;
        }
        return found;
    }

    const ResourceArchive::FileInfo* GetFileInfo(const String& fileName) const
    {
        const ResourceArchive::FileInfo* result = nullptr;
        const auto it = std::lower_bound(
            filesInfoSortedByName.begin(), filesInfoSortedByName.end(),
            fileName, [fileName](const ResourceArchive::FileInfo& info,
                                 const String& name)
            {
                return info.name < name;
            });
        if (it != filesInfoSortedByName.end())
        {
            result = &(*it);
        }
        return result;
    }

    bool LoadFile(const std::string& file_name,
                  char* destBuf,
                  std::size_t buff_size) const
    {
        bool result = false;
        std::size_t originalSize = 0;
        if (IsFileExist(file_name, &originalSize))
        {
            if (originalSize <= buff_size)
            {
                const FileTableEntry& fileEntry =
                    *mapFileData.find(file_name)->second;
                std::unique_ptr<char[]> compressedBuff(
                    new char[fileEntry.compressedSize]);
                if (file.Valid())
                {
                    bool isOk = file->Seek(fileEntry.fileStartPosition,
                                           File::SEEK_FROM_START);
                    if (isOk)
                    {
                        uint32 readOk = file->Read(compressedBuff.get(),
                                                   fileEntry.compressedSize);
                        if (readOk)
                        {
                            int decompressResult = LZ4_decompress_fast(
                                compressedBuff.get(), destBuf, originalSize);
                            if (decompressResult > 0)
                            {
                                result = true;
                            }
                            else
                            {
                                Logger::Error(
                                    "can't load file: %s  couse: decompress "
                                    "error\n",
                                    file_name.c_str());
                            }
                        }
                        else
                        {
                            Logger::Error(
                                "can't load file: %s couse: can't read "
                                "compressed content\n",
                                file_name.c_str());
                        }
                    }
                    else
                    {
                        Logger::Error(
                            "can't load file: %s couse: can't find start file "
                            "position in pack file\n",
                            file_name.c_str());
                    }
                }
                else
                {
                    Logger::Error(
                        "can't load file: %s couse: no opened packfile\n",
                        file_name.c_str());
                }
            }
            else
            {
                Logger::Error("cant't load file: %s couse: buffer too small\n",
                              file_name.c_str());
            }
        }
        else
        {
            Logger::Error("can't load file: %s couse: not found\n",
                          file_name.c_str());
        }
        return result;
    }

private:
    mutable RefPtr<File> file;
    PackFileStructure packFile;
    std::unordered_map<std::string, FileTableEntry*> mapFileData;
    std::vector<ResourceArchive::FileInfo> filesInfoSortedByName;
};

ResourceArchive::ResourceArchive() : impl(nullptr)
{
}

ResourceArchive::~ResourceArchive()
{
    Close();
}

// Open code
bool ResourceArchive::Open(const FilePath& archiveName)
{
    bool result = false;
    impl.reset(new ResourceArchiveImpl());
    result = impl->Open(archiveName.GetAbsolutePathname());
    if (!result)
    {
        impl.reset();
    }
    else
    {
        result = true;
    }
    return result;
}

const ResourceArchive::FileInfos& ResourceArchive::GetFilesInfo() const
{
    static std::vector<ResourceArchive::FileInfo> emptyVector;
    ResourceArchive::FileInfos& result = emptyVector;
    if (impl)
    {
        result = impl->GetFileList();
    }
    return result;
}

bool ResourceArchive::HasFile(const String& file_name) const
{
    bool result = false;
    if (impl)
    {
        result = impl->IsFileExist(file_name, nullptr);
    }
    return result;
}

const ResourceArchive::FileInfo* ResourceArchive::GetFileInfo(
    const String& fileName) const
{
    const ResourceArchive::FileInfo* result = nullptr;
    if (impl)
    {
        result = impl->GetFileInfo(fileName);
    }
    return result;
}

bool ResourceArchive::LoadFile(const String& fileName,
                               ResourceArchive::ContentAndSize& output) const
{
    bool result = false;
    if (impl)
    {
        if (impl->IsFileExist(fileName, &output.size))
        {
            output.content.reset(new char[output.size]);
            if (!impl->LoadFile(fileName, output.content.get(), output.size))
            {
                output.content.reset();
                output.size = 0;
            }
            else
            {
                result = true;
            }
        }
    }
    return result;
}

void ResourceArchive::Close()
{
    impl.reset();
}

bool ResourceArchive::CreatePack(const String& pacName,
                                 const String& basePath,
                                 const Vector<String>& sortedFileNames,
                                 const Vector<Rule>& compressionRules)
{
    bool result = false;

    if (!std::is_sorted(sortedFileNames.begin(), sortedFileNames.end()))
    {
        Logger::Error("ERROR: input sortedFileNames list is not sorted!\n");
    }
    else
    {
        RefPtr<File> packFileOutput(
            File::Create(pacName, File::CREATE | File::WRITE));
        if (packFileOutput.Valid())
        {
            PackFileStructure pack;

            auto& fileTable = pack.fileTableBlock.fileTable;

            Vector<char> fileBuffer;

            Vector<char> compressedFileBuffer;

            UnorderedSet<String> skippedFiles;

            auto compressedFileTmp = pacName + "_tmp_compressed_files.bin";

            RefPtr<File> outFile(
                File::Create(compressedFileTmp, File::CREATE | File::WRITE));

            if (outFile.Valid())
            {
            }
            else
            {
                Logger::Error("can't create tmp file for resource archive");
                return false;
            }

            pack.fileTableBlock.fileTable.reserve(sortedFileNames.size());

            std::for_each(
                std::begin(sortedFileNames), std::end(sortedFileNames),
                [&](const String& fileName)
                {
                    try
                    {
                        RefPtr<File> inFile(
                            File::Create(fileName, File::OPEN | File::READ));

                        DVASSERT(inFile);

                        inFile->Seek(0, File::SEEK_FROM_END);
                        uint32 inFileSize = inFile->GetPos();
                        inFile->Seek(0, File::SEEK_FROM_START);
                        fileBuffer.resize(inFileSize);
                        uint32 readOk =
                            inFile->Read(&fileBuffer[0], inFileSize);
                        if (readOk <= 0)
                        {
                            DVASSERT(false);
                        }
                        inFile->Release();

                        auto GetCompressionRuleFromFileName =
                            [&](const String& name) -> CompressionType
                        {
                            for (const auto& rule : compressionRules)
                            {
                                if (name.rfind(rule.fileExt, 0) != String::npos)
                                {
                                    return rule.compressionType;
                                }
                            }
                            return CompressionType::Lz4;
                        };

                        ResourceArchive::CompressionType compressionType =
                            GetCompressionRuleFromFileName(fileName);

                        uint32 startPos{0};
                        if (fileTable.empty() == false)
                        {
                            auto& prevCompressedFile = fileTable.back();
                            startPos = prevCompressedFile.fileStartPosition +
                                       prevCompressedFile.compressedSize;
                        }

                        switch (compressionType)
                        {
                            case CompressionType::Lz4:
                            {
                                int compressedSizeBound =
                                    LZ4_compressBound(inFileSize);
                                compressedFileBuffer.resize(
                                    compressedSizeBound);

                                int compressResult = LZ4_compress(
                                    &fileBuffer[0], &compressedFileBuffer[0],
                                    inFileSize);
                                if (compressResult <= 0)
                                {
                                    throw std::runtime_error(
                                        "can't compress packFileOutput: " +
                                        fileName);
                                }

                                uint32_t compressedSize =
                                    static_cast<uint32>(compressResult);

                                PackFileStructure::FileTableBlock::FileData
                                    fileData{startPos,
                                             compressedSize,
                                             inFileSize,
                                             CompressionType::Lz4};

                                fileTable.push_back(fileData);

                                uint32 writeOk = outFile->Write(
                                    &compressedFileBuffer[0], compressedSize);
                                if (writeOk <= 0)
                                {
                                    throw std::runtime_error(
                                        "can't write into tmp archive file");
                                }
                            }
                            break;
                            case CompressionType::Lz4HC:
                            {
                                int compressedSizeBound =
                                    LZ4_compressBound(inFileSize);
                                compressedFileBuffer.resize(
                                    compressedSizeBound);

                                int compressResult = LZ4_compressHC(
                                    &fileBuffer[0], &compressedFileBuffer[0],
                                    inFileSize);
                                if (compressResult <= 0)
                                {
                                    throw std::runtime_error(
                                        "can't compress packFileOutput: " +
                                        fileName);
                                }

                                uint32_t compressedSize =
                                    static_cast<uint32>(compressResult);

                                PackFileStructure::FileTableBlock::FileData
                                    fileData{startPos,
                                             compressedSize,
                                             inFileSize,
                                             CompressionType::Lz4HC};

                                fileTable.push_back(fileData);

                                uint32 writeOk = outFile->Write(
                                    &compressedFileBuffer[0], compressedSize);
                                if (writeOk <= 0)
                                {
                                    throw std::runtime_error(
                                        "can't write into tmp archive file");
                                }
                            }
                            break;
                            case CompressionType::None:
                            {
                                PackFileStructure::FileTableBlock::FileData
                                    fileData{startPos,
                                             inFileSize,
                                             inFileSize,
                                             CompressionType::None};

                                fileTable.push_back(fileData);
                                uint32 writeOk =
                                    outFile->Write(&fileBuffer[0], inFileSize);
                                if (writeOk <= 0)
                                {
                                    throw std::runtime_error(
                                        "can't write into tmp archive file");
                                }
                            }
                            break;
                        }

                        if (!outFile)
                        {
                            throw std::runtime_error(
                                "can't write compressed packFileOutput to tmp");
                        }
                    }
                    catch (std::exception& ex)
                    {
                        Logger::Error(
                            "ERROR: can't compress packFileOutput: %s  cause: "
                            "%s skip to next.\n", ex.what());
                        skippedFiles.insert(fileName);
                    }
                });
            outFile->Flush();

            PackFileStructure::FileHeaderBlock& headerBlock = pack.headerBlock;
            headerBlock.resPackMarker = PackFileMarker;
            headerBlock.magic = PackFileMagic;
            headerBlock.numFiles = sortedFileNames.size() - skippedFiles.size();

            StringStream ss;
            std::for_each(
                sortedFileNames.begin(), sortedFileNames.end(),
                [&](const std::string& fileName)
                {
                    if (skippedFiles.find(fileName) == skippedFiles.end())
                    {
                        ss << fileName << '\0';
                    }
                });

            PackFileStructure::StringsFileNamesBlock& stringsFileNamesBlock =
                pack.fileNamesBlock;
            stringsFileNamesBlock.fileNames = std::move(ss.str());

            headerBlock.fileNamesLength =
                stringsFileNamesBlock.fileNames.size();

            headerBlock.startPositionFileNames =
                sizeof(PackFileStructure::FileHeaderBlock);

            headerBlock.startPositionFilesData =
                headerBlock.startPositionFileNames +
                stringsFileNamesBlock.fileNames.size();

            uint32 sizeOfFilesTable =
                pack.fileTableBlock.fileTable.size() *
                sizeof(pack.fileTableBlock.fileTable[0]);

            headerBlock.sizeOfAllFilesData = sizeOfFilesTable;
            headerBlock.startPositionCompressedContent =
                headerBlock.startPositionFilesData + sizeOfFilesTable;

            std::for_each(
                std::begin(fileTable), std::end(fileTable),
                [&](PackFileStructure::FileTableBlock::FileData& fileData)
                {
                    fileData.fileStartPosition +=
                        headerBlock.startPositionCompressedContent;
                });

            uint32 writeOk = packFileOutput->Write(&pack.headerBlock,
                                 sizeof(pack.headerBlock));
            if (writeOk <= 0)
            {
                Logger::Error("can't write header block to archive");
                return false;
            }

            writeOk = packFileOutput->Write(&pack.fileNamesBlock.fileNames[0],
                                 pack.fileNamesBlock.fileNames.size());
            if(writeOk <= 0)
            {
                Logger::Error("can't write filenames block to archive");
                return false;
            }
            writeOk = packFileOutput->Write(
                &pack.fileTableBlock.fileTable[0],
                sizeOfFilesTable);

            if(writeOk <= 0)
            {
                Logger::Error("can't write filetable block to archive");
                return false;
            }

            RefPtr<File> compressedContentFile(File::Create(compressedFileTmp, File::OPEN | File::READ));
            if(!compressedContentFile)
            {
                Logger::Error("can't open compressed tmp file");
                return false;
            }

            std::array<char, 4096> copyBuf;
            uint32 lastRead = compressedContentFile->Read(&copyBuf[0], copyBuf.size());
            while(lastRead == copyBuf.size())
            {
                uint32 lastWrite = packFileOutput->Write(&copyBuf[0], copyBuf.size());
                if (lastWrite != copyBuf.size())
                {
                    Logger::Error("can't write part of tmp archive file into output packfile\n");
                    return false;
                }
                lastRead = compressedContentFile->Read(&copyBuf[0], copyBuf.size());
            }
            if (lastRead > 0)
            {
                uint32 lastWrite = packFileOutput->Write(&copyBuf[0], lastRead);
                if (lastWrite != lastRead)
                {
                    Logger::Error("can't write part of tmp archive file into output packfile\n");
                    return false;
                }
            }

            if (!compressedContentFile->IsEof())
            {
                Logger::Error("can't read full content of tmp compressed output file\n");
                return false;
            }

            compressedContentFile->Release();

            std::remove(compressedFileTmp.c_str());

            result = true;
        }
        else
        {
            Logger::Error("can't create packfile, can't Open: %s\n", pacName);
        }
    }
    return result;
}

}  // end namespace DAVA