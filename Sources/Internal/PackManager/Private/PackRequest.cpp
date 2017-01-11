#include "PackManager/Private/PackRequest.h"
#include "PackManager/Private/RequestManager.h"
#include "PackManager/Private/DCLManagerImpl.h"
#include "DLC/Downloader/DownloadManager.h"
#include "FileSystem/FileSystem.h"
#include "Utils/CRC32.h"
#include "Utils/StringFormat.h"
#include "Logger/Logger.h"
#include "DLC/DLC.h"
#include "Base/Exception.h"

namespace DAVA
{
PackRequest::PackRequest(DCLManagerImpl& packManager_, DLCManager::Pack& pack_)
    : packManagerImpl(&packManager_)
    , rootPack(&pack_)
{
    DVASSERT(packManagerImpl != nullptr);
    DVASSERT(rootPack != nullptr);
    // find all dependencies
    // put it all into vector and put final pack into vector too
    DCLManagerImpl::CollectDownloadableDependency(*packManagerImpl, rootPack->name, dependencyList);

    dependencies.reserve(dependencyList.size() + 1);

    for (DLCManager::Pack* depPack : dependencyList)
    {
        SubRequest subRequest;

        subRequest.pack = depPack;
        subRequest.status = SubRequest::Wait;
        subRequest.taskId = 0;

        dependencies.push_back(subRequest);
    }

    // last step download pack itself
    SubRequest subRequest;

    subRequest.pack = rootPack;
    subRequest.status = SubRequest::Wait;
    subRequest.taskId = 0;
    dependencies.push_back(subRequest);

    std::for_each(begin(dependencies), end(dependencies), [&](const SubRequest& request)
                  {
                      totalAllPacksSize += request.pack->totalSizeFromDB;
                  });
}

void PackRequest::AskFooter()
{
    DownloadManager* dm = DownloadManager::Instance();

    if (0 == fullSizeServerData)
    {
        if (0 == downloadTaskId)
        {
            const String& superPackUrl = packManagerImpl->GetSuperPackUrl();
            downloadTaskId = dm->Download(superPackUrl, "", GET_SIZE);
        }
        else
        {
            DownloadStatus status = DL_UNKNOWN;
            if (dm->GetStatus(downloadTaskId, status))
            {
                if (DL_FINISHED == status)
                {
                    DownloadError error = DLE_NO_ERROR;
                    dm->GetError(downloadTaskId, error);
                    if (DLE_NO_ERROR == error)
                    {
                        if (!dm->GetTotal(downloadTaskId, fullSizeServerData))
                        {
                            DAVA_THROW(DAVA::Exception, "can't get size of file on server side");
                        }
                    }
                    else
                    {
                        String err = DLC::ToString(error);
                        Logger::Error("can't get size of superpack from server: %s,\ntry again", err.c_str());
                        // try again
                        Restart();
                    }
                }
            }
        }
    }
    else
    {
        if (fullSizeServerData < sizeof(PackFormat::PackFile))
        {
            DAVA_THROW(DAVA::Exception, "too small superpack on server");
        }

        uint64 downloadOffset = fullSizeServerData - sizeof(footerOnServer);
        uint32 sizeofFooter = static_cast<uint32>(sizeof(footerOnServer));
        const String& superPackUrl = packManagerImpl->GetSuperPackUrl();
        downloadTaskId = dm->DownloadIntoBuffer(superPackUrl, &footerOnServer, sizeofFooter, downloadOffset, sizeofFooter);

        SubRequest& subRequest = dependencies.at(0);
        subRequest.status = SubRequest::Status::GetFooter;
    }
}

void PackRequest::GetFooter()
{
    DownloadManager* dm = DownloadManager::Instance();
    DownloadStatus status = DL_UNKNOWN;
    if (dm->GetStatus(downloadTaskId, status))
    {
        if (DL_FINISHED == status)
        {
            DownloadError error = DLE_NO_ERROR;
            dm->GetError(downloadTaskId, error);
            if (DLE_NO_ERROR == error)
            {
                PackFormat::PackFile::FooterBlock::Info& info = footerOnServer.info;
                uint32 crc32 = CRC32::ForBuffer(&info, sizeof(info));
                if (crc32 != footerOnServer.infoCrc32)
                {
                    Logger::Error("downloaded superpack footer crc32 not match, url: %s\ntry again", packManagerImpl->GetSuperPackUrl().c_str());
                    Restart();
                }
                else if (packManagerImpl->GetServerFooterCrc32() != footerOnServer.infoCrc32)
                {
                    // server Superpack changed during current session
                    Logger::Error("during current session server superpack file changed, crc32 not match, url: %s\n, try reinitialization for PM", packManagerImpl->GetSuperPackUrl().c_str());
                    Restart();
                    packManagerImpl->RetryInit();
                }
                else
                {
                    StartLoadingPackFile();
                }
            }
            else
            {
                rootPack->state = DLCManager::Pack::Status::ErrorLoading;
                rootPack->downloadError = error;
                rootPack->otherErrorMsg = "can't load superpack footer: " + DLC::ToString(error);

                Logger::Error("%s\ntry again", rootPack->otherErrorMsg.c_str());
                Restart();
            }
        }
    }
    else
    {
        DAVA_THROW(DAVA::Exception, "can't get status for download task");
    }
}

void PackRequest::StartLoadingPackFile()
{
    DVASSERT(!dependencies.empty());

    SubRequest& subRequest = dependencies.at(0);

    // build url to pack file and build filePath to pack file

    DLCManager::Pack& pack = *subRequest.pack;

    FilePath packPath = packManagerImpl->GetLocalPacksDirectory() + pack.name + RequestManager::packPostfix;
    String url = packManagerImpl->GetSuperPackUrl();

    // start downloading
    subRequest.taskId = packManagerImpl->DownloadPack(pack.name, packPath);

    // switch state to LoadingPackFile
    subRequest.status = SubRequest::LoadingPackFile;

    pack.state = DLCManager::Pack::Status::Downloading;

    packManagerImpl->packStateChanged.Emit(pack);
}

bool PackRequest::IsLoadingPackFileFinished()
{
    bool result = false;

    DVASSERT(!dependencies.empty());

    SubRequest& subRequest = dependencies.at(0);

    DLCManager::Pack& currentPack = *subRequest.pack;

    DownloadManager* dm = DownloadManager::Instance();
    DownloadStatus status = DL_UNKNOWN;
    if (!dm->GetStatus(subRequest.taskId, status))
    {
        DAVA_THROW(DAVA::Exception, "can't get status for download task");
    }
    uint64 progress = 0;
    switch (status)
    {
    case DL_IN_PROGRESS:
        if (dm->GetProgress(subRequest.taskId, progress))
        {
            uint64 total = 0;
            if (dm->GetTotal(subRequest.taskId, total))
            {
                if (total == 0) // empty file pack (never be)
                {
                    currentPack.downloadProgress = 1.0f;
                }
                else
                {
                    currentPack.downloadProgress = static_cast<float32>(progress) / total;
                }
                currentPack.downloadedSize = progress;
                currentPack.totalSize = total;
                // fire event on update progress
                packManagerImpl->packDownloadChanged.Emit(currentPack);
                packManagerImpl->requestProgressChanged.Emit(*this);
            }
        }
        break;
    case DL_FINISHED:
    {
        // first test error code
        DownloadError downloadError = DLE_NO_ERROR;
        if (dm->GetError(subRequest.taskId, downloadError))
        {
            if (DLE_NO_ERROR == downloadError)
            {
                dm->GetProgress(subRequest.taskId, progress);

                if (progress != currentPack.totalSizeFromDB)
                {
                    Logger::Error("size not match for downloaded pack: %s,\ntry again", currentPack.name.c_str());
                    Restart();
                }
                else
                {
                    result = true;
                    currentPack.downloadProgress = 1.0f;
                    currentPack.downloadedSize = progress;
                    currentPack.totalSize = progress;
                    packManagerImpl->packDownloadChanged.Emit(currentPack);
                    packManagerImpl->requestProgressChanged.Emit(*this);
                }
            }
            else
            {
                String errorMsg = DLC::ToString(downloadError);
                currentPack.state = DLCManager::Pack::Status::ErrorLoading;
                currentPack.downloadError = downloadError;
                currentPack.otherErrorMsg = "can't load pack: " + currentPack.name + " dlc: " + errorMsg;

                if (currentPack.name != rootPack->name)
                {
                    rootPack->state = DLCManager::Pack::Status::OtherError;
                    rootPack->otherErrorMsg = "can't load dependency: " + currentPack.name;
                }

                FilePath packPath = packManagerImpl->GetLocalPacksDirectory() + currentPack.name + RequestManager::packPostfix;

                FileSystem::Instance()->DeleteFile(packPath);

                Restart();
            }
        }
        else
        {
            DAVA_THROW(DAVA::Exception, Format("can't get download error code for pack file for pack: %s", subRequest.pack->name.c_str()));
        }
    }
    break;
    default:
        break;
    }
    return result;
}

void PackRequest::SetErrorStatusAndFireSignal(PackRequest::SubRequest& subRequest, DLCManager::Pack& currentPack)
{
    currentPack.state = DLCManager::Pack::Status::OtherError;
    subRequest.status = SubRequest::Error;

    if (rootPack->name != currentPack.name)
    {
        rootPack->state = DLCManager::Pack::Status::OtherError;
        rootPack->otherErrorMsg = "error with dependency: " + currentPack.name;
    }

    // inform user about problem with pack
    packManagerImpl->packStateChanged.Emit(currentPack);

    packManagerImpl->requestProgressChanged.Emit(*this);
}

void PackRequest::StartCheckHash()
{
    DVASSERT(Thread::IsMainThread());
    DVASSERT(!dependencies.empty());

    SubRequest& subRequest = dependencies.at(0);

    DLCManager::Pack& currentPack = *subRequest.pack;

    // calculate crc32 from PackFile
    FilePath packPath = packManagerImpl->GetLocalPacksDirectory() + subRequest.pack->name + RequestManager::packPostfix;

    if (!FileSystem::Instance()->IsFile(packPath))
    {
        DAVA_THROW(DAVA::Exception, "can't find just downloaded pack: " + packPath.GetStringValue());
    }

    uint32 realCrc32FromPack = CRC32::ForFile(packPath);

    if (realCrc32FromPack != currentPack.hashFromDB)
    {
        Logger::Error("pack crc32 from meta not match crc32 from local DB\ntry again");
        FileSystem::Instance()->DeleteFile(packPath);
        Restart();
    }
    else
    {
        subRequest.status = SubRequest::CheckHash;
    }
}

bool PackRequest::IsCheckingHashFinished()
{
    return true; // in future
}

void PackRequest::MountPack()
{
    DVASSERT(!dependencies.empty());

    SubRequest& subRequest = dependencies.at(0);

    DLCManager::Pack& pack = *subRequest.pack;

    if (pack.hashFromDB != RequestManager::emptyLZ4HCArchiveCrc32)
    {
        FilePath packPath = packManagerImpl->GetLocalPacksDirectory() + pack.name + RequestManager::packPostfix;
        FileSystem* fs = FileSystem::Instance();
        try
        {
            fs->Mount(packPath, "Data/");
        }
        catch (std::exception& ex)
        {
            Logger::Error("can't mount downloaded pack: %s", ex.what());
            fs->DeleteFile(packPath);
            Restart();
            return;
        }
    }

    subRequest.status = SubRequest::Mounted;

    pack.state = DLCManager::Pack::Status::Mounted;

    packManagerImpl->packStateChanged.Emit(pack);
}

void PackRequest::GoToNextSubRequest()
{
    if (!dependencies.empty())
    {
        dependencies.erase(begin(dependencies));
    }
}

void PackRequest::Start()
{
    Restart();
}

void PackRequest::Stop()
{
    if (!dependencies.empty())
    {
        if (!IsDone() && !IsError())
        {
            SubRequest& subRequest = dependencies.at(0);
            switch (subRequest.status)
            {
            case SubRequest::LoadingPackFile:
            {
                DownloadManager* dm = DownloadManager::Instance();
                dm->Cancel(subRequest.taskId);
                subRequest.taskId = 0;
                // start loading again this subRequest on resume
                subRequest.status = SubRequest::Wait;
            }
            break;
            default:
                break;
            }
        }
    }
}

void PackRequest::Restart()
{
    fullSizeServerData = 0;
    downloadTaskId = 0;
    Memset(&footerOnServer, 0, sizeof(footerOnServer));

    SubRequest& subRequest = dependencies.at(0);
    subRequest.status = SubRequest::AskFooter;
}

void PackRequest::Update()
{
    DVASSERT(Thread::IsMainThread());
    DVASSERT(packManagerImpl->IsInitialized());

    if (!IsDone() && !IsError())
    {
        SubRequest& subRequest = dependencies.at(0);

        switch (subRequest.status)
        {
        case SubRequest::Wait:
            Restart();
            break;
        case SubRequest::AskFooter:
            AskFooter(); // continue ask footer
            break;
        case SubRequest::GetFooter:
            GetFooter();
            break;
        case SubRequest::LoadingPackFile:
            if (IsLoadingPackFileFinished())
            {
                StartCheckHash();
            }
            break;
        case SubRequest::CheckHash:
            if (IsCheckingHashFinished())
            {
                MountPack();
            }
            break;
        case SubRequest::Mounted:
            GoToNextSubRequest();
            break;
        default:
            break;
        } // end switch status
    }
}

void PackRequest::ChangePriority(float32 newPriority)
{
    rootPack->priority = newPriority;
    for (SubRequest& subRequest : dependencies)
    {
        DLCManager::Pack& pack = *subRequest.pack;
        pack.priority = newPriority;
    }
}

bool PackRequest::IsDone() const
{
    return dependencies.empty();
}

bool PackRequest::IsError() const
{
    if (!dependencies.empty())
    {
        const SubRequest& subRequest = GetCurrentSubRequest();
        return subRequest.status == SubRequest::Error;
    }
    return false;
}

const PackRequest::SubRequest& PackRequest::GetCurrentSubRequest() const
{
    DVASSERT(!dependencies.empty());
    return dependencies.at(0); // at check index
}

uint64 PackRequest::GetFullSizeWithDependencies() const
{
    return totalAllPacksSize;
}

uint64 PackRequest::GetDownloadedSize() const
{
    uint64 result = 0ULL;

    for (auto pack : dependencyList)
    {
        result += pack->downloadedSize;
    }

    result += rootPack->downloadedSize;
    return result;
}

const DLCManager::Pack& PackRequest::GetErrorPack() const
{
    auto& subRequest = GetCurrentSubRequest();
    return *subRequest.pack;
}

const String& PackRequest::GetErrorMessage() const
{
    return rootPack->otherErrorMsg;
}

} // end namespace DAVA
