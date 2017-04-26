#include "DLCManager/Private/DLCDownloaderImpl.h"

#include "DLC/Downloader/CurlDownloader.h"

#include "Logger/Logger.h"
#include "FileSystem/File.h"
#include "FileSystem/FileSystem.h"
#include "Concurrency/LockGuard.h"

namespace DAVA
{
Thread::Id downlodThreadId = 0;

DLCDownloader::~DLCDownloader() = default;
IDownloaderSubTask::~IDownloaderSubTask() = default;
ICurlEasyStorage::~ICurlEasyStorage() = default;

class BufferWriter final : public DLCDownloader::IWriter
{
public:
    BufferWriter(const BufferWriter&) = delete;
    BufferWriter(BufferWriter&&) = delete;

    explicit BufferWriter(int64 size)
    {
        buf = new char[static_cast<unsigned>(size)];
        DVASSERT(buf != nullptr);
        DVASSERT(size > 0);

        current = buf;
        end = buf + size;
    }
    ~BufferWriter()
    {
        delete buf;
        buf = nullptr;
        current = nullptr;
        end = nullptr;
    }

    uint64 Save(const void* ptr, uint64 size) override
    {
        uint64 space = SpaceLeft();
        if (size > space)
        {
            DAVA_THROW(Exception, "memory corruption");
        }
        memcpy(current, ptr, static_cast<size_t>(size));
        current += size;
        return size;
    }

    uint64 GetSeekPos() override
    {
        return current - buf;
    }

    void Truncate() override
    {
        current = buf;
    }

    uint64 SpaceLeft() override
    {
        return end - current;
    }

    Buffer GetBuffer() const
    {
        Buffer b;
        b.ptr = buf;
        b.size = end - buf;
        return b;
    }

private:
    char* buf = nullptr;
    char* current = nullptr;
    char* end = nullptr;
};

DLCDownloader* DLCDownloader::Create()
{
    return new DLCDownloaderImpl();
}

void DLCDownloader::Destroy(DLCDownloader* downloader)
{
    delete downloader;
}

static void CurlSetTimeout(DLCDownloader::Task* justAddedTask, CURL* easyHandle)
{
    CURLcode code = CURLE_OK;

    long operationTimeout = static_cast<long>(justAddedTask->info.timeoutSec);

    code = curl_easy_setopt(easyHandle, CURLOPT_CONNECTTIMEOUT, operationTimeout);
    DVASSERT(CURLE_OK == code);

    // we could set operation time limit which produce timeout if operation takes time.
    code = curl_easy_setopt(easyHandle, CURLOPT_TIMEOUT, 0L);
    DVASSERT(CURLE_OK == code);

    code = curl_easy_setopt(easyHandle, CURLOPT_DNS_CACHE_TIMEOUT, operationTimeout);
    DVASSERT(CURLE_OK == code);

    code = curl_easy_setopt(easyHandle, CURLOPT_SERVER_RESPONSE_TIMEOUT, operationTimeout);
    DVASSERT(CURLE_OK == code);
}

static size_t CurlDataRecvHandler(void* ptr, size_t size, size_t nmemb, void* part)
{
    IDownloaderSubTask* subTask = static_cast<IDownloaderSubTask*>(part);
    DVASSERT(subTask != nullptr);
    DLCDownloader::IWriter* writer = subTask->GetIWriter();
    DVASSERT(writer != nullptr);

    size_t fullSizeToWrite = size * nmemb;

    uint64 writen = writer->Save(ptr, fullSizeToWrite);
    DVASSERT(writen == fullSizeToWrite);

    return static_cast<size_t>(writen);
}

struct DownloadChankSubTask : IDownloaderSubTask
{
    DLCDownloader::Task* task = nullptr;
    CURL* easy = nullptr;
    int64 offset;
    int64 size;
    BufferWriter chankBuf;

