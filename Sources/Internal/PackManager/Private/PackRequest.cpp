#include "PackManager/Private/PackRequest.h"
#include "PackManager/Private/RequestManager.h"
#include "PackManager/Private/PackManagerImpl.h"
#include "DLC/Downloader/DownloadManager.h"
#include "FileSystem/FileSystem.h"
#include "Utils/CRC32.h"

namespace DAVA
{
PackRequest::PackRequest(PackManagerImpl& packManager_, PackManager::Pack& pack_)
    : packManager(&packManager_)
    , pack(&pack_)
{
    DVASSERT(packManager != nullptr);
    DVASSERT(pack != nullptr);
    // find all dependenciec
    // put it all into vector and put final pack into vector too
    CollectDownlodbleDependency(pack->name, dependencySet);

    if (pack->hashFromDB != 0) // not fully virtual pack
    {
        dependencies.reserve(dependencySet.size() + 1);
    }
    else
    {
        dependencies.reserve(dependencySet.size());
    }

    for (PackManager::Pack* depPack : dependencySet)
    {
        SubRequest subRequest;

        subRequest.pack = depPack;
        subRequest.status = SubRequest::Wait;
        subRequest.taskId = 0;

        dependencies.push_back(subRequest);
    }

    // last step download pack itself (if it not virtual)
    if (pack->hashFromDB != 0)
    {
        SubRequest subRequest;

        subRequest.pack = pack;
        subRequest.status = SubRequest::Wait;
        subRequest.taskId = 0;
        dependencies.push_back(subRequest);
    }

    std::for_each(begin(dependencies), end(dependencies), [&](const SubRequest& request)
                  {
                      totalAllPacksSize += request.pack->totalSizeFromDB;
                  });
}

void PackRequest::CollectDownlodbleDependency(const String& packName, Set<PackManager::Pack*>& dependency)
{
    const PackManager::Pack& packState = packManager->GetPack(packName);
    for (const String& dependName : packState.dependency)
    {
        PackManager::Pack& dependPack = packManager->GetPack(dependName);
        if (dependPack.hashFromDB != 0 && dependPack.state != PackManager::Pack::Status::Mounted)
        {
            dependency.insert(&dependPack);
        }

        CollectDownlodbleDependency(dependName, dependency);
    }
}

void PackRequest::StartLoadingHashFile()
{
    DVASSERT(!dependencies.empty());

    SubRequest& subRequest = dependencies.at(0);

    // build url to pack_name_crc32_file

    PackManager::Pack& pack = *subRequest.pack;
    FilePath archiveCrc32Path = packManager->GetLocalPacksDir() + pack.name + RequestManager::hashPostfix;
    String url = packManager->GetRemotePacksURL(pack.isGPU) + pack.name + RequestManager::hashPostfix;

    // start downloading file

    DownloadManager* dm = DownloadManager::Instance();
    subRequest.taskId = dm->Download(url, archiveCrc32Path, RESUMED, 1);

    // set state to LoadingCRC32File
    subRequest.status = SubRequest::LoadingHaskFile;
}

static String DownloadErrorToString(DownloadError e)
{
    String errorMsg;
    switch (e)
    {
    case DLE_CANCELLED: // download was cancelled by our side
        errorMsg = "DLE_CANCELLED";
        break;
    case DLE_COULDNT_RESUME: // seems server doesn't supports download resuming
        errorMsg = "DLE_COULDNT_RESUME";
        break;
    case DLE_COULDNT_RESOLVE_HOST: // DNS request failed and we cannot to take IP from full qualified domain name
        errorMsg = "DLE_COULDNT_RESOLVE_HOST";
        break;
    case DLE_COULDNT_CONNECT: // we cannot connect to given adress at given port
        errorMsg = "DLE_COULDNT_CONNECT";
        break;
    case DLE_CONTENT_NOT_FOUND: // server replies that there is no requested content
        errorMsg = "DLE_CONTENT_NOT_FOUND";
        break;
    case DLE_NO_RANGE_REQUEST: // Range requests is not supported. Use 1 thread without reconnects only.
        errorMsg = "DLE_NO_RANGE_REQUEST";
        break;
    case DLE_COMMON_ERROR: // some common error which is rare and requires to debug the reason
        errorMsg = "DLE_COMMON_ERROR";
        break;
    case DLE_INIT_ERROR: // any handles initialisation was unsuccessful
        errorMsg = "DLE_INIT_ERROR";
        break;
    case DLE_FILE_ERROR: // file read and write errors
        errorMsg = "DLE_FILE_ERROR";
        break;
    case DLE_UNKNOWN: // we cannot determine the error
        errorMsg = "DLE_UNKNOWN";
        break;
    case DLE_NO_ERROR:
    {
        break;
    }
    default:
        DVASSERT(false);
        break;
    } // end switch downloadError
    return errorMsg;
}

bool PackRequest::IsLoadingHashFileFinished()
{
    bool result = false;

    DVASSERT(!dependencies.empty());

    SubRequest& subRequest = dependencies.at(0);

    DownloadManager* dm = DownloadManager::Instance();
    DownloadStatus status = DL_UNKNOWN;
    dm->GetStatus(subRequest.taskId, status);
    uint64 progress = 0;
    switch (status)
    {
    case DL_IN_PROGRESS:
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
            }
            else
            {
                String errorMsg = DownloadErrorToString(downloadError);

                // inform user about error
                {
                    PackManager::Pack& currentPack = *subRequest.pack;

                    currentPack.downloadError = downloadError;
                    currentPack.otherErrorMsg = "can't load CRC32 file for pack: " + currentPack.name + " dlc: " + errorMsg;

                    currentPack.state = PackManager::Pack::Status::ErrorLoading;

                    subRequest.status = SubRequest::Error;

                    packManager->onPackChange->Emit(currentPack);
                    packManager->onRequestChange->Emit(*this);
                    break;
                }
            }
        }
        else
        {
            throw std::runtime_error(Format("can't get download error code for download crc file for pack: %s", subRequest.pack->name.c_str()));
        }
    }
    break;
    default:
        break;
    }
    return result;
}

