#pragma once

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
    using NotifyFunctor = Function<void(const uint32&, const DownloadStatus&)>;

    DownloadManager() = default;
    virtual ~DownloadManager();

    // Downloader for further operations
    void SetDownloader(Downloader* _downloader);
    Downloader* GetDownloader();

    // Callback functor for tasks status reporting
    void SetNotificationCallback(NotifyFunctor callbackFn);
    NotifyFunctor GetNotificationCallback() const;

    // Checks tasks status and determine current task and handles tasks queues
    void Update();

    // Schedule download content or get content size (handles by DwonloadMode)
    uint32 Download(const String& srcUrl, const FilePath& storeToFilePath, const DownloadType downloadMode = RESUMED, const int16 partsCount = -1, int32 timeout = 30, int32 retriesCount = 3);

    // Retry finished download
    void Retry(const uint32& taskId);

    // Cancel active download
    void CancelCurrent();
    // Cancel download by ID (works for scheduled and current)
    void Cancel(const uint32& taskId);
    // Cancell all scheduled and current downloads
    void CancelAll();

    // wait for task statis = finished
    void Wait(const uint32& taskId);
    // wait to end of all tasks
    void WaitAll();

    bool GetCurrentId(uint32& id);
    bool GetUrl(const uint32& taskId, String& url);
    bool GetStorePath(const uint32& taskId, FilePath& path);
    bool GetType(const uint32& taskId, DownloadType& type);
    bool GetStatus(const uint32& taskId, DownloadStatus& status);
    bool GetTotal(const uint32& taskId, uint64& total);
    bool GetProgress(const uint32& taskId, uint64& progress);
    bool GetError(const uint32& taskId, DownloadError& error);
    bool GetFileErrno(const uint32& taskId, int32& fileErrno);
    DownloadStatistics GetStatistics();
    void SetDownloadSpeedLimit(uint64 limit);
    void SetPreferredDownloadThreadsCount(uint8 count);
    void ResetPreferredDownloadThreadsCount();

private:
    struct CallbackData
    {
        CallbackData(uint32 _id, DownloadStatus _status);

        uint32 id;
        DownloadStatus status;
    };

    void SetTaskStatus(DownloadTaskDescription* task, const DownloadStatus& status);

    void StartProcessingThread();
    void StopProcessingThread();
    void ThreadFunction(BaseObject* caller, void* callerData, void* userData);

    void ClearQueue(Deque<DownloadTaskDescription*>& queue);
    DownloadTaskDescription* ExtractFromQueue(Deque<DownloadTaskDescription*>& queue, const uint32& taskId);
    void PlaceToQueue(Deque<DownloadTaskDescription*>& queue, DownloadTaskDescription* task);

    DownloadTaskDescription* GetTaskForId(const uint32& taskId);

    void Clear(const uint32& taskId);
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

    Thread* thisThread = nullptr;
    bool isThreadStarted = false;

    Deque<DownloadTaskDescription*> pendingTaskQueue;
    Deque<DownloadTaskDescription*> doneTaskQueue;

    Deque<CallbackData> callbackMessagesQueue;
    Mutex callbackMutex;

    DownloadTaskDescription* currentTask = nullptr;
    static Mutex currentTaskMutex;

    Downloader* downloader = nullptr;
    const uint8 defaultDownloadThreadsCount = 4;
    uint8 preferredDownloadThreadsCount = defaultDownloadThreadsCount;
    NotifyFunctor callNotify;

    uint64 downloadedTotal = 0;
};
}
