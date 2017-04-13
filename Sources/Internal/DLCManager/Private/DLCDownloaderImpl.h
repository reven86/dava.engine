#pragma once
#include "DLCManager/DLCDownloader.h"
#include "Concurrency/Thread.h"
#include "Concurrency/Semaphore.h"
#include "Concurrency/Atomic.h"

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
    Set<CURL*> easyHandles;
    std::unique_ptr<IWriter> defaultWriter;
};

class DLCDownloaderImpl : public DLCDownloader
{
public:
    DLCDownloaderImpl();
    ~DLCDownloaderImpl();

    DLCDownloaderImpl(const DLCDownloaderImpl&) = delete;
    DLCDownloaderImpl(DLCDownloaderImpl&&) = delete;

    // Schedule download content or get content size (indicated by downloadMode)
    Task* StartTask(const String& srcUrl,
                    const String& dsrPath,
                    TaskType taskType,
                    IWriter* dstWriter = nullptr,
                    int64 rangeOffset = -1,
                    int64 rangeSize = -1,
                    int16 partsCount = -1,
                    int32 timeout = 30,
                    int32 retriesCount = 3) override;
    // Cancel download by ID (works for scheduled and current)
    void RemoveTask(Task* task) override;

    // wait for task status = finished
    void WaitTask(Task* task) override;

    const TaskInfo* GetTaskInfo(Task* task) override;
    TaskStatus GetTaskStatus(Task* task) override;

private:
    bool TakeOneNewTaskFromQueue();
    void DownloadThreadFunc();

    Deque<Task*> taskQueue;
    Vector<CURL*> easyHandlesAll;
    CURLM* multiHandle = nullptr;
    Thread* downloadThread = nullptr;
    Semaphore downloadSem; // to resume download thread
    Atomic<int> numOfNewTasks;
    Mutex mutexTaskQueue; // to protect access to taskQueue
};
} // end namespace DAVA