    DownloadChankSubTask(DLCDownloader::Task* task_, int64 offset_, int64 size_)
        : task(task_)
        , offset(offset_)
        , size(size_)
        , chankBuf(size_)
    {
        DVASSERT(task != nullptr);
        DVASSERT(offset != -1);
        DVASSERT(size > 0);

        ++(task->lastCreateSubTaskIndex);
        downloadOrderIndex = task->lastCreateSubTaskIndex;

        easy = task->curlStorage->CurlCreateHandle();
        DVASSERT(easy != nullptr);

        const char* url = task->info.srcUrl.c_str();
        DVASSERT(url != nullptr);

        CURLcode code = curl_easy_setopt(easy, CURLOPT_URL, url);
        DVASSERT(CURLE_OK == code);

        code = curl_easy_setopt(easy, CURLOPT_WRITEFUNCTION, CurlDataRecvHandler);
        DVASSERT(code == CURLE_OK);

        code = curl_easy_setopt(easy, CURLOPT_WRITEDATA, task);
        DVASSERT(code == CURLE_OK);

        char buf[128] = { 0 };
        int result = snprintf(buf, sizeof(buf), "%lld-%lld", offset, offset + size - 1);
        DVASSERT(result > 0 && result < sizeof(buf));

        code = curl_easy_setopt(easy, CURLOPT_RANGE, buf);
        DVASSERT(code == CURLE_OK);

        // set all timeouts
        CurlSetTimeout(task, easy);

        CURLM* multi = task->curlStorage->GetMultiHandle();
        DVASSERT(multi != nullptr);
        CURLMcode codem = curl_multi_add_handle(multi, easy);
        DVASSERT(CURLM_OK == codem);

        task->curlStorage->Map(easy, this);
    }

    ~DownloadChankSubTask()
    {
        Cleanup();
    }

    void Cleanup()
    {
        if (task)
        {
            ICurlEasyStorage* storage = task->curlStorage;
            if (easy)
            {
                task->curlStorage->UnMap(easy);

                CURLMcode code = curl_multi_remove_handle(storage->GetMultiHandle(), easy);
                DVASSERT(CURLM_OK == code);
                storage->CurlDeleteHandle(easy);
            }
        }
        easy = nullptr;
        task = nullptr;
        offset = 0;
        size = 0;
    }

    void OnDone(CURLMsg* curlMsg) override
    {
        if (curlMsg->data.result != CURLE_OK)
        {
            if (curlMsg->data.result == CURLE_PARTIAL_FILE && task->status.sizeDownloaded == task->info.rangeSize)
            {
                // all good
            }
            else
            {
                task->status.error.curlErr = curlMsg->data.result;
                task->status.error.curlEasyStrErr = curl_easy_strerror(curlMsg->data.result);
            }
        }

        Cleanup();
    }

    DLCDownloader::Task* GetTask() override
    {
        return task;
    }

    CURL* GetEasyHandle() override
    {
        return easy;
    }

    DLCDownloader::IWriter* GetIWriter() override
    {
        return &chankBuf;
    }

    Buffer GetBuffer() override
    {
        return chankBuf.GetBuffer();
    }
};

struct GetSizeSubTask : IDownloaderSubTask
{
    DLCDownloader::Task* task = nullptr;
    CURL* easy = nullptr;

    GetSizeSubTask(DLCDownloader::Task* task_)
        : task(task_)
    {
        DVASSERT(task != nullptr);

        ++(task->lastCreateSubTaskIndex);
        downloadOrderIndex = task->lastCreateSubTaskIndex;

        easy = task->curlStorage->CurlCreateHandle();
        DVASSERT(easy != nullptr);

        CURLcode code = curl_easy_setopt(easy, CURLOPT_HEADER, 0L);
        DVASSERT(CURLE_OK == code);

        const char* url = task->info.srcUrl.c_str();
        DVASSERT(url != nullptr);

        code = curl_easy_setopt(easy, CURLOPT_URL, url);
        DVASSERT(CURLE_OK == code);

        // Don't return the header (we'll use curl_getinfo();
        code = curl_easy_setopt(easy, CURLOPT_NOBODY, 1L);
        DVASSERT(CURLE_OK == code);

        CURLM* multi = task->curlStorage->GetMultiHandle();
        DVASSERT(multi != nullptr);
        CURLMcode codem = curl_multi_add_handle(multi, easy);
        DVASSERT(CURLM_OK == codem);

        task->curlStorage->Map(easy, this);
    }

