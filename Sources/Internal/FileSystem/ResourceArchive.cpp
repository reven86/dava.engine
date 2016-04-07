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
#include "FileSystem/Private/ZipArchive.h"
#include "FileSystem/Private/PackArchive.h"
#include "FileSystem/File.h"

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
ResourceArchive::ResourceArchive(const FilePath& archiveName)
{
    const String& fileName = archiveName.GetAbsolutePathname();

    ScopedPtr<File> f(File::Create(fileName, File::OPEN | File::READ));
    if (!f)
    {
        throw std::runtime_error("can't open resource archive: " + fileName);
    }
    Array<char8, 4> firstBytes;
    uint32 count = f->Read(firstBytes.data(), static_cast<uint32>(firstBytes.size()));
    if (count != firstBytes.size())
    {
        throw std::runtime_error("can't read from resource archive: " + fileName);
    }

    f.reset();

    if (PackFormat::FileMarker == firstBytes)
    {
        impl.reset(new PackArchive(fileName));
    }
    else
    {
        impl.reset(new ZipArchive(fileName));
    }
}

ResourceArchive::~ResourceArchive() = default;

ResourceArchive::FileInfo::FileInfo(const char8* relativePath_, uint32 originalSize_, uint32 compressedSize_, Compressor::Type compressionType_)
    : relativeFilePath(relativePath_)
    , originalSize(originalSize_)
    , compressedSize(compressedSize_)
    , compressionType(compressionType_)
{
}

const Vector<ResourceArchive::FileInfo>& ResourceArchive::GetFilesInfo() const
{
    return impl->GetFilesInfo();
}

bool ResourceArchive::HasFile(const String& relativeFilePath) const
{
    return impl->HasFile(relativeFilePath);
}

const ResourceArchive::FileInfo* ResourceArchive::GetFileInfo(const String& relativeFilePath) const
{
    return impl->GetFileInfo(relativeFilePath);
}

bool ResourceArchive::LoadFile(const String& relativeFilePath,
                               Vector<uint8>& output) const
{
    return impl->LoadFile(relativeFilePath, output);
}

} // end namespace DAVA
