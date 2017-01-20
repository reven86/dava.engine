#include "PackManager/Private/PackRequest.h"
#include "PackManager/Private/RequestManager.h"
#include "PackManager/Private/DLCManagerImpl.h"
#include "DLC/Downloader/DownloadManager.h"
#include "FileSystem/FileSystem.h"
#include "Utils/CRC32.h"
#include "Utils/StringFormat.h"
#include "Logger/Logger.h"
#include "DLC/DLC.h"
#include "Base/Exception.h"

namespace DAVA
{
PackRequest::PackRequest(DLCManagerImpl& packManager_, const String& pack_)
    : packManagerImpl(packManager_)
    , requestedPackName(pack_)
{
}

void PackRequest::Start()
{
    // TODO
}

void PackRequest::Stop()
{
    // TODO
}

void PackRequest::Restart()
{
    // TODO
}

void PackRequest::Update()
{
    DVASSERT(Thread::IsMainThread());
    DVASSERT(packManagerImpl.IsInitialized());

    if (!IsDone() && !IsError())
    {
        FileRequest& subRequest = dependencies.at(0);

        switch (subRequest.status)
        {
        case FileRequest::Wait:
            Restart();
            break;
        case FileRequest::AskFooter:
            AskFooter(); // continue ask footer
            break;
        case FileRequest::GetFooter:
            GetFooter();
            break;
        case FileRequest::LoadingPackFile:
            if (IsLoadingPackFileFinished())
            {
                StartCheckHash();
            }
            break;
        case FileRequest::CheckHash:
            if (IsCheckingHashFinished())
            {
                MountPack();
            }
            break;
        case FileRequest::Mounted:
            GoToNextSubRequest();
            break;
        default:
            break;
        } // end switch status
    }
}

void PackRequest::ChangePriority(float32 newPriority)
{
    requestedPackName->priority = newPriority;
    for (FileRequest& subRequest : dependencies)
    {
        IDLCManager::Pack& pack = *subRequest.pack;
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
        const FileRequest& subRequest = GetCurrentSubRequest();
        return subRequest.status == FileRequest::Error;
    }
    return false;
}

const PackRequest::FileRequest& PackRequest::GetCurrentSubRequest() const
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

    for (auto pack : dependencyPacks)
    {
        result += pack->downloadedSize;
    }

    result += requestedPackName->downloadedSize;
    return result;
}

const IDLCManager::Pack& PackRequest::GetErrorPack() const
{
    auto& subRequest = GetCurrentSubRequest();
    return *subRequest.pack;
}

const String& PackRequest::GetErrorMessage() const
{
    return requestedPackName->otherErrorMsg;
}

} // end namespace DAVA