    ~GetSizeSubTask()
    {
        task->curlStorage->UnMap(easy);
        task = nullptr;
        easy = nullptr;
    }

    void OnDone(CURLMsg* curlMsg) override
    {
        if (curlMsg->data.result != CURLE_OK)
        {
            task->status.error.curlErr = curlMsg->data.result;
            task->status.error.curlEasyStrErr = curl_easy_strerror(curlMsg->data.result);
        }
        else
        {
            float64 sizeToDownload = 0.0; // curl need double! do not change https://curl.haxx.se/libcurl/c/CURLINFO_CONTENT_LENGTH_DOWNLOAD.html
            CURLcode code = curl_easy_getinfo(easy, CURLINFO_CONTENT_LENGTH_DOWNLOAD, &sizeToDownload);
            DVASSERT(CURLE_OK == code);
            task->status.sizeTotal = static_cast<uint64>(sizeToDownload);
        }

        ICurlEasyStorage* storage = task->curlStorage;
        CURLMcode code = curl_multi_remove_handle(storage->GetMultiHandle(), easy);
        DVASSERT(CURLM_OK == code);
        storage->CurlDeleteHandle(easy);
    }

    DLCDownloader::Task* GetTask() override
    {
        return task;
    }

    CURL* GetEasyHandle() override
    {
        return easy;
    }

    DLCDownloader::IWriter* GetIWriter() override
    {
        return nullptr;
    }

    Buffer GetBuffer() override
    {
        return Buffer();
    }
};

DLCDownloader::Task::Task(ICurlEasyStorage* storage_,
                          const String& srcUrl,
                          const String& dstPath,
                          TaskType taskType,
                          IWriter* dstWriter,
                          int64 rangeOffset,
                          int64 rangeSize,
                          int32 timeout)
{
    curlStorage = storage_;
    DVASSERT(curlStorage != nullptr);

    info.rangeOffset = rangeOffset;
    info.rangeSize = rangeSize;
    info.srcUrl = srcUrl;
    info.dstPath = dstPath;
    info.timeoutSec = timeout;
    info.type = taskType;

    status.state = TaskState::JustAdded;
    status.error = TaskError();
    status.sizeDownloaded = 0;
    status.sizeTotal = info.rangeSize;

    defaultWriter.reset(dstWriter);
}

DLCDownloader::Task::~Task()
{
    status.error.curlErr = 0;
    status.error.curlMErr = 0;
    status.sizeDownloaded = 0;
    status.sizeTotal = 0;
    defaultWriter.reset();
    curlStorage = nullptr;
}

void DLCDownloader::Task::PrepareForDownloading()
{
    switch (info.type)
    {
    case TaskType::FULL:
        SetupFullDownload();
        break;
    case TaskType::RESUME:
        SetupResumeDownload();
        break;
    case TaskType::SIZE:
        SetupGetSizeDownload();
        break;
    }

    status.state = TaskState::Downloading;
}

bool DLCDownloader::Task::IsDone() const
{
    return subTasksWorking.empty() && subTasksReadyToWrite.empty();
}

bool DLCDownloader::Task::NeedHandle() const
{
    return restSize > 0;
}

void DLCDownloader::Task::OnSubTaskDone()
{
    if (info.type == TaskType::SIZE)
    {
        IDownloaderSubTask* subTask = subTasksReadyToWrite.front();
        delete subTask;
        subTasksReadyToWrite.remove(subTask);
    }
    else
    {
        subTasksReadyToWrite.sort([](IDownloaderSubTask* subLeft, IDownloaderSubTask* subRight)
                                  {
                                      return subLeft->downloadOrderIndex < subRight->downloadOrderIndex;
                                  });

        if (!subTasksReadyToWrite.empty())
        {
            for (IDownloaderSubTask* nextSubTask = subTasksReadyToWrite.front();
                 lastWritenSubTaskIndex + 1 == nextSubTask->downloadOrderIndex;
                 nextSubTask = subTasksReadyToWrite.front())
            {
                Buffer b = nextSubTask->GetBuffer();
                uint64 writen = defaultWriter->Save(b.ptr, b.size);
                DVASSERT(writen == b.size);
                delete nextSubTask;
                ++lastWritenSubTaskIndex;
                subTasksReadyToWrite.pop_front();
                if (subTasksReadyToWrite.empty())
                {
                    break;
                }
            }
        }
    }
}

struct DefaultWriter : DLCDownloader::IWriter
{
    explicit DefaultWriter(const String& outputFile)
    {
        FileSystem* fs = FileSystem::Instance();
        FilePath path = outputFile;
        fs->CreateDirectory(path.GetDirectory(), true);
        f = File::Create(outputFile, File::OPEN | File::WRITE | File::CREATE);
        if (!f)
        {
            const char* err = strerror(errno);
            DAVA_THROW(Exception, "can't create output file: " + outputFile + " " + err);
        }
    }
    ~DefaultWriter()
    {
        f->Release();
        f = nullptr;
    }

