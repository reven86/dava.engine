#include "DLCManager/Private/PackRequest.h"
#include "DLCManager/Private/DLCManagerImpl.h"
#include "FileSystem/Private/PackMetaData.h"
#include "FileSystem/FileSystem.h"
#include "Utils/CRC32.h"
#include "Utils/Utils.h"
#include "Logger/Logger.h"
#include "DLC/DLC.h"

#include <numeric>

namespace DAVA
{
PackRequest::PackRequest(DLCManagerImpl& packManager_, const String& pack_)
    : packManagerImpl(&packManager_)
    , requestedPackName(pack_)
    , delayedRequest(true)
{
}

PackRequest::PackRequest(DLCManagerImpl& packManager_, const String& pack_, Vector<uint32> fileIndexes_)
    : packManagerImpl(&packManager_)
    , requestedPackName(pack_)
    , delayedRequest(false)
{
    SetFileIndexes(move(fileIndexes_));
}

PackRequest& PackRequest::operator=(PackRequest&& other)
{
    packManagerImpl = std::move(other.packManagerImpl);
    requests = std::move(other.requests);
    fileIndexes = std::move(other.fileIndexes);
    requestedPackName = std::move(other.requestedPackName);
    numOfDownloadedFile = std::move(other.numOfDownloadedFile);
    delayedRequest = std::move(other.delayedRequest);

    return *this;
}

void PackRequest::CancelCurrentDownloadRequests()
{
    DLCDownloader* downloader = packManagerImpl->GetDownloader();
    if (downloader)
    {
        for (FileRequest& r : requests)
        {
            if (r.task != nullptr)
            {
                downloader->RemoveTask(r.task);
                r.task = nullptr;
            }
        }
        requests.clear();
        requests.shrink_to_fit();
    }
}

PackRequest::~PackRequest()
{
    CancelCurrentDownloadRequests();
    packManagerImpl = nullptr;
    requests.clear();
    fileIndexes.clear();
    requestedPackName.clear();
    numOfDownloadedFile = 0;
    delayedRequest = false;
}

void PackRequest::Start()
{
    // just continue call Update
}

void PackRequest::Stop()
{
    CancelCurrentDownloadRequests();
    numOfDownloadedFile = 0;
}

const String& PackRequest::GetRequestedPackName() const
{
    return requestedPackName;
}

Vector<uint32> PackRequest::GetDependencies() const
{
    if (dependencyCache.capacity() > 0)
    {
        return dependencyCache;
    }
    if (packManagerImpl->IsInitialized())
    {
        const PackMetaData& pack_meta_data = packManagerImpl->GetMeta();
        dependencyCache = pack_meta_data.GetPackDependencyIndexes(requestedPackName);
        if (dependencyCache.capacity() == 0)
        {
            dependencyCache.reserve(1); // just mark to know we already check it
        }
        return dependencyCache;
    }
    DAVA_THROW(Exception, "Error! Can't get pack dependencies before initialization is finished");
}
/** return size of files within this request without dependencies */
uint64 PackRequest::GetSize() const
{
    uint64 allFilesSize = 0;
    const auto& files = packManagerImpl->GetPack().filesTable.data.files;
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
    uint64 requestsSize = std::accumulate(begin(requests), end(requests), uint64(0), [](uint64 sum, const FileRequest& r) {
        return sum + r.downloadedFileSize;
    });
    return requestsSize;
}

bool PackRequest::IsDownloadedCheck() const
{
    if (delayedRequest)
    {
        return false;
    }

    if (requests.size() != fileIndexes.size())
    {
        return false;
        // not initialized yet
    }

    if (!packManagerImpl->IsInitialized())
    {
        return false;
    }

    for (const FileRequest& r : requests)
    {
        if (r.status != Ready)
        {
            return false;
        }
    }

    if (packManagerImpl->IsInQueue(this))
    {
        if (!packManagerImpl->IsTop(this))
        {
            return false;
            // wait for dependencies to download first
        }
    }

    return true;
}

/** return true when all files loaded and ready */
bool PackRequest::IsDownloaded() const
{
    bool alreadyDownloaded = IsDownloadedCheck();
    if (!alreadyDownloaded && !packManagerImpl->IsInitialized())
    {
        // if user want's to know if pack already downloaded, we need
        // set flag about it in case of False result, because during this
        // in other thread local files can be scanning and after scan finished
        // we need inform user about all finished request, may be this request
        // already downloaded
        packManagerImpl->AddToInformSetAfterInitializationDone(this);
    }
    return alreadyDownloaded;
}

void PackRequest::SetFileIndexes(Vector<uint32> fileIndexes_)
{
    fileIndexes = std::move(fileIndexes_);
    delayedRequest = false;
}

bool PackRequest::IsSubRequest(const PackRequest* other) const
{
    const auto& meta = packManagerImpl->GetMeta();
    uint32 thisPackIndex = meta.GetPackIndex(requestedPackName);
    uint32 childPackIndex = meta.GetPackIndex(other->requestedPackName);
    return meta.IsChild(thisPackIndex, childPackIndex);
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
            const auto& fileInfo = packManagerImpl->GetPack().filesTable.data.files.at(fileIndex);
            String relativePath = packManagerImpl->GetRelativeFilePath(fileIndex);
            FilePath localPath = packManagerImpl->GetLocalPacksDirectory() + relativePath;

            FileRequest& request = requests.at(requestIndex);

            InitializeFileRequest(fileIndex,
                                  localPath,
                                  fileInfo.compressedCrc32,
                                  fileInfo.startPosition,
                                  fileInfo.compressedSize,
                                  fileInfo.originalSize,
                                  packManagerImpl->GetSuperPackUrl(),
                                  fileInfo.type,
                                  request);
        }
    }
}