void PackRequest::StartLoadingPackFile()
{
    DVASSERT(!dependencies.empty());

    SubRequest& subRequest = dependencies.at(0);

    // build url to pack file and build filePath to pack file

    PackManager::Pack& pack = *subRequest.pack;

    FilePath packPath = packManager->GetLocalPacksDir() + pack.name + RequestManager::packPostfix;
    String url = packManager->GetRemotePacksURL(pack.isGPU) + pack.name + RequestManager::packPostfix;

    // start downloading

    DownloadManager* dm = DownloadManager::Instance();
    subRequest.taskId = dm->Download(url, packPath);

    // switch state to LoadingPackFile
    subRequest.status = SubRequest::LoadingPackFile;

    pack.state = PackManager::Pack::Status::Downloading;

    packManager->onPackChange->Emit(pack);
}

bool PackRequest::IsLoadingPackFileFinished()
{
    bool result = false;

    DVASSERT(!dependencies.empty());

    SubRequest& subRequest = dependencies.at(0);

    PackManager::Pack& currentPack = *subRequest.pack;

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
                    packManager->packDownload->Emit(currentPack);
                    packManager->onRequestChange->Emit(*this);
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
                packManager->packDownload->Emit(currentPack);
            }
            else
            {
                String errorMsg = DownloadErrorToString(downloadError);
                currentPack.state = PackManager::Pack::Status::ErrorLoading;
                currentPack.downloadError = downloadError;
                currentPack.otherErrorMsg = "can't load pack: " + currentPack.name + " dlc: " + errorMsg;

                if (currentPack.name != pack->name)
                {
                    pack->state = PackManager::Pack::Status::OtherError;
                    pack->otherErrorMsg = "can't load dependency: " + currentPack.name;
                }

                subRequest.status = SubRequest::Error;

                packManager->onPackChange->Emit(currentPack);
            }
            packManager->onRequestChange->Emit(*this);
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

