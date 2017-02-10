#include "DLCManager/Private/PackRequest.h"
#include "DLCManager/Private/DLCManagerImpl.h"
#include "FileSystem/Private/PackMetaData.h"
#include "DLC/Downloader/DownloadManager.h"
#include "FileSystem/FileSystem.h"
#include "Utils/CRC32.h"
#include "Utils/Utils.h"
#include "Logger/Logger.h"
#include "DLC/DLC.h"

#include <numeric>

namespace DAVA
{
PackRequest::PackRequest(DLCManagerImpl& packManager_, const String& pack_)
    : packManagerImpl(packManager_)
    , requestedPackName(pack_)
    , delayedRequest(true)
{
}

PackRequest::PackRequest(DLCManagerImpl& packManager_, const String& pack_, Vector<uint32> fileIndexes_)
    : packManagerImpl(packManager_)
    , requestedPackName(pack_)
    , delayedRequest(false)
{
    SetFileIndexes(std::move(fileIndexes_));
}

PackRequest::~PackRequest()
{
    if (taskId != 0)
    {
        DownloadManager::Instance()->Cancel(taskId);
        taskId = 0;
    }
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

Vector<String> PackRequest::GetDependencies() const
{
    Vector<String> requestNames;

    const PackMetaData& meta = packManagerImpl.GetMeta();
    const PackMetaData::PackInfo& packInfo = meta.GetPackInfo(requestedPackName);
    const String& dependencies = packInfo.packDependencies;
    String delimiter(", ");

    Split(dependencies, delimiter, requestNames);

    // convert every name from string representation of index to packName
    for (String& pack : requestNames)
    {
        uint32 index = 0;
        try
        {
            unsigned long i = stoul(pack);
            index = static_cast<uint32>(i);
        }
        catch (std::exception& ex)
        {
            Logger::Error("bad dependency index for pack: %s, index value: %s, error message: %s", packInfo.packName.c_str(), pack.c_str(), ex.what());
            DAVA_THROW(Exception, "bad dependency pack index see log");
        }
        const PackMetaData::PackInfo& info = meta.GetPackInfo(index);
        pack = info.packName;
    }

    return requestNames;
}
/** return size of files within this request without dependencies */
uint64 PackRequest::GetSize() const
{
    uint64 allFilesSize = 0;
    const auto& files = packManagerImpl.GetPack().filesTable.data.files;
    for (uint32 fileIndex : fileIndexes)
    {
        const auto& fileInfo = files.at(fileIndex);
        allFilesSize += (fileInfo.compressedSize + sizeof(PackFormat::LitePack::Footer));
    }
    return allFilesSize;
}
/** recalculate current downloaded size without dependencies */
uint64 PackRequest::GetDownloadedSize() const
{
    for (const auto& fileRequest : requests)
    {
    }
    uint64 requestsSize = std::accumulate(begin(requests), end(requests), uint64(0), [](uint64 sum, const FileRequest& r) {
        return sum + r.prevDownloadedSize;
    });
    return downloadedSize + requestsSize;
}
/** return true when all files loaded and ready */
bool PackRequest::IsDownloaded() const
{
    bool allReady = fileIndexes.size() == requests.size();
    for (const FileRequest& r : requests)
    {
        if (r.status != Ready)
        {
            allReady = false;
            break;
        }
    }
    return allReady;
    return !delayedRequest && numOfDownloadedFile == fileIndexes.size();
}

void PackRequest::SetFileIndexes(Vector<uint32> fileIndexes_)
{
    fileIndexes = std::move(fileIndexes_);
    delayedRequest = false;

    if (fileIndexes.empty())
    {
        // all files already loaded or empty virtual pack
        status = Ready;
        packManagerImpl.requestUpdated.Emit(*this);
    }
}

void PackRequest::InitializeFileRequests()
{
    if (fileIndexes.size() != requests.size())
    {
        requests.clear();
        requests.resize(fileIndexes.size());

        for (size_t requestIndex = 0; requestIndex < requests.size(); ++requestIndex)
        {
            uint32 fileIndex = fileIndexes.at(requestIndex);
            const auto& fileInfo = packManagerImpl.GetPack().filesTable.data.files.at(fileIndex);
            String relativePath = packManagerImpl.GetRelativeFilePath(fileIndex);
            FilePath localPath = packManagerImpl.GetLocalPacksDirectory() + relativePath;

            FileRequest& request = requests.at(requestIndex);

            InitializeFileRequest(fileIndex,
                                  localPath,
                                  fileInfo.compressedCrc32,
                                  fileInfo.startPosition,
                                  fileInfo.compressedSize,
                                  fileInfo.originalSize,
                                  packManagerImpl.GetSuperPackUrl(),
                                  fileInfo.type,
                                  request);
        }
    }
}

void PackRequest::Update()
{
    DVASSERT(Thread::IsMainThread());
    DVASSERT(packManagerImpl.IsInitialized());

    if (numOfDownloadedFile < fileIndexes.size())
    {
        if (requests.empty())
        {
            InitializeFileRequests();
        }

        if (IsDownloaded())
        {
            uint32 countChecksPerUpdate = packManagerImpl.GetHints().checkLocalFileExistPerUpdate;
        }
        else
        {
            UpdateFileRequests();
        }
    }
}

void PackRequest::InitializeFileRequest(const uint32 fileIndex_,
                                        const FilePath& fileName_,
                                        const uint32 hash_,
                                        const uint64 startLoadingPos_,
                                        const uint64 fileComressedSize_,
                                        const uint64 fileUncompressedSize_,
                                        const String& url_,
                                        const Compressor::Type compressionType_,
                                        FileRequest& fileRequest)
{
    fileRequest.localFile = fileName_ + ".dvpl";
    fileRequest.hashFromMeta = hash_;
    fileRequest.startLoadingPos = startLoadingPos_;
    fileRequest.sizeOfCompressedFile = fileComressedSize_;
    fileRequest.sizeOfUncompressedFile = fileUncompressedSize_;
    fileRequest.fileIndex = fileIndex_;
    fileRequest.status = Wait;
    fileRequest.taskId = 0;
    fileRequest.url = url_;
    fileRequest.prevDownloadedSize = 0;
    fileRequest.compressionType = compressionType_;
}

void PackRequest::UpdateFileRequests()
{
    for (FileRequest& fileRequest : requests)
    {
    case Wait:
        status = CheckLocalFile;
        break;
    case CheckLocalFile:
    {
        if (packManagerImpl.IsFileReady(fileIndex))
        {
            status = Ready;
            packManagerImpl.requestUpdated.Emit(*this);
        }
        break;
    case LoadingPackFile:
    {
        status = LoadingPackFile;
    }
    }
    break;
    case LoadingPackFile:
    {
        DownloadManager* dm = DownloadManager::Instance();
        if (taskId == 0)
        {
            FileSystem::Instance()->DeleteFile(localFile); // just in case (hash not match, size not match...)
            taskId = dm->DownloadRange(url, localFile, startLoadingPos, sizeOfCompressedFile);
        }
        else
        {
            DownloadStatus downloadStatus;
            if (dm->GetStatus(taskId, downloadStatus))
            {
                fileRequest.taskId = dm->DownloadRange(fileRequest.url,
                                                       fileRequest.localFile,
                                                       fileRequest.startLoadingPos,
                                                       fileRequest.sizeOfCompressedFile);
            }
            else
            {
                // TODO move to separete function
                DownloadStatus downloadStatus;
                if (dm->GetStatus(fileRequest.taskId, downloadStatus))
                {
                    switch (downloadStatus)
                    {
                    case DL_PENDING:
                        break;
                    case DL_IN_PROGRESS:
                    {
                        uint64 progress = 0;
                        if (dm->GetProgress(fileRequest.taskId, progress))
                        {
                            fileRequest.prevDownloadedSize = progress;
                            uint64 s = GetSize();
                            uint64 ds = GetDownloadedSize();
                            DVASSERT(s > 0);
                            DVASSERT(s >= ds);
                            packManagerImpl.requestUpdated.Emit(*this);
                        }
                    }
                    break;
                    case DL_FINISHED:
                    {
                        dm->GetTotal(fileRequest.taskId, fileRequest.prevDownloadedSize);
                        fileRequest.taskId = 0;
                        fileRequest.status = CheckHash;
                    }
                    break;
                    case DL_UNKNOWN:
                        break;
                    }
                }
                break;
            case DL_FINISHED:
            {
                fileRequest.taskId = 0;
                fileRequest.status = CheckLocalFile;
            }
            break;
            case DL_UNKNOWN:
                break;
            }
        }
    }
    }
    break;
case CheckHash:
{
    prevDownloadedSize = 0;

    {
        ScopedPtr<File> f(File::Create(fileRequest.localFile, File::WRITE | File::APPEND | File::OPEN));
        f->Write(&footer, sizeof(footer));
    }

    downloadedSize += (fileRequest.sizeOfCompressedFile + sizeof(footer));

    packManagerImpl.requestUpdated.Emit(*this);
    fileRequest.status = Ready;
}
    else
    {
        // try download again
        FileSystem::Instance()->DeleteFile(fileRequest.localFile);
        fileRequest.status = LoadingPackFile;
    }

    downloadedSize += (sizeOfCompressedFile + sizeof(footer));

    status = Ready;
    packManagerImpl.requestUpdated.Emit(*this);
}

break;
case Ready:
break;
case Error:
break;
}
} // end for requests
}

} // end namespace DAVA
