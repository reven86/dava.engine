#include "DLCManager/Private/DLCDownloaderImpl.h"

#include "DLC/Downloader/CurlDownloader.h"

#include "Logger/Logger.h"
#include "FileSystem/File.h"
#include "FileSystem/FileSystem.h"
#include "Concurrency/LockGuard.h"

#include <curl/curl.h>

namespace DAVA
{
static const size_t numOfMaxEasyHandles = 128;

struct DefaultWriter : DLCDownloader::IWriter
{
    DefaultWriter(const String& outputFile)
    {
        FileSystem* fs = FileSystem::Instance();
        FilePath path = outputFile;
        fs->CreateDirectory(path.GetDirectory(), true);
        f = File::Create(outputFile, File::OPEN | File::WRITE | File::CREATE);
        if (!f)
        {
            int32 errNo = errno;
            const char* err = strerror(errNo);
            DAVA_THROW(Exception, "can't create output file: " + outputFile + " " + err);
        }
    }
    ~DefaultWriter()
    {
        f->Release();
        f = nullptr;
    }

    /** save next buffer bytes into memory or file */
    uint64 Save(const void* ptr, uint64 size) override
    {
        uint32 writen = f->Write(ptr, static_cast<uint32>(size));
        if (writen != size)
        {
            DAVA_THROW(Exception, "bad write");
        }
        return writen;
    }
    /** return current size of saved byte stream */
    uint64 GetSeekPos() override
    {
        return f->GetPos();
    }

    void Truncate() override
    {
        f->Truncate(0);
    }

