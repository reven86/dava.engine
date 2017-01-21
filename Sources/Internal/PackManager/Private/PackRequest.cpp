#include "PackManager/Private/PackRequest.h"
#include "PackManager/Private/RequestManager.h"
#include "PackManager/Private/DLCManagerImpl.h"
#include "FileSystem/Private/PackMetaData.h"
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

PackRequest::PackRequest(DLCManagerImpl& packManager_, const String& pack_, Vector<uint32> fileIndexes_)
    : packManagerImpl(packManager_)
    , fileIndexes{ std::move(fileIndexes_) }
    , requestedPackName(pack_)
{
}

PackRequest::~PackRequest()
{
    Stop();
}

void PackRequest::Start()
{
    // TODO
}

void PackRequest::Stop()
{
    // TODO
}

const String& PackRequest::GetRequestedPackName() const
{
    return requestedPackName;
}

Vector<const IDLCManager::IRequest*> PackRequest::GetDependencies() const
{
    Vector<const IRequest*> result;

    const PackMetaData& meta = packManagerImpl.GetMeta();
    const auto& packInfo = meta.GetPackInfo(requestedPackName);
    String dependencies = std::get<1>(packInfo);
    String delimiter(", ");

    Vector<String> requestNames;
    Split(dependencies, delimiter, requestNames);

    for (const String& requestName : requestNames)
    {
        const IRequest* request = packManagerImpl.FindRequest(requestName);
        if (request != nullptr)
        {
            result.push_back(request);
        }
    }

    return result;
}
/** return size of files within this request without dependencies */
uint64 PackRequest::GetSize() const
{
    uint64 allFilesSize = 0;
    for (uint32 fileIndex : fileIndexes)
    {
        const auto& fileInfo = packManagerImpl.GetPack().filesTable.data.files.at(fileIndex);
        allFilesSize += fileInfo.compressedSize;
    }
    return allFilesSize;
}
/** recalculate current downloaded size without dependencies */
uint64 PackRequest::GetDownloadedSize() const
{
    uint64 allFilesSize = 0;
    for (uint32 fileIndex : fileIndexes)
    {
        const auto& fileInfo = packManagerImpl.GetPack().filesTable.data.files.at(fileIndex);
        // TODO allFilesSize += fileInfo.;
    }
    return allFilesSize;
}
/** return true when all files loaded and ready */
bool IsDownloaded() const override;

void PackRequest::Update()
{
    DVASSERT(Thread::IsMainThread());
    DVASSERT(packManagerImpl.IsInitialized());

    // TODO
}

uint64 PackRequest::GetFullSizeWithDependencies() const
{
    return totalAllPacksSize;
}

uint64 PackRequest::GetDownloadedSize() const
{
    return downloadedSize;
}

} // end namespace DAVA
