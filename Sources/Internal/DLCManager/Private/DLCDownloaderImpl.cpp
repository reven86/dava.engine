#include "DLCManager/Private/DLCDownloaderImpl.h"

#include "DLC/Downloader/CurlDownloader.h"

#include <curl/curl.h>

namespace DAVA
{
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

    for (CURL* easy : easyHandlesAll)
    {
        curl_easy_cleanup(easy);
    }
    easyHandlesAll.clear();

    if (!CurlDownloader::isCURLInit)
    {
        curl_global_cleanup();
    }
}

DLCDownloader::Task* DLCDownloaderImpl::StartTask(const String& srcUrl,
                                                  IWriter* dstWriter,
                                                  TaskType taskType,
                                                  uint64 rangeOffset,
                                                  uint64 rangeSize,
                                                  int16 partsCount,
                                                  int32 timeout,
                                                  int32 retriesCount)
{
    if (dstWriter == nullptr)
    {
        if (taskType != TaskType::SIZE)
        {
            return nullptr;
        }
    }

    Task* task = new Task();

    auto& info = task->info;

    info.retriesCount = retriesCount;
    info.downloadOffset = rangeOffset;
    info.downloadSize = rangeSize;
    info.partsCount = (partsCount == -1) ? 1 : partsCount;
    info.srcUrl = srcUrl;
    info.timeoutSec = timeout;
    info.type = taskType;
    info.writer = dstWriter;

    auto& state = task->status;
    state.state = TaskState::Downloading;
    state.error = TaskError();
    state.fileErrno = 0;
    state.id = 0;
    state.retriesLeft = info.retriesCount;
    state.sizeDownloaded = 0;
    state.sizeTotal = info.downloadSize;

    {
        LockGuard<Mutex> lock(mutex);
        taskQueue.push_back(task);
    }

    downloadSem.Post(1);

    return task;
}

void DLCDownloaderImpl::RemoveTask(Task* task)
{
    LockGuard<Mutex> lock(mutex);

    if (task->status.state == TaskState::Downloading)
    {
        for (CURL* easy : task->easyHandles)
        {
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
        Thread::Sleep(1); // TODO reimplement it!
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
        LockGuard<Mutex> lock(mutex);
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

static CURL* CurlSimpleInitHandle()
{
    CURL* curl_handle = curl_easy_init();
    DVASSERT(curl_handle != nullptr);

    CURLcode code = CURLE_OK;
    code = curl_easy_setopt(curl_handle, CURLOPT_NOPROGRESS, 1L);
    DVASSERT(CURLE_OK == code);

    code = curl_easy_setopt(curl_handle, CURLOPT_VERBOSE, 0L);
    DVASSERT(CURLE_OK == code);

    code = curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, "Mozilla/5.0 (Windows; U; Windows NT 5.1; en-US; rv:1.8.1.11) Gecko/20071127 Firefox/2.0.0.11");
    DVASSERT(CURLE_OK == code);

    code = curl_easy_setopt(curl_handle, CURLOPT_SSL_VERIFYPEER, 0L);
    DVASSERT(CURLE_OK == code);

    code = curl_easy_setopt(curl_handle, CURLOPT_FOLLOWLOCATION, 1L);
    DVASSERT(CURLE_OK == code);
    return curl_handle;
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

static void SetupFullDownload(DLCDownloader::Task* justAddedTask)
{
    CURLcode code = CURLE_OK;

    CURL* easyHandle = CurlSimpleInitHandle();
    DVASSERT(easyHandle != nullptr);

    justAddedTask->easyHandles.push_back(easyHandle);

    DLCDownloader::IWriter* writer = justAddedTask->info.writer;
    DVASSERT(writer != nullptr);

    const char* url = justAddedTask->info.srcUrl.c_str();
    DVASSERT(url != nullptr);

    code = curl_easy_setopt(easyHandle, CURLOPT_URL, url);
    DVASSERT(code == CURLE_OK);

    code = curl_easy_setopt(easyHandle, CURLOPT_WRITEFUNCTION, CurlDataRecvHandler);
    DVASSERT(code == CURLE_OK);

    code = curl_easy_setopt(easyHandle, CURLOPT_WRITEDATA, writer);
    DVASSERT(code == CURLE_OK);

    // set all timeouts
    CurlSetTimeout(justAddedTask, easyHandle);
}

static void SetupResumeDownload(DLCDownloader::Task* justAddedTask)
{
    CURLcode code = CURLE_OK;

    CURL* easyHandle = CurlSimpleInitHandle();
    DVASSERT(easyHandle != nullptr);

    justAddedTask->easyHandles.push_back(easyHandle);

    DLCDownloader::IWriter* writer = justAddedTask->info.writer;
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

static void SetupGetSizeDownload(DLCDownloader::Task* justAddedTask)
{
    CURLcode code = CURLE_OK;

    CURL* easyHandle = CurlSimpleInitHandle();
    DVASSERT(easyHandle != nullptr);

    justAddedTask->easyHandles.push_back(easyHandle);

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

static bool CurlPerform(CURLM* multiHandle);

void DLCDownloaderImpl::DownloadThreadFunc()
{
    Thread* currentThread = Thread::Current();
    DVASSERT(currentThread != nullptr);

    while (!currentThread->IsCancelling())
    {
        downloadSem.Wait();

        Task* justAddedTask = nullptr;

        {
            LockGuard<Mutex> lock(mutex);
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
                if (CURLM_OK != curl_multi_add_handle(multiHandle, easyHandle))
                {
                    DVASSERT(false);
                }
            }
        }

        bool allDone = false;

        while (!allDone)
        {
            allDone = CurlPerform(multiHandle);
            // TODO check more incoming tasks?
        }
    } // !currentThread->IsCancelling()
}

// from https://curl.haxx.se/libcurl/c/curl_multi_perform.html
static bool CurlPerform(CURLM* multiHandle)
{
    int stillRunning = 0;
    long curl_timeo;

    CURLMcode code = curl_multi_timeout(multiHandle, &curl_timeo);
    DVASSERT(CURLE_OK == code);

    if (curl_timeo < 0)
    {
        curl_timeo = 1000;
    }

    timeval timeout;

    timeout.tv_sec = curl_timeo / 1000;
    timeout.tv_usec = (curl_timeo % 1000) * 1000;

    fd_set fdread;
    fd_set fdwrite;
    fd_set fdexcep;

    FD_ZERO(&fdread);
    FD_ZERO(&fdwrite);
    FD_ZERO(&fdexcep);

    int maxfd = -1;
    /* get file descriptors from the transfers */
    code = curl_multi_fdset(multiHandle, &fdread, &fdwrite, &fdexcep, &maxfd);
    DVASSERT(CURLE_OK == code);

    int rc = -1; /* select() return code */

    if (maxfd == -1)
    {
        rc = 0;
    }
    else
    {
        rc = select(maxfd + 1, &fdread, &fdwrite, &fdexcep, &timeout);
    }

    switch (rc)
    {
    case -1:
        /* select error */
        break;
    case 0:
    default:
        /* timeout or readable/writable sockets */
        code = curl_multi_perform(multiHandle, &stillRunning);
        DVASSERT(CURLE_OK == code);
        break;
    }

    /* if there are still transfers, loop! */
    return stillRunning == 1;
}

} // end namespace DAVA
