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

#ifndef __DAVAENGINE_RESOURCE_ARCHIVE_H__
#define __DAVAENGINE_RESOURCE_ARCHIVE_H__

#include "Base/BaseObject.h"

namespace DAVA
{
class ResourceArchiveImpl;

class ResourceArchive : public BaseObject
{
protected:
    virtual ~ResourceArchive();

public:
    ResourceArchive();

    enum class CompressionType : uint32
    {
        None,
        Lz4,
        Lz4HC
    };

    struct FileInfo
    {
        const char* name;
        uint32 originalSize;
        uint32 compressedSize;
        CompressionType compressionType;
    };

    struct ContentAndSize
    {
        std::unique_ptr<char[]> content;
        uint32 size;
    };

    struct Rule
    {
        String fileExt;
        CompressionType compressionType;
    };

    using FileInfos = Vector<FileInfo>;

    bool Open(const FilePath& archiveName);
    const FileInfos& GetFilesInfo() const;
    const FileInfo* GetFileInfo(const String& fileName) const;
    bool HasFile(const String& fileName) const;
    bool LoadFile(const String& fileName, ContentAndSize& output) const;
    void Close();

    using Rules = Vector<Rule>;

    static bool CreatePack(const String& pacName,
                           const Vector<String>& sortedFileNames,
                           const Rules& compressionRules);

private:
    std::unique_ptr<ResourceArchiveImpl> impl;
};
};

#endif  // __DAVAENGINE_RESOURCE_ARCHIVE_H__
