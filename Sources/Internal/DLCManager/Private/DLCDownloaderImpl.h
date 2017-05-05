#pragma once
#include "DLCManager/DLCDownloader.h"
#include "Concurrency/Thread.h"
#include "Concurrency/Semaphore.h"

#include <curl/curl.h>

namespace DAVA
{
/**
  We have to share curl global initialization and cleanup
  cause it is not thread safe and is using in several modules
*/
void CurlGlobalInit();
void CurlGlobalDeinit();

struct Buffer
{
    void* ptr = nullptr;
    size_t size = 0;
};

struct IDownloaderSubTask
{
    DLCDownloader::Task& task;
    int downloadOrderIndex = 0;

    explicit IDownloaderSubTask(DLCDownloader::Task& t)
        : task(t)
    {
    }
    virtual ~IDownloaderSubTask();
    virtual void OnDone(CURLMsg* msg) = 0;
    virtual DLCDownloader::Task& GetTask() = 0;
    virtual CURL* GetEasyHandle() = 0;
    virtual DLCDownloader::IWriter* GetIWriter() = 0;
    virtual Buffer GetBuffer() = 0;
};

struct ICurlEasyStorage
{
    virtual ~ICurlEasyStorage();
    virtual CURLM* GetMultiHandle() = 0;
    virtual CURL* CurlCreateHandle() = 0;
    virtual void CurlDeleteHandle(CURL* easy) = 0;
    virtual int GetFreeHandleCount() = 0;
    virtual void Map(CURL* easy, IDownloaderSubTask& subTask) = 0;
    virtual IDownloaderSubTask& FindInMap(CURL* easy) = 0;
    virtual void UnMap(CURL* easy) = 0;
    virtual int GetChankSize() = 0;
};

struct DLCDownloader::Task
{
    TaskInfo info;
    TaskStatus status;
    List<IDownloaderSubTask*> subTasksWorking;
    List<IDownloaderSubTask*> subTasksReadyToWrite; // sorted list by subTaskIndex
    int lastCreateSubTaskIndex = -1;
    int lastWritenSubTaskIndex = -1;
    std::unique_ptr<IWriter> writer;
    bool userWriter = false;
    ICurlEasyStorage& curlStorage;

    int64 restOffset = -1;
    int64 restSize = -1;

    Task(ICurlEasyStorage& storage,
         const String& srcUrl,
         const String& dstPath,
         TaskType taskType,
         IWriter* dstWriter,
         int64 rangeOffset,
         int64 rangeSize,
         int32 timeout);
    void FlushWriterAndReset();
    ~Task();

    void PrepareForDownloading();
    bool IsDone() const;
    bool NeedHandle() const;
    void OnSubTaskDone();
    void GenerateChankSubRequests(const int chankSize);
    void SetupFullDownload();
    void CorrectRangeToResumeDownloading();
    void SetupResumeDownload();
    void SetupGetSizeDownload();

    // error handles
    static void OnErrorCurlMulti(int32 multiCode, Task* task, CURLM* multi, CURL* easy);
    static void OnErrorCurlEasy(int32 easyCode, Task* task, IDownloaderSubTask* subTask);
    static void OnErrorCurlErrno(int32 errnoVal, Task* task, IDownloaderSubTask* subTask);
    static void OnErrorHttpCode(long httpCode, Task* task, IDownloaderSubTask* subTask);
};

class DLCDownloaderImpl : public DLCDownloader, public ICurlEasyStorage
{
public:
    DLCDownloaderImpl();
    ~DLCDownloaderImpl();

    DLCDownloaderImpl(const DLCDownloaderImpl&) = delete;
    DLCDownloaderImpl(DLCDownloaderImpl&&) = delete;
    DLCDownloaderImpl& operator=(const DLCDownloader&) = delete;

    Task* StartGetContentSize(const String& srcUrl) override;

    Task* StartTask(const String& srcUrl, const String& dstPath, Range range = EmptyRange) override;

    Task* StartTask(const String& srcUrl, IWriter& customWriter, Range range = EmptyRange) override;

    Task* ResumeTask(const String& srcUrl, const String& dstPath, Range range = EmptyRange) override;

    Task* ResumeTask(const String& srcUrl, IWriter& customWriter, Range range = EmptyRange) override;

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
    void AddNewTasks();
    void ProcessMessagesFromMulti();
    void BalancingHandles();

    Task* StartAnyTask(const String& srcUrl,
                       const String& dsrPath,
                       TaskType taskType,
                       IWriter* dstWriter = nullptr,
                       Range range = EmptyRange);

    // [start] implement ICurlEasyStorage interface
    CURL* CurlCreateHandle() override;
    void CurlDeleteHandle(CURL* easy) override;
    CURLM* GetMultiHandle() override;
    int GetFreeHandleCount() override;
    void Map(CURL* easy, IDownloaderSubTask& subTask) override;
    IDownloaderSubTask& FindInMap(CURL* easy) override;
    void UnMap(CURL* easy) override;
    int GetChankSize() override;
    void DeleteSubTaskHandler(IDownloaderSubTask* t);
    // [end] implement ICurlEasyStorage interface

    void DownloadThreadFunc();
    void DeleteTask(Task* task);
    void RemoveDeletedTasks();
    Task* AddOneMoreTask();
    int CurlPerform();

    struct WaitingDescTask
    {
        Task* task = nullptr;
        Semaphore* semaphore = nullptr;
    };

    List<Task*> inputList;
    Mutex mutexInputList; // to protect access to taskQueue
    List<WaitingDescTask> waitingTaskList;
    Mutex mutexWaitingList;
    List<Task*> removedList;
    Mutex mutexRemovedList;

    Thread::Id downlodThreadId = 0;

    // [start] next variables used only from Download thread
    List<Task*> tasks;
    UnorderedMap<CURL*, IDownloaderSubTask*> taskMap;
    List<CURL*> reusableHandles;
    CURLM* multiHandle = nullptr;
    Thread* downloadThread = nullptr;
    int numOfRunningSubTasks = 0;
    // [end] variables

    Semaphore downloadSem; // to resume download thread

    Hints hints; // read only params
};
} // end namespace DAVA