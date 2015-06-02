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


#include <cstdio>

#include <Network/IOLoop.h>
#include <Network/Endpoint.h>
#include <Network/IPAddress.h>

#include "UDPReceiver.h"
#include "UDPSender.h"
#include "TimerTest.h"

using namespace DAVA;

struct CmdlineParams
{
    bool      sender;
    bool      tcp;
    IPAddress address;
    uint16    port;
    char8     filename[256];
};

void PrintParameters(const CmdlineParams* params);
bool ParseCmdline(int argc, char* argv[], CmdlineParams* params);
void PrintUsage();

void ReceiveOverUDP(IOLoop* loop, const CmdlineParams* params);
void SendOverUDP(IOLoop* loop, const CmdlineParams* params);
void TimerTestFunc(IOLoop* loop);

int main(int argc, char* argv[])
{
    CmdlineParams params = {0};
    if (!ParseCmdline(argc, argv, &params))
    {
        PrintUsage();
        return 1;
    }
    PrintParameters(&params);
    
    IOLoop loop;
    if (params.sender)
    {
        if (params.tcp)
        {
        }
        else
        {
            SendOverUDP(&loop, &params);
        }
    }
    else
    {
        if (params.tcp)
        {
        }
        else
        {
            ReceiveOverUDP(&loop, &params);
        }
    }
    return 0;
}

void ReceiveOverUDP(IOLoop* loop, const CmdlineParams* params)
{
    UDPReceiver receiver(loop);
    if (receiver.Start(params->port))
    {
        loop->Run();
    }
}

void SendOverUDP(IOLoop* loop, const CmdlineParams* params)
{
    const std::size_t BUF_LENGTH = 15000;
    char8* buf = new char8[BUF_LENGTH];
    Memset(buf, 'z', BUF_LENGTH);

    Endpoint endpoint(params->address, params->port);
    UDPSender sender(loop);

    if (sender.Start(endpoint, buf, BUF_LENGTH))
    {
        loop->Run();
    }
    delete [] buf;
}

void TimerTestFunc(IOLoop* loop)
{
    TimerTest tt(loop);
    tt.Start();
    loop->Run();
}

void PrintParameters(const CmdlineParams* params)
{
    if (params->sender)
    {
        printf("Send file '%s' to %s:%hu over %s\n", params->filename
                                                   , params->address.ToString().c_str()
                                                   , params->port
                                                   , params->tcp ? "TCP" : "UDP");
    }
    else
    {
        printf("Receive file '%s' over %s:%hu\n", params->filename
                                                , params->tcp ? "TCP" : "UDP"
                                                , params->port);
    }
}

bool ParseCmdline(int argc, char* argv[], CmdlineParams* params)
{
    if (argc < 6)
        return false;
        
    if (0 == strcmp(argv[1], "-send")) params->sender = true;
    else if (0 == strcmp(argv[1], "-receive")) params->sender = false;
    else return false;
        
    if (0 == strcmp(argv[2], "-tcp")) params->tcp = true;
    else if (0 == strcmp(argv[2], "-udp")) params->tcp = false;
    else return false;

    params->address = IPAddress::FromString(argv[3]);
    int port = atoi(argv[4]);
    if (0 < port && port < 65535) params->port = static_cast<uint16>(port);
    else return false;

#if defined(__DAVAENGINE_WIN32__) && defined(_MSC_VER)
    strncpy_s(params->filename, COUNT_OF(params->filename), argv[5], _TRUNCATE);
#else
    strncpy(params->filename, argv[5], COUNT_OF(params->filename));
    params->filename[COUNT_OF(params->filename) - 1] = '\0';
#endif
    return true;
}

void PrintUsage()
{
    static const char usage[] = "Usage:\n"
                                "filetransfer -send|receive -tcp|udp address port filename\n";
    printf("%s\n", usage);
}
