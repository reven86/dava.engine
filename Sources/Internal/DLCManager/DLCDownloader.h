#pragma once

#include "Base/BaseTypes.h"

#include <atomic>

namespace DAVA
{
class DLCDownloader
{
public:
    virtual ~DLCDownloader();
    /** Create new instance of DLCDownloader. You can customize it with
	    ```DLCDownloader::SetHints(const Hints)``` right after creation. */
    static DLCDownloader* Create();
    /** Destroy downloader instance */
    static void Destroy(DLCDownloader* downloader);

    struct Hints
    {
        int32 numOfMaxEasyHandles = 4; //!< how many curl easy handles will be used
        int32 chankMemBuffSize = 1024 * 1024; //!< max buffer size per one download operation per curl easy handler
    };

    enum class TaskState : int32
    {
        JustAdded, //!< only created task waiting for downloader thread.
        Downloading, //!< one or more curl handles is working.
        Finished //!< all done, now you can remove task with ```DLCDownloader::RemoveTask(Task*)```
    };

    enum class TaskType : int32
    {
        FULL, //!< truncate file if exist and download again
        RESUME, //!< resume download from current output file(buffer) size
        SIZE //!< just return size of remote file see ```TaskStatus::sizeTotal```
    };

    struct IWriter
    {
        virtual ~IWriter() = default;
        /** save next buffer bytes into memory or file, on error returned size differ from parameter size */
        virtual uint64 Save(const void* ptr, uint64 size) = 0;
        /** return current size of saved byte stream, return ```std::numeric_limits<uint64>::max()``` value on error */
        virtual uint64 GetSeekPos() = 0;
        /** truncate file(or buffer) to zero length, return false on error */
        virtual bool Truncate() = 0;
    };

    struct TaskInfo
    {
        String srcUrl; //!< URL to download from
        String dstPath; //!< path to file or empty string if loading into custom IWriter
        TaskType type = TaskType::RESUME; //!< type of download action
        int32 timeoutSec = 30; //!< timeout in seconds
        int64 rangeOffset = -1; //!< index of first byte to download from or -1 [0 - first byte index]
        int64 rangeSize = -1; //!< size of downloaded range in bytes
    };

    struct TaskError
    {
        int32 curlErr = 0; //!< CURLE_OK == 0 see https://curl.haxx.se/libcurl/c/libcurl-errors.html
        int32 curlMErr = 0; //!< CURLM_OK == 0 see https://curl.haxx.se/libcurl/c/libcurl-errors.html
        int32 fileErrno = 0; //!< errno value after bad (open|read|write|close|truncate) operation to file
        int32 httpCode = 0; //!< last received HTTP response code
        //!< on see http://en.cppreference.com/w/cpp/error/errno_macros
        const char* errStr = nullptr; //!< see https://curl.haxx.se/libcurl/c/curl_multi_strerror.html
        //!< and https://curl.haxx.se/libcurl/c/curl_easy_strerror.html
        bool errorHappened = false; //!< flag set to true if any error
    };

    struct TaskStatus
    {
        std::atomic<TaskState> state{ TaskState::JustAdded };
        TaskError error; //!< full error info
        uint64 sizeTotal = 0; //!< size of remote file or range size to download
        uint64 sizeDownloaded = 0; //!< size written into IWritable(file)

        TaskStatus();
        TaskStatus(const TaskStatus& other);
        TaskStatus& operator=(const TaskStatus& other);
    };

    struct Task;

    /** Schedule download content or get content size (indicated by downloadMode)*/
    virtual Task* StartTask(const String& srcUrl,
                            const String& dstPath,
                            TaskType taskType,
                            IWriter* customWriter = nullptr,
                            int64 rangeOffset = -1,
                            int64 rangeSize = -1,
                            int32 timeout = 30) = 0;

    /**  Clear task data and free resources */
    virtual void RemoveTask(Task* task) = 0;

    /** Wait for task status == finished */
    virtual void WaitTask(Task* task) = 0;

    virtual const TaskInfo& GetTaskInfo(Task* task) = 0;
    virtual const TaskStatus& GetTaskStatus(Task* task) = 0;

    //
    virtual void SetHints(const Hints& h) = 0;
};
}
