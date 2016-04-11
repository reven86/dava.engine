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

#include "FileSystem/Private/ZipArchive.h"
#include "FileSystem/FilePath.h"
#include <cstring>

namespace DAVA
{
ZipArchive::ZipArchive(const FilePath& fileName)
    : zipFile(fileName)
{
    // Get and print information about each file in the archive.
    uint32 count = zipFile.GetNumFiles();

    fileNames.reserve(count);
    fileInfos.reserve(count);
    for (uint32 i = 0u; i < count; i++)
    {
        String name;
        uint32 origSize = 0;
        uint32 compressedSize = 0;
        bool isDirectory = false;

        if (!zipFile.GetFileInfo(i, name, origSize, compressedSize, isDirectory))
        {
            throw std::runtime_error("failed! get file info");
        }

        if (!isDirectory)
        {
            fileNames.push_back(name);
            ResourceArchive::FileInfo info;
            info.relativeFilePath = fileNames.back().c_str();
            info.compressionType = Compressor::Type::RFC1951;
            info.compressedSize = compressedSize;
            info.originalSize = origSize;
            fileInfos.push_back(info);
        }
    }

    std::stable_sort(begin(fileInfos), end(fileInfos), [](const ResourceArchive::FileInfo& left, const ResourceArchive::FileInfo& right)
                     {
                         return CompareCaseInsensitive(left.relativeFilePath, right.relativeFilePath) < 0;
                     });
}

const Vector<ResourceArchive::FileInfo>& ZipArchive::GetFilesInfo() const
{
    return fileInfos;
}

const ResourceArchive::FileInfo* ZipArchive::GetFileInfo(const String& relativeFilePath) const
{
    auto it = std::lower_bound(begin(fileInfos), end(fileInfos), relativeFilePath, [](const ResourceArchive::FileInfo& left, const String& fileNameParam)
                               {
                                   return left.relativeFilePath < fileNameParam;
                               });

    if (it != end(fileInfos))
    {
        return &(*it);
    }
    return nullptr;
}

bool ZipArchive::HasFile(const String& relativeFilePath) const
{
    return GetFileInfo(relativeFilePath) != nullptr;
}

bool ZipArchive::LoadFile(const String& relativeFilePath, Vector<uint8>& output) const
{
    const ResourceArchive::FileInfo* info = GetFileInfo(relativeFilePath);
    if (info != nullptr)
    {
        output.resize(info->originalSize);

        if (!zipFile.LoadFile(relativeFilePath, output))
        {
            Logger::Error("can't extract file: %s into memory", relativeFilePath.c_str());
            return false;
        }
        return true;
    }
    return false;
}
} // end namespace DAVA