bool PackRequest::Update()
{
    DVASSERT(Thread::IsMainThread());
    DVASSERT(packManagerImpl->IsInitialized());

    bool needFireUpdateSignal = false;

    if (numOfDownloadedFile < fileIndexes.size())
    {
        if (requests.empty())
        {
            InitializeFileRequests();
            packManagerImpl->requestStartLoading.Emit(*this);
        }

        if (!IsDownloaded())
        {
            needFireUpdateSignal = UpdateFileRequests();
        }
    }

    return needFireUpdateSignal;
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
    fileRequest.localFile = fileName_ + extDvpl;
    fileRequest.hashFromMeta = hash_;
    fileRequest.startLoadingPos = startLoadingPos_;
    fileRequest.sizeOfCompressedFile = fileComressedSize_;
    fileRequest.sizeOfUncompressedFile = fileUncompressedSize_;
    fileRequest.fileIndex = fileIndex_;
    fileRequest.status = Wait;
    fileRequest.task = nullptr;
    fileRequest.url = url_;
    fileRequest.downloadedFileSize = 0;
    fileRequest.compressionType = compressionType_;
}

void PackRequest::DeleteJustDownloadedFileAndStartAgain(FileRequest& fileRequest)
{
    fileRequest.downloadedFileSize = 0;
    FileSystem* fs = GetEngineContext()->fileSystem;
    bool deleteOk = fs->DeleteFile(fileRequest.localFile);
    if (!deleteOk)
    {
        Logger::Error("DLCManager can't delete invalid file: %s", fileRequest.localFile.GetStringValue().c_str());
    }
    fileRequest.status = LoadingPackFile;
}

void PackRequest::DisableRequestingAndFireSignalNoSpaceLeft(FileRequest& fileRequest) const
{
    int32 errnoValue = errno; // save in local variable if other error happen
    packManagerImpl->GetLog() << "No space on device!!! errno(" << errnoValue << ") Can't create or write file: "
                              << fileRequest.localFile.GetAbsolutePathname()
                              << " disable DLCManager requesting" << std::endl;
    packManagerImpl->SetRequestingEnabled(false);
    packManagerImpl->fileErrorOccured.Emit(fileRequest.localFile.GetAbsolutePathname().c_str(), errnoValue);
}

bool PackRequest::CheckLocalFileState(FileSystem* fs, FileRequest& fileRequest)
{
    if (packManagerImpl->IsFileReady(fileRequest.fileIndex))
    {
        fileRequest.status = Ready;
        uint64 fileSize = 0;
        if (fs->GetFileSize(fileRequest.localFile, fileSize))
        {
            if (fileSize == fileRequest.sizeOfCompressedFile + sizeof(PackFormat::LitePack::Footer))
            {
                fileRequest.downloadedFileSize = fileSize;
                return true;
            }
            // file exist but may be invalid so let's check it out
            fileRequest.status = CheckHash;
            return false;
        }
    }
    else
    {
        fileRequest.status = LoadingPackFile;
    }
    return false;
}

bool PackRequest::CheckLoadingStatusOfFileRequest(FileRequest& fileRequest, DLCDownloader* dm, const String& dstPath)
{
    DLCDownloader::TaskStatus status = dm->GetTaskStatus(fileRequest.task);
    {
        switch (status.state)
        {
        case DLCDownloader::TaskState::JustAdded:
            break;
        case DLCDownloader::TaskState::Downloading:
        {
            if (fileRequest.downloadedFileSize != status.sizeDownloaded)
            {
                fileRequest.downloadedFileSize = status.sizeDownloaded;
                return true;
            }
        }
        break;
        case DLCDownloader::TaskState::Finished:
        {
            dm->RemoveTask(fileRequest.task);
            fileRequest.task = nullptr;

            bool allGood = !status.error.errorHappened;

            if (allGood)
            {
                fileRequest.task = nullptr;
                fileRequest.downloadedFileSize = status.sizeDownloaded;
                fileRequest.status = CheckHash;
                return true;
            }

            std::ostream& out = packManagerImpl->GetLog();

            out << "file_request failed: can't download file: " << dstPath << " status: " << status;

            if (status.error.fileErrno != 0)
            {
                out << " I/O error: " << status.error.errStr << std::endl;
                DisableRequestingAndFireSignalNoSpaceLeft(fileRequest);
                return false;
            }

            DeleteJustDownloadedFileAndStartAgain(fileRequest);
            return false;
        }
        }
    }
    return false;
}

