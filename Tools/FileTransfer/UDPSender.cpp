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


#include <Base/BaseTypes.h>
#include <Base/FunctionTraits.h>
#include <Network/NetworkUtils.h>

#include "CommonTypes.h"
#include "UDPSender.h"

namespace DAVA
{

UDPSender::UDPSender(IOLoop* loop) : socket(loop)
                                   , timer(loop)
                                   , endp()
                                   , stage(STAGE_INIT)
                                   , initSent(false)
                                   , fileBuf(NULL)
                                   , fileSize(0)
                                   , curOffset(0)
                                   , curChunkSize(0)
                                   , chunkSize(1000)
{
    Memset(inbuf, 0, sizeof(inbuf));
    Memset(outbuf, 0, sizeof(outbuf));

    socket.SetCloseHandler(MakeFunction(this, &UDPSender::HandleClose));
}

UDPSender::~UDPSender()
{
    
}

bool UDPSender::Start(const Endpoint& endpoint, const char8* buffer, std::size_t length)
{
    endp     = endpoint;
    fileBuf  = buffer;
    fileSize = length;

    //IssueWaitRequest(0);
    IssueWriteRequest();
    return true;
}

int32 UDPSender::IssueReadRequest()
{
    int32 error = socket.AsyncReceive(inbuf, INBUF_SIZE, MakeFunction(this, &UDPSender::HandleReceive));
    if (error != 0)
    {
        printf("*** AsyncReceive failed: %s\n", NetworkErrorToString(error));
    }
    return error;
}

int32 UDPSender::IssueWriteRequest()
{
    int32 error = socket.AsyncSend(endp, outbuf, OUTBUF_SIZE, MakeFunction(this, &UDPSender::HandleSend));
    socket.AsyncSend(endp, inbuf, INBUF_SIZE, MakeFunction(this, &UDPSender::HandleSend));
    if (error != 0)
    {
        printf("*** AsyncSend failed: %s\n", NetworkErrorToString(error));
    }
    return error;
}

int32 UDPSender::IssueWriteRequest(const char8* buf, std::size_t length)
{
    Memcpy(outbuf, buf, length);
    int32 error = socket.AsyncSend(endp, outbuf, length, MakeFunction(this, &UDPSender::HandleSend));
    if (error != 0)
    {
        printf("*** AsyncSend failed: %s\n", NetworkErrorToString(error));
    }
    return error;
}

int32 UDPSender::IssueWriteRequest(std::size_t offset, std::size_t length)
{
    int32 error = socket.AsyncSend(endp, fileBuf + offset, length, MakeFunction(this, &UDPSender::HandleSend));
    if (error != 0)
    {
        printf("*** AsyncSend failed: %s\n", NetworkErrorToString(error));
    }
    return error;
}

int32 UDPSender::IssueWaitRequest(uint32 timeout)
{
    int32 error = timer.AsyncStartWait(timeout, MakeFunction(this, &UDPSender::HandleTimer));
    if (error != 0)
    {
        printf("*** AsyncStartWait failed: %s\n", NetworkErrorToString(error));
    }
    return error;
}

void UDPSender::Cleanup()
{
    socket.Close();
    timer.Close();
}

void UDPSender::SendInit()
{
    printf("Sending INIT\n");

    InitRequest req = {0};
    req.sign     = 0xDEADBEEF;
    req.fileSize = static_cast<uint32>(fileSize);
    IssueWriteRequest(&req);
}

void UDPSender::SendNextChunk()
{
    std::size_t nleft = fileSize - curOffset;
    curChunkSize = Min(nleft, chunkSize);

    printf("Sending: curOffset="SIZET_FMT", curChunkSize="SIZET_FMT", fileSize="SIZET_FMT"\n", curOffset
                                                                                             , curChunkSize
                                                                                             , fileSize);
    IssueWriteRequest(curOffset, curChunkSize);
}

void UDPSender::HandleClose(SocketType* socket)
{
    printf("Socket closed\n");
}

void UDPSender::HandleTimer(DeadlineTimer* timer)
{
    static int32 n = 0;
    n += 1;
    if (n > 30)
        Cleanup();
    else
        SendInit();
}

void UDPSender::HandleReceive(SocketType* socket, int32 error, std::size_t nread, void* buffer, const Endpoint& endpoint, bool partial)
{
    timer.StopAsyncWait();
    if (error != 0 || partial)
    {
        if (partial)
            printf("*** Partial packet received\n");
        else
            printf("*** Receive failed: %s\n", NetworkErrorToString(error));
        Cleanup();
        return;
    }

    if (STAGE_INIT == stage)
    {
        InitReply* rep = reinterpret_cast<InitReply*>(inbuf);
        if (sizeof(InitReply) == nread && 0xACCA10DA == rep->sign)
        {
            if (rep->status == 0)
            {
                printf("Reciever ready\n");
                stage = STAGE_FILE;
                SendNextChunk();
            }
            else
            {
                printf("Reciever not ready\n");
                IssueWaitRequest(1000);
            }
        }
        else
        {
            printf("*** Unrecognizable reply: nread="SIZET_FMT"\n", nread);
            Cleanup();
            return;
        }
    }
    else if (STAGE_FILE == stage)
    {
        FileReply* rep = reinterpret_cast<FileReply*>(inbuf);
        if (sizeof(FileReply) == nread && 0xBABAABBA == rep->sign)
        {
            if (rep->status == 0)
            {
                SendNextChunk();
            }
            else
            {
                printf("Pause request\n");
            }
        }
        else
        {
            printf("*** Unrecognizable reply: nread="SIZET_FMT"\n", nread);
            Cleanup();
            return;
        }
    }
}

void UDPSender::HandleSend(SocketType* socket, int32 error, const void* buffer)
{
    if (error != 0)
    {
        printf("*** Send failed: %s\n", NetworkErrorToString(error));
        Cleanup();
        return;
    }

    static int n = 0;

    //printf("Sent %d, size=%u, count=%u, inv=%d\n", ++n, socket->SendQueueSize(), socket->SendRequestCount(), socket->Handle()->socket == INVALID_SOCKET);
    if (n < 100)
    {
        IssueWriteRequest();
    }
    else
        Cleanup();

#if 0
    if (STAGE_INIT == stage)
    {
        printf("INIT has been sent\n");
        if (!initSent)
        {
            IssueReadRequest();
            initSent = true;
        }
        IssueWaitRequest(1000);
    }
    else if (STAGE_FILE == stage)
    {
        curOffset += curChunkSize;
        if (fileSize == curOffset)
        {
            printf("File has been sent\n");
            Cleanup();
        }
        else
        {
            //SendNextChunk();
        }
    }
#endif
}

}   // namespace DAVA
