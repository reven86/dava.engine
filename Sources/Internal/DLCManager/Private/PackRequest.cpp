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

void PackRequest::CancelCurrentsDownloads()
{
    DLCDownloader* dm = packManagerImpl->GetDownloader();
    if (dm)
    {
        for (FileRequest& r : requests)
        {
            if (r.taskId != nullptr)
            {
                dm->RemoveTask(r.taskId);
                r.taskId = nullptr;
                r.status = LoadingPackFile; // to resume loading after update
            }
        }
    }
}

PackRequest::~PackRequest()
{
    CancelCurrentsDownloads();
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
    CancelCurrentsDownloads();
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
/** return true when all files loaded and ready */
bool PackRequest::IsDownloaded() const
{
    if (delayedRequest)
    {
        return false;
    }

    if (requests.size() != fileIndexes.size())
    {
        return false; // not initialized yet
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
            return false; // wait for dependencies to download first
        }
    }

    return true;
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
    fileRequest.localFile = fileName_ + ".dvpl";
    fileRequest.hashFromMeta = hash_;
    fileRequest.startLoadingPos = startLoadingPos_;
    fileRequest.sizeOfCompressedFile = fileComressedSize_;
    fileRequest.sizeOfUncompressedFile = fileUncompressedSize_;
    fileRequest.fileIndex = fileIndex_;
    fileRequest.status = Wait;
    fileRequest.taskId = 0;
    fileRequest.url = url_;
    fileRequest.downloadedFileSize = 0;
    fileRequest.compressionType = compressionType_;
}

void PackRequest::DeleteJustDownloadedFileAndStartAgain(FileRequest& fileRequest)
{
    fileRequest.downloadedFileSize = 0;
    bool deleteOk = FileSystem::Instance()->DeleteFile(fileRequest.localFile);
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

bool PackRequest::UpdateFileRequests()
{
    // TODO refactoring method
    bool callSignal = false;

    FileSystem* fs = FileSystem::Instance();

    for (FileRequest& fileRequest : requests)
    {
        switch (fileRequest.status)
        {
        case Wait:
            fileRequest.status = CheckLocalFile;
            break;
        case CheckLocalFile:
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
                        callSignal = true;
                    }
                    else
                    {
                        // file exist but may be invalid so let's check it out
                        fileRequest.status = CheckHash;
                        callSignal = false;
                    }
                }
            }
            else
            {
                fileRequest.status = LoadingPackFile;
            }
            break;
        }
        case LoadingPackFile:
        {
            DLCDownloader* dm = packManagerImpl->GetDownloader();
            String dstPath = fileRequest.localFile.GetAbsolutePathname();
            if (fileRequest.taskId == nullptr)
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
                    fileRequest.taskId = nullptr;
                    fileRequest.status = CheckHash;
                    callSignal = true;
                }
                else
                {
                    fs->DeleteFile(fileRequest.localFile); // just in case (hash not match, size not match...)

                    DLCDownloader::Range range = DLCDownloader::Range(fileRequest.startLoadingPos, fileRequest.sizeOfCompressedFile);
                    fileRequest.taskId = dm->ResumeTask(fileRequest.url, dstPath, range);
                    if (nullptr == fileRequest.taskId)
                    {
                        DAVA_THROW(Exception, "can't be, something very very wrong");
                    }
                }
            }
            else
            {
                // TODO move to separate function
                DLCDownloader::TaskStatus status = dm->GetTaskStatus(fileRequest.taskId);
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
                            callSignal = true;
                        }
                    }
                    break;
                    case DLCDownloader::TaskState::Finished:
                    {
                        dm->RemoveTask(fileRequest.taskId);
                        fileRequest.taskId = nullptr;

                        bool allGood = !status.error.errorHappened;

                        if (allGood)
                        {
                            fileRequest.taskId = nullptr;
                            fileRequest.downloadedFileSize = status.sizeDownloaded;
                            fileRequest.status = CheckHash;
                            callSignal = true;
                        }
                        else
                        {
                            std::ostream& out = packManagerImpl->GetLog();

                            out << "can't download file: " << dstPath;

                            if (status.error.fileErrno != 0)
                            {
                                out << " I/O error: " << status.error.errStr << std::endl;
                                DisableRequestingAndFireSignalNoSpaceLeft(fileRequest);
                                return true;
                            }

                            if (status.error.httpCode >= 400)
                            {
                                out << " httpCode error(" << status.error.httpCode << "): " << status.error.errStr << std::endl;
                            }

                            if (status.error.curlErr != 0)
                            {
                                out << " curl easy error(" << status.error.curlErr << "): " << status.error.errStr << std::endl;
                            }

                            if (status.error.curlMErr != 0)
                            {
                                out << " curl multi error(" << status.error.curlMErr << "): " << status.error.errStr << std::endl;
                            }

                            DeleteJustDownloadedFileAndStartAgain(fileRequest);
                            return false;
                        }
                    }
                    break;
                    }
                }
            }
            break;
        }
        case CheckHash:
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
                    ScopedPtr<File> f(File::Create(fileRequest.localFile, File::WRITE | File::APPEND | File::OPEN));
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
                callSignal = true;

                packManagerImpl->SetFileIsReady(fileRequest.fileIndex);
            }
            else
            {
                // try download again
                DeleteJustDownloadedFileAndStartAgain(fileRequest);
            }
        }
        break;
        case Ready:
            break;
        case Error:
            break;
        } // end switch
    } // end for requests

    // call signal only once during update
    return callSignal;
}

} // end namespace DAVA
