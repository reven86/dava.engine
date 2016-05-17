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

#include "Debug/DVAssert.h"
#include "Concurrency/LockGuard.h"

namespace DAVA
{

StreamBuffer::~StreamBuffer()
{
    Clear();
}

void StreamBuffer::Clear()
{
    LockGuard<Mutex> lock(interactionsLock);
    pages.clear();
}

void StreamBuffer::Write(uint8* dataIn, uint32 len)
{
    DVASSERT(nullptr != dataIn);
    LockGuard<Mutex> lock(interactionsLock);
    WriteInternal(dataIn, len);
}

uint32 StreamBuffer::Read(uint8* dataOut, uint32 len)
{
    DVASSERT(nullptr != dataOut);
    LockGuard<Mutex> lock(interactionsLock);
    uint32 bytesRead = 0;
    uint32 readMore = len;
    while (pages.size() > 0 && readMore > 0)
    {
        uint32 readSize = ReadInternal(dataOut + bytesRead, readMore);
        readMore -= readSize;
        bytesRead += readSize;
        currentPageReadPos += readSize;
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
    pages.emplace_back();

    pages.back().resize(len);
    Memcpy(pages.back().data(), dataIn, len);

    size += len;
}

uint32 StreamBuffer::ReadInternal(uint8* dataOut, uint32 len)
{
    if (pages.size() == 0)
    {
        return 0;
    }

    uint32 dataSize = static_cast<uint32>(pages.front().size()) - currentPageReadPos;
    uint32 sizeToRead = Min(dataSize, len);
    Memcpy(dataOut, pages.front().data() + currentPageReadPos, sizeToRead);

    if (dataSize == 0)
    {
        pages.pop_front();
        currentPageReadPos = 0;
    }

    return sizeToRead;
}
}