    // save next buffer bytes into memory or file
    uint64 Save(const void* ptr, uint64 size) override
    {
        return f->Write(ptr, static_cast<uint32>(size));
    }
    // return current size of saved byte stream
    uint64 GetSeekPos() override
    {
        return f->GetPos();
    }

    void Truncate() override
    {
        bool result = f->Truncate(0);
        DVASSERT(result);
    }

    uint64 SpaceLeft() override
    {
        return std::numeric_limits<uint64>::max(); // no limit
    }

private:
    File* f = nullptr;
};

DLCDownloader::TaskStatus::TaskStatus() = default;

DLCDownloader::TaskStatus::TaskStatus(const TaskStatus& other)
    : state(other.state.Get())
    , error(other.error)
    , sizeTotal(other.sizeTotal)
    , sizeDownloaded(other.sizeDownloaded)
{
}

DLCDownloader::TaskStatus& DLCDownloader::TaskStatus::operator=(const TaskStatus& other)
{
    state = other.state.Get();
    error = other.error;
    sizeTotal = other.sizeTotal;
    sizeDownloaded = other.sizeDownloaded;
    return *this;
}

void DLCDownloaderImpl::Initialize()
{
    if (!CurlDownloader::isCURLInit && CURLE_OK != curl_global_init(CURL_GLOBAL_ALL))
    {
        DAVA_THROW(Exception, "curl_global_init fail");
    }

    multiHandle = curl_multi_init();

    if (multiHandle == nullptr)
    {
        DAVA_THROW(Exception, "curl_multi_init fail");
    }

    downloadThread = Thread::Create([this] { DownloadThreadFunc(); });
    downloadThread->SetName("DLCDownloader");
    downloadThread->Start();
}

void DLCDownloaderImpl::Deinitialize()
{
    if (downloadThread != nullptr)
    {
        if (downloadThread->IsJoinable())
        {
            downloadThread->Cancel();
            downloadSem.Post(100); // just to resume if waiting
            downloadThread->Join();
        }
    }

    curl_multi_cleanup(multiHandle);
    multiHandle = nullptr;

    for (CURL* easy : reusableHandles)
    {
        curl_easy_cleanup(easy);
    }
    reusableHandles.clear();

    if (!CurlDownloader::isCURLInit)
    {
        curl_global_cleanup();
    }
}

DLCDownloaderImpl::DLCDownloaderImpl()
{
    Initialize();
}

DLCDownloaderImpl::~DLCDownloaderImpl()
{
    Deinitialize();
}

DLCDownloader::Task* DLCDownloaderImpl::StartTask(const String& srcUrl,
                                                  const String& dstPath,
                                                  TaskType taskType,
                                                  IWriter* dstWriter,
                                                  int64 rangeOffset,
                                                  int64 rangeSize,
                                                  int32 timeout)
{
    Task* task = new Task(this,
                          srcUrl,
                          dstPath,
                          taskType,
                          dstWriter,
                          rangeOffset,
                          rangeSize,
                          timeout);

    {
        LockGuard<Mutex> lock(mutexInputList);
        inputList.push_back(task);
    }

    downloadSem.Post(1);

    return task;
}

void DLCDownloaderImpl::RemoveTask(Task* task)
{
    DVASSERT(task);
    if (task != nullptr)
    {
        LockGuard<Mutex> lock(mutexRemovedList);
        removedList.push_back(task);
    }
}

void DLCDownloaderImpl::WaitTask(Task* task)
{
    DVASSERT(task != nullptr);
    if (task->status.state != TaskState::Finished)
    {
        Semaphore semaphore;

        WaitingDescTask wt;
        wt.task = task;
        wt.semaphore = &semaphore;

        {
            LockGuard<Mutex> lock(mutexWaitingList);
            waitingTaskList.push_back(wt);
        }
        // if download thread waiting for signal (all downloaded already)
        downloadSem.Post();
        semaphore.Wait();
    }
}

const DLCDownloader::TaskInfo& DLCDownloaderImpl::GetTaskInfo(Task* task)
{
    return task->info;
}

const DLCDownloader::TaskStatus& DLCDownloaderImpl::GetTaskStatus(Task* task)
{
    return task->status;
}

void DLCDownloaderImpl::SetHints(const Hints& h)
{
    DVASSERT(inputList.empty());
    Deinitialize();
    hints = h;
    Initialize();
}

void DLCDownloaderImpl::RemoveDeletedTasks()
{
    if (!removedList.empty())
    {
        LockGuard<Mutex> lock(mutexRemovedList);
        removedList.remove_if([this](Task* task) {

            if (task->status.state == TaskState::JustAdded)
            {
                LockGuard<Mutex> lock2(mutexInputList);
                auto it = find(begin(inputList), end(inputList), task);
                if (it != end(inputList))
                {
                    inputList.erase(it);
                }
            }

            if (!waitingTaskList.empty())
            {
                LockGuard<Mutex> lock3(mutexWaitingList);
                auto it = find_if(begin(waitingTaskList), end(waitingTaskList), [task](WaitingDescTask& wd)
                                  {
                                      return wd.task == task;
                                  });
                if (it != end(waitingTaskList))
                {
                    it->semaphore->Post(1);
                    waitingTaskList.erase(it);
                }
            }

            DeleteTask(task);
            return true;
        });
    }
}

DLCDownloader::Task* DLCDownloaderImpl::AddOneMoreTask()
{
    if (inputList.empty())
    {
        return nullptr;
    }
    Task* task = inputList.front();
    inputList.pop_front();
    return task;
}

CURL* DLCDownloaderImpl::CurlCreateHandle()
{
    DVASSERT(Thread::GetCurrentId() == downlodThreadId);
    CURL* curl_handle = nullptr;

    if (reusableHandles.empty())
    {
        curl_handle = curl_easy_init();
    }
    else
    {
        curl_handle = reusableHandles.front();
        reusableHandles.pop_front();
    }

    DVASSERT(curl_handle != nullptr);

    CURLcode code = CURLE_OK;
    code = curl_easy_setopt(curl_handle, CURLOPT_NOPROGRESS, 1L); // Do we need it?
    DVASSERT(CURLE_OK == code);

    code = curl_easy_setopt(curl_handle, CURLOPT_VERBOSE, 0L);
    DVASSERT(CURLE_OK == code);

    code = curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, "DAVA DLC");
    DVASSERT(CURLE_OK == code);

