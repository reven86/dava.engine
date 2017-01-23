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
bool PackRequest::IsDownloaded() const
{
    // TODO checkit out
    return GetDownloadedSize() == GetSize();
}

void PackRequest::Update()
{
    DVASSERT(Thread::IsMainThread());
    DVASSERT(packManagerImpl.IsInitialized());

    // TODO
}

void PackRequest::FileRequest::Initialize(const uint32 fileIndex_,
                                          const FilePath& fileName_,
                                          const uint32 hash_,
                                          const uint64 startLoadingPos_,
                                          const uint64 fileComressedSize_,
                                          const uint64 fileUncompressedSize_,
                                          const String& url_)
{
    localFile = fileName_;
    hashFromMeta = hash_;
    startLoadingPos = startLoadingPos;
    sizeOfCompressedFile = fileComressedSize_;
    sizeOfUncompressedFile = fileUncompressedSize_;
    fileIndex = fileIndex_;
    status = Wait;
    taskId = 0;
    url = url_;
}

void PackRequest::FileRequest::Update()
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
                    break;
                case DL_FINISHED:
                    taskId = 0;
                    status = CheckHash;
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
        uint32 fileCrc32 = CRC32::ForFile(localFile);
        if (fileCrc32 == hashFromMeta)
        {
            // write 20 bytes LitePack footer
            PackFormat::LitePack::Footer footer;
            footer.type = Compressor::Type::Lz4HC;
            footer.crc32Compressed = hashFromMeta;
            footer.sizeUncompressed = sizeOfUncompressedFile;
            footer.sizeCompressed = sizeOfCompressedFile;

            ScopedPtr<File> f(File::Create(localFile, File::WRITE | File::APPEND | File::OPEN));
            f->Write(&footer, sizeof(footer));

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

bool PackRequest::FileRequest::IsDownloaded()
{
    return status == Ready;
}

} // end namespace DAVA
