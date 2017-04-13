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
        TaskType type;
        int32 partsCount = -1;
        int32 timeoutSec = 30;
        int32 retriesCount = 3;
        int64 rangeOffset = -1;
        int64 rangeSize = -1;
        IWriter* customWriter = nullptr;
    };

    struct TaskError
    {
        int32 curlErr = 0; // CURLE_OK == 0
        int32 curlMErr = 0; // CURLM_OK == 0
        int32 errnoVal = 0;
    };

    struct TaskStatus
    {
        int32 fileErrno;
        int32 retriesLeft;
        TaskState state;
        TaskError error;
        uint64 sizeTotal;
        uint64 sizeDownloaded;
    };

    struct Task;

    // Schedule download content or get content size (indicated by downloadMode)
    virtual Task* StartTask(const String& srcUrl,
                            const String& dstPath,
                            TaskType taskType,
                            IWriter* customWriter = nullptr,
                            int64 rangeOffset = -1,
                            int64 rangeSize = -1,
                            int16 partsCount = -1,
                            int32 timeout = 30,
                            int32 retriesCount = 3) = 0;

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