bool PackRequest::LoadingPackFileState(FileSystem* fs, FileRequest& fileRequest)
{
    DLCDownloader* dm = packManagerImpl->GetDownloader();
    String dstPath = fileRequest.localFile.GetAbsolutePathname();
    if (fileRequest.task == nullptr)
    {
        if (fileRequest.sizeOfCompressedFile == 0)
        {
            // just create empty file, and go to next state
            FilePath dirPath = fileRequest.localFile.GetDirectory();
            FileSystem::eCreateDirectoryResult dirCreate = fs->CreateDirectory(dirPath, true);
            if (dirCreate == FileSystem::DIRECTORY_CANT_CREATE)
            {
                DisableRequestingAndFireSignalNoSpaceLeft(fileRequest);
                return false;
            }
            ScopedPtr<File> f(File::Create(fileRequest.localFile, File::CREATE | File::WRITE));
            if (!f)
            {
                DisableRequestingAndFireSignalNoSpaceLeft(fileRequest);
                return false;
            }
            f->Truncate(0);
            fileRequest.task = nullptr;
            fileRequest.status = CheckHash;
            return true;
        }

        DLCDownloader::Range range = DLCDownloader::Range(fileRequest.startLoadingPos, fileRequest.sizeOfCompressedFile);
        fileRequest.task = dm->ResumeTask(fileRequest.url, dstPath, range);
        if (nullptr == fileRequest.task)
        {
            Logger::Error("can't create task: url: %s, dstPath: %s, range: %lld-%lld", fileRequest.url.c_str(), dstPath.c_str(), range.size, range.offset);
            fileRequest.status = Wait; // lets start all over again
        }
        return false;
    }

    return CheckLoadingStatusOfFileRequest(fileRequest, dm, dstPath);
}

bool PackRequest::CheckHaskState(FileRequest& fileRequest)
{
    uint32 fileCrc32 = CRC32::ForFile(fileRequest.localFile);
    if (fileCrc32 == fileRequest.hashFromMeta)
    {
        // write 20 bytes LitePack footer
        PackFormat::LitePack::Footer footer;
        footer.type = fileRequest.compressionType;
        footer.crc32Compressed = fileRequest.hashFromMeta;
        footer.sizeUncompressed = static_cast<uint32>(fileRequest.sizeOfUncompressedFile);
        footer.sizeCompressed = static_cast<uint32>(fileRequest.sizeOfCompressedFile);
        footer.packMarkerLite = PackFormat::FILE_MARKER_LITE;

        {
            ScopedPtr<File> f(File::Create(fileRequest.localFile, File::WRITE | File::APPEND));
            uint32 written = f->Write(&footer, sizeof(footer));
            if (written != sizeof(footer))
            {
                // not enough space
                DisableRequestingAndFireSignalNoSpaceLeft(fileRequest);
                return false;
            }
        }

        DVASSERT(fileRequest.downloadedFileSize == footer.sizeCompressed);

        fileRequest.downloadedFileSize += sizeof(footer);
        fileRequest.status = Ready;

        packManagerImpl->SetFileIsReady(fileRequest.fileIndex);

        return true;
    }

    // try download again
    DeleteJustDownloadedFileAndStartAgain(fileRequest);
    return false;
}

bool PackRequest::UpdateFileRequests()
{
    // return true if at least one part file continue downloading
    bool callUpdateSignal = false;

    FileSystem* fs = GetEngineContext()->fileSystem;

    for (FileRequest& fileRequest : requests)
    {
        bool downloadedMore = false;
        switch (fileRequest.status)
        {
        case Wait:
        {
            fileRequest.status = CheckLocalFile;
            break;
        }
        case CheckLocalFile:
        {
            downloadedMore = CheckLocalFileState(fs, fileRequest);
            break;
        }
        case LoadingPackFile:
        {
            downloadedMore = LoadingPackFileState(fs, fileRequest);
            break;
        }
        case CheckHash:
        {
            downloadedMore = CheckHaskState(fileRequest);
            break;
        }
        case Ready:
            break;
        case Error:
            break;
        } // end switch

        if (downloadedMore)
        {
            callUpdateSignal = true;
        }
    } // end for requests

    // call signal only once during update
    return callUpdateSignal;
}

} // end namespace DAVA
