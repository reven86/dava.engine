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

#include "FileSystem/ResourceArchive.h"

#include <lz4/lz4.h>
#include <lz4/lz4hc.h>

#include "FileSystem/FileSystem.h"
#include "FileSystem/File.h"

namespace
{
const std::array<char, 4> PackFileMarker = {'P', 'A', 'C', 'K'};
const DAVA::uint32 PackFileMagic = 20150817;

using PackingType = ::DAVA::ResourceArchive::CompressionType;

struct PackFile
{
    struct HeaderBlock
    {
        std::array<char, 4> resPackMarker;
        DAVA::uint32 magic;
        DAVA::uint32 namesBlockSize;
        DAVA::uint32 filesTableBlockSize;
        DAVA::uint32 startFileNames;
        DAVA::uint32 startFilesTable;
        DAVA::uint32 startPackedFiles;
        DAVA::uint32 numFiles;
    } headerBlock;

    struct NamesBlock
    {
        DAVA::String sortedNames;
    } namesBlock;

    struct FilesDataBlock
    {
        struct Data
        {
            DAVA::uint32 start;
            DAVA::uint32 compressed;
            DAVA::uint32 original;
            PackingType packType;
            std::array<char, 16> reserved;
        };
        DAVA::Vector<Data> fileTable;
    } filesDataBlock;

    struct PackedFilesBlock
    {
        uint8_t* packedFiles;
    } notUsedReadDirectlyFromFile;
};

using FileTableEntry = PackFile::FilesDataBlock::Data;

static_assert(sizeof(PackFile::HeaderBlock) == 32, "fix compiler padding");
static_assert(sizeof(FileTableEntry) == 32, "fix compiler padding");
}  // end of anonymous namespace

namespace DAVA
{
class ResourceArchiveImpl
{
public:
    explicit ResourceArchiveImpl() = default;

    bool Open(const String& file_name)
    {
        file.Set(File::Create(file_name, File::OPEN | File::READ));
        if (!file)
        {
            Logger::Error("can't Open file: %s\n", file_name.c_str());
            return false;
        }
        auto& header_block = packFile.headerBlock;
        uint32 isOk = file->Read(&header_block, sizeof(header_block));
        if (isOk <= 0)
        {
            Logger::Error("can't read header from packfile: %s\n",
                          file_name.c_str());
            return false;
        }
        if (header_block.resPackMarker != PackFileMarker)
        {
            Logger::Error("incorrect marker in pack file: %s\n",
                          file_name.c_str());
            return false;
        }
        if (PackFileMagic != header_block.magic)
        {
            Logger::Error("can't read packfile incorrect magic: 0x%X\n",
                          header_block.magic);
            return false;
        }
        if (header_block.numFiles == 0)
        {
            Logger::Error("can't load packfile no files inside");
            return false;
        }
        if (header_block.startFileNames != file->GetPos())
        {
            Logger::Error(
                "error in header of packfile start position for file names "
                "incorrect");
            return false;
        }
        String& fileNames = packFile.namesBlock.sortedNames;
        fileNames.resize(header_block.namesBlockSize, '\0');

        isOk = file->Read(&fileNames[0], fileNames.size());
        if (isOk <= 0)
        {
            Logger::Error("can't read file names from packfile");
            return false;
        }

        if (header_block.startFilesTable != file->GetPos())
        {
            Logger::Error(
                "can't load packfile incorrect start position of files data");
            return false;
        }

        Vector<FileTableEntry>& fileTable = packFile.filesDataBlock.fileTable;
        fileTable.resize(header_block.numFiles);

        if (header_block.filesTableBlockSize / header_block.numFiles !=
            sizeof(FileTableEntry))
        {
            Logger::Error("can't load packfile bad originalSize of file table");
            return false;
        }

        isOk = file->Read(reinterpret_cast<char*>(&fileTable[0]),
                          header_block.filesTableBlockSize);
        if (isOk <= 0)
        {
            Logger::Error("can't read files table from packfile");
            return false;
        }

        if (header_block.startPackedFiles != file->GetPos())
        {
            Logger::Error(
                "can't read packfile, incorrect start position of compressed "
                "content");
            return false;
        }

        filesInfoSortedByName.reserve(header_block.numFiles);

        size_t num_files =
            std::count_if(fileNames.begin(), fileNames.end(), [](const char& ch)
                          {
                              return '\0' == ch;
                          });
        if (num_files != fileTable.size())
        {
            Logger::Error("number of file names not match with table");
            return false;
        }

        // now fill support structures for fast search by filename
        size_t fileNameIndex{0};

        std::for_each(
            begin(fileTable), end(fileTable), [&](FileTableEntry& fileEntry)
            {
                const char* fileName = &fileNames[fileNameIndex];
                mapFileData.emplace(fileName, &fileEntry);

                filesInfoSortedByName.push_back(
                    ResourceArchive::FileInfo{fileName,
                                              fileEntry.original,
                                              fileEntry.compressed,
                                              fileEntry.packType});

                fileNameIndex = fileNames.find('\0', fileNameIndex + 1);
                ++fileNameIndex;
            });

        bool result = std::is_sorted(filesInfoSortedByName.begin(),
                                     filesInfoSortedByName.end(),
                                     [](const ResourceArchive::FileInfo& first,
                                        const ResourceArchive::FileInfo& second)
                                     {
                                         return first.name < second.name;
                                     });
        if (!result)
        {
            Logger::Error("file names in pakfile not sorted!");
            return false;
        }

        return true;
    }

