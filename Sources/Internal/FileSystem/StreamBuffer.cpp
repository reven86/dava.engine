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
#include "StreamBuffer.h"

#include "Concurrency/LockGuard.h"
#include "FileSystem/DynamicMemoryFile.h"

namespace DAVA
{
StreamBuffer::OneBuffer::OneBuffer()
{
    buffer = DynamicMemoryFile::Create(File::CREATE | File::READ | File::WRITE);
}

StreamBuffer::OneBuffer::~OneBuffer()
{
    SafeRelease(buffer);
}

uint32 StreamBuffer::OneBuffer::GetRemainSize() const
{
    return buffer->GetSize() - readPos;
}

uint32 StreamBuffer::OneBuffer::GetSize() const
{
    return buffer->GetSize();
}

uint32 StreamBuffer::OneBuffer::Write(uint8* data, uint32 len)
{
    if (0 == len || !buffer->Seek(writePos, File::SEEK_FROM_START))
    {
        return 0;
    }

    uint32 written = buffer->Write(data, len);
    writePos += written;
    return written;
}

uint32 StreamBuffer::OneBuffer::Read(uint8* data, uint32 len)
{
    if (!buffer->Seek(readPos, File::SEEK_FROM_START))
    {
        return 0;
    }

    uint32 read = buffer->Read(data, len);
    readPos += read;
    return read;
}

StreamBuffer::~StreamBuffer()
{
    Flush();
}

void StreamBuffer::Flush()
{
    LockGuard<Mutex> lock(interactionsLock);
    for (OneBuffer* page : pages)
    {
        SafeRelease(page);
    }

    pages.clear();
}

void StreamBuffer::Write(uint8* dataIn, uint32 len)
{
    LockGuard<Mutex> lock(interactionsLock);
    WriteInternal(dataIn, len);
}

uint32 StreamBuffer::Read(uint8* dataOut, uint32 len)
{
    LockGuard<Mutex> lock(interactionsLock);
    uint32 bytesRead = 0;
    uint32 readMore = len;
    while (pages.size() > 0 && readMore > 0)
    {
        uint32 readSize = ReadInternal(dataOut + bytesRead, readMore);
        readMore -= readSize;
        bytesRead += readSize;
    }

    size -= bytesRead;
    return bytesRead;
}

uint32 StreamBuffer::GetSize()
{
    return size;
}

void StreamBuffer::WriteInternal(uint8* dataIn, uint32 len)
{
    writePage = new OneBuffer();
    pages.push_back(writePage);
    uint32 written = writePage->Write(dataIn, len);
    size += written;
}

uint32 StreamBuffer::ReadInternal(uint8* dataOut, uint32 len)
{
    if (nullptr == readPage)
        readPage = pages.front();

    OneBuffer* page = readPage;
    uint32 read = page->Read(dataOut, len);
    if (readPage->GetRemainSize() == 0)
    {
        pages.remove(readPage);
        SafeRelease(readPage);
    }

    return read;
}
}
