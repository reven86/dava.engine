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
    DownloadManager* dm = DownloadManager::Instance();
    if (dm)
    {
        for (FileRequest& r : requests)
        {
            if (r.taskId != 0)
            {
                dm->Cancel(r.taskId);
                r.taskId = 0;
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

Vector<String> PackRequest::GetDependencies() const
{
    if (packManagerImpl->IsInitialized())
    {
        const PackMetaData& pack_meta_data = packManagerImpl->GetMeta();
        return pack_meta_data.GetDependencyNames(requestedPackName);
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
    Vector<String> dep = GetDependencies();
    for (const String& s : dep)
    {
        PackRequest* r = packManagerImpl->FindRequest(s);
        if (r != nullptr)
        {
            if (r == other)
            {
                return true;
            }

            if (r->IsSubRequest(other))
            {
                return true;
            }
        }
    }
    return false;
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
    FileSystem::Instance()->DeleteFile(fileRequest.localFile);
    fileRequest.status = LoadingPackFile;
}

void PackRequest::DisableRequestingAndFireSignalNoSpaceLeft(PackRequest::FileRequest& fileRequest)
{
    int32 errnoValue = errno; // save in local variable if other error happen
    Logger::Error("No space on device!!! Can't create or write file: %s disable DLCManager requesting", fileRequest.localFile.GetAbsolutePathname().c_str());
    packManagerImpl->SetRequestingEnabled(false);
    packManagerImpl->fileErrorOccured.Emit(fileRequest.localFile.GetAbsolutePathname().c_str(), errnoValue);
}

bool PackRequest::UpdateFileRequests()
{
    // TODO refactor method
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
                    DVASSERT(fileSize == fileRequest.sizeOfCompressedFile + sizeof(PackFormat::LitePack::Footer));
                    fileRequest.downloadedFileSize = fileSize;
                }
                callSignal = true;
            }
            else
            {
                fileRequest.status = LoadingPackFile;
            }
            break;
        }
        case LoadingPackFile:
        {
            DownloadManager* dm = DownloadManager::Instance();
            if (fileRequest.taskId == 0)
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
                    fileRequest.taskId = 0;
                    fileRequest.status = CheckHash;
                    callSignal = true;
                }
                else
                {
                    fs->DeleteFile(fileRequest.localFile); // just in case (hash not match, size not match...)
                    fileRequest.taskId = dm->DownloadRange(fileRequest.url, fileRequest.localFile, fileRequest.startLoadingPos, fileRequest.sizeOfCompressedFile);
                }
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
                            fileRequest.downloadedFileSize = progress;
                            uint64 s = GetSize();
                            uint64 ds = GetDownloadedSize();
                            DVASSERT(s > 0);
                            DVASSERT(s >= ds);
                            callSignal = true;
                        }
                    }
                    break;
                    case DL_FINISHED:
                    {
                        DownloadError downloadError = DLE_NO_ERROR;
                        if (dm->GetError(fileRequest.taskId, downloadError))
                        {
                            if (DLE_NO_ERROR != downloadError)
                            {
                                String err = DLC::ToString(downloadError);
                                Logger::Error("can't download file: %s couse: %s",
                                              fileRequest.localFile.GetAbsolutePathname().c_str(),
                                              err.c_str());

                                if (DLE_FILE_ERROR == downloadError)
                                {
                                    DisableRequestingAndFireSignalNoSpaceLeft(fileRequest);
                                    return false;
                                }
                            }
                        }
                        dm->GetProgress(fileRequest.taskId, fileRequest.downloadedFileSize);

                        fileRequest.taskId = 0;
                        fileRequest.status = CheckHash;
                        callSignal = true;
                    }
                    break;
                    case DL_UNKNOWN:
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
