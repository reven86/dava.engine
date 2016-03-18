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
#include "FileSystem/File.h"

#include <fstream>

#include <lz4/lz4.h>
#include <lz4/lz4hc.h>
#include <miniz/miniz.c>

//     +---------------+       +-------------------+
//     |ResourceArchive+-------+ResourceArchiveImpl|
//     +---------------+       +-------------------+
//                          +----^   ^------+
//                          |               |
//                          |               |
//                          |               |
//                    +-----+-----+   +-----+----+
//                    |DavaArchive|   |ZipArchive|
//                    +-----------+   +----------+

namespace DAVA
{
class ResourceArchiveImpl
{
public:
    virtual ~ResourceArchiveImpl() = default;

    virtual bool Open(const FilePath& archiveName) = 0;
    virtual const ResourceArchive::FileInfos& GetFilesInfo() const = 0;
    virtual const ResourceArchive::FileInfo* GetFileInfo(const String& fileName) const = 0;
    virtual bool HasFile(const String& fileName) const = 0;
    virtual bool LoadFile(const String& fileName, Vector<char8>& output) const = 0;
};
} // end namespace DAVA

namespace resource_archive_details
{
using namespace DAVA;
const Array<char8, 4> PackFileMarker = { 'P', 'A', 'C', 'K' };
const uint32 PackFileMagic = 20150817;

struct PackFile
{
    struct HeaderBlock
    {
        Array<char8, 4> resPackMarker;
        uint32 magic;
        uint32 namesBlockSize;
        uint32 filesTableBlockSize;
        uint32 startFileNames;
        uint32 startFilesTable;
        uint32 startPackedFiles;
        uint32 numFiles;
    } headerBlock;

    struct NamesBlock
    {
        String sortedNames;
    } namesBlock;

    struct FilesDataBlock
    {
        struct Data
        {
            uint32 start;
            uint32 compressed;
            uint32 original;
            Compressor::Type packType;
            Array<char8, 16> reserved;
        };
        Vector<Data> fileTable;
    } filesDataBlock;

    struct PackedFilesBlock
    {
        uint8* packedFiles;
    } notUsedReadDirectlyFromFile;
}; // end PackFile struct

using FileTableEntry = PackFile::FilesDataBlock::Data;

static_assert(sizeof(PackFile::HeaderBlock) == 32, "fix compiler padding");
static_assert(sizeof(FileTableEntry) == 32, "fix compiler padding");

class DavaArchive : public ResourceArchiveImpl
{
public:
    bool Open(const FilePath& archiveName) override;
    const ResourceArchive::FileInfos& GetFilesInfo() const override;
    const ResourceArchive::FileInfo* GetFileInfo(const String& fileName) const override;
    bool HasFile(const String& fileName) const override;
    bool LoadFile(const String& fileName, ResourceArchive::ContentAndSize& output) const override;

private:
    mutable RefPtr<File> file;
    PackFile packFile;
    UnorderedMap<String, FileTableEntry*> mapFileData;
    ResourceArchive::FileInfos filesInfoSortedByName;
};

class ZipArchive : public ResourceArchiveImpl
{
public:
    ZipArchive();
    ~ZipArchive() override;

    bool Open(const FilePath& archiveName) override;
    const ResourceArchive::FileInfos& GetFilesInfo() const override;
    const ResourceArchive::FileInfo* GetFileInfo(const String& fileName) const override;
    bool HasFile(const String& fileName) const override;
    bool LoadFile(const String& fileName, ResourceArchive::ContentAndSize& output) const override;
private:
    mz_zip_archive zipArchive;
    ResourceArchive::FileInfos fileInfos;
    Vector<String> fileNames;
};

} // end of resource_archive_details namespace

