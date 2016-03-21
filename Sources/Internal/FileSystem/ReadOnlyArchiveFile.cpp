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

#include "FileSystem/ReadOnlyArchiveFile.h"

using namespace DAVA;

ReadOnlyArchiveFile::ReadOnlyArchiveFile()
    : pos_(0)
{
}

ReadOnlyArchiveFile::~ReadOnlyArchiveFile()
{
}

ReadOnlyArchiveFile* ReadOnlyArchiveFile::Create(const FilePath& filePath, Vector<char8>&& data)
{
    auto file = new ReadOnlyArchiveFile();
    file->filePath_ = filePath;
    file->data_ = std::move(data);
    return file;
}

const FilePath& ReadOnlyArchiveFile::GetFilename()
{
    return filePath_;
}

uint32 ReadOnlyArchiveFile::Write(const void* pointerToData, uint32 dataSize)
{
    DVASSERT(false && "not supported");
    return 0;
}

uint32 ReadOnlyArchiveFile::Read(void* pointerToData, uint32 dataSize)
{
    if (pos_ + dataSize <= data_.size())
    {
        Memcpy(pointerToData, data_.data() + pos_, dataSize);
        pos_ += dataSize;
        return dataSize;
    }
    if (!IsEof())
    {
        uint32 last = static_cast<uint32>(data_.size() - pos_);
        Memcpy(pointerToData, data_.data() + pos_, last);
        pos_ += last;
        return last;
    }
    return 0;
}

uint32 ReadOnlyArchiveFile::GetPos() const
{
    return pos_;
}

uint32 ReadOnlyArchiveFile::GetSize() const
{
    return static_cast<uint32>(data_.size());
}

bool ReadOnlyArchiveFile::Seek(int32 position, uint32 seekType)
{
    switch (seekType)
    {
    case SEEK_FROM_START:
    {
        if (static_cast<uint32>(position) > data_.size())
        {
            return false;
        }
        pos_ = static_cast<uint32>(position);
        return true;
    }
    case SEEK_FROM_END:
    {
        int32 newPos = static_cast<int32>(data_.size()) + position;
        if (static_cast<uint32>(newPos) > data_.size())
        {
            return false;
        }
        pos_ = static_cast<uint32>(newPos);
        return true;
    }
    case SEEK_FROM_CURRENT:
    {
        uint32 newPos = pos_ + static_cast<uint32>(position);
        if (newPos > data_.size())
        {
            return false;
        }
        pos_ = newPos;
        return true;
    }
    }
    return false;
}

bool ReadOnlyArchiveFile::IsEof() const
{
    return pos_ == data_.size();
}
