#include "DLCManager/Private/DLCDownloaderImpl.h"

#include "DLC/Downloader/CurlDownloader.h"

#include "Logger/Logger.h"
#include "FileSystem/File.h"
#include "FileSystem/FileSystem.h"
#include "Concurrency/LockGuard.h"

namespace DAVA
{
const DLCDownloader::Range DLCDownloader::EmptyRange;

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
        DVASSERT(size > 0);
        buf = new char[static_cast<size_t>(size)];
        current = buf;
        end = buf + size;
    }
    ~BufferWriter() override
    {
        delete[] buf;
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

    bool Truncate() override
    {
        current = buf;
        return true;
    }

    uint64 SpaceLeft() const
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

static void CheckHttpCode(CURL* easy, DLCDownloader::Task& task, IDownloaderSubTask* subTask)
{
    long httpCode = 0; // use long! sizeof(long) == 8 on macosx
    CURLcode code = curl_easy_getinfo(easy, CURLINFO_RESPONSE_CODE, &httpCode);
    if (code != CURLE_OK)
    {
        DLCDownloader::Task::OnErrorCurlEasy(code, &task, subTask);
    }
    else
    {
        task.status.error.httpCode = httpCode; // copy anyway

        // https://en.wikipedia.org/wiki/List_of_HTTP_status_codes
        if (httpCode >= 400)
        {
            DLCDownloader::Task::OnErrorHttpCode(httpCode, &task, subTask);
        }
    }
}

static void CurlSetTimeout(DLCDownloader::Task* task, CURL* easyHandle)
{
    CURLcode code = CURLE_OK;

    long operationTimeout = static_cast<long>(task->info.timeoutSec);

    code = curl_easy_setopt(easyHandle, CURLOPT_CONNECTTIMEOUT, operationTimeout);
    if (code != CURLE_OK)
    {
        DLCDownloader::Task::OnErrorCurlEasy(code, task, nullptr);
        return;
    }

    // we could set operation time limit which produce timeout if operation takes time.
    code = curl_easy_setopt(easyHandle, CURLOPT_TIMEOUT, 0L);
    if (code != CURLE_OK)
    {
        DLCDownloader::Task::OnErrorCurlEasy(code, task, nullptr);
        return;
    }

    code = curl_easy_setopt(easyHandle, CURLOPT_DNS_CACHE_TIMEOUT, operationTimeout);
    if (code != CURLE_OK)
    {
        DLCDownloader::Task::OnErrorCurlEasy(code, task, nullptr);
        return;
    }

    code = curl_easy_setopt(easyHandle, CURLOPT_SERVER_RESPONSE_TIMEOUT, operationTimeout);
    if (code != CURLE_OK)
    {
        DLCDownloader::Task::OnErrorCurlEasy(code, task, nullptr);
    }
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

struct DownloadChunkSubTask : IDownloaderSubTask
{
    CURL* easy = nullptr;
    int64 offset;
    int64 size;
    BufferWriter chankBuf;

    DownloadChunkSubTask(DLCDownloader::Task& task_, int64 offset_, int64 size_)
        : IDownloaderSubTask(task_)
        , offset(offset_)
        , size(size_)
        , chankBuf(size_)
    {
        if (offset >= 0 && size <= 0)
        {
            DAVA_THROW(Exception, "incorrect offset or size");
        }

        ++(task.lastCreateSubTaskIndex);
        downloadOrderIndex = task.lastCreateSubTaskIndex;

        easy = task.curlStorage.CurlCreateHandle();
        if (easy == nullptr)
        {
            DAVA_THROW(Exception, "can't create easy handle, something bad happened");
        }

        const char* url = task.info.srcUrl.c_str();
        if (url == nullptr)
        {
            DAVA_THROW(Exception, "URL can't be nullptr");
        }

        CURLcode code = curl_easy_setopt(easy, CURLOPT_URL, url);
        if (CURLE_OK != code)
        {
            DLCDownloader::Task::OnErrorCurlEasy(code, &task, this);
            return;
        }

        code = curl_easy_setopt(easy, CURLOPT_WRITEFUNCTION, CurlDataRecvHandler);
        if (code != CURLE_OK)
        {
            DLCDownloader::Task::OnErrorCurlEasy(code, &task, this);
            return;
        }

        code = curl_easy_setopt(easy, CURLOPT_WRITEDATA, this);
        if (code != CURLE_OK)
        {
            DLCDownloader::Task::OnErrorCurlEasy(code, &task, this);
            return;
        }

        char buf[128] = { 0 };
        int result = snprintf(buf, sizeof(buf), "%lld-%lld", offset, offset + size - 1);
        if (result < 0 || result >= sizeof(buf))
        {
            DAVA_THROW(Exception, "range format failed");
        }

        code = curl_easy_setopt(easy, CURLOPT_RANGE, buf);
        if (code != CURLE_OK)
        {
            DLCDownloader::Task::OnErrorCurlEasy(code, &task, this);
            return;
        }

        // set all timeouts
        CurlSetTimeout(&task, easy);

        CURLM* multi = task.curlStorage.GetMultiHandle();
        if (multi == nullptr)
        {
            DAVA_THROW(Exception, "multi is nullptr");
        }
        CURLMcode codem = curl_multi_add_handle(multi, easy);
        if (CURLM_OK != codem)
        {
            DLCDownloader::Task::OnErrorCurlMulti(codem, &task, multi, easy);
        }

        task.curlStorage.Map(easy, *this);
    }

    ~DownloadChunkSubTask()
    {
        Cleanup();
    }

    void Cleanup()
    {
        if (easy != nullptr)
        {
            ICurlEasyStorage& storage = task.curlStorage;

            storage.UnMap(easy);
            CURLM* multiHandle = storage.GetMultiHandle();
            CURLMcode code = curl_multi_remove_handle(multiHandle, easy);
            if (CURLM_OK != code)
            {
                DLCDownloader::Task::OnErrorCurlMulti(code, &task, multiHandle, easy);
            }
            storage.CurlDeleteHandle(easy);
        }

        easy = nullptr;
        offset = 0;
        size = 0;
    }

    void OnDone(CURLMsg* curlMsg) override
    {
        if (curlMsg->data.result != CURLE_OK)
        {
            if (curlMsg->data.result == CURLE_PARTIAL_FILE && task.status.sizeDownloaded == task.info.rangeSize)
            {
                // all good
                CheckHttpCode(easy, task, this);
            }
            else
            {
                DLCDownloader::Task::OnErrorCurlEasy(curlMsg->data.result, &task, this);
            }
        }
        else
        {
            CheckHttpCode(easy, task, this);
        }

        Cleanup();
    }

    DLCDownloader::Task& GetTask() override
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
    CURL* easy = nullptr;

    GetSizeSubTask(DLCDownloader::Task& task_)
        : IDownloaderSubTask(task_)
    {
        ++(task.lastCreateSubTaskIndex);
        downloadOrderIndex = task.lastCreateSubTaskIndex;

        easy = task.curlStorage.CurlCreateHandle();
        if (easy == nullptr)
        {
            DAVA_THROW(Exception, "can't create easy handle, something bad happened");
        }

        CURLcode code = curl_easy_setopt(easy, CURLOPT_HEADER, 0L);
        if (code != CURLE_OK)
        {
            DLCDownloader::Task::OnErrorCurlEasy(code, &task, this);
            return;
        }

        const char* url = task.info.srcUrl.c_str();
        if (url == nullptr)
        {
            DAVA_THROW(Exception, "URL is nullptr, something bad happened");
        }

        code = curl_easy_setopt(easy, CURLOPT_URL, url);
        if (code != CURLE_OK)
        {
            DLCDownloader::Task::OnErrorCurlEasy(code, &task, this);
            return;
        }

        // Don't return the header (we'll use curl_getinfo();
        code = curl_easy_setopt(easy, CURLOPT_NOBODY, 1L);
        if (code != CURLE_OK)
        {
            DLCDownloader::Task::OnErrorCurlEasy(code, &task, this);
            return;
        }

        CURLM* multi = task.curlStorage.GetMultiHandle();
        if (multi == nullptr)
        {
            DAVA_THROW(Exception, "multi is nullptr, something bad happened");
        }

        CURLMcode codem = curl_multi_add_handle(multi, easy);
        if (code != CURLE_OK)
        {
            DLCDownloader::Task::OnErrorCurlMulti(codem, &task, multi, easy);
            return;
        }

        task.curlStorage.Map(easy, *this);
    }

    ~GetSizeSubTask()
    {
        task.curlStorage.UnMap(easy);
        easy = nullptr;
    }

    void OnDone(CURLMsg* curlMsg) override
    {
        if (curlMsg->data.result != CURLE_OK)
        {
            DLCDownloader::Task::OnErrorCurlEasy(curlMsg->data.result, &task, this);
        }
        else
        {
            float64 sizeToDownload = 0.0; // curl need double! do not change https://curl.haxx.se/libcurl/c/CURLINFO_CONTENT_LENGTH_DOWNLOAD.html
            CURLcode code = curl_easy_getinfo(easy, CURLINFO_CONTENT_LENGTH_DOWNLOAD, &sizeToDownload);
            if (CURLE_OK != code)
            {
                DLCDownloader::Task::OnErrorCurlEasy(code, &task, this);
            }
            else
            {
                CheckHttpCode(easy, task, this);

                if (!task.status.error.errorHappened)
                {
                    task.status.sizeTotal = static_cast<uint64>(sizeToDownload);

                    if (task.info.type != DLCDownloader::TaskType::SIZE)
                    {
                        // we ask about size(range)
                        task.info.rangeSize = task.status.sizeTotal;
                        task.info.rangeOffset = 0;
                        task.restSize = task.info.rangeSize;
                        task.restOffset = 0;

                        if (task.info.type == DLCDownloader::TaskType::RESUME)
                        {
                            uint64 posInFile = task.writer->GetSeekPos();
                            if (posInFile != std::numeric_limits<uint64>::max())
                            {
                                task.restOffset += posInFile;
                                task.restSize -= posInFile;
                            }
                            else
                            {
                                DLCDownloader::Task::OnErrorCurlErrno(errno, &task, this);
                            }
                        }
                    }
                }
            }
        }

        ICurlEasyStorage& storage = task.curlStorage;
        CURLM* multi = storage.GetMultiHandle();
        CURLMcode code = curl_multi_remove_handle(multi, easy);
        if (CURLM_OK != code)
        {
            DLCDownloader::Task::OnErrorCurlMulti(code, &task, multi, easy);
        }
        storage.CurlDeleteHandle(easy);
    }

    DLCDownloader::Task& GetTask() override
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

DLCDownloader::Task::Task(ICurlEasyStorage& storage_,
                          const String& srcUrl,
                          const String& dstPath,
                          TaskType taskType,
                          IWriter* dstWriter,
                          int64 rangeOffset,
                          int64 rangeSize,
                          int32 timeout)
    : curlStorage(storage_)
{
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

    writer.reset(dstWriter);
    if (writer)
    {
        userWriter = true;
    }
}

void DLCDownloader::Task::FlushWriterAndReset()
{
    if (userWriter)
    {
        writer.release();
    }
    else
    {
        writer.reset();
    }
}

DLCDownloader::Task::~Task()
{
    status.error.curlErr = 0;
    status.error.curlMErr = 0;
    status.error.httpCode = 0;
    status.error.errorHappened = false;
    status.sizeDownloaded = 0;
    status.sizeTotal = 0;

    FlushWriterAndReset();

    userWriter = false;
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

    if (status.state == TaskState::JustAdded)
    {
        status.state = TaskState::Downloading;
    }
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
                uint64 writen = writer->Save(b.ptr, b.size);
                DVASSERT(writen == b.size);

                Task& task = nextSubTask->GetTask();
                task.status.sizeDownloaded += b.size;

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

        f.reset(File::Create(outputFile, File::WRITE | File::APPEND));

        if (!f)
        {
            const char* err = strerror(errno);
            DAVA_THROW(Exception, "can't create output file: " + outputFile + " " + err);
        }
    }
    ~DefaultWriter() = default;

    void MoveToEndOfFile() const
    {
        bool result = f->Seek(0, File::eFileSeek::SEEK_FROM_END);
        DVASSERT(result);
    }

    // save next buffer bytes into memory or file
    uint64 Save(const void* ptr, uint64 size) override
    {
        uint64 writen = f->Write(ptr, static_cast<uint32>(size));
        return writen;
    }
    // return current size of saved byte stream
    uint64 GetSeekPos() override
    {
        return f->GetPos();
    }

    bool Truncate() override
    {
        return f->Truncate(0);
    }

private:
    ScopedPtr<File> f;
};

DLCDownloader::TaskStatus::TaskStatus() = default;

DLCDownloader::TaskStatus::TaskStatus(const TaskStatus& other)
    : state(other.state.load())
    , error(other.error)
    , sizeTotal(other.sizeTotal)
    , sizeDownloaded(other.sizeDownloaded)
{
}

DLCDownloader::TaskStatus& DLCDownloader::TaskStatus::operator=(const TaskStatus& other)
{
    state = other.state.load();
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

    CURLMcode result = curl_multi_cleanup(multiHandle);
    if (result != CURLM_OK)
    {
        const char* strErr = curl_multi_strerror(result);
        DAVA_THROW(Exception, strErr);
    }
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

DLCDownloader::Task* DLCDownloaderImpl::StartAnyTask(const String& srcUrl,
                                                     const String& dstPath,
                                                     TaskType taskType,
                                                     IWriter* dstWriter,
                                                     Range range)
{
    if (srcUrl.empty())
    {
        return nullptr;
    }

    if (dstPath.empty() && dstWriter == nullptr && taskType != TaskType::SIZE)
    {
        return nullptr;
    }

    if (taskType != TaskType::RESUME &&
        taskType != TaskType::FULL &&
        taskType != TaskType::SIZE)
    {
        return nullptr;
    }

    if (range.offset >= 0 && range.size < 0)
    {
        return nullptr;
    }

    Task* task = new Task(*this,
                          srcUrl,
                          dstPath,
                          taskType,
                          dstWriter,
                          range.offset,
                          range.size,
                          hints.timeout);

    {
        LockGuard<Mutex> lock(mutexInputList);
        inputList.push_back(task);
    }

    downloadSem.Post(1);

    return task;
}

DLCDownloader::Task* DLCDownloaderImpl::StartGetContentSize(const String& srcUrl)
{
    if (srcUrl.empty())
    {
        return nullptr;
    }
    return StartAnyTask(srcUrl, "", TaskType::SIZE);
}

DLCDownloader::Task* DLCDownloaderImpl::StartTask(const String& srcUrl, const String& dstPath, Range range)
{
    return StartAnyTask(srcUrl, dstPath, TaskType::FULL, nullptr, range);
}

DLCDownloader::Task* DLCDownloaderImpl::StartTask(const String& srcUrl, IWriter& customWriter, Range range)
{
    return StartAnyTask(srcUrl, "", TaskType::FULL, &customWriter, range);
}

DLCDownloader::Task* DLCDownloaderImpl::ResumeTask(const String& srcUrl, const String& dstPath, Range range)
{
    return StartAnyTask(srcUrl, dstPath, TaskType::RESUME, nullptr, range);
}

DLCDownloader::Task* DLCDownloaderImpl::ResumeTask(const String& srcUrl, IWriter& customWriter, Range range)
{
    return StartAnyTask(srcUrl, "", TaskType::RESUME, &customWriter, range);
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
    if (task != nullptr && task->status.state != TaskState::Finished)
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
    if (task == nullptr)
    {
        DAVA_THROW(Exception, "task is nullptr");
    }
    return task->info;
}

const DLCDownloader::TaskStatus& DLCDownloaderImpl::GetTaskStatus(Task* task)
{
    if (task == nullptr)
    {
        DAVA_THROW(Exception, "task is nullptr");
    }
    return task->status;
}

void DLCDownloaderImpl::SetHints(const Hints& h)
{
    if (h.numOfMaxEasyHandles != hints.numOfMaxEasyHandles || h.chunkMemBuffSize != hints.chunkMemBuffSize)
    {
        if (h.numOfMaxEasyHandles <= 0)
        {
            DAVA_THROW(Exception, "you should set hints.numOfMaxEasyHandles > 0");
        }
        if (h.chunkMemBuffSize <= 0)
        {
            DAVA_THROW(Exception, "you should set hints.chankMemBuffSize > 0");
        }
        if (!inputList.empty())
        {
            DAVA_THROW(Exception, "you should set hints before start any download task");
        }
        Deinitialize();
        hints = h;
        Initialize();
    }
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
                    Semaphore* sem = it->semaphore;
                    waitingTaskList.erase(it);
                    sem->Post(1);
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
    CURL* curlHandle = nullptr;

    if (reusableHandles.empty())
    {
        // https://curl.haxx.se/libcurl/c/curl_easy_init.html
        curlHandle = curl_easy_init();
        if (curlHandle == nullptr)
        {
            DAVA_THROW(Exception, "can't create new easy handle! something bad happened");
        }
    }
    else
    {
        curlHandle = reusableHandles.front();
        reusableHandles.pop_front();
    }

    if (curlHandle == nullptr)
    {
        DAVA_THROW(Exception, "curlHandle is nullptr");
    }

    CURLcode code = CURLE_OK;

    // https://curl.haxx.se/libcurl/c/CURLOPT_SSL_VERIFYPEER.html
    code = curl_easy_setopt(curlHandle, CURLOPT_SSL_VERIFYPEER, 0L);
    if (CURLE_OK != code)
    {
        const char* strErr = curl_easy_strerror(code);
        DAVA_THROW(Exception, strErr);
    }

    // https://curl.haxx.se/libcurl/c/CURLOPT_FOLLOWLOCATION.html
    code = curl_easy_setopt(curlHandle, CURLOPT_FOLLOWLOCATION, 1L);
    if (CURLE_OK != code)
    {
        const char* strErr = curl_easy_strerror(code);
        DAVA_THROW(Exception, strErr);
    }

    // https://curl.haxx.se/libcurl/c/CURLOPT_TCP_KEEPALIVE.html
    code = curl_easy_setopt(curlHandle, CURLOPT_TCP_KEEPALIVE, 1L);
    if (CURLE_OK != code)
    {
        const char* strErr = curl_easy_strerror(code);
        DAVA_THROW(Exception, strErr);
    }

    ++numOfRunningSubTasks;

    return curlHandle;
}

void DLCDownloaderImpl::CurlDeleteHandle(CURL* easy)
{
    if (easy == nullptr)
    {
        DAVA_THROW(Exception, "easy is nullptr");
    }

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
    if (numFree < 0)
    {
        DAVA_THROW(Exception, "can't be!!! algorithm is broken, see downloader thread function");
    }
    return numFree;
}

void DLCDownloaderImpl::Map(CURL* easy, IDownloaderSubTask& subTask)
{
    if (easy == nullptr)
    {
        DAVA_THROW(Exception, "easy is nullptr");
    }
    taskMap.emplace(easy, &subTask);
}

IDownloaderSubTask& DLCDownloaderImpl::FindInMap(CURL* easy)
{
    if (easy == nullptr)
    {
        DAVA_THROW(Exception, "easy is nullptr");
    }
    auto it = taskMap.find(easy);
    if (it != end(taskMap))
    {
        IDownloaderSubTask* subTask = it->second;
        if (subTask != nullptr)
        {
            return *subTask;
        }
    }
    DAVA_THROW(Exception, "can't find easy handle in map");
}

void DLCDownloaderImpl::UnMap(CURL* easy)
{
    if (easy == nullptr)
    {
        DAVA_THROW(Exception, "easy is nullptr");
    }
    taskMap.erase(easy);
}

int DLCDownloaderImpl::GetChankSize()
{
    return hints.chunkMemBuffSize;
}

void DLCDownloaderImpl::DeleteSubTaskHandler(IDownloaderSubTask* t)
{
    CURL* easy = t->GetEasyHandle();
    if (easy != nullptr)
    {
        size_t numOfRemovedElements = taskMap.erase(easy);
        if (numOfRemovedElements != 1)
        {
            DAVA_THROW(Exception, "one element should be in map, something bad is happened.");
        }

        CURLMcode r = curl_multi_remove_handle(multiHandle, easy);
        if (r != CURLM_OK)
        {
            const char* strErr = curl_multi_strerror(r);
            DAVA_THROW(Exception, strErr);
        }

        CurlDeleteHandle(easy);
    }
    else
    {
        DAVA_THROW(Exception, "bad easy subtask handler, something bad is happened");
    }
}

void DLCDownloaderImpl::DeleteTask(Task* task)
{
    if (task->status.state == TaskState::Downloading)
    {
        for (auto& t : task->subTasksWorking)
        {
            DeleteSubTaskHandler(t);
        }

        task->subTasksWorking.clear();

        for (auto& t : task->subTasksReadyToWrite)
        {
            DeleteSubTaskHandler(t);
        }

        task->subTasksReadyToWrite.clear();
    }

    tasks.remove(task);

    delete task;
}

void DLCDownloader::Task::GenerateChankSubRequests(const int chankSize)
{
    while (NeedHandle() && curlStorage.GetFreeHandleCount() > 0)
    {
        if (restSize < chankSize)
        {
            // take rest range
            IDownloaderSubTask* subTask = new DownloadChunkSubTask(*this, restOffset, restSize);
            subTasksWorking.push_back(subTask);
            restOffset += restSize;
            restSize = 0;
            break;
        }

        IDownloaderSubTask* subTask = new DownloadChunkSubTask(*this, restOffset, chankSize);
        subTasksWorking.push_back(subTask);
        restOffset += chankSize;
        restSize -= chankSize;
    }
}

void DLCDownloader::Task::SetupFullDownload()
{
    CURLcode code = CURLE_OK;

    if (!writer)
    {
        try
        {
            writer.reset(new DefaultWriter(info.dstPath));
        }
        catch (std::exception& ex)
        {
            OnErrorCurlErrno(errno, this, nullptr);
            Logger::Error("%s", ex.what());
            return;
        }
    }
    if (!writer->Truncate())
    {
        OnErrorCurlErrno(errno, this, nullptr);
        return;
    }

    if (info.rangeOffset != -1 && info.rangeSize != -1)
    {
        // we already know size to download
        restOffset = info.rangeOffset;
        restSize = info.rangeSize;
        const int chankSize = curlStorage.GetChankSize();

        GenerateChankSubRequests(chankSize);
    }
    else
    {
        // first get size of full file
        IDownloaderSubTask* subTask = new GetSizeSubTask(*this);
        subTasksWorking.push_back(subTask);
    }
}

void DLCDownloader::Task::CorrectRangeToResumeDownloading()
{
    uint64 pos = writer->GetSeekPos();
    if (pos != std::numeric_limits<uint64>::max())
    {
        restOffset += pos;
        restSize -= pos;
    }
    else
    {
        OnErrorCurlErrno(errno, this, nullptr);
    }
}

void DLCDownloader::Task::SetupResumeDownload()
{
    if (writer.get() == nullptr)
    {
        DefaultWriter* w = nullptr;
        try
        {
            w = new DefaultWriter(info.dstPath);
        }
        catch (Exception& ex)
        {
            OnErrorCurlErrno(errno, this, nullptr);
            Logger::Error("%s %s %d", ex.what(), ex.file.c_str(), ex.line);
            return;
        }
        if (w != nullptr)
        {
            writer.reset(w);
            w->MoveToEndOfFile();
        }
    }

    if (info.rangeOffset != -1 && info.rangeSize != -1)
    {
        // we already know size to download
        // so correct range to download only rest of file
        CorrectRangeToResumeDownloading();
    }
    else
    {
        // first get size of full file
        IDownloaderSubTask* subTask = new GetSizeSubTask(*this);
        subTasksWorking.push_back(subTask);
    }
}

void DLCDownloader::Task::SetupGetSizeDownload()
{
    IDownloaderSubTask* subTask = new GetSizeSubTask(*this);

    subTasksWorking.push_back(subTask);
}

bool DLCDownloaderImpl::TakeNewTaskFromInputList()
{
    Task* task = AddOneMoreTask();

    if (task != nullptr)
    {
        task->PrepareForDownloading();
        tasks.push_back(task);
        return true;
    }
    return false;
}

void DLCDownloaderImpl::SignalOnFinishedWaitingTasks()
{
    if (!waitingTaskList.empty())
    {
        LockGuard<Mutex> lock(mutexWaitingList);
        waitingTaskList.remove_if([this](WaitingDescTask& wt) {
            if (wt.task != nullptr)
            {
                if (wt.task->status.state == TaskState::Finished)
                {
                    wt.semaphore->Post();
                    return true;
                }
            }
            else
            {
                DAVA_THROW(Exception, "task is nullptr, something bad happened");
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

            IDownloaderSubTask& subTask = FindInMap(easyHandle);
            Task& task = subTask.GetTask();

            subTask.OnDone(curlMsg);
            if (!task.status.error.errorHappened)
            {
                task.subTasksWorking.remove(&subTask);
                task.subTasksReadyToWrite.push_back(&subTask);

                task.OnSubTaskDone();
                if (!task.status.error.errorHappened)
                {
                    task.GenerateChankSubRequests(hints.chunkMemBuffSize);

                    if (task.IsDone())
                    {
                        task.FlushWriterAndReset();
                        task.status.state = TaskState::Finished;
                    }
                }
            }
        }
    } while (curlMsg != nullptr);
}

void DLCDownloaderImpl::BalancingHandles()
{
    if (GetFreeHandleCount() > 0)
    {
        // find first not finished task
        for (auto task : tasks)
        {
            if (task->NeedHandle())
            {
                task->GenerateChankSubRequests(hints.chunkMemBuffSize);
                if (GetFreeHandleCount() == 0)
                {
                    break;
                }
            }
        }
    }
}

void DLCDownloaderImpl::Task::OnErrorCurlMulti(int32 multiCode, Task* task, CURLM* multi, CURL* easy)
{
    DVASSERT(multiCode != 0);
    DVASSERT(task != nullptr);
    DVASSERT(multi != nullptr);
    DVASSERT(easy != nullptr);

    if (task->status.state == TaskState::Finished)
    {
        return; // do not overwrite previous error state
    }

    task->status.error.errorHappened = true;
    task->status.error.curlMErr = multiCode;
    // static string literal from curl
    task->status.error.errStr = curl_multi_strerror(static_cast<CURLMcode>(multiCode));
    task->status.state = TaskState::Finished;
}
void DLCDownloaderImpl::Task::OnErrorCurlEasy(int32 easyCode, Task* task, IDownloaderSubTask* subTask)
{
    DVASSERT(easyCode != 0);
    DVASSERT(task != nullptr);

    if (task->status.state == TaskState::Finished)
    {
        return; // do not overwrite previous error state
    }

    task->status.error.errorHappened = true;
    task->status.error.curlErr = easyCode;
    // static string literal from curl
    task->status.error.errStr = curl_easy_strerror(static_cast<CURLcode>(easyCode));
    task->status.state = TaskState::Finished;
}
void DLCDownloaderImpl::Task::OnErrorCurlErrno(int32 errnoVal, Task* task, IDownloaderSubTask* subTask)
{
    DVASSERT(errnoVal != 0);
    DVASSERT(task != nullptr);

    if (task->status.state == TaskState::Finished)
    {
        return; // do not overwrite previous error state
    }

    task->status.error.errorHappened = true;
    task->status.error.fileErrno = errnoVal;
    // if other thread call strerror and change internal buffer - it will not crush still,
    // and we have fileErrno saved, so I am satisfied
    task->status.error.errStr = strerror(errnoVal);
    task->status.state = TaskState::Finished;
}

void DLCDownloaderImpl::Task::OnErrorHttpCode(long httpCode, Task* task, IDownloaderSubTask* subTask)
{
    if (task->status.state == TaskState::Finished)
    {
        return; // do not overwrite previous error state
    }

    task->status.error.errorHappened = true;
    task->status.error.httpCode = static_cast<int32>(httpCode);
    // if other thread call strerror and change internal buffer - it will not crush still,
    // and we have fileErrno saved, so I am satisfied
    task->status.error.errStr = "bad http result code";
    task->status.state = TaskState::Finished;
}

void DLCDownloaderImpl::DownloadThreadFunc()
{
    try
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
                int numOfCurlWorkingHandles = CurlPerform();

                ProcessMessagesFromMulti();

                if (numOfCurlWorkingHandles == 0 && numOfRunningSubTasks == 0)
                {
                    downloading = false;
                }

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
    catch (Exception& ex)
    {
        Logger::Error("%s(%d)", ex.file.c_str(), ex.line);
        throw;
    }
}

int DLCDownloaderImpl::CurlPerform()
{
    int stillRunning = 0;

    // from https://curl.haxx.se/libcurl/c/curl_multi_perform.html
    CURLMcode code = curl_multi_perform(multiHandle, &stillRunning);
    if (code != CURLM_OK)
    {
        const char* strErr = curl_multi_strerror(code);
        Logger::Error("curl_multi_perform failed: %s", strErr);
        DAVA_THROW(Exception, strErr);
    }

    // wait for activity, timeout or "nothing"
    // from https://curl.haxx.se/libcurl/c/curl_multi_wait.html
    code = curl_multi_wait(multiHandle, nullptr, 0, 1000, nullptr);
    if (code != CURLM_OK)
    {
        const char* strErr = curl_multi_strerror(code);
        Logger::Error("curl_multi_wait failed: %s", strErr);
        DAVA_THROW(Exception, strErr);
    }

    // if there are still transfers, loop!
    return stillRunning;
}

} // end namespace DAVA
