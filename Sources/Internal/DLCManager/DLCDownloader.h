#pragma once

#include "Base/BaseTypes.h"
#include "FileSystem/FilePath.h"
#include "Functional/Signal.h"

namespace DAVA
{
class DLCDownloader
{
public:
    virtual ~DLCDownloader() = default;

    enum class TaskState
    {
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
        virtual size_t Save(const void* ptr, size_t size) = 0;
    };

    struct TaskInfo
    {
        String srcUrl;
        FilePath dstPath;
        TaskType type;
        int32 partsCount = -1;
        int32 timeoutSec = 30;
        int32 retriesCount = 3;
        uint64 downloadOffset = 0;
        uint64 downloadSize = 0;
        IWriter* writer = nullptr;
    };

    struct TaskError
    {
        enum Error
        {
            NO_ERROR_,
            SOME_ERROR_
        };
        Error error = NO_ERROR_;
        int32 curlErr = 0;
        int32 errnoVal = 0;
    };

    struct TaskStatus
    {
        uint32 id;
        int32 fileErrno;
        int32 retriesLeft;
        TaskState state;
        TaskError error;
        uint64 sizeTotal;
        uint64 sizeDownloaded;
    };

    struct Task;

    // Schedule download content or get content size (indicated by downloadMode)
    virtual Task* StartTask(
    const String& srcUrl,
    IWriter* dstWriter,
    TaskType taskType,
    uint64 rangeOffset = 0,
    uint64 rangeSize = 0,
    int16 partsCount = -1,
    int32 timeout = 30,
    int32 retriesCount = 3
    ) = 0;

    // Cancel download by ID (works for scheduled and current)
    virtual void RemoveTask(Task* task) = 0;

    // wait for task status = finished
    virtual void WaitTask(Task* task) = 0;

    virtual const TaskInfo* GetTaskInfo(Task* task) = 0;
    virtual TaskStatus GetTaskStatus(Task* task) = 0;

    // Signal about download task state changing
    Signal<uint32, const TaskStatus&> taskStatusChanged;
};
}
