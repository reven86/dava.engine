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
#ifndef DAVAENGINE_FILE_SYSTEM_DAVA_ARCHIVE_H
#define DAVAENGINE_FILE_SYSTEM_DAVA_ARCHIVE_H

#include "FileSystem/Private/ResourceArchivePrivate.h"

namespace DAVA
{
namespace PackFormat
{
const Array<char8, 4> FileMarker = { 'P', 'A', 'C', 'K' };

struct PackFile
{
    struct HeaderBlock
    {
        Array<char8, 4> marker;
        uint32 namesBlockSizeCompressedLZ4HC;
        uint32 namesBlockSizeOriginal;
        uint32 filesTableBlockSize;
        uint32 startNamesBlockPosition;
        uint32 startFilesDataBlockPosition;
        uint32 startPackedFilesBlockPosition;
        uint32 numFiles;
    } header;

    struct NamesBlock
    {
        String sortedNamesLz4hc; // '\0' separated all file names in pack file compressed with lz4hc
    } names;

    struct FilesDataBlock
    {
        struct Data
        {
            uint32 startPositionInPackedFilesBlock;
            uint32 compressed;
            uint32 original;
            Compressor::Type packType;
            Array<char8, 16> reserved; // do we really need it? leave for future
        };
        Vector<Data> files;
    } filesData;

    struct PackedFilesBlock
    {
        uint8* packedFiles;
    } notUsedReadDirectlyFromFile;
}; // end PackFile struct

using FileTableEntry = PackFile::FilesDataBlock::Data;

static_assert(sizeof(PackFile::HeaderBlock) == 32, "header block size changed, something bad happened!");
static_assert(sizeof(FileTableEntry) == 32, "file table entry size changed, something bad happened!");
} // end of dava_pack_private namespace

class PackArchive final : public ResourceArchiveImpl
{
public:
    explicit PackArchive(const FilePath& archiveName);

    const Vector<ResourceArchive::FileInfo>& GetFilesInfo() const override;
    const ResourceArchive::FileInfo* GetFileInfo(const String& relativeFilePath) const override;
    bool HasFile(const String& relativeFilePath) const override;
    bool LoadFile(const String& relativeFilePath, Vector<uint8>& output) const override;

private:
    mutable RefPtr<File> file;
    PackFormat::PackFile packFile;
    UnorderedMap<String, PackFormat::FileTableEntry*> mapFileData;
    Vector<ResourceArchive::FileInfo> filesInfoSortedByName;
};
} // end namespace DAVA

#endif // DAVAENGINE_FILE_SYSTEM_DAVA_ARCHIVE_H