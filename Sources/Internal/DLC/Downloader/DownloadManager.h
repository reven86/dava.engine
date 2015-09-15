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


#ifndef __DATA_DOWNLOAD_MANAGER_H__
#define __DATA_DOWNLOAD_MANAGER_H__

#include "Base/BaseTypes.h"
#include "Base/Singleton.h"
#include "Functional/Function.h"

#include "DownloaderCommon.h"

namespace DAVA
{

class Thread;
class Downloader;

class DownloadManager : public Singleton<DownloadManager>
{
    friend class Downloader;

public:
    using NotifyFunctor = Function<void(const uint32 &, const DownloadStatus &)>;

public:
    DownloadManager();
    virtual ~DownloadManager();

    // Downloader for further operations
    void SetDownloader(Downloader *_downloader);
    Downloader *GetDownloader();
    
    // Callback functor for tasks status reporting
    void SetNotificationCallback(NotifyFunctor callbackFn);
    NotifyFunctor GetNotificationCallback() const;

    // Checks tasks status and determine current task and handles tasks queues
    void Update();

    // Schedule download content or get content size (handles by DwonloadMode)
    uint32 Download(const String &srcUrl, const FilePath &storeToFilePath, const DownloadType downloadMode = RESUMED, const uint8 partsCount = 4, int32 timeout = 30, int32 retriesCount = 3);
    
    // Retry finished download
    void Retry(const uint32 &taskId);

    // Cancel active download
    void CancelCurrent();
    // Cancel download by ID (works for scheduled and current)
    void Cancel(const uint32 &taskId);
    // Cancell all scheduled and current downloads
    void CancelAll();

    // wait for task statis = finished
    void Wait(const uint32 &taskId);
    // wait to end of all tasks
    void WaitAll();

    bool GetCurrentId(uint32 &id);
    bool GetUrl(const uint32 &taskId, String &url);
    bool GetStorePath(const uint32 &taskId, FilePath &path);
    bool GetType(const uint32 &taskId, DownloadType &type);
    bool GetStatus(const uint32 &taskId, DownloadStatus &status);
    bool GetTotal(const uint32 &taskId, uint64 &total);
    bool GetProgress(const uint32 &taskId, uint64 &progress);
    bool GetError(const uint32 &taskId, DownloadError &error);
    bool GetFileErrno(const uint32 &taskId, int32 &fileErrno);
    DownloadStatistics GetStatistics();
    void SetDownloadSpeedLimit(const uint64 limit);

private:
    struct CallbackData
    {
        CallbackData(uint32 _id, DownloadStatus _status);

        uint32 id;
        DownloadStatus status;
    };



private:
    void SetTaskStatus(DownloadTaskDescription *task, const DownloadStatus &status);

    void StartProcessingThread();
    void StopProcessingThread();
    void ThreadFunction(BaseObject *caller, void *callerData, void *userData);
    
    void ClearQueue(Deque<DownloadTaskDescription *> &queue);
    DownloadTaskDescription *ExtractFromQueue(Deque<DownloadTaskDescription *> &queue, const uint32 &taskId);
    void PlaceToQueue(Deque<DownloadTaskDescription *> &queue, DownloadTaskDescription *task);

    DownloadTaskDescription *GetTaskForId(const uint32 &taskId);

    void Clear(const uint32 &taskId);
    void ClearAll(); 
    void ClearPending();
    void ClearDone();

    DownloadError Download();
    DownloadError TryDownload();
    void Interrupt();
    bool IsInterrupting();
    void MakeFullDownload();
    void MakeResumedDownload();
    void ResetRetriesCount();
    void OnCurrentTaskProgressChanged(uint64 progressDelta);

private:
    Thread *thisThread;
    bool isThreadStarted;

    Deque<DownloadTaskDescription *> pendingTaskQueue;
    Deque<DownloadTaskDescription *> doneTaskQueue;

    Deque<CallbackData> callbackMessagesQueue;
    Mutex callbackMutex;

    DownloadTaskDescription *currentTask;
    static Mutex currentTaskMutex;

    Downloader *downloader;
    NotifyFunctor callNotify;

    uint64 downloadedTotal;
};

}

#endif
