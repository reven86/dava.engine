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

#include "FileSystem/ResourceArchivePrivate.h"

namespace dava_pack_private
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
} // end of dava_pack_private namespace

namespace DAVA
{
class DavaArchive : public ResourceArchiveImpl
{
public:
    explicit DavaArchive(const FilePath& archiveName);

    const Vector<ResourceArchive::FileInfo>& GetFilesInfo() const override;
    const ResourceArchive::FileInfo* GetFileInfo(const String& fileName) const override;
    bool HasFile(const String& fileName) const override;
    bool LoadFile(const String& fileName, Vector<uint8>& output) const override;

private:
    mutable RefPtr<File> file;
    dava_pack_private::PackFile packFile;
    UnorderedMap<String, dava_pack_private::FileTableEntry*> mapFileData;
    Vector<ResourceArchive::FileInfo> filesInfoSortedByName;
};
}