namespace DAVA
{
using namespace resource_archive_details;

static ResourceArchive::Type GetPackingBestType(const String& fileName, const Vector<ResourceArchive::Rule>& rules)
{
    ResourceArchive::Type result = ResourceArchive::Type::Lz4HC;

    for (const auto& rule : rules)
    {
        if (fileName.rfind(rule.fileExt, 0) != String::npos)
        {
            result = rule.compressionType;
            break;
        }
    }

    std::ifstream in(fileName, std::ios::ate | std::ios::binary);
    if (in.tellg() <= 128) // TODO make it better in rule
    {
        result = ResourceArchive::Type::None;
    }

    return result;
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
    using namespace resource_archive_details;

    int sizeBound = LZ4_compressBound(origSize);
    if (packingType == PackingType::RFC1951)
    {
        sizeBound = static_cast<int>(mz_compressBound(origSize));
    }
    packingBuf.resize(sizeBound);

    int packResult = packFunction(&inBuffer[0], &packingBuf[0], origSize);
    if (packResult <= 0)
    {
        Logger::Error("can't compress lz4 file: %s\n", fileName.c_str());
        return false;
    }

    uint32_t packedSize = static_cast<uint32>(packResult);

    // if packed size worse then raw leave raw bytes
    uint32 writeOk = 0;
    if (packedSize >= origSize)
    {
        packedSize = origSize;
        packingType = PackingType::None;
        writeOk = output->Write(&inBuffer[0], origSize);
    }
    else
    {
        writeOk = output->Write(&packingBuf[0], packedSize);
    }

    if (writeOk == 0)
    {
        Logger::Error("can't write into tmp archive file");
        return false;
    }

    PackFile::FilesDataBlock::Data fileData{
        startPos, packedSize, origSize, packingType
    };

    fileTable.push_back(fileData);

    return true;
}

static int Deflate—ompress(const char* source, char* dest, int inputSize)
{
    uLong destLen = inputSize;
    int result = compress(reinterpret_cast<uint8*>(dest), &destLen, reinterpret_cast<const uint8*>(source), inputSize);
    if (result != Z_OK)
    {
        Logger::Error("can't compress buffer");
        return 0;
    }
    return static_cast<int>(destLen);
}

static bool CompressFileAndWriteToOutput(const String& fileName, const Vector<ResourceArchive::Rule>& packingRules,
                                         Vector<resource_archive_details::FileTableEntry>& fileTable,
                                         Vector<char>& inBuffer,
                                         Vector<char>& packingBuf,
                                         File* output)
{
    using namespace resource_archive_details;

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

    PackingType compressionType = GetPackingBestType(fileName, packingRules);

    uint32 startPos{ 0 };
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
            ResourceArchive::Type::None
        };

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
    case PackingType::RFC1951:
        if (!Packing(fileName, fileTable, inBuffer, packingBuf, output, origSize, startPos, Deflate—ompress, PackingType::RFC1951))
        {
            return false;
        }
        break;
    case PackingType::None:
    {
        PackFile::FilesDataBlock::Data fileData{
            startPos,
            origSize,
            origSize,
            ResourceArchive::Type::None
        };

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

ResourceArchive::ResourceArchive(const FilePath& archiveName)
{
    using namespace resource_archive_details;

    const String& fileName = archiveName.GetAbsolutePathname();

    ScopedPtr<File> f(File::Create(fileName, File::OPEN | File::READ));
    if (!f)
    {
        throw std::runtime_error("can't open resource archive: " + fileName);
    }
    Array<char8, 4> firstBytes;
    unsigned count = f->Read(firstBytes.data(), firstBytes.size());
    if (count != firstBytes.size())
    {
        throw std::runtime_error("can't read from resource archive: " + fileName);
    }

    f.reset();

    if (PackFileMarker == firstBytes)
    {
        impl.reset(new DavaArchive());
    }
    else
    {
        impl.reset(new ZipArchive());
    }

    return impl->Open(fileName);
}

ResourceArchive::~ResourceArchive()
{
    impl.reset();
}

bool ResourceArchive::Open(

const Vector<ResourceArchive::FileInfo>& ResourceArchive::GetFilesInfo() const
{
    DVASSERT(impl != nullptr);
    return impl->GetFilesInfo();
}

bool ResourceArchive::HasFile(const String& fileName) const
{
    DVASSERT(impl != nullptr);
    return impl->HasFile(fileName);
}

const ResourceArchive::FileInfo* ResourceArchive::GetFileInfo(const String& fileName) const
{
    DVASSERT(impl != nullptr);
    return impl->GetFileInfo(fileName);
}

bool ResourceArchive::LoadFile(const String& fileName,
                               Vector<char8>& output) const
{
    DVASSERT(impl != nullptr);
    return impl->LoadFile(fileName, output);
}

static bool CopyTmpfileToPackfile(RefPtr<File> packFileOutput,
                                  RefPtr<File> packedFile)
{
    Array<char, 4096> copyBuf;
    uint32 lastRead = packedFile->Read(&copyBuf[0], static_cast<uint32>(copyBuf.size()));
    while (lastRead == copyBuf.size())
    {
        uint32 lastWrite = packFileOutput->Write(&copyBuf[0], static_cast<uint32>(copyBuf.size()));
        if (lastWrite != copyBuf.size())
        {
            Logger::Error("can't write part of tmp archive file into output packfile\n");
            return false;
        }
        lastRead = packedFile->Read(&copyBuf[0], static_cast<uint32>(copyBuf.size()));
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
                                 const Rules& compressionRules,
                                 void (*onPackOneFile)(const FileInfo&))
{
    using namespace resource_archive_details;

    bool result = false;

    if (!std::is_sorted(sortedFileNames.begin(), sortedFileNames.end()))
    {
        Logger::Error("sortedFileNames list is not sorted!\n");
        return false;
    }

    RefPtr<File> packFileOutput(File::Create(pacName, File::CREATE | File::WRITE));

    if (!packFileOutput)
    {
        Logger::Error("can't create packfile, can't Open: %s\n", pacName.c_str());
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
        SafeRelease(f);
        if (0 != std::remove(name.c_str()))
        {
            Logger::Error("can't delete tmp file: %s", name.c_str());
        }

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
                      bool is_ok = CompressFileAndWriteToOutput(fileName, compressionRules, fileTable, fileBuffer, compressedFileBuffer, outTmpFile.get());
                      if (!is_ok)
                      {
                          Logger::Info("can't pack file: %s, skip it\n", fileName.c_str());
                          skippedFiles.insert(fileName);
                      }
                      else if (onPackOneFile != nullptr)
                      {
                          FileTableEntry& last = fileTable.back();
                          FileInfo info = { fileName.c_str(),
                                            last.original,
                                            last.compressed,
                                            last.packType };
                          onPackOneFile(info);
                      }
                  });
    outTmpFile->Flush();

    PackFile::HeaderBlock& headerBlock = pack.headerBlock;
    headerBlock.resPackMarker = PackFileMarker;
    headerBlock.magic = PackFileMagic;
    headerBlock.numFiles = static_cast<uint32>(sortedFileNames.size() - skippedFiles.size());

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

    headerBlock.namesBlockSize = static_cast<uint32>(stringsFileNamesBlock.sortedNames.size());

    headerBlock.startFileNames = sizeof(PackFile::HeaderBlock);

    headerBlock.startFilesTable = headerBlock.startFileNames + headerBlock.namesBlockSize;

    uint32 sizeOfFilesTable = static_cast<uint32>(pack.filesDataBlock.fileTable.size() *
                                                  sizeof(pack.filesDataBlock.fileTable[0]));

    headerBlock.filesTableBlockSize = sizeOfFilesTable;
    headerBlock.startPackedFiles = headerBlock.startFilesTable + sizeOfFilesTable;

    uint32 delta = headerBlock.startPackedFiles;

    for (auto& fileData : fileTable)
    {
        fileData.start += delta;
    }

    uint32 writeOk = packFileOutput->Write(&pack.headerBlock, sizeof(pack.headerBlock));
    if (writeOk != sizeof(pack.headerBlock))
    {
        Logger::Error("can't write header block to archive");
        return false;
    }

    const String& sortedNames = pack.namesBlock.sortedNames;

    writeOk = packFileOutput->Write(sortedNames.data(), static_cast<uint32>(sortedNames.size()));
    if (writeOk != sortedNames.size())
    {
        Logger::Error("can't write filenames block to archive");
        return false;
    }
    writeOk = packFileOutput->Write(&pack.filesDataBlock.fileTable[0],
                                    sizeOfFilesTable);

    if (writeOk != sizeOfFilesTable)
    {
        Logger::Error("can't write file table block to archive");
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

} // end namespace DAVA

namespace resource_archive_details
{
ZipArchive::ZipArchive()
{
    std::memset(&zipArchive, 0, sizeof(zipArchive));
}

ZipArchive::~ZipArchive()
{
    mz_zip_reader_end(&zipArchive);
}

bool ZipArchive::Open(const FilePath& archiveName)
{
    String fileName = archiveName.GetAbsolutePathname();

    std::memset(&zipArchive, 0, sizeof(zipArchive));
    mz_bool status = mz_zip_reader_init_file(&zipArchive, fileName.c_str(), 0);
    if (!status)
    {
        Logger::Error("mz_zip_reader_init_file() failed!\n");
        return false;
    }
    // Get and print information about each file in the archive.
    unsigned count = mz_zip_reader_get_num_files(&zipArchive);
    fileNames.clear();
    fileInfos.clear();
    fileNames.reserve(count);
    fileInfos.reserve(count);
    for (unsigned i = 0u; i < count; i++)
    {
        mz_zip_archive_file_stat fileStat;
        if (!mz_zip_reader_file_stat(&zipArchive, i, &fileStat))
        {
            Logger::Error("mz_zip_reader_file_stat() failed!\n");
            mz_zip_reader_end(&zipArchive);
            return false;
        }

        if (mz_zip_reader_is_file_a_directory(&zipArchive, i) == 0)
        {
            fileNames.push_back(fileStat.m_filename);
            ResourceArchive::FileInfo info;
            info.fileName = fileNames.back().c_str();
            info.compressionType = ResourceArchive::Type::RFC1951;
            info.compressedSize = static_cast<uint32>(fileStat.m_comp_size);
            info.originalSize = static_cast<uint32>(fileStat.m_uncomp_size);
            fileInfos.push_back(info);
        }
    }
    std::stable_sort(begin(fileInfos), end(fileInfos), [](const ResourceArchive::FileInfo& left, const ResourceArchive::FileInfo& right)
                     {
                         return std::strcmp(left.fileName, right.fileName) < 0;
                     });
    return true;
}

const ResourceArchive::FileInfos& ZipArchive::GetFilesInfo() const
{
    return fileInfos;
}

const ResourceArchive::FileInfo* ZipArchive::GetFileInfo(const String& fileName) const
{
    auto it = std::lower_bound(begin(fileInfos), end(fileInfos), fileName, [](const ResourceArchive::FileInfo& left, const String& fileNameParam)
                               {
                                   return left.fileName < fileNameParam;
                               });

    if (it != end(fileInfos))
    {
        return &(*it);
    }
    return nullptr;
}

bool ZipArchive::HasFile(const String& fileName) const
{
    return GetFileInfo(fileName) != nullptr;
}

bool ZipArchive::LoadFile(const String& fileName, ResourceArchive::ContentAndSize& output) const
{
    const ResourceArchive::FileInfo* info = GetFileInfo(fileName);
    if (info)
    {
        char* buf = new char[info->originalSize];

        output.content.reset(buf);
        output.size = info->originalSize;

        mz_zip_archive* p_zip = const_cast<mz_zip_archive*>(&zipArchive);

        mz_bool result = mz_zip_reader_extract_file_to_mem(p_zip, fileName.c_str(), buf, info->originalSize, 0);
        if (result == 0)
        {
            Logger::Error("can't extract file: %s into memory", fileName.c_str());
            return false;
        }
        return true;
    }
    return false;
}

DavaArchive::DavaArchive(const FilePath& archiveName)
{
    using namespace resource_archive_details;

    String fileName = archiveName.GetAbsolutePathname();

    file.Set(File::Create(fileName, File::OPEN | File::READ));
    if (!file)
    {
        Logger::Error("can't Open file: %s", fileName.c_str());
        return false;
    }
    auto& header_block = packFile.headerBlock;
    uint32 isOk = file->Read(&header_block, sizeof(header_block));
    if (isOk <= 0)
    {
        Logger::Error("can't read header from packfile: %s",
                      fileName.c_str());
        return false;
    }
    if (header_block.resPackMarker != PackFileMarker)
    {
        Logger::Error("incorrect marker in pack file: %s",
                      fileName.c_str());
        return false;
    }
    if (PackFileMagic != header_block.magic)
    {
        Logger::Error("can't read packfile incorrect magic: 0x%X",
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
        Logger::Error("error in header of packfile start position for file names incorrect");
        return false;
    }
    String& fileNames = packFile.namesBlock.sortedNames;
    fileNames.resize(header_block.namesBlockSize, '\0');

    isOk = file->Read(&fileNames[0], static_cast<uint32>(fileNames.size()));
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
        Logger::Error("can't read packfile, incorrect start position of compressed content");
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
    size_t fileNameIndex{ 0 };

    std::for_each(begin(fileTable), end(fileTable), [&](FileTableEntry& fileEntry)
                  {
                      const char* fileNameLoc = &fileNames[fileNameIndex];
                      mapFileData.emplace(fileName, &fileEntry);

                      filesInfoSortedByName.push_back(
                      ResourceArchive::FileInfo{ fileNameLoc,
                                                 fileEntry.original,
                                                 fileEntry.compressed,
                                                 fileEntry.packType });

                      fileNameIndex = fileNames.find('\0', fileNameIndex + 1);
                      ++fileNameIndex;
                  });

    bool result = std::is_sorted(filesInfoSortedByName.begin(),
                                 filesInfoSortedByName.end(),
                                 [](const ResourceArchive::FileInfo& first,
                                    const ResourceArchive::FileInfo& second)
                                 {
                                     return first.fileName < second.fileName;
                                 });
    if (!result)
    {
        Logger::Error("file names in pakfile not sorted!");
        return false;
    }
    return true;
}

const ResourceArchive::FileInfos& DavaArchive::GetFilesInfo() const
{
    return filesInfoSortedByName;
}

const ResourceArchive::FileInfo* DavaArchive::GetFileInfo(const String& fileName) const
{
    const ResourceArchive::FileInfo* result = nullptr;
    const auto it = std::lower_bound(
    filesInfoSortedByName.begin(), filesInfoSortedByName.end(),
    fileName, [fileName](const ResourceArchive::FileInfo& info,
                         const String& name)
    {
        return info.fileName < name;
    });
    if (it != filesInfoSortedByName.end())
    {
        result = &(*it);
    }
    return result;
}

bool DavaArchive::HasFile(const String& fileName) const
{
    auto iterator = mapFileData.find(fileName);
    return iterator != mapFileData.end();
}

bool DavaArchive::LoadFile(const String& fileName, ResourceArchive::ContentAndSize& output) const
{
    using namespace resource_archive_details;

    if (!HasFile(fileName))
    {
        Logger::Error("can't load file: %s course: not found\n",
                      fileName.c_str());
        return false;
    }

    const FileTableEntry& fileEntry = *mapFileData.find(fileName)->second;
    output.size = fileEntry.original;

    if (!file)
    {
        Logger::Error("can't load file: %s course: no opened packfile\n", fileName.c_str());
        return false;
    }

    bool isOk = file->Seek(fileEntry.start, File::SEEK_FROM_START);
    if (!isOk)
    {
        Logger::Error(
        "can't load file: %s course: can't find start file "
        "position in pack file\n",
        fileName.c_str());
        return false;
    }

    switch (fileEntry.packType)
    {
    case ResourceArchive::Type::None:
    {
        output.content.reset(new char[fileEntry.compressed]);
        uint32 readOk =
        file->Read(output.content.get(), fileEntry.compressed);
        if (readOk != fileEntry.compressed)
        {
            Logger::Error("can't load file: %s course: can't read uncompressed content\n", fileName.c_str());
            return false;
        }
    }
    break;
    case ResourceArchive::Type::Lz4:
    case ResourceArchive::Type::Lz4HC:
    {
        std::unique_ptr<char8[]> packedBuf(new char8[fileEntry.compressed]);

        uint32 readOk =
        file->Read(packedBuf.get(), fileEntry.compressed);
        if (readOk != fileEntry.compressed)
        {
            Logger::Error("can't load file: %s course: can't read compressed content\n", fileName.c_str());
            return false;
        }

        output.content.reset(new char[fileEntry.original]);

        int decompressResult = LZ4_decompress_fast(packedBuf.get(), output.content.get(), output.size);
        if (decompressResult < 0)
        {
            Logger::Error("can't load file: %s  course: decompress error\n", fileName.c_str());
            return false;
        }
    }
    break;
    case ResourceArchive::Type::RFC1951:
        std::unique_ptr<uint8[]> packedBuf(new uint8[fileEntry.compressed]);

        uint32 readOk =
        file->Read(packedBuf.get(), fileEntry.compressed);
        if (readOk != fileEntry.compressed)
        {
            Logger::Error("can't load file: %s course: can't read compressed content\n", fileName.c_str());
            return false;
        }

        output.content.reset(new char8[fileEntry.original]);

        uLong uncompressedSize = fileEntry.original;

        int decompressResult = uncompress(reinterpret_cast<uint8*>(output.content.get()), &uncompressedSize, packedBuf.get(), fileEntry.compressed);
        if (decompressResult != Z_OK)
        {
            Logger::Error("can't load file: %s  course: decompress error\n", fileName.c_str());
            return false;
        }
    }
    return true;
}
} // end namespace resource_archive_details
