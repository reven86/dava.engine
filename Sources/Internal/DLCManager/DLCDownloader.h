#pragma once

#include "Base/BaseTypes.h"
#include "FileSystem/FilePath.h"
#include "Concurrency/Atomic.h"

namespace DAVA
{
class DLCDownloader
{
public:
    virtual ~DLCDownloader();

    static DLCDownloader* Create();
    static void Destroy(DLCDownloader* downloader);

    struct Hints
    {
        int32 numOfMaxEasyHandles = 4;
        int32 chankMemBuffSize = 1024 * 1024;
    };

    enum class TaskState
    {
        JustAdded,
        Downloading,
        Finished
    };

    enum class TaskType
    {
        FULL,
        RESUME,
        SIZE
    };

    struct IWriter
    {
        virtual ~IWriter() = default;
        /** save next buffer bytes into memory or file */
        virtual uint64 Save(const void* ptr, uint64 size) = 0;
        /** return current size of saved byte stream */
        virtual uint64 GetSeekPos() = 0;
        /** truncate file */
        virtual void Truncate() = 0;
        /** return maximum size of memory buffer*/
        virtual uint64 SpaceLeft() = 0;
    };

    struct TaskInfo
    {
        String srcUrl;
        String dstPath;
        TaskType type = TaskType::FULL;
        int32 timeoutSec = 30;
        int64 rangeOffset = -1;
        int64 rangeSize = -1;
    };

    struct TaskError
    {
        int32 curlErr = 0; // CURLE_OK == 0
        int32 curlMErr = 0; // CURLM_OK == 0
        int32 fileErrno = 0;
        const char* curlEasyStrErr = nullptr; // see curl_easy_strerr
    };

    struct TaskStatus
    {
        Atomic<TaskState> state = TaskState::JustAdded;
        TaskError error;
        uint64 sizeTotal = 0;
        uint64 sizeDownloaded = 0;

        TaskStatus();
        TaskStatus(const TaskStatus& other);
        TaskStatus& operator=(const TaskStatus& other);
    };

    struct Task;

    // Schedule download content or get content size (indicated by downloadMode)
    virtual Task* StartTask(const String& srcUrl,
                            const String& dstPath,
                            TaskType taskType,
                            IWriter* customWriter = nullptr,
                            int64 rangeOffset = -1,
                            int64 rangeSize = -1,
                            int32 timeout = 30) = 0;

    //  task pointer become an invalid after this function
    virtual void RemoveTask(Task* task) = 0;

    // wait for task status == finished
    virtual void WaitTask(Task* task) = 0;

    virtual const TaskInfo& GetTaskInfo(Task* task) = 0;
    virtual const TaskStatus& GetTaskStatus(Task* task) = 0;

    virtual void SetHints(const Hints& h) = 0;
};
}
