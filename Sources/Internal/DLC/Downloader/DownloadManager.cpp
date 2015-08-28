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

#include "Functional/Function.h"
#include "FileSystem/File.h"
#include "FileSystem/FileSystem.h"
#include "Concurrency/Thread.h"
#include "Concurrency/Mutex.h"

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
    DVASSERT(NULL != _downloader);
    
    while(NULL != currentTask)
    {
        Thread::Sleep(10);
        Update();
    }

    SafeDelete(downloader);
    downloader = _downloader;
    downloader->SetProgressNotificator(MakeFunction(this, &DownloadManager::OnCurrentTaskProgressChanged));
}

Downloader *DownloadManager::GetDownloader()
{
    return downloader;
}

void DownloadManager::SetNotificationCallback(DownloadManager::NotifyFunctor callbackFn)
{
    callNotify = callbackFn;
}

DownloadManager::NotifyFunctor DownloadManager::GetNotificationCallback() const
{
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
    isThreadStarted = false;
    thisThread->Join();

    SafeRelease(thisThread);
}

void DownloadManager::Update()
{
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
            if (callNotify != nullptr)
            {
                callNotify(cbData.id, cbData.status);
            }
        }
    }
    callbackMutex.Unlock();
}

uint32 DownloadManager::Download(const String &srcUrl, const FilePath &storeToFilePath, const DownloadType downloadMode, const uint8 partsCount , int32 timeout, int32 retriesCount)
{
    DownloadTaskDescription *task = new DownloadTaskDescription(srcUrl, storeToFilePath, downloadMode, timeout, retriesCount, partsCount);
 
    static uint32 prevId = 1;
    task->id = prevId++;

    PlaceToQueue(pendingTaskQueue, task);

    task->status = DL_PENDING;

    return task->id;
}

void DownloadManager::Retry(const uint32 &taskId)
{
    DownloadTaskDescription *taskToRetry = ExtractFromQueue(doneTaskQueue, taskId);
    if (taskToRetry)
    {
        taskToRetry->error = DLE_NO_ERROR;
        taskToRetry->type = RESUMED;
        SetTaskStatus(taskToRetry, DL_PENDING);
        PlaceToQueue(pendingTaskQueue, taskToRetry);
    }
}

void DownloadManager::Cancel(const uint32 &taskId)
{
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
    DownloadTaskDescription * curTaskToCancel = currentTask;
    if (!curTaskToCancel)
        return;

    Interrupt();
    Wait(curTaskToCancel->id);
}

void DownloadManager::CancelAll()
{
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
        Thread::Sleep(20);

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
        Thread::Sleep(20);
        Update();
    }
}

void DownloadManager::WaitAll()
{
    while (true)
    {
        bool needToWait = currentTask || !pendingTaskQueue.empty();

        if (needToWait)
        {
            Thread::Sleep(20);
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

bool DownloadManager::GetFileErrno(const uint32 &taskId, int32 &fileErrno)
{
    DownloadTaskDescription *task = GetTaskForId(taskId);
    if(!task)
        return false;

    fileErrno = task->fileErrno;

    return true;
}
    
DownloadStatistics DownloadManager::GetStatistics()
{
    return downloader->GetStatistics();
}
    
void DownloadManager::SetDownloadSpeedLimit(const uint64 limit)
{
    downloader->SetDownloadSpeedLimit(limit);
}

void DownloadManager::ClearQueue(Deque<DownloadTaskDescription *> &queue)
{
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

        if (DLE_CONTENT_NOT_FOUND == error
            || DLE_CANCELLED == error
            || DLE_FILE_ERROR == error)
            break;
        
        currentTask->type = RESUMED;

    }while (0 < currentTask->retriesLeft-- && DLE_NO_ERROR != error);

    SetTaskStatus(currentTask, DL_FINISHED);
    return error;
}

DownloadError DownloadManager::TryDownload()
{
    // retrieve remote file size
    currentTask->error = downloader->GetSize(currentTask->url, currentTask->downloadTotal, currentTask->timeout);
    currentTask->fileErrno = downloader->GetFileErrno();
    if(DLE_NO_ERROR != currentTask->error)
    {        
        return currentTask->error;
    }

    if (GET_SIZE == currentTask->type)
    {
        return currentTask->error;
    }

    DVASSERT(GET_SIZE != currentTask->type);
    currentTask->downloadProgress = 0;

    if (RESUMED == currentTask->type)
    {
        MakeResumedDownload();
    }
    else    
    {
        MakeFullDownload();
    }

    if (DLE_NO_ERROR != currentTask->error)
    {
        return currentTask->error;
    }
    
    currentTask->error = downloader->Download(currentTask->url, currentTask->storePath, currentTask->partsCount, currentTask->timeout);
    currentTask->fileErrno = downloader->GetFileErrno();

    // seems server doesn't supports download resuming. So we need to download whole file.
    if (DLE_COULDNT_RESUME == currentTask->error)
    {
        MakeFullDownload();
        if (DLE_NO_ERROR == currentTask->error)
        {
            currentTask->error = downloader->Download(currentTask->url, currentTask->storePath, currentTask->partsCount, currentTask->timeout);
            currentTask->fileErrno = downloader->GetFileErrno();
        }
    }

    return currentTask->error;
}

void DownloadManager::MakeFullDownload()
{
    currentTask->type = FULL;

    if (currentTask->storePath.Exists())
    {
        if (FileSystem::Instance()->DeleteFile(currentTask->storePath))
        {
            currentTask->error = DLE_NO_ERROR;
        }
        else
        {
            currentTask->error = DLE_FILE_ERROR;
        }
    }
    currentTask->downloadProgress = 0;
}

void DownloadManager::MakeResumedDownload()
{
    currentTask->type = RESUMED;
    // if file is particulary downloaded, we will try to download rest part of it        
    File *file = File::Create(currentTask->storePath, File::OPEN | File::READ);
    if (NULL == static_cast<File *>(file))
    {
        // download fully if there is no file.
        MakeFullDownload();
    }
    else
    {
        currentTask->downloadProgress = file->GetSize();
        SafeRelease(file);
        // if exsisted file have not the same size as downloading file
        if (currentTask->downloadProgress > currentTask->downloadTotal)
        {
            MakeFullDownload();
        }
    }
}

void DownloadManager::ResetRetriesCount()
{
    currentTask->retriesLeft = currentTask->retriesCount;
}

void DownloadManager::OnCurrentTaskProgressChanged(uint64 progressDelta)
{
    currentTask->downloadProgress += progressDelta;
}
    
}
