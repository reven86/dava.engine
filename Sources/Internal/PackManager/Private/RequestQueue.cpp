#include "PackManager/Private/RequestQueue.h"
#include "FileSystem/FileSystem.h"
#include "Utils/CRC32.h"
#include "DLC/Downloader/DownloadManager.h"
#include "PackManager/Private/PackManagerImpl.h"

namespace DAVA
{
const String RequestManager::packPostfix = ".pack";
const String RequestManager::hashPostfix = ".hash";

PackRequest::PackRequest(PackManagerImpl& packManager_, PackManager::Pack& pack_)
    : packManager(&packManager_)
    , pack(&pack_)
{
    DVASSERT(packManager != nullptr);
    DVASSERT(pack != nullptr);
    // find all dependenciec
    // put it all into vector and put final pack into vector too
    Set<PackManager::Pack*> dependencySet;
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

void PackRequest::StartLoadingCRC32File()
{
    DVASSERT(!dependencies.empty());

    SubRequest& subRequest = dependencies.at(0);

    // build url to pack_name_crc32_file

    FilePath archiveCrc32Path = packManager->GetLocalPacksDir() + subRequest.pack->name + RequestManager::hashPostfix;
    String url = packManager->GetRemotePacksURL() + subRequest.pack->name + RequestManager::hashPostfix;

    // start downloading file

    DownloadManager* dm = DownloadManager::Instance();
    subRequest.taskId = dm->Download(url, archiveCrc32Path, RESUMED, 1);

    // set state to LoadingCRC32File
    subRequest.status = SubRequest::LoadingCRC32File;
}

bool PackRequest::DoneLoadingCRC32File()
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
            switch (downloadError)
            {
            case DLE_CANCELLED: // download was cancelled by our side
            case DLE_COULDNT_RESUME: // seems server doesn't supports download resuming
            case DLE_COULDNT_RESOLVE_HOST: // DNS request failed and we cannot to take IP from full qualified domain name
            case DLE_COULDNT_CONNECT: // we cannot connect to given adress at given port
            case DLE_CONTENT_NOT_FOUND: // server replies that there is no requested content
            case DLE_NO_RANGE_REQUEST: // Range requests is not supported. Use 1 thread without reconnects only.
            case DLE_COMMON_ERROR: // some common error which is rare and requires to debug the reason
            case DLE_INIT_ERROR: // any handles initialisation was unsuccessful
            case DLE_FILE_ERROR: // file read and write errors
            case DLE_UNKNOWN: // we cannot determine the error
                // inform user about error
                {
                    PackManager::Pack& pack = *subRequest.pack;

                    pack.state = PackManager::Pack::Status::ErrorLoading;
                    pack.downloadError = downloadError;
                    pack.otherErrorMsg = "can't load CRC32 file for pack: " + pack.name;

                    subRequest.status = SubRequest::Error;

                    packManager->onPackChange->Emit(pack, PackManager::Pack::Change::State);
                    break;
                }
            case DLE_NO_ERROR:
            {
                result = true;
                break;
            }
            } // end switch downloadError
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

    FilePath packPath = packManager->GetLocalPacksDir() + subRequest.pack->name + RequestManager::packPostfix;
    String url = packManager->GetRemotePacksURL() + subRequest.pack->name + RequestManager::packPostfix;

    // start downloading

    DownloadManager* dm = DownloadManager::Instance();
    subRequest.taskId = dm->Download(url, packPath);

    // switch state to LoadingPackFile
    subRequest.status = SubRequest::LoadingPackFile;

    PackManager::Pack& pack = *subRequest.pack;
    pack.state = PackManager::Pack::Status::Downloading;

    packManager->onPackChange->Emit(pack, PackManager::Pack::Change::State);
}

bool PackRequest::DoneLoadingPackFile()
{
    bool result = false;

    DVASSERT(!dependencies.empty());

    SubRequest& subRequest = dependencies.at(0);

    PackManager::Pack& pack = *subRequest.pack;

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
                    pack.downloadProgress = std::min(1.0f, static_cast<float>(progress) / total);
                    // fire event on update progress
                    packManager->onPackChange->Emit(pack, PackManager::Pack::Change::DownloadProgress);
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
            switch (downloadError)
            {
            case DLE_CANCELLED: // download was cancelled by our side
            case DLE_COULDNT_RESUME: // seems server doesn't supports download resuming
            case DLE_COULDNT_RESOLVE_HOST: // DNS request failed and we cannot to take IP from full qualified domain name
            case DLE_COULDNT_CONNECT: // we cannot connect to given adress at given port
            case DLE_CONTENT_NOT_FOUND: // server replies that there is no requested content
            case DLE_NO_RANGE_REQUEST: // Range requests is not supported. Use 1 thread without reconnects only.
            case DLE_COMMON_ERROR: // some common error which is rare and requires to debug the reason
            case DLE_INIT_ERROR: // any handles initialisation was unsuccessful
            case DLE_FILE_ERROR: // file read and write errors
            case DLE_UNKNOWN: // we cannot determine the error
                // inform user about error
                {
                    pack.state = PackManager::Pack::Status::ErrorLoading;
                    pack.downloadError = downloadError;
                    pack.otherErrorMsg = "can't load pack: " + pack.name;

                    subRequest.status = SubRequest::Error;

                    packManager->onPackChange->Emit(pack, PackManager::Pack::Change::State);
                    break;
                }
            case DLE_NO_ERROR:
            {
                result = true;

                pack.downloadProgress = 1.0f;
                packManager->onPackChange->Emit(pack, PackManager::Pack::Change::DownloadProgress);
                break;
            }
            } // end switch downloadError
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

void PackRequest::StartCheckCRC32()
{
    DVASSERT(!dependencies.empty());

    SubRequest& subRequest = dependencies.at(0);

    PackManager::Pack& pack = *subRequest.pack;

    // build crcMetaFilePath
    FilePath archiveCrc32Path = packManager->GetLocalPacksDir() + subRequest.pack->name + RequestManager::hashPostfix;
    // read crc32 from meta file
    ScopedPtr<File> crcFile(File::Create(archiveCrc32Path, File::OPEN | File::READ));
    if (!crcFile)
    {
        pack.state = PackManager::Pack::Status::OtherError;
        pack.otherErrorMsg = "can't read crc meta file";
        throw std::runtime_error("can't open just downloaded crc meta file: " + archiveCrc32Path.GetStringValue());
    }
    String fileContent;
    if (0 < crcFile->ReadString(fileContent))
    {
        StringStream ss;
        ss << std::hex << fileContent;
        ss >> pack.hashFromMeta;
    }
    // calculate crc32 from PackFile
    FilePath packPath = packManager->GetLocalPacksDir() + subRequest.pack->name + RequestManager::packPostfix;

    if (!FileSystem::Instance()->IsFile(packPath))
    {
        throw std::runtime_error("can't find just downloaded pack: " + packPath.GetStringValue());
    }

    // TODO if it take lot of time move to job on other thread and wait
    uint32 realCrc32FromPack = CRC32::ForFile(packPath);

    if (realCrc32FromPack != pack.hashFromMeta)
    {
        pack.state = PackManager::Pack::Status::OtherError;
        pack.otherErrorMsg = "calculated pack crc32 not match with crc32 from meta";
        // inform user about problem with pack
        packManager->onPackChange->Emit(pack, PackManager::Pack::Change::State);
    }
    else if (pack.hashFromMeta != pack.hashFromDB)
    {
        pack.state = PackManager::Pack::Status::OtherError;
        pack.otherErrorMsg = "pack crc32 from meta not match crc32 from local DB";

        // inform user about problem with pack
        packManager->onPackChange->Emit(pack, PackManager::Pack::Change::State);
    }
    else
    {
        subRequest.status = SubRequest::CheckCRC32;
    }
}

bool PackRequest::DoneCheckingCRC32()
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

    packManager->onPackChange->Emit(pack, PackManager::Pack::Change::State);
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

void PackRequest::Pause()
{
    if (!dependencies.empty())
    {
        if (!IsDone() && !IsError())
        {
            SubRequest& subRequest = dependencies.at(0);
            switch (subRequest.status)
            {
            case SubRequest::LoadingCRC32File:
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
            StartLoadingCRC32File();
            break;
        case SubRequest::LoadingCRC32File:
            if (DoneLoadingCRC32File())
            {
                StartLoadingPackFile();
            }
            break;
        case SubRequest::LoadingPackFile:
            if (DoneLoadingPackFile())
            {
                StartCheckCRC32();
            }
            break;
        case SubRequest::CheckCRC32:
            if (DoneCheckingCRC32())
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
            packManager->onPackChange->Emit(pack, PackManager::Pack::Change::Priority);
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

void RequestManager::Start()
{
    if (packManager.IsProcessingEnabled())
    {
        if (!Empty())
        {
            Top().Start();
        }
    }
}

void RequestManager::Stop()
{
    if (!Empty())
    {
        Top().Pause();
    }
}

void RequestManager::Update()
{
    if (!Empty())
    {
        PackRequest& request = Top();
        request.Update();
        if (request.IsDone())
        {
            Pop();
        }
        else if (request.IsError())
        {
            const PackRequest::SubRequest& subRequest = request.GetCurrentSubRequest();
            if (request.GetPackName() != subRequest.pack->name)
            {
                PackManager::Pack& rootPack = packManager.GetPack(request.GetPackName());
                rootPack.state = PackManager::Pack::Status::OtherError;
                rootPack.otherErrorMsg = Format("can't load (%s) pack becouse dependent (%s) pack error: %s",
                                                rootPack.name.c_str(), subRequest.pack->name.c_str(), subRequest.errorMsg.c_str());

                Pop(); // first pop current request and only then inform user

                packManager.onPackChange->Emit(rootPack, PackManager::Pack::Change::State);
            }
            else
            {
                // we already inform client about error in subRequest during Update()
            }
        }
    }
}

bool RequestManager::IsInQueue(const String& packName) const
{
    auto it = std::find_if(begin(items), end(items), [packName](const PackRequest& r) -> bool
                           {
                               return r.GetPackName() == packName;
                           });
    return it != end(items);
}

bool RequestManager::Empty() const
{
    return items.empty();
}

size_type RequestManager::Size() const
{
    return static_cast<size_type>(items.size());
}

PackRequest& RequestManager::Top()
{
    DVASSERT(!items.empty());

    PackRequest& topItem = items.front();
    return topItem;
}

PackRequest& RequestManager::Find(const String& packName)
{
    auto it = std::find_if(begin(items), end(items), [packName](const PackRequest& r) -> bool
                           {
                               return r.GetPackName() == packName;
                           });
    if (it == end(items))
    {
        throw std::runtime_error("can't fined pack by name: " + packName);
    }
    return *it;
}

void RequestManager::CheckRestartLoading()
{
    DVASSERT(!items.empty());

    PackRequest& top = Top();

    if (Size() == 1)
    {
        currrentTopLoadingPack = top.GetPackName();
        top.Start();
    }
    else if (!currrentTopLoadingPack.empty() && top.GetPackName() != currrentTopLoadingPack)
    {
        // we have to cancel current pack request and start new with higher priority
        PackRequest& prevTopRequest = Find(currrentTopLoadingPack);
        prevTopRequest.Pause();
        currrentTopLoadingPack = top.GetPackName();
        top.Start();
    }
}

void RequestManager::Push(const String& packName, float32 priority)
{
    if (IsInQueue(packName))
    {
        throw std::runtime_error("second time push same pack in queue, pack: " + packName);
    }

    PackManager::Pack& pack = packManager.GetPack(packName);

    pack.state = PackManager::Pack::Status::Requested;
    pack.priority = priority;

    items.emplace_back(packManager, pack);
    std::push_heap(begin(items), end(items));

    packManager.onPackChange->Emit(pack, PackManager::Pack::Change::State);
    packManager.onPackChange->Emit(pack, PackManager::Pack::Change::Priority);

    CheckRestartLoading();
}

void RequestManager::UpdatePriority(const String& packName, float32 newPriority)
{
    if (IsInQueue(packName))
    {
        PackRequest& packRequest = Find(packName);
        if (packRequest.GetPriority() != newPriority)
        {
            packRequest.ChangePriority(newPriority);
            std::sort_heap(begin(items), end(items));

            CheckRestartLoading();
        }
    }
}

void RequestManager::Pop()
{
    DVASSERT(!items.empty());

    std::pop_heap(begin(items), end(items));
    items.pop_back();
}

} // end namespace DAVA
