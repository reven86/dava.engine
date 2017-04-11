#pragma once
#include "DLCManager/DLCDownloader.h"
#include "Concurrency/Thread.h"
#include "Concurrency/Semaphore.h"

extern "C"
{
typedef void CURL;
typedef void CURLM;
}

namespace DAVA
{
struct DLCDownloader::Task
{
    TaskInfo info;
    TaskStatus status;
    Vector<CURL*> easyHandles;
};

class DLCDownloaderImpl : public DLCDownloader
{
public:
    DLCDownloaderImpl();
    ~DLCDownloaderImpl();

    // Schedule download content or get content size (indicated by downloadMode)
    Task* StartTask(
    const String& srcUrl,
    IWriter* dstWriter,
    TaskType taskType,
    uint64 rangeOffset = 0,
    uint64 rangeSize = 0,
    int16 partsCount = -1,
    int32 timeout = 30,
    int32 retriesCount = 3
    ) override;
    // Cancel download by ID (works for scheduled and current)
    void RemoveTask(Task* task) override;

    // wait for task status = finished
    void WaitTask(Task* task) override;

    const TaskInfo* GetTaskInfo(Task* task) override;
    TaskStatus GetTaskStatus(Task* task) override;

    void DownloadThreadFunc();

private:
    Deque<Task*> taskQueue;
    Vector<CURL*> easyHandlesAll;
    CURLM* multiHandle = nullptr;
    Thread* downloadThread = nullptr;
    Semaphore downloadSem; // to resume download thread
    Mutex mutex; // to protect access to taskQueue
};
} // end namespace DAVA