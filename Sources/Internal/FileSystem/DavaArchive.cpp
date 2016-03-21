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

namespace DAVA
{
DavaArchive::DavaArchive(const FilePath& archiveName)
{
    using namespace DAVA;

    String fileName = archiveName.GetAbsolutePathname();

    file.Set(File::Create(fileName, File::OPEN | File::READ));
    if (!file)
    {
        throw std::runtime_error("can't Open file: " + fileName);
    }
    auto& header_block = packFile.headerBlock;
    uint32 isOk = file->Read(&header_block, sizeof(header_block));
    if (isOk <= 0)
    {
        throw std::runtime_error("can't read header from packfile: " + fileName);
    }
    if (header_block.resPackMarker != PackFileMarker)
    {
        throw std::runtime_error("incorrect marker in pack file: " + fileName);
    }
    if (PackFileMagic != header_block.magic)
    {
        throw std::runtime_error("can't read packfile incorrect magic");
    }
    if (header_block.numFiles == 0)
    {
        throw std::runtime_error("can't load packfile no files inside");
    }
    if (header_block.startFileNames != file->GetPos())
    {
        throw std::runtime_error("error in header of packfile start position for file names incorrect");
    }
    String& fileNames = packFile.namesBlock.sortedNames;
    fileNames.resize(header_block.namesBlockSize, '\0');

    isOk = file->Read(&fileNames[0], static_cast<uint32>(fileNames.size()));
    if (isOk <= 0)
    {
        throw std::runtime_error("can't read file names from packfile");
    }

    if (header_block.startFilesTable != file->GetPos())
    {
        throw std::runtime_error("can't load packfile incorrect start position of files data");
    }

    Vector<FileTableEntry>& fileTable = packFile.filesDataBlock.fileTable;
    fileTable.resize(header_block.numFiles);

    if (header_block.filesTableBlockSize / header_block.numFiles !=
        sizeof(FileTableEntry))
    {
        throw std::runtime_error("can't load packfile bad originalSize of file table");
    }

    isOk = file->Read(reinterpret_cast<char*>(&fileTable[0]),
                      header_block.filesTableBlockSize);
    if (isOk <= 0)
    {
        throw std::runtime_error("can't read files table from packfile");
    }

    if (header_block.startPackedFiles != file->GetPos())
    {
        throw std::runtime_error("can't read packfile, incorrect start position of compressed content");
    }

    filesInfoSortedByName.reserve(header_block.numFiles);

    size_t num_files =
    std::count_if(fileNames.begin(), fileNames.end(), [](const char& ch)
                  {
                      return '\0' == ch;
                  });
    if (num_files != fileTable.size())
    {
        throw std::runtime_error("number of file names not match with table");
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

bool DavaArchive::LoadFile(const String& fileName, Vector<char8>& output) const
{
    using namespace DAVA;

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
        Vector<char8> packedBuf(fileEntry.compressed);

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
        Vector<char8> packedBuf(fileEntry.compressed);

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
} // end namespace DAVA