    const ResourceArchive::FileInfos& GetFileList() const
    {
        return filesInfoSortedByName;
    }

    bool IsFileExist(const String& file_name,
                     std::size_t* uncompressed_size) const
    {
        auto iterator = mapFileData.find(file_name);
        bool found = iterator != mapFileData.end();
        if (found && uncompressed_size)
        {
            *uncompressed_size = iterator->second->original;
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

    bool LoadFile(const String& file_name,
                  ResourceArchive::ContentAndSize& output) const
    {
        if (!IsFileExist(file_name, &output.size))
        {
            Logger::Error("can't load file: %s couse: not found\n",
                          file_name.c_str());
            return false;
        }

        const FileTableEntry& fileEntry = *mapFileData.find(file_name)->second;

        if (!file)
        {
            Logger::Error("can't load file: %s couse: no opened packfile\n",
                          file_name.c_str());
            return false;
        }

        bool isOk = file->Seek(fileEntry.start, File::SEEK_FROM_START);
        if (!isOk)
        {
            Logger::Error(
                "can't load file: %s couse: can't find start file "
                "position in pack file\n",
                file_name.c_str());
            return false;
        }

        switch (fileEntry.packType)
        {
            case ResourceArchive::CompressionType::None:
            {
                output.content.reset(new char[fileEntry.compressed]);
                uint32 readOk =
                    file->Read(output.content.get(), fileEntry.compressed);
                if (readOk != fileEntry.compressed)
                {
                    Logger::Error(
                        "can't load file: %s couse: can't read "
                        "uncompressed content\n",
                        file_name.c_str());
                    return false;
                }
            }
            break;
            case ResourceArchive::CompressionType::Lz4:
            case ResourceArchive::CompressionType::Lz4HC:
            {
                std::unique_ptr<char[]> packedBuf(
                    new char[fileEntry.compressed]);

                uint32 readOk =
                    file->Read(packedBuf.get(), fileEntry.compressed);
                if (readOk != fileEntry.compressed)
                {
                    Logger::Error(
                        "can't load file: %s couse: can't read "
                        "compressed content\n",
                        file_name.c_str());
                    return false;
                }

                output.content.reset(new char[fileEntry.original]);

                int decompressResult = LZ4_decompress_fast(
                    packedBuf.get(), output.content.get(), output.size);
                if (decompressResult < 0)
                {
                    Logger::Error(
                        "can't load file: %s  couse: decompress "
                        "error\n",
                        file_name.c_str());
                    return false;
                }
            }
            break;
        }
        return true;
    }

private:
    mutable RefPtr<File> file;
    PackFile packFile;
    UnorderedMap<String, FileTableEntry*> mapFileData;
    ResourceArchive::FileInfos filesInfoSortedByName;
};

static ResourceArchive::CompressionType GetPackingByExt(
    const String& fileName,
    const Vector<ResourceArchive::Rule>& rules)
{
    for (const auto& rule : rules)
    {
        if (fileName.rfind(rule.fileExt, 0) != String::npos)
        {
            return rule.compressionType;
        }
    }
    return ResourceArchive::CompressionType::Lz4HC;
};

static bool Packing(const String& fileName,
                    Vector<FileTableEntry>& fileTable,
                    Vector<char>& inBuffer,
                    Vector<char>& packingBuf,
                    File* output,
                    uint32 origSize,
                    uint32 startPos,
                    int (*packFunction)(const char*, char*, int),
                    PackingType packingType)
{
    int sizeBound = LZ4_compressBound(origSize);
    packingBuf.resize(sizeBound);

    int packResult = packFunction(&inBuffer[0], &packingBuf[0], origSize);
    if (packResult <= 0)
    {
        Logger::Error("can't compress lz4 file: %s\n", fileName.c_str());
        return false;
    }

    uint32_t packedSize = static_cast<uint32>(packResult);

    uint32 writeOk = output->Write(&packingBuf[0], packedSize);
    if (writeOk <= 0)
    {
        Logger::Error("can't write into tmp archive file");
        return false;
    }

    PackFile::FilesDataBlock::Data fileData{
        startPos, packedSize, origSize, packingType};

    fileTable.push_back(fileData);

    return true;
}

static bool CompressFileAndWriteToOutput(
    const String& fileName,
    const Vector<ResourceArchive::Rule>& packingRules,
    Vector<FileTableEntry>& fileTable,
    Vector<char>& inBuffer,
    Vector<char>& packingBuf,
    File* output)
{
    RefPtr<File> inFile(File::Create(fileName, File::OPEN | File::READ));

    if (!inFile)
    {
        Logger::Error("can't read input file: %s\n", fileName.c_str());
        return false;
    }

    if (!inFile->Seek(0, File::SEEK_FROM_END))
    {
        Logger::Error("can't seek inside file: %s\n", fileName.c_str());
        return false;
    }
    uint32 origSize = inFile->GetPos();
    if (!inFile->Seek(0, File::SEEK_FROM_START))
    {
        Logger::Error("can't seek inside file: %s\n", fileName.c_str());
        return false;
    }

    PackingType compressionType = GetPackingByExt(fileName, packingRules);

    uint32 startPos{0};
    if (fileTable.empty() == false)
    {
        auto& prevPackData = fileTable.back();
        startPos = prevPackData.start + prevPackData.compressed;
    }

    if (origSize > 0)
    {
        inBuffer.resize(origSize);
        uint32 readOk = inFile->Read(&inBuffer[0], origSize);
        if (readOk <= 0)
        {
            Logger::Error("can't read input file: %s\n", fileName.c_str());
            return false;
        }
    }
    else
    {
        PackFile::FilesDataBlock::Data fileData{
            startPos,
            origSize,
            origSize,
            ResourceArchive::CompressionType::None};

        fileTable.push_back(fileData);
        return true;
    }

    switch (compressionType)
    {
        case PackingType::Lz4:
        {
            if (!Packing(fileName, fileTable, inBuffer, packingBuf, output,
                         origSize, startPos, LZ4_compress, PackingType::Lz4))
            {
                return false;
            }
        }
        break;
        case PackingType::Lz4HC:
        {
            if (!Packing(fileName, fileTable, inBuffer, packingBuf, output,
                         origSize, startPos, LZ4_compressHC,
                         PackingType::Lz4HC))
            {
                return false;
            }
        }
        break;
        case PackingType::None:
        {
            PackFile::FilesDataBlock::Data fileData{
                startPos,
                origSize,
                origSize,
                ResourceArchive::CompressionType::None};

            fileTable.push_back(fileData);
            uint32 writeOk = output->Write(&inBuffer[0], origSize);
            if (writeOk <= 0)
            {
                Logger::Error("can't write into tmp archive file");
                return false;
            }
        }
        break;
    }
    return true;
}

ResourceArchive::ResourceArchive() : impl(nullptr)
{
}

ResourceArchive::~ResourceArchive()
{
    Close();
}

bool ResourceArchive::Open(const FilePath& archiveName)
{
    impl.reset(new ResourceArchiveImpl());
    if (impl->Open(archiveName.GetAbsolutePathname()))
    {
        return true;
    }
    impl.reset();
    return false;
}

const ResourceArchive::FileInfos& ResourceArchive::GetFilesInfo() const
{
    static FileInfos emptyVector;
    if (impl)
    {
        return impl->GetFileList();
    }
    return emptyVector;
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
    const FileInfo* result = nullptr;
    if (impl)
    {
        result = impl->GetFileInfo(fileName);
    }
    return result;
}

bool ResourceArchive::LoadFile(const String& fileName,
                               ContentAndSize& output) const
{
    if (impl)
    {
        return impl->LoadFile(fileName, output);
    }
    return false;
}

void ResourceArchive::Close()
{
    impl.reset();
}

static bool CopyTmpfileToPackfile(RefPtr<File> packFileOutput,
                                  RefPtr<File> packedFile)
{
    std::array<char, 4096> copyBuf;
    uint32 lastRead = packedFile->Read(&copyBuf[0], copyBuf.size());
    while (lastRead == copyBuf.size())
    {
        uint32 lastWrite = packFileOutput->Write(&copyBuf[0], copyBuf.size());
        if (lastWrite != copyBuf.size())
        {
            Logger::Error(
                "can't write part of tmp archive file into output "
                "packfile\n");
            return false;
        }
        lastRead = packedFile->Read(&copyBuf[0], copyBuf.size());
    }
    if (lastRead > 0)
    {
        uint32 lastWrite = packFileOutput->Write(&copyBuf[0], lastRead);
        if (lastWrite != lastRead)
        {
            Logger::Error(
                "can't write part of tmp archive file into output "
                "packfile\n");
            return false;
        }
    }

    if (!packedFile->IsEof())
    {
        Logger::Error(
            "can't read full content of tmp compressed output file\n");
        return false;
    }
    return true;
}

bool ResourceArchive::CreatePack(const String& pacName,
                                 const Vector<String>& sortedFileNames,
                                 const Rules& compressionRules)
{
    bool result = false;

    if (!std::is_sorted(sortedFileNames.begin(), sortedFileNames.end()))
    {
        Logger::Error("sortedFileNames list is not sorted!\n");
        return false;
    }

    RefPtr<File> packFileOutput(
        File::Create(pacName, File::CREATE | File::WRITE));

    if (!packFileOutput)
    {
        Logger::Error("can't create packfile, can't Open: %s\n", pacName);
        return false;
    }

    PackFile pack;

    auto& fileTable = pack.filesDataBlock.fileTable;

    Vector<char> fileBuffer;
    Vector<char> compressedFileBuffer;
    UnorderedSet<String> skippedFiles;

    auto packedFileTmp = pacName + "_tmp_compressed_files.bin";

    std::unique_ptr<File, void (*)(File*)> outTmpFile(
        File::Create(packedFileTmp, File::CREATE | File::WRITE), [](File* f)
        {
            auto name = f->GetFilename().GetAbsolutePathname();
            if (0 != std::remove(name.c_str()))
            {
                Logger::Error("can't delete tmp file: %s", name.c_str());
            }
            SafeRelease(f);
        });

    if (!outTmpFile)
    {
        Logger::Error("can't create tmp file for resource archive");
        return false;
    }

    pack.filesDataBlock.fileTable.reserve(sortedFileNames.size());

    std::for_each(std::begin(sortedFileNames), std::end(sortedFileNames),
                  [&](const String& fileName)
                  {
                      bool result = CompressFileAndWriteToOutput(
                          fileName, compressionRules, fileTable, fileBuffer,
                          compressedFileBuffer, outTmpFile.get());
                      if (!result)
                      {
                          Logger::Info("can't pack file: %s, skip it\n",
                                       fileName.c_str());
                          skippedFiles.insert(fileName);
                      }
                  });
    outTmpFile->Flush();

    PackFile::HeaderBlock& headerBlock = pack.headerBlock;
    headerBlock.resPackMarker = PackFileMarker;
    headerBlock.magic = PackFileMagic;
    headerBlock.numFiles = sortedFileNames.size() - skippedFiles.size();

    StringStream ss;
    std::for_each(sortedFileNames.begin(), sortedFileNames.end(),
                  [&skippedFiles, &ss](const std::string& fileName)
                  {
                      if (skippedFiles.find(fileName) == skippedFiles.end())
                      {
                          ss << fileName << '\0';
                      }
                  });

    PackFile::NamesBlock& stringsFileNamesBlock = pack.namesBlock;
    stringsFileNamesBlock.sortedNames = std::move(ss.str());

    headerBlock.namesBlockSize = stringsFileNamesBlock.sortedNames.size();

    headerBlock.startFileNames = sizeof(PackFile::HeaderBlock);

    headerBlock.startFilesTable =
        headerBlock.startFileNames + stringsFileNamesBlock.sortedNames.size();

    uint32 sizeOfFilesTable = pack.filesDataBlock.fileTable.size() *
                              sizeof(pack.filesDataBlock.fileTable[0]);

    headerBlock.filesTableBlockSize = sizeOfFilesTable;
    headerBlock.startPackedFiles =
        headerBlock.startFilesTable + sizeOfFilesTable;

    uint32 delta = headerBlock.startPackedFiles;

    std::for_each(std::begin(fileTable), std::end(fileTable),
                  [delta](PackFile::FilesDataBlock::Data& fileData)
                  {
                      fileData.start += delta;
                  });

    uint32 writeOk =
        packFileOutput->Write(&pack.headerBlock, sizeof(pack.headerBlock));
    if (writeOk <= 0)
    {
        Logger::Error("can't write header block to archive");
        return false;
    }

    const String& sortedNames = pack.namesBlock.sortedNames;

    writeOk = packFileOutput->Write(&sortedNames[0], sortedNames.size());
    if (writeOk <= 0)
    {
        Logger::Error("can't write filenames block to archive");
        return false;
    }
    writeOk = packFileOutput->Write(&pack.filesDataBlock.fileTable[0],
                                    sizeOfFilesTable);

    if (writeOk <= 0)
    {
        Logger::Error("can't write filetable block to archive");
        return false;
    }

    RefPtr<File> tmpfile(File::Create(packedFileTmp, File::OPEN | File::READ));
    if (!tmpfile)
    {
        Logger::Error("can't open compressed tmp file");
        return false;
    }

    if (!CopyTmpfileToPackfile(packFileOutput, tmpfile))
    {
        return false;
    }

    tmpfile.Set(nullptr);

    return true;
}

}  // end namespace DAVA