    code = curl_easy_setopt(curl_handle, CURLOPT_SSL_VERIFYPEER, 0L);
    DVASSERT(CURLE_OK == code);

    code = curl_easy_setopt(curl_handle, CURLOPT_FOLLOWLOCATION, 1L);
    DVASSERT(CURLE_OK == code);

    code = curl_easy_setopt(curl_handle, CURLOPT_TCP_KEEPALIVE, 1L);
    DVASSERT(CURLE_OK == code);

    ++numOfRunningSubTasks;

    return curl_handle;
}

void DLCDownloaderImpl::CurlDeleteHandle(CURL* easy)
{
    DVASSERT(Thread::GetCurrentId() == downlodThreadId);
    curl_easy_reset(easy);
    reusableHandles.push_back(easy);
    --numOfRunningSubTasks;
}

CURLM* DLCDownloaderImpl::GetMultiHandle()
{
    return multiHandle;
}

int DLCDownloaderImpl::GetFreeHandleCount()
{
    int numFree = hints.numOfMaxEasyHandles - numOfRunningSubTasks;
    DVASSERT(numFree >= 0);
    return numFree;
}

void DLCDownloaderImpl::Map(CURL* easy, IDownloaderSubTask* subTask)
{
    DVASSERT(easy != nullptr);
    DVASSERT(subTask != nullptr);
    taskMap.emplace(easy, subTask);
}

