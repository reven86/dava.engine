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
    DLCDownloaderImpl& operator=(const DLCDownloader&) = delete;

    // Schedule download content or get content size (indicated by downloadMode)
    Task* StartTask(const String& srcUrl,
                    const String& dsrPath,
                    TaskType taskType,
                    IWriter* dstWriter = nullptr,
                    int64 rangeOffset = -1,
                    int64 rangeSize = -1,
                    int32 timeout = 30) override;
    // Cancel download by ID (works for scheduled and current)
    void RemoveTask(Task* task) override;

    // wait for task status = finished
    void WaitTask(Task* task) override;

    const TaskInfo& GetTaskInfo(Task* task) override;
    const TaskStatus& GetTaskStatus(Task* task) override;

    void SetHints(const Hints& h) override;

private:
    void Initialize();
    void Deinitialize();
    bool TakeNewTaskFromInputList();
    void SignalOnFinishedWaitingTasks();
    void AddNewTasks(int& numOfAddedTasks);
    void ProcessMessagesFromMulti(int& numOfAddedTasks);
    CURL* CurlCreateHandle();
    void CurlDeleteHandle(CURL* easy);
    void SetupFullDownload(Task* justAddedTask);
    void SetupResumeDownload(Task* justAddedTask);
    void SetupGetSizeDownload(Task* justAddedTask);
    void DownloadThreadFunc();
    void StoreHandle(Task* justAddedTask, CURL* easyHandle);
    void DeleteTask(Task* task);
    void RemoveDeletedTasks();
    Task* FindJustEddedTask();
    int CurlPerform();

    struct WaitingDescTask
    {
        Task* task = nullptr;
        Semaphore* semaphore = nullptr;
    };

    List<Task*> inputList;
    Mutex mutexTaskList; // to protect access to taskQueue
    List<WaitingDescTask> waitingTaskList;
    Mutex mutexWaitingList;
    List<Task*> removedList;
    Mutex mutexRemovedList;

    // [start] next variables used only from DownloadThreadFunc TODO move in local variables
    UnorderedMap<CURL*, Task*> taskMap;
    List<CURL*> reusableHandles;
    CURLM* multiHandle = nullptr;
    Thread* downloadThread = nullptr;
    // [finish] variables

    Semaphore downloadSem; // to resume download thread

    Hints hints; // read only params
};
} // end namespace DAVA