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


#include "DLC.h"
#include "Job/JobManager.h"
#include "Patcher/PatchFile.h"
#include "Platform/DeviceInfo.h"
#include "Downloader/DownloadManager.h"
#include "Render/GPUFamilyDescriptor.h"

#include "FileSystem/File.h"
#include "FileSystem/FileSystem.h"
#include <functional>

namespace DAVA
{

DLC::DLC(const String &url, const FilePath &sourceDir, const FilePath &destinationDir, const FilePath &workingDir, const String &gameVersion, const FilePath &resVersionPath, bool forceFullUpdate)
: dlcState(DS_INIT)
, dlcError(DE_NO_ERROR)
, patchingThread(NULL)
{
    DVASSERT(workingDir.IsDirectoryPathname());
    DVASSERT(workingDir.Exists());

    DVASSERT(destinationDir.IsDirectoryPathname());
    DVASSERT(destinationDir.Exists());

    DVASSERT(sourceDir.IsDirectoryPathname());
    DVASSERT(sourceDir.Exists());

    DVASSERT(!gameVersion.empty());

    //  we suppose that downloaded data should not be media data and exclude it from index.
    FileSystem::Instance()->MarkFolderAsNoMedia(destinationDir);

    // initial values
    dlcContext.remoteUrl = url;
    dlcContext.localVer = 0;
    dlcContext.forceFullUpdate = forceFullUpdate;

    dlcContext.localWorkingDir = workingDir;
    dlcContext.localDestinationDir = destinationDir;
    dlcContext.localSourceDir = sourceDir;
    dlcContext.localVerStorePath = resVersionPath;

    dlcContext.remoteVerUrl = Format("%s/g%s.info", url.c_str(), gameVersion.c_str());
    dlcContext.remoteVer = 0;
    dlcContext.remoteVerDownloadId = 0;
    dlcContext.remoteFullSizeDownloadId = 0;
    dlcContext.remoteLiteSizeDownloadId = 0;
    dlcContext.remoteMetaDownloadId = 0;
    dlcContext.remoteVerStotePath = workingDir + "RemoteVersion.info";
    dlcContext.remotePatchDownloadId = 0;
    dlcContext.remotePatchSize = 0;
    dlcContext.remotePatchReadySize = 0;
    dlcContext.remotePatchStorePath = workingDir + "Remote.patch";
    dlcContext.remoteMetaStorePath = workingDir + "Remote.meta";

    dlcContext.patchInProgress = true;
    dlcContext.totalPatchCount = 0;
    dlcContext.appliedPatchCount = 0;
    dlcContext.patchingError = PatchFileReader::ERROR_NO;
    dlcContext.lastErrno = 0;

    dlcContext.downloadInfoStorePath = workingDir + "Download.info";
    dlcContext.stateInfoStorePath = workingDir + "State.info";
    dlcContext.prevState = 0;

    ReadUint32(dlcContext.stateInfoStorePath, dlcContext.prevState);
    ReadUint32(dlcContext.remoteVerStotePath, dlcContext.remoteVer);

    // FSM variables
    fsmAutoReady = false;
}

DLC::~DLC()
{
    DVASSERT((dlcState == DS_INIT || dlcState == DS_READY || dlcState == DS_DONE) && "DLC can be safely destroyed only in certain modes");
}

void DLC::Check()
{
    PostEvent(EVENT_CHECK_START);
}

void DLC::Start()
{ 
    PostEvent(EVENT_DOWNLOAD_START);
}

void DLC::Cancel()
{
    PostEvent(EVENT_CANCEL);
}

void DLC::GetProgress(uint64 &cur, uint64 &total) const
{
    switch(dlcState)
    {
        case DS_READY:
            total = dlcContext.remotePatchSize;
            cur = dlcContext.remotePatchReadySize;
            break;
        case DS_DOWNLOADING:
            total = dlcContext.remotePatchSize;
            DownloadManager::Instance()->GetProgress(dlcContext.remotePatchDownloadId, cur);
            break;
        case DS_PATCHING:
            total = dlcContext.totalPatchCount;
            cur = dlcContext.appliedPatchCount;
            break;
        default:
            cur = 0;
            total = 0;
            break;
    }
}

DLC::DLCState DLC::GetState() const
{
    return dlcState;
}

DLC::DLCError DLC::GetError() const
{
    return dlcError;
}

int32 DLC::GetLastErrno() const
{
    return dlcContext.lastErrno;
}

PatchFileReader::PatchError DLC::GetPatchError() const
{
    return dlcContext.patchingError;
}

FilePath DLC::GetMetaStorePath() const
{
    return dlcContext.remoteMetaStorePath;
}
    
void DLC::PostEvent(DLCEvent event)
{
    Function<void()> fn = Bind(&DLC::FSM, this, event);
	JobManager::Instance()->CreateMainJob(fn);
}

void DLC::PostError(DLCError error)
{
    dlcError = error;
    PostEvent(EVENT_ERROR);
}

void DLC::FSM(DLCEvent event)
{
    bool eventHandled = true;
    DLCState oldState = dlcState;

    // State Machine implementation 
    // Read more - http://www.ict.edu.ru/ft/001845/switch.pdf

    switch(dlcState)
    {
        case DS_INIT:
            switch(event)
            {
                case EVENT_DOWNLOAD_START:
                    fsmAutoReady = true;
                    // don't break here

                case EVENT_CHECK_START:
                    // if last time stopped on the patching state and patch file exists - continue patching
                    if( !dlcContext.forceFullUpdate &&
                        DS_PATCHING == dlcContext.prevState &&
                        dlcContext.remotePatchStorePath.Exists() &&
                        dlcContext.remoteVerStotePath.Exists())
                    {
                        dlcContext.prevState = 0;
                        dlcState = DS_PATCHING;
                    }
                    else
                    {
                        dlcState = DS_CHECKING_INFO;
                    }
                    break;
                case EVENT_CANCEL:
                    dlcError = DE_WAS_CANCELED;
                    dlcState = DS_DONE;
                    break;
                default:
                    eventHandled = false;
                    break;
            }
            break;

        case DS_CHECKING_INFO:
            switch(event)
            {
                case EVENT_CHECK_OK:
                    // if remote version is newer than local we should continue DLC
                    if(dlcContext.localVer < dlcContext.remoteVer)
                    {
                        // check if patch exists on server
                        dlcState = DS_CHECKING_PATCH;
                    }
                    else
                    {
                        // local version is up do date. finish DLC
                        dlcState = DS_CLEANING;
                    }
                    break;
                case EVENT_ERROR:
                    dlcState = DS_DONE;
                    break;
                case EVENT_CANCEL:
                    dlcState = DS_CANCELLING;
                    break;
                default:
                    eventHandled = false;
                    break;
            }
            break;

        case DS_CHECKING_PATCH:
            switch(event)
            {
                case EVENT_CHECK_OK:
                    dlcState = DS_CHECKING_META;
                    break;
                case EVENT_ERROR:
                    dlcState = DS_DONE;
                    break;
                case EVENT_CANCEL:
                    dlcState = DS_CANCELLING;
                    break;
                default:
                    eventHandled = false;
                    break;
            }
            break;

        case DS_CHECKING_META:
            switch(event)
            {
                case EVENT_CHECK_OK:
                    // automatically start download after check?
                    if(fsmAutoReady)
                    {
                        // download patch
                        dlcState = DS_DOWNLOADING;
                    }
                    else
                    {
                        dlcState = DS_READY;
                    }
                    break;
                case EVENT_ERROR:
                    dlcState = DS_DONE;
                    break;
                case EVENT_CANCEL:
                    dlcState = DS_CANCELLING;
                    break;
                default:
                    eventHandled = false;
                    break;
            }
            break;

        case DS_READY:
            switch(event)
            {
                case EVENT_DOWNLOAD_START:
                    dlcState = DS_DOWNLOADING;
                    break;
                case EVENT_CANCEL:
                    dlcError = DE_WAS_CANCELED;
                    dlcState = DS_DONE;
                    break;
                default:
                    eventHandled = false;
                    break;
            }
            break;

        case DS_DOWNLOADING:
            switch(event)
            {
                case EVENT_DOWNLOAD_OK:
                    dlcState = DS_PATCHING;
                    break;
                case EVENT_ERROR:
                    dlcState = DS_DONE;
                    break;
                case EVENT_CANCEL:
                    dlcState = DS_CANCELLING;
                    break;
                default:
                    eventHandled = false;
                    break;
            }
            break;
        case DS_PATCHING:
            switch(event)
            {
                case EVENT_PATCH_OK:
                    // a new version can appear while we doing patching
                    // so we need to check it once again
                    fsmAutoReady = true;
                    dlcState = DS_CHECKING_INFO;
                    break;
                case EVENT_ERROR:
                    dlcState = DS_DONE;
                    break;
                case EVENT_CANCEL:
                    dlcState = DS_CANCELLING;
                    break;
                default:
                    eventHandled = false;
                    break;
            }
            break;
        case DS_CANCELLING:
            switch(event)
            {
                case EVENT_CHECK_OK:
                case EVENT_DOWNLOAD_OK:
                case EVENT_PATCH_OK:
                case EVENT_ERROR:
                    dlcError = DE_WAS_CANCELED;
                    dlcState = DS_DONE;
                    break;
                default:
                    eventHandled = false;
                    break;
            }
            break;
        case DS_CLEANING:
            switch(event)
            {
                case EVENT_CLEAN_OK:
                    dlcState = DS_DONE;
                    break;
                case EVENT_CANCEL:
                    break;
                default:
                    eventHandled = false;
                    break;
            }
            break;
        case DS_DONE:
            break;
        default:
            break;
    }

    if(EVENT_ERROR == event)
    {
        DVASSERT(DE_NO_ERROR != dlcError && "Unhandled error, dlcError is set to NO_ERROR");
    }

    if(!eventHandled)
    {
        Logger::Error("Unhanded event %d in state %d\n", event, dlcState);
        DVASSERT(false); 
    }

    // what should we do, when state changed
    if(oldState != dlcState)
    {
        Logger::Info("DLC: Changing state %d->%d", oldState, dlcState);

        switch(dlcState)
        {
            case DS_INIT:
                break;
            case DS_CHECKING_INFO:
                StepCheckInfoBegin();
                break;
            case DS_CHECKING_PATCH:
                StepCheckPatchBegin();
                break;
            case DS_CHECKING_META:
                StepCheckMetaBegin();
                break;
            case DS_READY:
                break;
            case DS_DOWNLOADING:
                StepDownloadPatchBegin();
                break;
            case DS_PATCHING:
                StepPatchBegin();
                break;
            case DS_CANCELLING:
                switch(oldState)
                {
                    case DS_CHECKING_INFO:
                        StepCheckInfoCancel();
                        break;
                    case DS_CHECKING_META:
                        StepCheckMetaCancel();
                        break;
                    case DS_CHECKING_PATCH:
                        StepCheckPatchCancel();
                        break;
                    case DS_READY:
                        break;
                    case DS_DOWNLOADING:
                        StepDownloadPatchCancel();
                        break;
                    case DS_PATCHING:
                        StepPatchCancel();
                        break;
                    default:
                        Logger::Error("Unhanded state %d canceling\n", oldState);
                        DVASSERT(false);
                        break;
                }
                break;
            case DS_CLEANING:
                StepClean();
                break;
            case DS_DONE:
                StepDone();
                break;
            default:
                break;
        }
    }
}

// start downloading remove DLC version 
void DLC::StepCheckInfoBegin()
{
    // write current dlcState into state-file
    if(!WriteUint32(dlcContext.stateInfoStorePath, dlcState))
    {
        dlcContext.lastErrno = errno;
        PostError(DE_WRITE_ERROR);
        return;
    }

    if(!dlcContext.forceFullUpdate)
    {
        ReadUint32(dlcContext.localVerStorePath, dlcContext.localVer);
    }

    Logger::Info("DLC: Downloading game-info\n\tfrom: %s\n\tto: %s", dlcContext.remoteVerUrl.c_str(), dlcContext.remoteVerStotePath.GetAbsolutePathname().c_str());

    DownloadManager::Instance()->SetNotificationCallback(DownloadManager::NotifyFunctor(this, &DLC::StepCheckInfoFinish));
    dlcContext.remoteVerDownloadId = DownloadManager::Instance()->Download(dlcContext.remoteVerUrl, dlcContext.remoteVerStotePath.GetAbsolutePathname(), FULL, 1);
}

// downloading DLC version file finished. need to read removeVersion
void DLC::StepCheckInfoFinish(const uint32 &id, const DownloadStatus &status)
{
    if(id == dlcContext.remoteVerDownloadId)
    {
        if(DL_FINISHED == status)
        {
            DownloadError downloadError;
            DownloadManager::Instance()->GetError(id, downloadError);

            if(DLE_NO_ERROR == downloadError && dlcContext.remoteVerStotePath.Exists())
            {
                if(ReadUint32(dlcContext.remoteVerStotePath, dlcContext.remoteVer))
                {
                    PostEvent(EVENT_CHECK_OK);
                }
                else
                {
                    dlcContext.lastErrno = errno;
                    PostError(DE_READ_ERROR);
                }
            }
            else
            {
                Logger::FrameworkDebug("DLC: error %d", downloadError);
                if(DLE_COULDNT_RESOLVE_HOST == downloadError || DLE_COULDNT_CONNECT == downloadError)
                {
                    // connection problem
                    PostError(DE_CONNECT_ERROR);
                }
                else if(DLE_FILE_ERROR == downloadError)
                {
                    // writing file problem
                    dlcContext.lastErrno = errno;
                    PostError(DE_WRITE_ERROR);
                }
                else
                {
                    // some other unexpected error during check process
                    PostError(DE_CHECK_ERROR);
                }
            }
        }
    }
}

void DLC::StepCheckInfoCancel()
{
    DownloadManager::Instance()->Cancel(dlcContext.remoteVerDownloadId);
}

void DLC::StepCheckPatchBegin()
{
    File *f = File::Create(dlcContext.remotePatchStorePath, File::OPEN | File::READ);
    if(nullptr != f)
    {
        dlcContext.remotePatchReadySize = f->GetSize();
        f->Release();
    }

    dlcContext.remotePatchLiteSize = 0;
    dlcContext.remotePatchFullSize = 0;

    dlcContext.remotePatchFullUrl = dlcContext.remoteUrl + MakePatchUrl(0, dlcContext.remoteVer);
    dlcContext.remotePatchLiteUrl = dlcContext.remoteUrl + MakePatchUrl(dlcContext.localVer, dlcContext.remoteVer);

    Logger::Info("DLC: Retrieving full-patch size from: %s", dlcContext.remotePatchLiteUrl.c_str());
    Logger::Info("DLC: Retrieving lite-patch size from: %s", dlcContext.remotePatchFullUrl.c_str());

    DownloadManager::Instance()->SetNotificationCallback(DownloadManager::NotifyFunctor(this, &DLC::StepCheckPatchFinish));
    dlcContext.remoteFullSizeDownloadId = DownloadManager::Instance()->Download(dlcContext.remotePatchFullUrl, dlcContext.remotePatchStorePath, GET_SIZE); // full size should be first
    dlcContext.remoteLiteSizeDownloadId = DownloadManager::Instance()->Download(dlcContext.remotePatchLiteUrl, dlcContext.remotePatchStorePath, GET_SIZE); // lite size should be last
}

void DLC::StepCheckPatchFinish(const uint32 &id, const DownloadStatus &status)
{
    if(id == dlcContext.remoteLiteSizeDownloadId)
    {
        if(DL_FINISHED == status)
        {
            DownloadStatus statusFull;
            DownloadError downloadErrorFull;
            DownloadError downloadErrorLite;

            DownloadManager::Instance()->GetStatus(dlcContext.remoteFullSizeDownloadId, statusFull);
            DownloadManager::Instance()->GetError(dlcContext.remoteFullSizeDownloadId, downloadErrorFull);
            DownloadManager::Instance()->GetError(dlcContext.remoteLiteSizeDownloadId, downloadErrorLite);

            // when lite id finishing, full id should be already finished
            DVASSERT(DL_FINISHED == statusFull);

            DownloadManager::Instance()->GetTotal(dlcContext.remoteLiteSizeDownloadId, dlcContext.remotePatchLiteSize);
            DownloadManager::Instance()->GetTotal(dlcContext.remoteFullSizeDownloadId, dlcContext.remotePatchFullSize);

            if(DLE_NO_ERROR == downloadErrorLite)
            {
                dlcContext.remotePatchUrl = dlcContext.remotePatchLiteUrl;
                dlcContext.remotePatchSize = dlcContext.remotePatchLiteSize;

                PostEvent(EVENT_CHECK_OK);
            }
            else
            {
                if(DLE_NO_ERROR == downloadErrorFull)
                {
                    dlcContext.remotePatchUrl = dlcContext.remotePatchFullUrl;
                    dlcContext.remotePatchSize = dlcContext.remotePatchFullSize;

                    PostEvent(EVENT_CHECK_OK);
                }
                else
                {
                    if(DLE_COULDNT_RESOLVE_HOST == downloadErrorFull || DLE_COULDNT_CONNECT == downloadErrorFull)
                    {
                        // connection problem
                        PostError(DE_CONNECT_ERROR);
                    }
                    else if(DLE_FILE_ERROR == downloadErrorFull)
                    {
                        // writing file problem
                        dlcContext.lastErrno = errno;
                        PostError(DE_WRITE_ERROR);
                    }
                    else
                    {
                        // some other unexpected error during check process
                        PostError(DE_CHECK_ERROR);
                    }
                }
            }
        }
    }
}

void DLC::StepCheckPatchCancel()
{
    DownloadManager::Instance()->Cancel(dlcContext.remoteFullSizeDownloadId);
    DownloadManager::Instance()->Cancel(dlcContext.remoteLiteSizeDownloadId);
}

void DLC::StepCheckMetaBegin()
{
    dlcContext.remoteMetaUrl = dlcContext.remotePatchUrl + ".meta";

    Logger::Info("DLC: Downloading game-meta\n\tfrom: %s\n\tto :%s", dlcContext.remoteMetaUrl.c_str(), dlcContext.remoteMetaStorePath.GetAbsolutePathname().c_str());

    FileSystem::Instance()->DeleteFile(dlcContext.remoteMetaStorePath);
    DownloadManager::Instance()->SetNotificationCallback(DownloadManager::NotifyFunctor(this, &DLC::StepCheckMetaFinish));
    dlcContext.remoteMetaDownloadId = DownloadManager::Instance()->Download(dlcContext.remoteMetaUrl, dlcContext.remoteMetaStorePath, FULL, 1);
}

void DLC::StepCheckMetaFinish(const uint32 &id, const DownloadStatus &status)
{
    if(id == dlcContext.remoteMetaDownloadId)
    {
        if(DL_FINISHED == status)
        {
            DownloadError downloadError;
            DownloadManager::Instance()->GetError(dlcContext.remoteMetaDownloadId, downloadError);

            if(DLE_COULDNT_RESOLVE_HOST == downloadError || DLE_COULDNT_CONNECT == downloadError)
            {
                // connection problem
                PostError(DE_CONNECT_ERROR);
            }
            else if(DLE_FILE_ERROR == downloadError)
            {
                // writing file problem
                dlcContext.lastErrno = errno;
                PostError(DE_WRITE_ERROR);
            }
            else
            {
                // any other error should be thread as OK-status, when retrieving meta-info file
                PostEvent(EVENT_CHECK_OK);
            }
        }
    }
}

void DLC::StepCheckMetaCancel()
{
    DownloadManager::Instance()->Cancel(dlcContext.remoteMetaDownloadId);
}

// download patch file
void DLC::StepDownloadPatchBegin()
{
    // write current dlcState into state-file
    if(!WriteUint32(dlcContext.stateInfoStorePath, dlcState))
    {
        dlcContext.lastErrno = errno;
        PostError(DE_WRITE_ERROR);
        return;
    }

    // what mode should be used for download?
    // by default - full download
    DownloadType donwloadType = FULL;

    // check what URL was downloaded last time
    File *downloadInfoFile = File::Create(dlcContext.downloadInfoStorePath, File::OPEN | File::READ);
    if(NULL != downloadInfoFile)
    {
        String lastUrl;
        String lastSizeStr;
        uint32 lastSize;

        downloadInfoFile->ReadString(lastSizeStr);
        downloadInfoFile->ReadString(lastUrl);

        lastSize = atoi(lastSizeStr.c_str());

        // if last url is same as current full or lite url we should continue downloading it
        if( (lastUrl == dlcContext.remotePatchFullUrl && lastSize == dlcContext.remotePatchFullSize) ||
            (lastUrl == dlcContext.remotePatchLiteUrl && lastSize == dlcContext.remotePatchLiteSize))
        {
            dlcContext.remotePatchUrl = lastUrl;
            dlcContext.remotePatchSize = lastSize;

            // now we can resume last download
            donwloadType = RESUMED;
        }
        else
        {
            // ensure that there is no already downloaded file with another version
            FileSystem::Instance()->DeleteFile(dlcContext.remotePatchStorePath);
        }

        SafeRelease(downloadInfoFile);
    }

    if (donwloadType != RESUMED)//if 'RESUMED' downloadInfoFile contains correct info and we don't want to recreate it to prevent issues when disk is full
    {
        // save URL that we gonna download
        downloadInfoFile = File::Create(dlcContext.downloadInfoStorePath, File::CREATE | File::WRITE);
        if(NULL != downloadInfoFile)
        {
            String sizeStr = Format("%u", dlcContext.remotePatchSize);
            downloadInfoFile->WriteString(sizeStr);
            downloadInfoFile->WriteString(dlcContext.remotePatchUrl);
            SafeRelease(downloadInfoFile);
        }
    }
    
    Logger::Info("DLC: Downloading patch-file\n\tfrom: %s\n\tto: %s", dlcContext.remotePatchUrl.c_str(), dlcContext.remotePatchStorePath.GetAbsolutePathname().c_str());

    // start download and notify about download status into StepDownloadPatchFinish
    DownloadManager::Instance()->SetNotificationCallback(DownloadManager::NotifyFunctor(this, &DLC::StepDownloadPatchFinish));
    dlcContext.remotePatchDownloadId = DownloadManager::Instance()->Download(dlcContext.remotePatchUrl, dlcContext.remotePatchStorePath.GetAbsolutePathname(), donwloadType);
}

void DLC::StepDownloadPatchFinish(const uint32 &id, const DownloadStatus &status)
{
    if(id == dlcContext.remotePatchDownloadId)
    {
        if(DL_FINISHED == status)
        {
            DownloadError downloadError;
            DownloadManager::Instance()->GetError(id, downloadError);

            switch(downloadError)
            {
                case DAVA::DLE_NO_ERROR:
                    //we want to have this switch from DS_DOWNLOADING to DS_PATCHING when app is in background,
                    //that's why we call FSM() directly instead of using job in PostEvent()
                    FSM(EVENT_DOWNLOAD_OK);
                    break;

                case DAVA::DLE_COULDNT_RESOLVE_HOST:
                case DAVA::DLE_COULDNT_CONNECT:
                    // connection problem
                    PostError(DE_CONNECT_ERROR);
                    break;

                case DAVA::DLE_FILE_ERROR:
                    // writing file problem
                    DownloadManager::Instance()->GetFileErrno(id, dlcContext.lastErrno);
                    PostError(DE_WRITE_ERROR);
                    break;

                default:
                    // some other unexpected error during download process
                    PostError(DE_DOWNLOAD_ERROR);
                    break;
            }
        }
    }
}

void DLC::StepDownloadPatchCancel()
{
    DownloadManager::Instance()->Cancel(dlcContext.remotePatchDownloadId);
}

void DLC::StepPatchBegin()
{
    // write current dlcState into state-file
    if(!WriteUint32(dlcContext.stateInfoStorePath, dlcState))
    {
        PostError(DE_WRITE_ERROR);
        return;
    }

    dlcContext.patchingError = PatchFileReader::ERROR_NO;
    dlcContext.lastErrno = 0;
    dlcContext.patchInProgress = true;
    dlcContext.appliedPatchCount = 0;
    dlcContext.totalPatchCount = 0;

    // read number of available patches
    PatchFileReader patchReader(dlcContext.remotePatchStorePath);
    if(patchReader.ReadFirst())
    {
        do
        {
            dlcContext.totalPatchCount++;
        }
        while(patchReader.ReadNext());
    }
    
    Logger::Info("DLC: Patching, %d files to patch", dlcContext.totalPatchCount);
    patchingThread = Thread::Create(Message(this, &DLC::PatchingThread));
    patchingThread->Start();
}

void DLC::StepPatchFinish()
{
    bool errors = true;

    patchingThread->Join();
    SafeRelease(patchingThread);

    dlcContext.prevState = 0;
    dlcContext.localVer = -1;

    switch(dlcContext.patchingError)
    {
        case PatchFileReader::ERROR_NO:
            errors = false;
            PostEvent(EVENT_PATCH_OK);
            break;

        case PatchFileReader::ERROR_CANT_READ:
            PostError(DE_READ_ERROR);
            break;

        case PatchFileReader::ERROR_NEW_CREATE:
        case PatchFileReader::ERROR_NEW_WRITE:
            PostError(DE_WRITE_ERROR);
            break;

        default:
            if(!dlcContext.remotePatchUrl.empty() && dlcContext.remotePatchUrl == dlcContext.remotePatchFullUrl)
            {
                PostError(DE_PATCH_ERROR_FULL);
            }
            else
            {
                PostError(DE_PATCH_ERROR_LITE);
            }
            break;
    }

    if(errors)
    {
        Logger::Error("DLC: Error applying patch: %u, errno %u", dlcContext.patchingError, dlcContext.lastErrno);
    }
}

void DLC::StepPatchCancel()
{
    dlcContext.patchInProgress = false;
    patchingThread->Join();
}
    
void DLC::PatchingThread(BaseObject *caller, void *callerData, void *userData)
{
    PatchFileReader patchReader(dlcContext.remotePatchStorePath);

    bool applySuccess = true;
    const PatchInfo *patchInfo = nullptr;

    auto applyPatchesFn = [&](bool allowTruncate, std::function<bool(const PatchInfo* info)> conditionFn)
    {
        if(applySuccess)
        {
            bool truncate = false;

            // To be able to truncate patch-file we should go from last patch to the first one.
            // If incoming patch-file doesn't support reverse pass we will try to apply it without truncation.
            // This will allow us to support old patch files.
            if(allowTruncate && patchReader.ReadLast())
            {
                truncate = true;
            }

            if(!truncate)
            {
                patchReader.ReadFirst();
            }

            // Get current patch info
            patchInfo = patchReader.GetCurInfo();
            while(applySuccess && nullptr != patchInfo && dlcContext.patchInProgress)
            {
                // Patch will be applied only if it fit condition, specified by caller
                if(conditionFn(patchInfo))
                {
                    applySuccess = patchReader.Apply(dlcContext.localSourceDir, FilePath(), dlcContext.localDestinationDir, FilePath());
                    if(applySuccess)
                    {
                        dlcContext.appliedPatchCount++;
                    }
                }

                // Go to the next patch and check if we need to truncate applied patch
                if(applySuccess && dlcContext.patchInProgress)
                {
                    if(truncate)
                    {
                        patchReader.Truncate();
                        patchReader.ReadPrev();
                    }
                    else
                    {
                        patchReader.ReadNext();
                    }

                    patchInfo = patchReader.GetCurInfo();
                }
            }
        }
    };

    // first step - apply patches, that either reduce or don't change resources size
    applyPatchesFn(false, 
        [](const PatchInfo* info) 
        { 
            return info->newSize <= info->origSize; 
        });

    // no errors on first step - continue applying patches, that increase resources size
    applyPatchesFn(true,
        [](const PatchInfo* info) 
        { 
            return info->newSize > info->origSize; 
        });
    
    // check if no errors occurred during patching
    dlcContext.lastErrno = patchReader.GetErrno();
    dlcContext.patchingError = patchReader.GetLastError();
    if(dlcContext.patchInProgress && PatchFileReader::ERROR_NO == dlcContext.patchingError)
    {
        DVASSERT(dlcContext.appliedPatchCount == dlcContext.totalPatchCount);
        
        // ensure directory, where resource version file should be, exists
        FileSystem::Instance()->CreateDirectory(dlcContext.localVerStorePath.GetDirectory(), true);

        // update local version
        if(!WriteUint32(dlcContext.localVerStorePath, dlcContext.remoteVer))
        {
            // error, version can't be written
            dlcContext.patchingError = PatchFileReader::ERROR_NEW_WRITE;
            dlcContext.lastErrno = errno;
        }
        
        // clean patch file if it was fully truncated
        // if we don't do that - we have Empty Patch Error if patching was finished in background and application was closed
        // because in that case StepPatchFinish losts and at application restart DLC follows to patching state.
        File *patchFile = File::Create(dlcContext.remotePatchStorePath, File::OPEN | File::READ);
        int32 patchSizeAfterPatching = patchFile->GetSize();
        SafeRelease(patchFile);
        
        if (0 == patchSizeAfterPatching)
        {
            FileSystem::Instance()->DeleteFile(dlcContext.remotePatchStorePath);
        }
    }

	Function<void()> fn(this, &DLC::StepPatchFinish);
	JobManager::Instance()->CreateMainJob(fn);
}

void DLC::StepClean()
{
    Logger::Info("DLC: Cleaning");

    FileSystem::Instance()->DeleteFile(dlcContext.downloadInfoStorePath);
    FileSystem::Instance()->DeleteFile(dlcContext.remoteMetaStorePath);
    FileSystem::Instance()->DeleteFile(dlcContext.remotePatchStorePath);

    PostEvent(EVENT_CLEAN_OK);
}

void DLC::StepDone()
{
    if(DE_NO_ERROR == dlcError)
    {
        FileSystem::Instance()->DeleteFile(dlcContext.remoteVerStotePath);
        FileSystem::Instance()->DeleteFile(dlcContext.stateInfoStorePath);
    }

    Logger::Info("DLC: Done!");
}

bool DLC::ReadUint32(const FilePath &path, uint32 &value)
{
    bool ret = false;

    // check if there is some unfinished download
    File *f = File::Create(path, File::OPEN | File::READ);
    if(NULL != f)
    {
        char8 tmp[64];
        tmp[0] = 0;
        if(f->ReadLine(tmp, sizeof(tmp)) > 0)
        {
            if(sscanf(tmp, "%u", &value) > 0)
            {
                ret = true;
            }
        }
        SafeRelease(f);
    }

    return ret;
}

bool DLC::WriteUint32(const FilePath &path, uint32 value)
{
    bool ret = false;

    // check if there is some unfinished download
    File *f = File::Create(path, File::CREATE | File::WRITE);
    if(NULL != f)
    {
        String tmp = Format("%u", value);
        ret = f->WriteLine(tmp);
        SafeRelease(f);
    }

    return ret;
}

String DLC::MakePatchUrl(uint32 localVer, uint32 remoteVer)
{
    String ret;

    eGPUFamily gpu = DeviceInfo::GetGPUFamily();
    if(gpu < GPU_FAMILY_COUNT)
    {
        String gpuString = GPUFamilyDescriptor::GetGPUName(gpu);
        ret = Format("/r%u/r%u-%u.%s.patch", remoteVer, localVer, remoteVer, gpuString.c_str());
    }
    else
    {
        ret = Format("/r%u/r%u-%u.patch", remoteVer, localVer, remoteVer);
    }

    return ret;
}

}
