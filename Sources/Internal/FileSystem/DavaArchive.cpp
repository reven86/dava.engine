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
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL binaryzebra BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/

#include "FileSystem/DavaArchive.h"
#include "Compression/ZipCompressor.h"
#include "Compression/LZ4Compressor.h"
#include "FileSystem/File.h"
#include <cstring> // for std::strcmp

namespace DAVA
{
DavaArchive::DavaArchive(const FilePath& archiveName)
{
    using namespace dava_pack_private;

    String fileName = archiveName.GetAbsolutePathname();

    file.Set(File::Create(fileName, File::OPEN | File::READ));
    if (!file)
    {
        throw std::runtime_error("can't Open file: " + fileName);
    }
    auto& headerBlock = packFile.headerBlock;
    uint32 numBytesRead = file->Read(&headerBlock, sizeof(headerBlock));
    if (numBytesRead != sizeof(headerBlock))
    {
        throw std::runtime_error("can't read header from packfile: " + fileName);
    }
    if (headerBlock.resPackMarker != PackFileMarker)
    {
        throw std::runtime_error("incorrect marker in pack file: " + fileName);
    }
    if (PackFileMagic != headerBlock.magic)
    {
        throw std::runtime_error("can't read packfile incorrect magic");
    }
    if (headerBlock.numFiles == 0)
    {
        throw std::runtime_error("can't load packfile no files inside");
    }
    if (headerBlock.startFileNames != file->GetPos())
    {
        throw std::runtime_error("error in header of packfile start position for file names incorrect");
    }
    String& fileNames = packFile.namesBlock.sortedNames;
    fileNames.resize(headerBlock.namesBlockSize, '\0');

    numBytesRead = file->Read(&fileNames[0], static_cast<uint32>(fileNames.size()));
    if (numBytesRead != fileNames.size())
    {
        throw std::runtime_error("can't read file names from packfile");
    }

    if (headerBlock.startFilesTable != file->GetPos())
    {
        throw std::runtime_error("can't load packfile incorrect start position of files data");
    }

    Vector<FileTableEntry>& fileTable = packFile.filesDataBlock.fileTable;
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

    if (headerBlock.startPackedFiles != file->GetPos())
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
                                     return std::strcmp(first.fileName, second.fileName) <= 0;
                                 });
    if (!result)
    {
        throw std::runtime_error("file names in pakfile not sorted!");
    }
}

const Vector<ResourceArchive::FileInfo>& DavaArchive::GetFilesInfo() const
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

bool DavaArchive::LoadFile(const String& fileName, Vector<uint8>& output) const
{
    using namespace dava_pack_private;

    if (!HasFile(fileName))
    {
        Logger::Error("can't load file: %s course: not found\n",
                      fileName.c_str());
        return false;
    }

    const FileTableEntry& fileEntry = *mapFileData.find(fileName)->second;
    output.resize(fileEntry.original);

    if (!file)
    {
        Logger::Error("can't load file: %s course: no opened packfile\n", fileName.c_str());
        return false;
    }

    bool isOk = file->Seek(fileEntry.start, File::SEEK_FROM_START);
    if (!isOk)
    {
        Logger::Error("can't load file: %s course: can't find start file position in pack file", fileName.c_str());
        return false;
    }

    switch (fileEntry.packType)
    {
    case Compressor::Type::None:
    {
        uint32 readOk = file->Read(output.data(), fileEntry.original);
        if (readOk != fileEntry.original)
        {
            Logger::Error("can't load file: %s course: can't read uncompressed content\n", fileName.c_str());
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
            Logger::Error("can't load file: %s course: can't read compressed content\n", fileName.c_str());
            return false;
        }

        if (!LZ4Compressor().Uncompress(packedBuf, output))
        {
            Logger::Error("can't load file: %s  course: decompress error\n", fileName.c_str());
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
            Logger::Error("can't load file: %s course: can't read compressed content\n", fileName.c_str());
            return false;
        }

        if (!ZipCompressor().Uncompress(packedBuf, output))
        {
            Logger::Error("can't load file: %s  course: decompress error\n", fileName.c_str());
            return false;
        }
    }
    break;
    } // end switch
    return true;
}