    uint64 SpaceLeft() override
    {
        return std::numeric_limits<uint64>::max();
    }

private:
    File* f = nullptr;
};

// ONLY use this global variable from DLCDownloader thread
static UnorderedMap<CURL*, DLCDownloader::Task*> taskMap;
Mutex mutexTaskMap;

DLCDownloaderImpl::DLCDownloaderImpl()
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

DLCDownloaderImpl::~DLCDownloaderImpl()
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

DLCDownloader::Task* DLCDownloaderImpl::StartTask(const String& srcUrl,
                                                  const String& dstPath,
                                                  TaskType taskType,
                                                  IWriter* dstWriter,
                                                  int64 rangeOffset,
                                                  int64 rangeSize,
                                                  int16 partsCount,
                                                  int32 timeout,
                                                  int32 retriesCount)
{
    Task* task = new Task();

    auto& info = task->info;

    info.retriesCount = retriesCount;
    info.rangeOffset = rangeOffset;
    info.rangeSize = rangeSize;
    info.partsCount = (partsCount == -1) ? 1 : partsCount;
    info.srcUrl = srcUrl;
    info.dstPath = dstPath;
    info.timeoutSec = timeout;
    info.type = taskType;
    info.customWriter = dstWriter;

    auto& state = task->status;
    state.state = TaskState::JustAdded;
    state.error = TaskError();
    state.fileErrno = 0;
    state.retriesLeft = info.retriesCount;
    state.sizeDownloaded = 0;
    state.sizeTotal = info.rangeSize;

    {
        LockGuard<Mutex> lock(mutexTaskQueue);
        taskQueue.push_back(task);
    }

    ++numOfNewTasks;

    downloadSem.Post(1);

    return task;
}

void DLCDownloaderImpl::RemoveTask(Task* task)
{
    LockGuard<Mutex> lock(mutexTaskQueue);

    if (task->status.state == TaskState::Downloading)
    {
        for (CURL* easy : task->easyHandles)
        {
            {
                LockGuard<Mutex> l(mutexTaskMap);
                taskMap.erase(easy);
            }

            CURLMcode r = curl_multi_remove_handle(multiHandle, easy);
            DVASSERT(CURLM_OK == r);

            curl_easy_cleanup(easy);
        }
        task->easyHandles.clear();
    }
    auto it = std::find(begin(taskQueue), end(taskQueue), task);
    if (it != end(taskQueue))
    {
        taskQueue.erase(it);
    }

    delete task;
}

void DLCDownloaderImpl::WaitTask(Task* task)
{
    while (true)
    {
        TaskStatus status = GetTaskStatus(task);
        if (status.state == TaskState::Finished)
        {
            break;
        }
        Thread::Sleep(10); // TODO reimplement it!
    }
}

const DLCDownloader::TaskInfo* DLCDownloaderImpl::GetTaskInfo(Task* task)
{
    return &task->info;
}

DLCDownloader::TaskStatus DLCDownloaderImpl::GetTaskStatus(Task* task)
{
    TaskStatus status;
    {
        LockGuard<Mutex> lock(mutexTaskQueue);
        status = task->status;
    }

    return status;
}

static DLCDownloader::Task* FindJustEddedTask(Deque<DLCDownloader::Task*>& taskQueue)
{
    for (auto task : taskQueue)
    {
        if (task->status.state == DLCDownloader::TaskState::JustAdded)
        {
            return task;
        }
    }
    return nullptr;
}

CURL* DLCDownloaderImpl::CurlCreateHandle()
{
    CURL* curl_handle = nullptr;

    if (reusableHandles.empty())
    {
        curl_handle = curl_easy_init();
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
    }
    else
    {
        curl_handle = reusableHandles.front();
        reusableHandles.pop_front();
    }

    DVASSERT(curl_handle != nullptr);
    return curl_handle;
}

void DLCDownloaderImpl::CurlDeleteHandle(CURL* easy)
{
    reusableHandles.push_back(easy);
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
    DLCDownloader::IWriter* writer = static_cast<DLCDownloader::IWriter*>(part);
    DVASSERT(writer != nullptr);

    size_t fullSizeToWrite = size * nmemb;

    uint64 writen = 0;

    size_t spaceLeft = static_cast<size_t>(writer->SpaceLeft());
    if (spaceLeft >= fullSizeToWrite)
    {
        writen = writer->Save(ptr, fullSizeToWrite);
    }
    else
    {
        writen = writer->Save(ptr, spaceLeft);
    }

    return static_cast<size_t>(writen);
}

static void StoreHandle(DLCDownloader::Task* justAddedTask, CURL* easyHandle)
{
    justAddedTask->easyHandles.emplace(easyHandle);
    {
        LockGuard<Mutex> l(mutexTaskMap);
        taskMap.emplace(easyHandle, justAddedTask);
    }
}

void DLCDownloaderImpl::SetupFullDownload(Task* justAddedTask)
{
    CURLcode code = CURLE_OK;

    auto& info = justAddedTask->info;

    if (info.customWriter == nullptr && TaskType::SIZE != info.type)
    {
        justAddedTask->defaultWriter.reset(new DefaultWriter(info.dstPath));
    }

    CURL* easyHandle = CurlCreateHandle();
    DVASSERT(easyHandle != nullptr);

    StoreHandle(justAddedTask, easyHandle);

    IWriter* writer = info.customWriter != nullptr ? info.customWriter : justAddedTask->defaultWriter.get();
    DVASSERT(writer != nullptr);

    const char* url = justAddedTask->info.srcUrl.c_str();
    DVASSERT(url != nullptr);

    code = curl_easy_setopt(easyHandle, CURLOPT_URL, url);
    DVASSERT(code == CURLE_OK);

    code = curl_easy_setopt(easyHandle, CURLOPT_WRITEFUNCTION, CurlDataRecvHandler);
    DVASSERT(code == CURLE_OK);

    code = curl_easy_setopt(easyHandle, CURLOPT_WRITEDATA, writer);
    DVASSERT(code == CURLE_OK);

    bool hasRangeStart = info.rangeOffset != -1;
    bool hasRangeFinish = info.rangeSize != -1;

    if (hasRangeStart || hasRangeFinish)
    {
        StringStream ss;
        if (hasRangeStart)
        {
            ss << info.rangeOffset;
        }
        ss << '-';
        if (hasRangeFinish)
        {
            DVASSERT(info.rangeSize > 0);
            ss << info.rangeOffset + info.rangeSize - 1;
        }
        String s = ss.str();
        code = curl_easy_setopt(easyHandle, CURLOPT_RANGE, s.c_str());
        DVASSERT(code == CURLE_OK);
    }

    // set all timeouts
    CurlSetTimeout(justAddedTask, easyHandle);
}

void DLCDownloaderImpl::SetupResumeDownload(Task* justAddedTask)
{
    CURLcode code = CURLE_OK;

    CURL* easyHandle = CurlCreateHandle();
    DVASSERT(easyHandle != nullptr);

    StoreHandle(justAddedTask, easyHandle);

    auto& info = justAddedTask->info;

    if (info.customWriter == nullptr && TaskType::SIZE != info.type)
    {
        justAddedTask->defaultWriter.reset(new DefaultWriter(info.dstPath));
    }

    IWriter* writer = info.customWriter != nullptr ? info.customWriter : justAddedTask->defaultWriter.get();
    DVASSERT(writer != nullptr);

    const char* url = justAddedTask->info.srcUrl.c_str();
    DVASSERT(url != nullptr);

    code = curl_easy_setopt(easyHandle, CURLOPT_URL, url);
    DVASSERT(code == CURLE_OK);

    code = curl_easy_setopt(easyHandle, CURLOPT_WRITEFUNCTION, CurlDataRecvHandler);
    DVASSERT(code == CURLE_OK);

    code = curl_easy_setopt(easyHandle, CURLOPT_WRITEDATA, writer);
    DVASSERT(code == CURLE_OK);

    {
        char8 rangeStr[32];
        sprintf(rangeStr, "%lld-", writer->GetSeekPos());
        code = curl_easy_setopt(easyHandle, CURLOPT_RANGE, rangeStr);
        DVASSERT(code == CURLE_OK);
    }

    // set all timeouts
    CurlSetTimeout(justAddedTask, easyHandle);
}

void DLCDownloaderImpl::SetupGetSizeDownload(DLCDownloader::Task* justAddedTask)
{
    CURLcode code = CURLE_OK;

    CURL* easyHandle = CurlCreateHandle();
    DVASSERT(easyHandle != nullptr);

    StoreHandle(justAddedTask, easyHandle);

    code = curl_easy_setopt(easyHandle, CURLOPT_HEADER, 0L);
    DVASSERT(CURLE_OK == code);

    const char* url = justAddedTask->info.srcUrl.c_str();
    DVASSERT(url != nullptr);

    code = curl_easy_setopt(easyHandle, CURLOPT_URL, url);
    DVASSERT(CURLE_OK == code);

    // Don't return the header (we'll use curl_getinfo();
    code = curl_easy_setopt(easyHandle, CURLOPT_NOBODY, 1L);
    DVASSERT(CURLE_OK == code);
}

// return number of still running handles
static size_t CurlPerform(CURLM* multiHandle);

bool DLCDownloaderImpl::TakeOneNewTaskFromQueue()
{
    Task* justAddedTask = nullptr;

    {
        LockGuard<Mutex> lock(mutexTaskQueue);
        justAddedTask = FindJustEddedTask(taskQueue);
    }

    if (justAddedTask != nullptr)
    {
        switch (justAddedTask->info.type)
        {
        case TaskType::FULL:
            SetupFullDownload(justAddedTask);
            break;
        case TaskType::RESUME:
            SetupResumeDownload(justAddedTask);
            break;
        case TaskType::SIZE:
            SetupGetSizeDownload(justAddedTask);
            break;
        }

        for (CURL* easyHandle : justAddedTask->easyHandles)
        {
            CURLMcode code = curl_multi_add_handle(multiHandle, easyHandle);
            DVASSERT(CURLM_OK == code);
        }
        justAddedTask->status.state = TaskState::Downloading;
    }
    return justAddedTask != nullptr;
}

void DLCDownloaderImpl::DownloadThreadFunc()
{
    const int maxSimultaniousDownloads = 8;
    Thread* currentThread = Thread::Current();
    DVASSERT(currentThread != nullptr);

    bool stillRuning = false;
    int numOfRunningHandles = 0;
    int numOfAddedTasks = 0;

    while (!currentThread->IsCancelling())
    {
        if (!stillRuning)
        {
            downloadSem.Wait();
            stillRuning = true;
        }

        if (numOfNewTasks > 0 && numOfAddedTasks < maxSimultaniousDownloads)
        {
            while (numOfAddedTasks < maxSimultaniousDownloads)
            {
                bool justAdded = TakeOneNewTaskFromQueue();
                if (!justAdded)
                {
                    break; // no more new tasks
                }
                ++numOfAddedTasks;
                --numOfNewTasks;
            }
        }

        while (stillRuning)
        {
            numOfRunningHandles = CurlPerform(multiHandle);

            if (numOfRunningHandles == 0)
            {
                stillRuning = false;
            }

            CURLMsg* curlMsg = nullptr;

            /* call curl_multi_perform or curl_multi_socket_action first, then loop
			through and check if there are any transfers that have completed */
            do
            {
                int msgq = 0;
                curlMsg = curl_multi_info_read(multiHandle, &msgq);
                if (curlMsg && (curlMsg->msg == CURLMSG_DONE))
                {
                    CURL* easyHandle = curlMsg->easy_handle;
                    Task* finishedTask = nullptr;

                    {
                        LockGuard<Mutex> l(mutexTaskMap);
                        auto it = taskMap.find(easyHandle);
                        DVASSERT(it != end(taskMap));

                        finishedTask = it->second;

                        taskMap.erase(it);
                    }

                    finishedTask->status.error.curlErr = curlMsg->data.result;

                    {
                        float64 sizeToDownload = 0.0; // curl need double! do not change https://curl.haxx.se/libcurl/c/CURLINFO_CONTENT_LENGTH_DOWNLOAD.html
                        CURLcode code = curl_easy_getinfo(easyHandle, CURLINFO_CONTENT_LENGTH_DOWNLOAD, &sizeToDownload);
                        DVASSERT(CURLE_OK == code);
                        finishedTask->status.sizeDownloaded = static_cast<uint64>(sizeToDownload);
                        if (finishedTask->info.type == TaskType::SIZE)
                        {
                            finishedTask->status.sizeTotal = finishedTask->status.sizeDownloaded;
                        }
                    }
                    if (curlMsg->data.result != CURLE_OK)
                    {
                        if (curlMsg->data.result == CURLE_PARTIAL_FILE && finishedTask->status.sizeDownloaded == finishedTask->info.rangeSize)
                        {
                            // all good
                        }
                        else
                        {
                            DVASSERT(false); // error downloading
                        }
                    }

                    finishedTask->easyHandles.erase(easyHandle);
                    if (finishedTask->easyHandles.empty())
                    {
                        finishedTask->defaultWriter.reset();
                        finishedTask->status.state = TaskState::Finished;
                        --numOfAddedTasks;
                    }

                    CURLMcode code = curl_multi_remove_handle(multiHandle, easyHandle);
                    DVASSERT(CURLM_OK == code);
                    CurlDeleteHandle(easyHandle);
                }
            } while (curlMsg);

            if (stillRuning)
            {
                if (numOfAddedTasks < maxSimultaniousDownloads)
                {
                    if (numOfNewTasks > 0)
                    {
                        break; // get more new task to do it simultaneously
                    }
                }
            }
        }
    } // !currentThread->IsCancelling()
}

// from https://curl.haxx.se/libcurl/c/curl_multi_perform.html
// from https://curl.haxx.se/libcurl/c/curl_multi_wait.html
static size_t CurlPerform(CURLM* multiHandle)
{
    int stillRunning = 0;
    //long curl_timeo;

    CURLMcode code;
    int numfds;

    code = curl_multi_perform(multiHandle, &stillRunning);
    DVASSERT(code == CURLM_OK);

    if (code == CURLM_OK)
    {
        /* wait for activity, timeout or "nothing" */
        code = curl_multi_wait(multiHandle, nullptr, 0, 1000, &numfds);
    }

    if (code != CURLM_OK)
    {
        Logger::Error("curl_multi failed, code %d.n", code);
        return 0;
    }

    /* if there are still transfers, loop! */
    return static_cast<size_t>(stillRunning);
}

} // end namespace DAVA
