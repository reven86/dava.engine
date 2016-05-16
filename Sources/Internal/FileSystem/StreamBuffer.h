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

#ifndef DAVAENGINE_STREAM_BUFFER_H
#define DAVAENGINE_STREAM_BUFFER_H

#include "Base/BaseTypes.h"
#include "Concurrency/Mutex.h"

namespace DAVA
{
class StreamBuffer
{
public:
    ~StreamBuffer();
    void Clear();
    void Write(uint8* dataIn, uint32 len);
    uint32 Read(uint8* dataOut, uint32 len);
    uint32 GetSize();

private:
    void WriteInternal(uint8* dataIn, uint32 len);
    uint32 ReadInternal(uint8* dataOut, uint32 len);

private:
    Mutex interactionsLock;
    List<Vector<uint8>> pages;
    uint32 currentPageReadPos = 0;
    uint32 size = 0;
};
}

#endif // !DAVAENGINE_STREAM_BUFFER_H
