#include "PackManager/Private/PackRequest.h"
#include "PackManager/Private/RequestManager.h"
#include "PackManager/Private/PackManagerImpl.h"
#include "DLC/Downloader/DownloadManager.h"
#include "FileSystem/FileSystem.h"
#include "Utils/CRC32.h"
#include "Utils/StringFormat.h"
#include "DLC/DLC.h"

namespace DAVA
{
PackRequest::PackRequest(PackManagerImpl& packManager_, IPackManager::Pack& pack_)
    : packManagerImpl(&packManager_)
    , rootPack(&pack_)
{
    DVASSERT(packManagerImpl != nullptr);
    DVASSERT(rootPack != nullptr);
    // find all dependencies
    // put it all into vector and put final pack into vector too
    PackManagerImpl::CollectDownloadableDependency(*packManagerImpl, rootPack->name, dependencyList);

    dependencies.reserve(dependencyList.size() + 1);

    for (IPackManager::Pack* depPack : dependencyList)
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
                            throw std::runtime_error("can't get size of file on server side");
                        }
                    }
                    else
                    {
                        throw std::runtime_error("can't get size of superpack from server");
                    }
                }
            }
        }
    }
    else
    {
        if (fullSizeServerData < sizeof(PackFormat::PackFile))
        {
            throw std::runtime_error("too small superpack on server");
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
                uint32 crc32 = CRC32::ForBuffer(reinterpret_cast<char*>(&footerOnServer.info), sizeof(footerOnServer.info));
                if (crc32 != footerOnServer.infoCrc32)
                {
                    throw std::runtime_error("downloaded superpack footer is broken: " + packManagerImpl->GetSuperPackUrl());
                }

                if (packManagerImpl->GetInitFooter().infoCrc32 != footerOnServer.infoCrc32)
                {
                    // server Superpack changed during current session
                    throw std::runtime_error("during pack request server superpack file changed, crc32 not match, url: " + packManagerImpl->GetSuperPackUrl());
                }

                StartLoadingPackFile();
            }
            else
            {
                rootPack->state = IPackManager::Pack::Status::ErrorLoading;
                rootPack->downloadError = error;
                rootPack->otherErrorMsg = "can't load superpack footer";

                packManagerImpl->requestProgressChanged.Emit(*this);
            }
        }
    }
    else
    {
        throw std::runtime_error("can't get status for download task");
    }
}

void PackRequest::StartLoadingPackFile()
{
    DVASSERT(!dependencies.empty());

    SubRequest& subRequest = dependencies.at(0);

    // build url to pack file and build filePath to pack file

    IPackManager::Pack& pack = *subRequest.pack;

    FilePath packPath = packManagerImpl->GetLocalPacksDirectory() + pack.name + RequestManager::packPostfix;
    String url = packManagerImpl->GetSuperPackUrl();

    // start downloading
    subRequest.taskId = packManagerImpl->DownloadPack(pack.name, packPath);

    // switch state to LoadingPackFile
    subRequest.status = SubRequest::LoadingPackFile;

    pack.state = IPackManager::Pack::Status::Downloading;

    packManagerImpl->packStateChanged.Emit(pack);
}

