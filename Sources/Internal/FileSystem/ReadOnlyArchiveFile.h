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

#ifndef DAVAENGINE_READ_ONLY_ARCHIVE_FILE_H__
#define DAVAENGINE_READ_ONLY_ARCHIVE_FILE_H__

#include "Base/BaseTypes.h"
#include "FileSystem/File.h"
#include "FileSystem/ResourceArchive.h"

namespace DAVA
{
class ReadOnlyArchiveFile : public File
{
protected:
    ReadOnlyArchiveFile();
    virtual ~ReadOnlyArchiveFile();

public:
    static ReadOnlyArchiveFile* Create(ResourceArchive::ContentAndSize& data,
                                       const FilePath& filePath);

    const FilePath& GetFilename() override;

    uint32 Write(const void* pointerToData, uint32 dataSize) override;
    uint32 Read(void* pointerToData, uint32 dataSize) override;
    uint32 GetPos() const override;
    uint32 GetSize() const override;
    bool Seek(int32 position, uint32 seekType) override;
    bool IsEof() const override;

private:
    ResourceArchive::ContentAndSize data_;
    uint32 pos_;
    FilePath filePath_;
};
};

#endif