void PackRequest::SetErrorStatusAndFireSignal(PackRequest::SubRequest& subRequest, PackManager::Pack& currentPack)
{
    currentPack.state = PackManager::Pack::Status::OtherError;
    subRequest.status = SubRequest::Error;

    if (pack->name != currentPack.name)
    {
        pack->state = PackManager::Pack::Status::OtherError;
        pack->otherErrorMsg = "error with dependency: " + currentPack.name;
    }

    // inform user about problem with pack
    packManager->onPackChange->Emit(currentPack);

    packManager->onRequestChange->Emit(*this);
}

void PackRequest::StartCheckHash()
{
    DVASSERT(!dependencies.empty());

    SubRequest& subRequest = dependencies.at(0);

    PackManager::Pack& currentPack = *subRequest.pack;

    // build crcMetaFilePath
    FilePath archiveCrc32Path = packManager->GetLocalPacksDir() + subRequest.pack->name + RequestManager::hashPostfix;
    // read crc32 from meta file
    ScopedPtr<File> crcFile(File::Create(archiveCrc32Path, File::OPEN | File::READ));
    if (!crcFile)
    {
        currentPack.state = PackManager::Pack::Status::OtherError;
        currentPack.otherErrorMsg = "can't read crc meta file";
        throw std::runtime_error("can't open just downloaded crc meta file: " + archiveCrc32Path.GetStringValue());
    }
    String fileContent;
    if (0 < crcFile->ReadString(fileContent))
    {
        StringStream ss;
        ss << std::hex << fileContent;
        ss >> currentPack.hashFromMeta;
    }
    // calculate crc32 from PackFile
    FilePath packPath = packManager->GetLocalPacksDir() + subRequest.pack->name + RequestManager::packPostfix;

    if (!FileSystem::Instance()->IsFile(packPath))
    {
        throw std::runtime_error("can't find just downloaded pack: " + packPath.GetStringValue());
    }

    // TODO if it take lot of time move to job on other thread and wait
    uint32 realCrc32FromPack = CRC32::ForFile(packPath);

    if (realCrc32FromPack != currentPack.hashFromMeta)
    {
        currentPack.otherErrorMsg = "calculated pack crc32 not match with crc32 from meta";

        SetErrorStatusAndFireSignal(subRequest, currentPack);
    }
    else if (currentPack.hashFromMeta != currentPack.hashFromDB)
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

    PackManager::Pack& pack = *subRequest.pack;

    if (pack.hashFromDB != RequestManager::emptyZipArchiveHash)
    {
        FilePath packPath = packManager->GetLocalPacksDir() + pack.name + RequestManager::packPostfix;
        FileSystem* fs = FileSystem::Instance();
        fs->Mount(packPath, "Data/");
    }

    subRequest.status = SubRequest::Mounted;

    pack.state = PackManager::Pack::Status::Mounted;

    packManager->onPackChange->Emit(pack);
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
            case SubRequest::LoadingHaskFile:
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

void PackRequest::Update()
{
    if (!IsDone() && !IsError())
    {
        SubRequest& subRequest = dependencies.at(0);

        switch (subRequest.status)
        {
        case SubRequest::Wait:
            StartLoadingHashFile();
            break;
        case SubRequest::LoadingHaskFile:
            if (IsLoadingHashFileFinished())
            {
                StartLoadingPackFile();
            }
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
        PackManager::Pack& pack = *subRequest.pack;
        if (pack.priority < newPriority)
        {
            pack.priority = newPriority;
        }
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
    uint64 result = 0;
    std::for_each(begin(dependencySet), end(dependencySet), [&](PackManager::Pack* p)
                  {
                      result += p->downloadedSize;
                  });

    result += pack->downloadedSize;
    return result;
}

const String& PackRequest::GetErrorMessage() const
{
    return pack->otherErrorMsg;
}

} // end namespace DAVA
