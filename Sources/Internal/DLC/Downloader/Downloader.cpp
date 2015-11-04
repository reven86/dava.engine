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


#include "Downloader.h"
#include "DLC/Downloader/DownloadManager.h"
#include "Platform/SystemTimer.h"
#include "Concurrency/LockGuard.h"

namespace DAVA
{

Downloader::Downloader()
    : fileErrno(0)
{ }

bool Downloader::SaveData(const void *ptr, const FilePath& storePath, uint64 size)
{
    size_t written = 0;
    File *destFile = File::Create(storePath, File::OPEN | File::WRITE | File::APPEND);
    if (destFile)
    {
        DownloadManager::Instance()->ResetRetriesCount();
#if defined(__DAVAENGINE_ANDROID__) 
        uint32 posBeforeWrite = destFile->GetPos();
#endif
        written = destFile->Write(ptr, static_cast<int32>(size)); // only 32 bit write is supported

#if defined(__DAVAENGINE_ANDROID__) 
        //for Android value returned by 'Write()' is incorrect in case of full disk, that's why we calculate 'written' using 'GetPos()'
        DVASSERT(destFile->GetPos() >= posBeforeWrite);
        written = destFile->GetPos() - posBeforeWrite;
#endif
        SafeRelease(destFile);
        
        notifyProgress(written);
        
        if (written != size)
        {
            Logger::Error("[Downloader::SaveData] Cannot save data to the file");
            return false;
        }
    }
    else
    {
        Logger::Error("[Downloader::SaveData] Cannot open file to save data");
        return false;
    }

    return true;
}
    
void Downloader::SetProgressNotificator(Function<void (uint64)> progressNotifier)
{
    notifyProgress = progressNotifier;
}

void Downloader::ResetStatistics(uint64 sizeToDownload)
{
    dataToDownloadLeft = sizeToDownload;
    statistics.downloadSpeedBytesPerSec = 0;
    statistics.timeLeftSecs = static_cast<uint64>(DownloadStatistics::VALUE_UNKNOWN);
    statistics.dataCameTotalBytes = 0;
}

void Downloader::CalcStatistics(uint32 dataCame)
{
    dataToDownloadLeft -= dataCame;
    
    static uint64 curTime = SystemTimer::Instance()->AbsoluteMS();
    static uint64 prevTime = curTime;
    static uint64 timeDelta = 0;
    
    static uint64 dataSizeCame = 0;
    dataSizeCame += dataCame;
    
    curTime = SystemTimer::Instance()->AbsoluteMS();
    timeDelta += curTime - prevTime;
    prevTime = curTime;
    
    DownloadStatistics tmpStats(statistics);
    
    tmpStats.dataCameTotalBytes += dataCame;

    // update download speed 5 times per second
    if (200 <= timeDelta)
    {
        tmpStats.downloadSpeedBytesPerSec = 1000*dataSizeCame/timeDelta;
        if (0 < tmpStats.downloadSpeedBytesPerSec)
        {
            tmpStats.timeLeftSecs = static_cast<uint64>(dataToDownloadLeft / tmpStats.downloadSpeedBytesPerSec);
        }
        else
        {
            tmpStats.timeLeftSecs = static_cast<uint64>(DownloadStatistics::VALUE_UNKNOWN);
        }
        
        timeDelta = 0;
        dataSizeCame = 0;
    }

    statisticsMutex.Lock();
    statistics = tmpStats;
    statisticsMutex.Unlock();
}
    
DownloadStatistics Downloader::GetStatistics()
{
    LockGuard<Spinlock> lock(statisticsMutex);
    return statistics;
}

int32 Downloader::GetFileErrno() const
{
    return fileErrno;
}
    
}