bool PackRequest::IsLoadingPackFileFinished()
{
    bool result = false;

    DVASSERT(!dependencies.empty());

    SubRequest& subRequest = dependencies.at(0);

    IPackManager::Pack& currentPack = *subRequest.pack;

    DownloadManager* dm = DownloadManager::Instance();
    DownloadStatus status = DL_UNKNOWN;
    dm->GetStatus(subRequest.taskId, status);
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
                    // skip current iteration  pack.downloadProgress = 1.0f;
                }
                else
                {
                    currentPack.downloadProgress = std::min(1.0f, static_cast<float32>(progress) / total);
                    currentPack.downloadedSize = static_cast<uint32>(progress);
                    currentPack.totalSize = static_cast<uint32>(total);
                    // fire event on update progress
                    packManagerImpl->packDownloadChanged.Emit(currentPack);
                    packManagerImpl->requestProgressChanged.Emit(*this);
                }
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
                result = true;

                dm->GetProgress(subRequest.taskId, progress);

                currentPack.downloadProgress = 1.0f;
                currentPack.downloadedSize = progress;
                packManagerImpl->packDownloadChanged.Emit(currentPack);
            }
            else
            {
                String errorMsg = DLC::ToString(downloadError);
                currentPack.state = IPackManager::Pack::Status::ErrorLoading;
                currentPack.downloadError = downloadError;
                currentPack.otherErrorMsg = "can't load pack: " + currentPack.name + " dlc: " + errorMsg;

                if (currentPack.name != rootPack->name)
                {
                    rootPack->state = IPackManager::Pack::Status::OtherError;
                    rootPack->otherErrorMsg = "can't load dependency: " + currentPack.name;
                }

                subRequest.status = SubRequest::Error;

                packManagerImpl->packStateChanged.Emit(currentPack);
            }
            packManagerImpl->requestProgressChanged.Emit(*this);
        }
        else
        {
            throw std::runtime_error(Format("can't get download error code for pack file for pack: %s", subRequest.pack->name.c_str()));
        }
    }
    break;
    default:
        break;
    }
    return result;
}

void PackRequest::SetErrorStatusAndFireSignal(PackRequest::SubRequest& subRequest, IPackManager::Pack& currentPack)
{
    currentPack.state = IPackManager::Pack::Status::OtherError;
    subRequest.status = SubRequest::Error;

    if (rootPack->name != currentPack.name)
    {
        rootPack->state = IPackManager::Pack::Status::OtherError;
        rootPack->otherErrorMsg = "error with dependency: " + currentPack.name;
    }

    // inform user about problem with pack
    packManagerImpl->packStateChanged.Emit(currentPack);

    packManagerImpl->requestProgressChanged.Emit(*this);
}

void PackRequest::StartCheckHash()
{
    DVASSERT(!dependencies.empty());

    SubRequest& subRequest = dependencies.at(0);

    IPackManager::Pack& currentPack = *subRequest.pack;

    // calculate crc32 from PackFile
    FilePath packPath = packManagerImpl->GetLocalPacksDirectory() + subRequest.pack->name + RequestManager::packPostfix;

    if (!FileSystem::Instance()->IsFile(packPath))
    {
        throw std::runtime_error("can't find just downloaded pack: " + packPath.GetStringValue());
    }

    uint32 realCrc32FromPack = CRC32::ForFile(packPath);

    if (realCrc32FromPack != currentPack.hashFromDB)
    {
        currentPack.otherErrorMsg = "pack crc32 from meta not match crc32 from local DB";

        SetErrorStatusAndFireSignal(subRequest, currentPack);
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

    IPackManager::Pack& pack = *subRequest.pack;

    if (pack.hashFromDB != RequestManager::emptyLZ4HCArchiveCrc32)
    {
        FilePath packPath = packManagerImpl->GetLocalPacksDirectory() + pack.name + RequestManager::packPostfix;
        FileSystem* fs = FileSystem::Instance();
        fs->Mount(packPath, "Data/");
    }

    subRequest.status = SubRequest::Mounted;

    pack.state = IPackManager::Pack::Status::Mounted;

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
    // do nothing
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

void PackRequest::ClearSuperpackData()
{
    fullSizeServerData = 0;
    downloadTaskId = 0;
    std::memset(&footerOnServer, 0, sizeof(footerOnServer));

    SubRequest& subRequest = dependencies.at(0);
    subRequest.status = SubRequest::AskFooter;
}

void PackRequest::Update()
{
    if (!IsDone() && !IsError())
    {
        SubRequest& subRequest = dependencies.at(0);

        switch (subRequest.status)
        {
        case SubRequest::Wait:
            ClearSuperpackData();
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
    for (SubRequest& subRequest : dependencies)
    {
        IPackManager::Pack& pack = *subRequest.pack;
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

const IPackManager::Pack& PackRequest::GetErrorPack() const
{
    auto& subRequest = GetCurrentSubRequest();
    return *subRequest.pack;
}

const String& PackRequest::GetErrorMessage() const
{
    return rootPack->otherErrorMsg;
}

} // end namespace DAVA
