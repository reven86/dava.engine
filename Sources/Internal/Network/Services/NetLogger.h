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


#ifndef __DAVAENGINE_NETLOGGER_H__
#define __DAVAENGINE_NETLOGGER_H__

#include <ctime>

#include <Base/Noncopyable.h>
#include <FileSystem/Logger.h>
#include <Platform/DateTime.h>
#include <Concurrency/Mutex.h>

#include <Network/NetService.h>

namespace DAVA
{
namespace Net
{

/*
 This is network logger
*/
class NetLogger : public NetService
                , public LoggerOutput
                , private Noncopyable
{
private:
    struct LogRecord
    {
        LogRecord() : timestamp(), level(), message() {}
        LogRecord(time_t tstamp, Logger::eLogLevel ll, const char8* text) : timestamp(tstamp)
                                                                          , level(ll)
                                                                          , message(text) {}

        time_t            timestamp;
        Logger::eLogLevel level;
        String            message;
    };

public:
    NetLogger(bool selfInstallFlag = true, size_t queueSize = 100);
    virtual ~NetLogger();

    void Install();
    void Uninstall();

    // IChannelListener
    void OnPacketSent(IChannel* channel, const void* buffer, size_t length) override;
    void OnPacketDelivered(IChannel* channel, uint32 packetId) override;

    // LoggerOutput
    void Output(Logger::eLogLevel ll, const char8* text) override;

    void ChannelOpen() override;

private:
    void DoOutput(Logger::eLogLevel ll, const char8* text);
    void SendNextRecord();

    bool EnqueueMessage(Logger::eLogLevel ll, const char8* message);
    bool GetFirstMessage(LogRecord& record);
    void RemoveFirstMessage();
    
    String TimestampToString(time_t timestamp) const;

private:
    bool selfInstall;
    bool isInstalled;
    size_t maxQueueSize;
    Mutex mutex;
    Deque<LogRecord> recordQueue;
};

}   // namespace Net
}   // namespace DAVA

#endif  // __DAVAENGINE_NETLOGGER_H__