IDownloaderSubTask* DLCDownloaderImpl::FindInMap(CURL* easy)
{
    DVASSERT(easy != nullptr);
    auto it = taskMap.find(easy);
    if (it != end(taskMap))
    {
        return it->second;
    }
    return nullptr;
}

void DLCDownloaderImpl::UnMap(CURL* easy)
{
    DVASSERT(easy != nullptr);
    taskMap.erase(easy);
}

int DLCDownloaderImpl::GetChankSize()
{
    return hints.chankMemBuffSize;
}

void DLCDownloaderImpl::DeleteTask(Task* task)
{
    if (task->status.state == TaskState::Downloading)
    {
        for (auto& t : task->subTasksWorking)
        {
            CURL* easy = t->GetEasyHandle();
            taskMap.erase(easy);

            CURLMcode r = curl_multi_remove_handle(multiHandle, easy);
            DVASSERT(CURLM_OK == r);

            CurlDeleteHandle(easy);
        }
        for (auto& t : task->subTasksReadyToWrite)
        {
            CURL* easy = t->GetEasyHandle();
            taskMap.erase(easy);

            CURLMcode r = curl_multi_remove_handle(multiHandle, easy);
            DVASSERT(CURLM_OK == r);

            CurlDeleteHandle(easy);
        }
    }

    delete task;
}

void DLCDownloader::Task::GenerateChankSubRequests(const int chankSize)
{
    while (NeedHandle() && curlStorage->GetFreeHandleCount() > 0)
    {
        if (restSize < chankSize)
        {
            // take rest range
            IDownloaderSubTask* subTask = new DownloadChankSubTask(this, restOffset, restSize);
            subTasksWorking.push_back(subTask);
            restOffset += restSize;
            restSize = 0;
            break;
        }

        IDownloaderSubTask* subTask = new DownloadChankSubTask(this, restOffset, chankSize);
        subTasksWorking.push_back(subTask);
        restOffset += chankSize;
        restSize -= chankSize;
    }
}

void DLCDownloader::Task::SetupFullDownload()
{
    CURLcode code = CURLE_OK;

    if (defaultWriter.get() == nullptr)
    {
        defaultWriter.reset(new DefaultWriter(info.dstPath));
    }
    defaultWriter->Truncate();

    if (info.rangeOffset != -1 && info.rangeSize != -1)
    {
        // we already know size to download
        restOffset = info.rangeOffset;
        restSize = info.rangeSize;
        const int chankSize = curlStorage->GetChankSize();

        GenerateChankSubRequests(chankSize);
    }
    else
    {
        DVASSERT(info.rangeOffset != -1);
        DVASSERT(info.rangeSize != -1);
        // first get size of full file
        IDownloaderSubTask* subTask = new GetSizeSubTask(this);
        subTasksWorking.push_back(subTask);
    }
}

void DLCDownloader::Task::SetupResumeDownload()
{
    if (defaultWriter.get() == nullptr)
    {
        defaultWriter.reset(new DefaultWriter(info.dstPath));
    }

    if (info.rangeOffset != -1 && info.rangeSize != -1)
    {
        // we already know size to download
        // TODO generate range sub_requests
    }
    else
    {
        // first get size of full file
        IDownloaderSubTask* subTask = new GetSizeSubTask(this);
        subTasksWorking.push_back(subTask);
    }
}

void DLCDownloader::Task::SetupGetSizeDownload()
{
    IDownloaderSubTask* subTask = new GetSizeSubTask(this);

    subTasksWorking.push_back(subTask);
}

bool DLCDownloaderImpl::TakeNewTaskFromInputList()
{
    Task* justAddedTask = AddOneMoreTask();

    if (justAddedTask != nullptr)
    {
        justAddedTask->PrepareForDownloading();
    }
    tasks.push_back(justAddedTask);
    return justAddedTask != nullptr;
}

