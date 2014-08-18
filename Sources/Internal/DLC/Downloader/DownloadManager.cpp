/*==================================================================================
    Copyright (c) 2008, binaryzebra
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    * Neither the name of the binaryzebra nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE binaryzebra AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL binaryzebra BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/

#include "FileSystem/File.h"
#include "FileSystem/FileSystem.h"
#include "Platform/Thread.h"
#include "Platform/Mutex.h"

#include "DownloadManager.h"
#include "Downloader.h"

namespace DAVA
{

DownloadManager::CallbackData::CallbackData(uint32 _id, DownloadStatus _status)
    : id(_id)
    , status(_status)
{
    
}

Mutex DownloadManager::currentTaskMutex;

DownloadManager::DownloadManager()
    : thisThread(NULL)
    , isThreadStarted(false)
    , currentTask(NULL)
    , downloader(0)
    , callNotify(NULL)
    , remoteFileSize(0)
    , downloadedTotal(0)
{
}

DownloadManager::~DownloadManager()
{
    isThreadStarted = false;

    if (currentTask)
    {
        Interrupt();
        Wait(currentTask->id);
    }

    ClearAll();

    SafeRelease(thisThread);
    SafeDelete(downloader);
}

void DownloadManager::SetDownloader(Downloader *_downloader)
{
    DVASSERT(true == Thread::IsMainThread());
    DVASSERT(NULL != _downloader);
    
    while(NULL != currentTask)
    {
        Thread::SleepThread(10);
        Update();
    }

    SafeDelete(downloader);
    downloader = _downloader;
}

Downloader *DownloadManager::GetDownloader()
{
    return downloader;
}

void DownloadManager::SetNotificationCallback(DownloadManager::NotifyFunctor callbackFn)
{
    DVASSERT(true == Thread::IsMainThread());

    callNotify = callbackFn;
}

DownloadManager::NotifyFunctor DownloadManager::GetNotificationCallback() const
{
    DVASSERT(true == Thread::IsMainThread());

    return callNotify;
}

void DownloadManager::StartProcessingThread()
{
    // thread should be stopped
    DVASSERT(!isThreadStarted);
    DVASSERT(NULL == thisThread);

    thisThread = Thread::Create(Message(this, &DownloadManager::ThreadFunction));
    isThreadStarted = true;
    thisThread->Start();
}

void DownloadManager::StopProcessingThread()
{
    DVASSERT(true == Thread::IsMainThread());

    isThreadStarted = false;
    thisThread->Join();

    SafeRelease(thisThread);
}

void DownloadManager::Update()
{
    // use it only from Main thread
    DVASSERT(true == Thread::IsMainThread());

    if (!currentTask)
    {
        if (!pendingTaskQueue.empty())
        {
            currentTaskMutex.Lock();
            currentTask = pendingTaskQueue.front();
            DVASSERT(0 < currentTask->id);
            pendingTaskQueue.pop_front();
            currentTaskMutex.Unlock();

            if (!isThreadStarted)
                StartProcessingThread();
        }
    }
    else
    {
        // if task is done and we have no pending tasks - stop processing thread.
        if (DL_FINISHED == currentTask->status)
        {
            PlaceToQueue(doneTaskQueue, currentTask);

            if (pendingTaskQueue.empty())
                StopProcessingThread();

            currentTaskMutex.Lock();
            currentTask = NULL;
            currentTaskMutex.Unlock();
        }
    }

    callbackMutex.Lock();
    if (!callbackMessagesQueue.empty())
    {
        for (Deque<CallbackData>::iterator it = callbackMessagesQueue.begin(); it != callbackMessagesQueue.end();)
        {
           CallbackData cbData = (*it);
           it = callbackMessagesQueue.erase(it);
           callNotify(cbData.id, cbData.status);
        }
    }
    callbackMutex.Unlock();
}

uint32 DownloadManager::Download(const String &srcUrl, const FilePath &storeToFilePath, DownloadType downloadMode, int32 timeout, int32 retriesCount)
{
    DVASSERT(true == Thread::IsMainThread());

    int32 fullTimeout = timeout;
    if (GET_SIZE == downloadMode && timeout >= 2)
        fullTimeout /= 2;

    DownloadTaskDescription *task = new DownloadTaskDescription(srcUrl, storeToFilePath, downloadMode, fullTimeout, retriesCount);
 
    static uint32 prevId = 1;
    task->id = prevId++;

    PlaceToQueue(pendingTaskQueue, task);

    task->status = DL_PENDING;

    return task->id;
}

void DownloadManager::Retry(const uint32 &taskId)
{
    DVASSERT(true == Thread::IsMainThread());

    DownloadTaskDescription *taskToRetry = ExtractFromQueue(doneTaskQueue, taskId);
    if (taskToRetry)
    {
        taskToRetry->error = DLE_NO_ERROR;
        //taskToRetry->type = RESUMED;
        SetTaskStatus(taskToRetry, DL_PENDING);
        PlaceToQueue(pendingTaskQueue, taskToRetry);
    }
}

void DownloadManager::Cancel(const uint32 &taskId)
{
    DVASSERT(true == Thread::IsMainThread());

    DownloadTaskDescription * curTaskToDelete = currentTask;

    if (curTaskToDelete && taskId == curTaskToDelete->id)
    {
       Interrupt();
       Wait(taskId);
    }
    else
    {
        DownloadTaskDescription *pendingTask = NULL;
        pendingTask = ExtractFromQueue(pendingTaskQueue, taskId);
        if (pendingTask)
        {
            pendingTask->error = DLE_CANCELLED;
            SetTaskStatus(pendingTask, DL_FINISHED);
            PlaceToQueue(doneTaskQueue, pendingTask);
        }
    }
}

void DownloadManager::CancelCurrent()
{
    DVASSERT(true == Thread::IsMainThread());

    DownloadTaskDescription * curTaskToCancel = currentTask;
    if (!curTaskToCancel)
        return;

    Interrupt();
    Wait(curTaskToCancel->id);
}

void DownloadManager::CancelAll()
{
    //queues changing allowed only from main thread
    DVASSERT(true == Thread::IsMainThread());

    if (!pendingTaskQueue.empty())
    {
        Deque<DownloadTaskDescription *>::iterator it;
        for (it = pendingTaskQueue.begin(); it != pendingTaskQueue.end();)
        {
            DownloadTaskDescription * task = (*it);
            task->error = DLE_CANCELLED;
            SetTaskStatus(task, DL_FINISHED);
            doneTaskQueue.push_back(task);
            it = pendingTaskQueue.erase(it);
        }
    }

    if (currentTask)
    {
        Interrupt();
        Wait(currentTask->id);
    }
}

void DownloadManager::Clear(const uint32 &taskId)
{
    //queues changing allowed only from main thread
    DVASSERT(true == Thread::IsMainThread());

    // cancel task if possible
    Cancel(taskId);

    DownloadTaskDescription * task = NULL;
    task = ExtractFromQueue(pendingTaskQueue, taskId);
    if (task)
    {
        SafeDelete(task);
    }
    else
    {
        task = ExtractFromQueue(doneTaskQueue, taskId);
        if (task)
        {
            SafeDelete(task);
        }
    }
}

void DownloadManager::ThreadFunction(BaseObject *caller, void *callerData, void *userData)
{
    while(isThreadStarted)
    {
        Thread::SleepThread(20);

        currentTaskMutex.Lock();
        if (!currentTask || DL_FINISHED == currentTask->status)
        {
            currentTaskMutex.Unlock();
            continue;
        }

        currentTask->error = Download();

        currentTaskMutex.Unlock();
        
        // if we need to stop thread (finish current task end exit)
        if (!isThreadStarted)
            break;
    }
    currentTask = NULL;
    isThreadStarted = false;
}

void DownloadManager::ClearAll()
{
    ClearPending();
    ClearDone();

    DownloadTaskDescription *currentTaskToClear = NULL;

    currentTaskToClear = currentTask;

    Clear(currentTaskToClear->id);
}

void DownloadManager::ClearPending()
{
    ClearQueue(pendingTaskQueue);
}

void DownloadManager::ClearDone()
{
    ClearQueue(doneTaskQueue);
}

void DownloadManager::Wait(const uint32 &taskId)
{
    DVASSERT(true == Thread::IsMainThread());

    // if you called it from other thread than main - you should be sured that Update() method calls periodically from Main Thread.

    DownloadTaskDescription *waitTask = NULL;
    DownloadTaskDescription *currentTaskToWait = NULL;

    currentTaskToWait = currentTask;

    if (currentTaskToWait && currentTaskToWait->id == taskId)
        waitTask = currentTaskToWait;

    if (!waitTask)
    {
        Deque<DownloadTaskDescription *>::iterator it;
        for (it = pendingTaskQueue.begin(); it != pendingTaskQueue.end(); ++it)
        {
            DownloadTaskDescription * task = (*it);
            if (taskId == task->id)
                waitTask = task;
        }
    }

    // if there is no such task or task is completed - we don;t need to wait
    if (NULL == waitTask || (DL_PENDING != waitTask->status && DL_IN_PROGRESS != waitTask->status))
        return;

    // wait until task is finished
    while (waitTask
       && (waitTask->status == DL_IN_PROGRESS || waitTask->status == DL_PENDING))
    {
        Thread::SleepThread(20);
        Update();
    }
}

void DownloadManager::WaitAll()
{
    DVASSERT(true == Thread::IsMainThread());
    while (true)
    {
        bool needToWait = currentTask || !pendingTaskQueue.empty();

        if (needToWait)
        {
            Thread::SleepThread(20);
            Update();
        }
        else
            break;
    }
}

bool DownloadManager::GetCurrentId(uint32 &id)
{
    DownloadTaskDescription * curTaskToGet = currentTask;

    if (curTaskToGet)
    {
        id = curTaskToGet->id;
        return true;
    }

    return false;
}

bool DownloadManager::GetUrl(const uint32 &taskId, String &url)
{
    DownloadTaskDescription *task = GetTaskForId(taskId);
    if (!task)
        return false;

    url = task->url;

    return true;
}

bool DownloadManager::GetStorePath(const uint32 &taskId, FilePath &path)
{
    DownloadTaskDescription *task = GetTaskForId(taskId);
    if (!task)
        return false;

    path = task->storePath;

    return true;
}

bool DownloadManager::GetType(const uint32 &taskId, DownloadType &type)
{
    DownloadTaskDescription *task = GetTaskForId(taskId);
    if (!task)
        return false;

    type = task->type;

    return true;
}

bool DownloadManager::GetStatus(const uint32 &taskId, DownloadStatus &status)
{
    DownloadTaskDescription *task = GetTaskForId(taskId);
    if (!task)
        return false;

    status = task->status;

    return true;
}

bool DownloadManager::GetTotal(const uint32 &taskId, uint64 &total)
{
    DownloadTaskDescription *task = GetTaskForId(taskId);
    if (!task)
        return false;

    total = task->downloadTotal;

    return true;
}

bool DownloadManager::GetProgress(const uint32 &taskId, uint64 &progress)
{
    DownloadTaskDescription *task = GetTaskForId(taskId);
    if (!task)
        return false;

    progress = task->downloadProgress;

    return true;
}

bool DownloadManager::GetError(const uint32 &taskId, DownloadError &error)
{
    DownloadTaskDescription *task = GetTaskForId(taskId);
    if (!task)
        return false;

    error = task->error;

    return true;
}

bool DownloadManager::SetOperationTimeout(const uint32 operationTimeout)
{
    if (NULL == downloader)
        return false;

    downloader->timeout = operationTimeout;
    return true;
}

void DownloadManager::ClearQueue(Deque<DownloadTaskDescription *> &queue)
{
    DVASSERT(true == Thread::IsMainThread());

    if (!queue.empty())
    {
        for (Deque<DownloadTaskDescription *>::iterator it = queue.begin(); it != queue.end();)
        {
           DownloadTaskDescription *task = (*it);
           delete task;
           it = queue.erase(it);
        }
    }
}

DownloadTaskDescription *DownloadManager::ExtractFromQueue(Deque<DownloadTaskDescription *> &queue, const uint32 &taskId)
{
    DownloadTaskDescription *extractedTask = NULL;

    if (!queue.empty())
    {
        for (Deque<DownloadTaskDescription *>::iterator it = queue.begin(); it != queue.end();)
        {
            DownloadTaskDescription *task = (*it);
            if (task->id == taskId)
            {
                extractedTask = task;

                it = queue.erase(it);
            }
            else
                ++it;
        }
    }
    return extractedTask;
}

void DownloadManager::PlaceToQueue(Deque<DownloadTaskDescription *> &queue, DownloadTaskDescription *task)
{
    queue.push_back(task);
}

DownloadTaskDescription *DownloadManager::GetTaskForId(const uint32 &taskId)
{
    DVASSERT(true == Thread::IsMainThread());

    DownloadTaskDescription *retPointer = NULL;

    if (currentTask && taskId == currentTask->id)
    {
        retPointer = currentTask;
        return retPointer;
    }

    Deque<DownloadTaskDescription *>::iterator it;
    for (it = pendingTaskQueue.begin(); it != pendingTaskQueue.end(); ++it)
    {
        DownloadTaskDescription *task = (*it);
        if (task->id == taskId)
            return task;
    }

    for (it = doneTaskQueue.begin(); it != doneTaskQueue.end(); ++it)
    {
        DownloadTaskDescription *task = (*it);
        if (task->id == taskId)
            return task;
    }

    return retPointer;
}

void DownloadManager::SetTaskStatus(DownloadTaskDescription *task, const DownloadStatus &status)
{
    DVASSERT(task);
    DVASSERT(status != task->status);

    task->status = status;

    callbackMutex.Lock();
    callbackMessagesQueue.push_back(CallbackData(task->id, task->status));  
    callbackMutex.Unlock();
}


void DownloadManager::Interrupt()
{
    DVASSERT(currentTask);
    DVASSERT(downloader);

    downloader->Interrupt();
}



DownloadError DownloadManager::Download()
{
    FilePath path(currentTask->storePath);
    FileSystem::Instance()->CreateDirectory(path.GetDirectory(), true);

    ResetRetriesCount();
    DownloadError error = DLE_NO_ERROR;
    SetTaskStatus(currentTask, DL_IN_PROGRESS);
    downloadedTotal = 0;

    do
    {
        error = TryDownload();

        if (DLE_CONTENT_NOT_FOUND == error || DLE_CANCELLED == error)
            break;

    }while (0 < currentTask->retriesLeft-- && DLE_NO_ERROR != error);

    SetTaskStatus(currentTask, DL_FINISHED);
    return error;
}

DownloadError DownloadManager::TryDownload()
{
    int64 loadFrom = 0;

    // retrieve remote file size
    DownloadError error = downloader->GetSize(currentTask->url, remoteFileSize, currentTask->timeout);
    if (DLE_NO_ERROR != error)
    {
        currentTask->error = error;
        return error;
    }

    if (GET_SIZE == currentTask->type)
    {
        currentTask->downloadTotal = remoteFileSize;
        currentTask->error = error;
        return error;
    }

    DVASSERT(GET_SIZE != currentTask->type);

    // get downloaded part of file size
    if (RESUMED == currentTask->type)
    {
        // if file is particulary downloaded, we will try to download rest part of it        
        File *fileToGetSize = File::Create(currentTask->storePath, File::OPEN | File::READ);
        if (!fileToGetSize)
        {
            // create new file if there is no file.
            SafeRelease(fileToGetSize);
            fileToGetSize = File::Create(currentTask->storePath, File::CREATE | File::WRITE | File::READ);
            if (!fileToGetSize)
                return DLE_FILE_ERROR;
        }

        loadFrom = fileToGetSize->GetSize();
        currentTask->downloadProgress = loadFrom;
        SafeRelease(fileToGetSize);
    }
    else
    {
        MakeFullDownload(currentTask);
    }

    currentTask->downloadTotal = remoteFileSize;
    
    downloadedTotal = loadFrom;

    // if downloaded part of file is larger or equals to expected size
    if (loadFrom > remoteFileSize)
    {        
        // here we can interrupt download or reload file for example
        MakeFullDownload(currentTask);
        loadFrom = 0;
    }
    else if (loadFrom == remoteFileSize)
    {
        currentTask->error = DLE_NO_ERROR;
        return currentTask->error;
    }

    error = downloader->Download(currentTask->url, loadFrom, currentTask->timeout);

    // seems server doesn't supports download resuming. So we need to download whole file.
    if (DLE_CANNOT_RESUME == error)
    {
        loadFrom = 0;
        MakeFullDownload(currentTask);
        error = downloader->Download(currentTask->url, loadFrom);
    }

    currentTask->error = error;

    return error;
}

void DownloadManager::MakeFullDownload(DownloadTaskDescription *task)
{
    task->type = FULL;
    FileSystem::Instance()->DeleteFile(task->storePath);
    File *file = File::Create(task->storePath, File::CREATE | File::WRITE);
    SafeRelease(file);
    task->downloadProgress = 0;
}

void DownloadManager::ResetRetriesCount()
{
    currentTask->retriesLeft = currentTask->retriesCount;
}

}