static bool Packing(const String& fileName,
                    Vector<dava_pack_private::FileTableEntry>& fileTable,
                    Vector<uint8>& inBuffer,
                    Vector<uint8>& packingBuf,
                    File* output,
                    uint32 origSize,
                    uint32 startPos,
                    Compressor::Type packingType)
{
    using namespace DAVA;

    bool result = false;

    if (packingType == Compressor::Type::Lz4HC)
    {
        result = LZ4HCCompressor().Compress(inBuffer, packingBuf);
    }
    else if (packingType == Compressor::Type::Lz4)
    {
        result = LZ4Compressor().Compress(inBuffer, packingBuf);
    }
    else if (packingType == Compressor::Type::RFC1951)
    {
        result = ZipCompressor().Compress(inBuffer, packingBuf);
    }

    if (!result)
    {
        return false;
    }

    uint32_t packedSize = static_cast<uint32>(packingBuf.size());

    // if packed size worse then raw leave raw bytes
    uint32 writeOk = 0;
    if (packedSize >= origSize)
    {
        packedSize = origSize;
        packingType = Compressor::Type::None;
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

    dava_pack_private::PackFile::FilesDataBlock::Data fileData{
        startPos, packedSize, origSize, packingType
    };

    fileTable.push_back(fileData);

    return result;
}

static bool CompressFileAndWriteToOutput(const ResourceArchive::FileInfo& fInfo,
                                         const FilePath& baseFolder,
                                         Vector<dava_pack_private::FileTableEntry>& fileTable,
                                         Vector<uint8>& inBuffer,
                                         Vector<uint8>& packingBuf,
                                         File* output)
{
    using namespace dava_pack_private;

    String fullPath(baseFolder.GetAbsolutePathname() + fInfo.fileName);

    RefPtr<File> inFile(File::Create(fullPath, File::OPEN | File::READ));

    if (!inFile)
    {
        Logger::Error("can't read input file: %s\n", fInfo.fileName);
        return false;
    }

    if (!inFile->Seek(0, File::SEEK_FROM_END))
    {
        Logger::Error("can't seek inside file: %s\n", fInfo.fileName);
        return false;
    }
    uint32 origSize = inFile->GetPos();
    if (!inFile->Seek(0, File::SEEK_FROM_START))
    {
        Logger::Error("can't seek inside file: %s\n", fInfo.fileName);
        return false;
    }

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
            Logger::Error("can't read input file: %s\n", fInfo.fileName);
            return false;
        }
    }
    else
    {
        PackFile::FilesDataBlock::Data fileData{
            startPos,
            origSize,
            origSize,
            Compressor::Type::None
        };

        fileTable.push_back(fileData);
        return true;
    }

    Compressor::Type compressType = fInfo.compressionType;

    if (fInfo.compressionType == Compressor::Type::None)
    {
        PackFile::FilesDataBlock::Data fileData{
            startPos,
            origSize,
            origSize,
            Compressor::Type::None
        };

        fileTable.push_back(fileData);
        uint32 writeOk = output->Write(&inBuffer[0], origSize);
        if (writeOk <= 0)
        {
            Logger::Error("can't write into tmp archive file");
            return false;
        }
    }
    else
    {
        if (!Packing(fInfo.fileName, fileTable, inBuffer, packingBuf, output,
                     origSize, startPos, fInfo.compressionType))
        {
            return false;
        }
    }
    return true;
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
            Logger::Error("can't write part of tmp archive file into output packfile\n");
            return false;
        }
    }

    if (!packedFile->IsEof())
    {
        Logger::Error("can't read full content of tmp compressed output file\n");
        return false;
    }
    return true;
}

bool DavaArchive::Create(const FilePath& archiveName,
                         const FilePath& baseFolder,
                         Vector<ResourceArchive::FileInfo>& infos,
                         void (*onPackOneFile)(const ResourceArchive::FileInfo&))
{
    using namespace dava_pack_private;

    std::stable_sort(begin(infos), end(infos), [](const ResourceArchive::FileInfo& left, const ResourceArchive::FileInfo& right) -> bool
                     {
                         return std::strcmp(left.fileName, right.fileName) <= 0;
                     });

    RefPtr<File> packFileOutput(File::Create(archiveName, File::CREATE | File::WRITE));

    if (!packFileOutput)
    {
        Logger::Error("can't create packfile, can't Open: %s\n", archiveName.GetAbsolutePathname().c_str());
        return false;
    }

    PackFile pack;

    auto& fileTable = pack.filesDataBlock.fileTable;

    Vector<uint8> fileBuffer;
    Vector<uint8> compressedFileBuffer;
    UnorderedSet<String> skippedFiles;

    auto packedFileTmp = archiveName.GetAbsolutePathname() + "_tmp_compressed_files.bin";

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

    pack.filesDataBlock.fileTable.reserve(infos.size());

    std::for_each(begin(infos), end(infos),
                  [&](const ResourceArchive::FileInfo& fileInfo)
                  {
                      bool is_ok = CompressFileAndWriteToOutput(fileInfo, baseFolder, fileTable, fileBuffer, compressedFileBuffer, outTmpFile.get());
                      if (!is_ok)
                      {
                          Logger::Info("can't pack file: %s, skip it\n", fileInfo.fileName);
                          skippedFiles.insert(fileInfo.fileName);
                      }
                      else if (onPackOneFile != nullptr)
                      {
                          FileTableEntry& last = fileTable.back();
                          ResourceArchive::FileInfo info = { fileInfo.fileName,
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
    headerBlock.numFiles = static_cast<uint32>(infos.size() - skippedFiles.size());

    StringStream ss;
    std::for_each(begin(infos), end(infos),
                  [&skippedFiles, &ss](const ResourceArchive::FileInfo& fInfo)
                  {
                      if (skippedFiles.find(fInfo.fileName) == skippedFiles.end())
                      {
                          ss << fInfo.fileName << '\0';
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

    if (fileTable.empty())
    {
        Logger::Error("no input files for dava pack");
        return false;
    }

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