void DLCDownloaderImpl::SignalOnFinishedWaitingTasks()
{
    if (!waitingTaskList.empty())
    {
        LockGuard<Mutex> lock(mutexWaitingList);
        waitingTaskList.remove_if([this](WaitingDescTask& wt) {
            DVASSERT(wt.task != nullptr);
            if (wt.task->status.state == TaskState::Finished)
            {
                DVASSERT(wt.semaphore != nullptr);
                wt.semaphore->Post();
                return true;
            }
            return false;
        });
    }
}

void DLCDownloaderImpl::AddNewTasks()
{
    if (!inputList.empty() && GetFreeHandleCount() > 0)
    {
        LockGuard<Mutex> lock(mutexInputList);
        while (!inputList.empty() && GetFreeHandleCount() > 0)
        {
            bool justAdded = TakeNewTaskFromInputList();
            if (!justAdded)
            {
                break; // no more new tasks
            }
        }
    }
}

void DLCDownloaderImpl::ProcessMessagesFromMulti()
{
    CURLMsg* curlMsg = nullptr;

    do
    {
        int msgq = 0;
        curlMsg = curl_multi_info_read(multiHandle, &msgq);
        if (curlMsg && (curlMsg->msg == CURLMSG_DONE))
        {
            CURL* easyHandle = curlMsg->easy_handle;

            IDownloaderSubTask* subTask = FindInMap(easyHandle);
            DVASSERT(subTask != nullptr);
            Task* task = subTask->GetTask();
            DVASSERT(task != nullptr);

            subTask->OnDone(curlMsg);

            task->subTasksWorking.remove(subTask);
            task->subTasksReadyToWrite.push_back(subTask);

            task->OnSubTaskDone();

            if (task->IsDone())
            {
                task->status.state = TaskState::Finished;
            }
        }
    } while (curlMsg);
}

void DLCDownloaderImpl::BalancingHandles()
{
    while (GetFreeHandleCount() > 0)
    {
        // find first not finished task
        auto it = std::find_if(begin(tasks), end(tasks), [](Task* t)
                               {
                                   return t->NeedHandle();
                               });
        // add more sub-task to it
        if (it != end(tasks))
        {
            Task* task = *it;
            task->GenerateChankSubRequests(hints.chankMemBuffSize);
        }
        else
        {
            break;
        }
    }
}

void DLCDownloaderImpl::DownloadThreadFunc()
{
    Thread* currentThread = Thread::Current();
    DVASSERT(currentThread != nullptr);

    downlodThreadId = currentThread->GetId();

    bool downloading = false;

    while (!currentThread->IsCancelling())
    {
        if (!downloading)
        {
            downloadSem.Wait();
            downloading = true;
        }

        SignalOnFinishedWaitingTasks();

        RemoveDeletedTasks();

        AddNewTasks();

        BalancingHandles();

        while (downloading)
        {
            downloading = CurlPerform() != 0;

            ProcessMessagesFromMulti();

            if (downloading)
            {
                if (!inputList.empty() && GetFreeHandleCount() > 0)
                {
                    // get more new task to do it simultaneously
                    // and check removed and waiting tasks
                    break;
                }
            }
        }
    } // end while(!currentThread->IsCancelling())
}

// from https://curl.haxx.se/libcurl/c/curl_multi_perform.html
// from https://curl.haxx.se/libcurl/c/curl_multi_wait.html
int DLCDownloaderImpl::CurlPerform()
{
    int stillRunning = 0;

    CURLMcode code = curl_multi_perform(multiHandle, &stillRunning);
    DVASSERT(code == CURLM_OK);

    if (code == CURLM_OK)
    {
        // wait for activity, timeout or "nothing"
        code = curl_multi_wait(multiHandle, nullptr, 0, 1000, nullptr);
        DVASSERT(code == CURLM_OK);
    }

    if (code != CURLM_OK)
    {
        Logger::Error("curl_multi failed, code %d.n", code);
    }

    // if there are still transfers, loop!
    return stillRunning;
}

} // end namespace DAVA
