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
    , fileIndexes(std::move(fileIndexes_))
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
        allFilesSize += fileInfo.compressedSize + sizeof(PackFormat::LitePack::Footer);
    }
    return allFilesSize;
}
/** recalculate current downloaded size without dependencies */
uint64 PackRequest::GetDownloadedSize() const
{
    return downloadedSize + prevDownloadedSize;
}
/** return true when all files loaded and ready */
bool PackRequest::IsDownloaded() const
{
    return status == Ready;
}

void PackRequest::SetFileIndexes(Vector<uint32> fileIndexes_)
{
    fileIndexes_ = std::move(fileIndexes_);
}

void PackRequest::InitializeCurrentFileRequest()
{
    const auto& fileInfo = packManagerImpl.GetPack().filesTable.data.files.at(currentFileIndex);
    String relativePath = packManagerImpl.GetRelativeFilePath(currentFileIndex);
    FilePath localPath = packManagerImpl.GetLocalPacksDirectory() + relativePath;

    InitializeFileRequest(currentFileIndex,
                          localPath,
                          fileInfo.compressedCrc32,
                          fileInfo.startPosition,
                          fileInfo.compressedSize,
                          fileInfo.originalSize,
                          packManagerImpl.GetSuperPackUrl());
}

void PackRequest::Update()
{
    DVASSERT(Thread::IsMainThread());
    DVASSERT(packManagerImpl.IsInitialized());

    uint32 countChecksPerUpdate = packManagerImpl.GetHints().checkLocalFileExistPerUpdate;

    if (currentFileIndex < fileIndexes.size() && countChecksPerUpdate > 0)
    {
        if (localFile.IsEmpty())
        {
            InitializeCurrentFileRequest();
            UpdateFileRequest();
        }

        if (IsDownloadedFileRequest())
        {
            packManagerImpl.requestUpdated.Emit(*this);
            do
            {
                currentFileIndex++;
                if (currentFileIndex < fileIndexes.size())
                {
                    // go to next file
                    InitializeCurrentFileRequest();
                    // start loading as soon as posible
                    UpdateFileRequest();
                }
                else
                {
                    // all downloaded
                    break;
                }
                countChecksPerUpdate++;
            } while (IsDownloadedFileRequest() && countChecksPerUpdate > 0);
        }
        else
        {
            UpdateFileRequest();
        }
    }
}

void PackRequest::InitializeFileRequest(const uint32 fileIndex_,
                                        const FilePath& fileName_,
                                        const uint32 hash_,
                                        const uint64 startLoadingPos_,
                                        const uint64 fileComressedSize_,
                                        const uint64 fileUncompressedSize_,
                                        const String& url_)
{
    localFile = fileName_;
    hashFromMeta = hash_;
    startLoadingPos = startLoadingPos_;
    sizeOfCompressedFile = fileComressedSize_;
    sizeOfUncompressedFile = fileUncompressedSize_;
    fileIndex = fileIndex_;
    status = Wait;
    taskId = 0;
    url = url_;
    prevDownloadedSize = 0;
}

void PackRequest::UpdateFileRequest()
{
    switch (status)
    {
    case Wait:
        status = CheckLocalFile;
        break;
    case CheckLocalFile:
    {
        FileSystem* fs = FileSystem::Instance();
        if (fs->IsFile(localFile))
        {
            uint32 size = 0;
            fs->GetFileSize(localFile, size);
            if (size == (sizeOfCompressedFile + sizeof(PackFormat::LitePack)))
            {
                status = CheckHash;
            }
            else
            {
                fs->DeleteFile(localFile);
                status = LoadingPackFile;
            }
        }
        else
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
            taskId = dm->DownloadRange(url, localFile, startLoadingPos, sizeOfCompressedFile);
        }
        else
        {
            DownloadStatus downloadStatus;
            if (dm->GetStatus(taskId, downloadStatus))
            {
                switch (downloadStatus)
                {
                case DL_PENDING:
                    break;
                case DL_IN_PROGRESS:
                {
                    uint64 progress = 0;
                    if (dm->GetProgress(taskId, progress))
                    {
                        if (prevDownloadedSize < progress)
                        {
                            prevDownloadedSize = progress;
                            packManagerImpl.requestUpdated.Emit(*this);
                        }
                    }
                }
                    break;
                case DL_FINISHED:
                {
                    dm->GetTotal(taskId, prevDownloadedSize);
                    taskId = 0;
                    status = CheckHash;
                }
                    break;
                case DL_UNKNOWN:
                    break;
                }
            }
            else
            {
                taskId = 0;
                status = CheckLocalFile;
            }
        }
    }
    break;
    case CheckHash:
    {
        prevDownloadedSize = 0;
        if (FileSystem::Instance()->IsFile(localFile + ".dvpl"))
        {
            uint64 size = 0;
            FileSystem::Instance()->GetFileSize(localFile + ".dvpl", size);
            if (size == (sizeOfCompressedFile + sizeof(PackFormat::LitePack::Footer)))
            {
                downloadedSize += (sizeOfCompressedFile + sizeof(PackFormat::LitePack::Footer));
                status = Ready;
                break;
            }

            FileSystem::Instance()->DeleteFile(localFile + ".dvpl");
        }

        uint32 fileCrc32 = CRC32::ForFile(localFile);
        if (fileCrc32 == hashFromMeta)
        {
            // write 20 bytes LitePack footer
            PackFormat::LitePack::Footer footer;
            footer.type = Compressor::Type::Lz4HC;
            footer.crc32Compressed = hashFromMeta;
            footer.sizeUncompressed = static_cast<uint32>(sizeOfUncompressedFile);
            footer.sizeCompressed = static_cast<uint32>(sizeOfCompressedFile);

            {
                ScopedPtr<File> f(File::Create(localFile, File::WRITE | File::APPEND | File::OPEN));
                f->Write(&footer, sizeof(footer));
            }

            FileSystem::Instance()->MoveFile(localFile, localFile + ".dvpl", true);

            downloadedSize += (sizeOfCompressedFile + sizeof(footer));

            status = Ready;
        }
        else
        {
            // try download again
            FileSystem::Instance()->DeleteFile(localFile);
            status = LoadingPackFile;
        }
    }

    break;
    case Ready:
        break;
    case Error:
        break;
    }
}

bool PackRequest::IsDownloadedFileRequest() const
{
    return status == Ready;
}

} // end namespace DAVA
