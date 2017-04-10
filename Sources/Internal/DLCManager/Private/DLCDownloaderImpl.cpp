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
}

DLCDownloaderImpl::~DLCDownloaderImpl()
{
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

DLCDownloader::Task* DLCDownloaderImpl::StartTask(
const String& srcUrl,
IWriter* dstWriter,
TaskType taskType,
uint64 rangeOffset = 0,
uint64 rangeSize = 0,
int16 partsCount = -1,
int32 timeout = 30,
int32 retriesCount = 3
)
{
    // TODO implement it
    return nullptr;
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

void DLCDownloaderImpl::DownloadThreadFunc(DLCDownloaderImpl* downloader)
{
    // TODO implement it
}
} // end namespace DAVA
