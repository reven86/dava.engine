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
#include "FileSystem/ZipArchive.h"
#include "FileSystem/DavaArchive.h"
#include "FileSystem/File.h"
#include "Compression/LZ4Compressor.h"
#include "Compression/ZipCompressor.h"

#include <fstream>

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
static Compressor::Type GetPackingBestType(const String& fileName)
{
    return Compressor::Type::Lz4HC;
};

static bool Packing(const String& fileName,
                    Vector<FileTableEntry>& fileTable,
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

    PackFile::FilesDataBlock::Data fileData{
        startPos, packedSize, origSize, packingType
    };

    fileTable.push_back(fileData);

    return result;
}

static bool CompressFileAndWriteToOutput(const String& fileName,
                                         Vector<FileTableEntry>& fileTable,
                                         Vector<uint8>& inBuffer,
                                         Vector<uint8>& packingBuf,
                                         File* output)
{
    using namespace DAVA;

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
            Compressor::Type::None
        };

        fileTable.push_back(fileData);
        return true;
    }

    Compressor::Type compressType = GetPackingBestType(fileName);

    switch (compressType)
    {
    case Compressor::Type::Lz4:
    {
        if (!Packing(fileName, fileTable, inBuffer, packingBuf, output,
                     origSize, startPos, Compressor::Type::Lz4))
        {
            return false;
        }
    }
    break;
    case Compressor::Type::Lz4HC:
    {
        if (!Packing(fileName, fileTable, inBuffer, packingBuf, output,
                     origSize, startPos, Compressor::Type::Lz4HC))
        {
            return false;
        }
    }
    break;
    case Compressor::Type::RFC1951:
        if (!Packing(fileName, fileTable, inBuffer, packingBuf, output, origSize, startPos, Compressor::Type::RFC1951))
        {
            return false;
        }
        break;
    case Compressor::Type::None:
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
    break;
    }
    return true;
}

ResourceArchive::ResourceArchive(const FilePath& archiveName)
{
    using namespace DAVA;

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
        impl.reset(new DavaArchive(fileName));
    }
    else
    {
        impl.reset(new ZipArchive(fileName));
    }
}

ResourceArchive::~ResourceArchive()
{
    impl.reset();
}

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
                               Vector<uint8>& output) const
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

bool ResourceArchive::CreatePack(const FilePath& pacName,
                                 const Vector<String>& sortedFileNames,
                                 void (*onPackOneFile)(const FileInfo&))
{
    using namespace DAVA;

    if (!std::is_sorted(begin(sortedFileNames), end(sortedFileNames)))
    {
        Logger::Error("sortedFileNames list is not sorted!\n");
        return false;
    }

    RefPtr<File> packFileOutput(File::Create(pacName, File::CREATE | File::WRITE));

    if (!packFileOutput)
    {
        Logger::Error("can't create packfile, can't Open: %s\n", pacName.GetAbsolutePathname().c_str());
        return false;
    }

    PackFile pack;

    auto& fileTable = pack.filesDataBlock.fileTable;

    Vector<uint8> fileBuffer;
    Vector<uint8> compressedFileBuffer;
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
                      bool is_ok = CompressFileAndWriteToOutput(fileName, fileTable, fileBuffer, compressedFileBuffer, outTmpFile.get());
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
