#include "DLCManager/Private/DLCManagerImpl.h"
#include "FileSystem/FileList.h"
#include "FileSystem/File.h"
#include "FileSystem/Private/PackArchive.h"
#include "FileSystem/Private/PackMetaData.h"
#include "FileSystem/FileAPIHelper.h"
#include "DLCManager/DLCDownloader.h"
#include "Utils/CRC32.h"
#include "DLC/DLC.h"
#include "Logger/Logger.h"
#include "Base/Exception.h"
#include "Time/SystemTimer.h"
#include "Engine/Engine.h"
#include "Debug/Backtrace.h"

namespace DAVA
{
DLCManager::~DLCManager() = default;
DLCManager::IRequest::~IRequest() = default;

class MemoryBufferWriter final : public DLCDownloader::IWriter
{
public:
    MemoryBufferWriter(void* buff, size_t size)
    {
        DVASSERT(buff != nullptr);
        DVASSERT(size > 0);

        start = static_cast<char*>(buff);
        current = start;
        end = start + size;
    }

    uint64 Save(const void* ptr, uint64 size) override
    {
        uint64 space = SpaceLeft();

        if (size > space)
        {
            DVASSERT(false && "not enough buffer size");
            memcpy(current, ptr, static_cast<size_t>(space));
            current += space;
            return space;
        }

        memcpy(current, ptr, static_cast<size_t>(size));
        current += size;
        return size;
    }

    uint64 GetSeekPos() override
    {
        return current - start;
    }

    bool Truncate() override
    {
        current = start;
        return true;
    }

    uint64 SpaceLeft() const
    {
        return end - current;
    }

private:
    char* start = nullptr;
    char* current = nullptr;
    char* end = nullptr;
};

const String& DLCManagerImpl::ToString(InitState state)
{
    static const Vector<String> states{
        "Starting",
        "LoadingRequestAskFooter",
        "LoadingRequestGetFooter",
        "LoadingRequestAskFileTable",
        "LoadingRequestGetFileTable",
        "CalculateLocalDBHashAndCompare",
        "LoadingRequestAskMeta",
        "LoadingRequestGetMeta",
        "UnpakingDB",
        "DeleteDownloadedPacksIfNotMatchHash",
        "LoadingPacksDataFromLocalMeta",
        "WaitScanThreadToFinish",
        "MoveDeleyedRequestsToQueue",
        "Ready",
        "Offline"
    };
    DVASSERT(states.size() == static_cast<uint32>(InitState::State_COUNT));
    return states.at(static_cast<size_t>(state));
}

const String& DLCManagerImpl::ToString(InitError state)
{
    static const Vector<String> states{
        "AllGood",
        "CantCopyLocalDB",
        "CantMountLocalPacks",
        "LoadingRequestFailed",
        "UnpackingDBFailed",
        "DeleteDownloadedPackFailed",
        "LoadingPacksDataFailed",
        "MountingDownloadedPackFailed"
    };
    DVASSERT(states.size() == static_cast<size_t>(InitError::Error_COUNT));
    return states.at(static_cast<size_t>(state));
}

static void WriteBufferToFile(const Vector<uint8>& outDB, const FilePath& path)
{
    ScopedPtr<File> f(File::Create(path, File::WRITE | File::CREATE));
    if (!f)
    {
        DAVA_THROW(DAVA::Exception, "can't create file for local DB: " + path.GetStringValue());
    }

    uint32 written = f->Write(outDB.data(), static_cast<uint32>(outDB.size()));
    if (written != outDB.size())
    {
        DAVA_THROW(DAVA::Exception, "can't write file for local DB: " + path.GetStringValue());
    }
}

std::ostream& DLCManagerImpl::GetLog() const
{
    return log;
}

#ifdef __DAVAENGINE_COREV2__
DLCManagerImpl::DLCManagerImpl(Engine* engine_)
    : engine(*engine_)
{
    DVASSERT(Thread::IsMainThread());
    engine.update.Connect(this, &DLCManagerImpl::Update);
    engine.backgroundUpdate.Connect(this, &DLCManagerImpl::Update);

    downloader.reset(DLCDownloader::Create());
}
#endif

void DLCManagerImpl::ClearResouces()
{
    if (scanThread)
    {
        scanThread->Cancel();
        metaDataLoadedSem.Post();
        if (scanThread->IsJoinable())
        {
            scanThread->Join();
        }
        scanThread->Release();
        scanThread = nullptr;
    }

    for (auto request : requests)
    {
        delete request;
    }

    requests.clear();

    for (auto request : delayedRequests)
    {
        delete request;
    }

    initState = InitState::Starting;
    initError = InitError::AllGood;

    delayedRequests.clear();
    meta.reset();
    requestManager.reset();

    buffer.clear();
    uncompressedFileNames.clear();
    mapFileData.clear();
    startFileNameIndexesInUncompressedNames.clear();

    if (downloadTaskId != 0)
    {
        if (downloader != nullptr)
        {
            downloader->RemoveTask(downloadTaskId);
            downloadTaskId = 0;
        }
    }
    fullSizeServerData = 0;

    timeWaitingNextInitializationAttempt = 0;
    retryCount = 0;
}

DLCManagerImpl::~DLCManagerImpl()
{
    DVASSERT(Thread::IsMainThread());

#ifdef __DAVAENGINE_COREV2__
    engine.update.Disconnect(this);
    engine.backgroundUpdate.Disconnect(this);
#endif

    ClearResouces();
}

void DLCManagerImpl::Initialize(const FilePath& dirToDownloadPacks_,
                                const String& urlToServerSuperpack_,
                                const Hints& hints_)
{
    DVASSERT(Thread::IsMainThread());

    FilePath logPath(hints_.logFilePath);
    String fullLogPath = logPath.GetAbsolutePathname();

    Logger::Info("DLCManager::Initialize(\ndirToDownloadPacks:%s, "
                 "\nurlToServerSuperpack:%s, \nlogFilePath:%s, "
                 "\nretryConnectMilliseconds:%d, \nmaxFilesToDownload: %d",
                 dirToDownloadPacks_.GetAbsolutePathname().c_str(),
                 urlToServerSuperpack_.c_str(),
                 fullLogPath.c_str(),
                 hints_.retryConnectMilliseconds,
                 hints_.maxFilesToDownload);

    if (!log.is_open())
    {
        log.open(fullLogPath.c_str(), std::ios::trunc);
        if (!log)
        {
            const char* err = strerror(errno);
            Logger::Error("can't create dlc_manager.log error: %s", err);
            DAVA_THROW(DAVA::Exception, err);
        }
    }

    log << __FUNCTION__ << std::endl;

    DLCDownloader::Hints downloaderHints;
    downloaderHints.numOfMaxEasyHandles = static_cast<int>(hints.downloaderMaxHandles);
    downloaderHints.chankMemBuffSize = static_cast<int>(hints.downloaderChankBufSize);

    downloader->SetHints(downloaderHints);

    // TODO check if signal asyncConnectStateChanged has any subscriber

    if (!IsInitialized())
    {
        dirToDownloadedPacks = dirToDownloadPacks_;
        localCacheMeta = dirToDownloadPacks_ + "local_copy_server_meta.meta";
        localCacheFileTable = dirToDownloadPacks_ + "local_copy_server_file_table.block";

        FileSystem* fs = FileSystem::Instance();
        if (FileSystem::DIRECTORY_CANT_CREATE == fs->CreateDirectory(dirToDownloadedPacks, true))
        {
            String err = "can't create directory for packs: " + dirToDownloadedPacks.GetStringValue();
            DAVA_THROW(DAVA::Exception, err);
        }

        ScopedPtr<File> f(File::Create(dirToDownloadPacks_ + "tmp.file", File::WRITE | File::CREATE));
        if (!f)
        {
            String err = "can't write into directory: " + dirToDownloadedPacks.GetStringValue();
            DAVA_THROW(DAVA::Exception, err);
        }

        urlToSuperPack = urlToServerSuperpack_;
        hints = hints_;
    }

    // if Initialize called second time
    fullSizeServerData = 0;
    if (0 != downloadTaskId)
    {
        downloader->RemoveTask(downloadTaskId);
        downloadTaskId = nullptr;
    }

    initError = InitError::AllGood;
    initState = InitState::LoadingRequestAskFooter;

    StartScanDownloadedFiles(); // safe to call several times, only first will work

    SetRequestingEnabled(true);
}

void DLCManagerImpl::Deinitialize()
{
    DVASSERT(Thread::IsMainThread());

    log << __FUNCTION__ << std::endl;

    if (IsInitialized())
    {
        SetRequestingEnabled(false);
    }

    ClearResouces();
}

bool DLCManagerImpl::IsInitialized() const
{
    DVASSERT(Thread::IsMainThread());
    return nullptr != requestManager && delayedRequests.empty();
}

DLCManagerImpl::InitState DLCManagerImpl::GetInitState() const
{
    DVASSERT(Thread::IsMainThread());
    return initState;
}

DLCManagerImpl::InitError DLCManagerImpl::GetInitError() const
{
    DVASSERT(Thread::IsMainThread());
    return initError;
}

const String& DLCManagerImpl::GetLastErrorMessage() const
{
    DVASSERT(Thread::IsMainThread());
    return initErrorMsg;
}

void DLCManagerImpl::RetryInit()
{
    DVASSERT(Thread::IsMainThread());

    log << __FUNCTION__ << std::endl;

    // clear error state
    Initialize(dirToDownloadedPacks, urlToSuperPack, hints);

    // wait and then try again
    timeWaitingNextInitializationAttempt = hints.retryConnectMilliseconds / 1000.f; // to seconds
    retryCount++;
    initState = InitState::Offline;
}

// end Initialization ////////////////////////////////////////

void DLCManagerImpl::Update(float frameDelta)
{
    DVASSERT(Thread::IsMainThread());

    try
    {
        if (InitState::Starting != initState)
        {
            if (initState != InitState::Ready)
            {
                ContinueInitialization(frameDelta);
            }
            else if (isProcessingEnabled)
            {
                if (requestManager)
                {
                    requestManager->Update();
                }
            }
        }
    }
    catch (std::exception& ex)
    {
        Logger::Error("PackManager error: %s", ex.what());
        throw; // crush or let parent code decide
    }
}

void DLCManagerImpl::WaitScanThreadToFinish()
{
    // TODO how to know scanThread - finished?
    if (scanState == ScanState::Done)
    {
        initState = InitState::MoveDeleyedRequestsToQueue;
    }
}

void DLCManagerImpl::ContinueInitialization(float frameDelta)
{
    if (timeWaitingNextInitializationAttempt > 0.f)
    {
        timeWaitingNextInitializationAttempt -= frameDelta;
        if (timeWaitingNextInitializationAttempt <= 0.f)
        {
            timeWaitingNextInitializationAttempt = 0.f;
            initState = InitState::LoadingRequestAskFooter;
        }
        else
        {
            return;
        }
    }

    const InitState beforeState = initState;

    if (InitState::Starting == initState)
    {
        initState = InitState::LoadingRequestAskFooter;
    }
    else if (InitState::LoadingRequestAskFooter == initState)
    {
        AskFooter();
    }
    else if (InitState::LoadingRequestGetFooter == initState)
    {
        GetFooter();
    }
    else if (InitState::LoadingRequestAskFileTable == initState)
    {
        AskFileTable();
    }
    else if (InitState::LoadingRequestGetFileTable == initState)
    {
        GetFileTable();
    }
    else if (InitState::CalculateLocalDBHashAndCompare == initState)
    {
        CompareLocalMetaWitnRemoteHash();
    }
    else if (InitState::LoadingRequestAskMeta == initState)
    {
        AskServerMeta();
    }
    else if (InitState::LoadingRequestGetMeta == initState)
    {
        GetServerMeta();
    }
    else if (InitState::UnpakingDB == initState)
    {
        ParseMeta();
    }
    else if (InitState::DeleteDownloadedPacksIfNotMatchHash == initState)
    {
        DeleteOldPacks();
    }
    else if (InitState::LoadingPacksDataFromLocalMeta == initState)
    {
        LoadPacksDataFromMeta();
    }
    else if (InitState::WaitScanThreadToFinish == initState)
    {
        WaitScanThreadToFinish();
    }
    else if (InitState::MoveDeleyedRequestsToQueue == initState)
    {
        StartDelayedRequests();
    }
    else if (InitState::Ready == initState)
    {
        // happy end
    }

    const InitState newState = initState;

    if (newState != beforeState || initError != InitError::AllGood)
    {
        if (initError != InitError::AllGood)
        {
            isNetworkReadyLastState = false;
            networkReady.Emit(isNetworkReadyLastState);
            RetryInit();
        }
        else
        {
            if (!isNetworkReadyLastState)
            {
                isNetworkReadyLastState = true;
                networkReady.Emit(isNetworkReadyLastState);
            }
        }
    }
}

PackRequest* DLCManagerImpl::AddDelayedRequest(const String& requestedPackName)
{
    for (auto* request : delayedRequests)
    {
        if (request->GetRequestedPackName() == requestedPackName)
        {
            return request;
        }
    }

    delayedRequests.push_back(new PackRequest(*this, requestedPackName));
    return delayedRequests.back();
}

PackRequest* DLCManagerImpl::CreateNewRequest(const String& requestedPackName)
{
    for (auto* request : requests)
    {
        if (request->GetRequestedPackName() == requestedPackName)
        {
            return request;
        }
    }

    log << __FUNCTION__ << " requestedPackName: " << requestedPackName << std::endl;

    Vector<uint32> packIndexes = meta->GetFileIndexes(requestedPackName);

    // check all requested files already downloaded
    auto isFileDownloaded = [&](uint32 index) { return IsFileReady(index); };
    auto removeIt = remove_if(begin(packIndexes), end(packIndexes), isFileDownloaded);
    if (removeIt != end(packIndexes))
    {
        packIndexes.erase(removeIt, end(packIndexes));
    }

    PackRequest* request = new PackRequest(*this, requestedPackName, std::move(packIndexes));

    Vector<uint32> deps = request->GetDependencies();

    for (uint32 dependent : deps)
    {
        const String& depPackName = meta->GetPackInfo(dependent).packName;
        PackRequest* r = FindRequest(depPackName);
        if (nullptr == r)
        {
            // recursive call
            PackRequest* dependentRequest = CreateNewRequest(depPackName);
            DVASSERT(dependentRequest != nullptr);
        }
    }

    requests.push_back(request);
    requestManager->Push(request);

    return request;
}

void DLCManagerImpl::AskFooter()
{
    //Logger::FrameworkDebug("pack manager ask_footer");

    DVASSERT(0 == fullSizeServerData);

    if (0 == downloadTaskId)
    {
        downloadTaskId = downloader->StartTask(urlToSuperPack, "", DLCDownloader::TaskType::SIZE);
    }
    else
    {
        DLCDownloader::TaskStatus status = downloader->GetTaskStatus(downloadTaskId);

        if (DLCDownloader::TaskState::Finished == status.state)
        {
            bool allGood = status.error.curlErr == 0 && status.error.curlMErr == 0;
            if (allGood)
            {
                fullSizeServerData = status.sizeTotal;
                if (fullSizeServerData == 0)
                {
                    DAVA_THROW(DAVA::Exception, "can't get size of file on server side");
                }

                if (fullSizeServerData < sizeof(PackFormat::PackFile))
                {
                    DAVA_THROW(DAVA::Exception, "too small superpack on server");
                }
                // start downloading footer from server superpack
                uint64 downloadOffset = fullSizeServerData - sizeof(initFooterOnServer);
                uint32 sizeofFooter = static_cast<uint32>(sizeof(initFooterOnServer));

                memBufWriter.reset(new MemoryBufferWriter(&initFooterOnServer, sizeofFooter));
                downloadTaskId = downloader->StartTask(urlToSuperPack, "", DLCDownloader::TaskType::FULL, memBufWriter.get(), downloadOffset, sizeofFooter);
                initState = InitState::LoadingRequestGetFooter;
                log << "initState: " << ToString(initState) << std::endl;
            }
            else
            {
                DVASSERT(false); // TODO implement it better
                initError = InitError::LoadingRequestFailed;
                initErrorMsg = "failed get superpack size on server, download error: ";
                log << initErrorMsg << std::endl;
            }
        }
    }
}

void DLCManagerImpl::GetFooter()
{
    //Logger::FrameworkDebug("pack manager get_footer");

    DLCDownloader::TaskStatus status = downloader->GetTaskStatus(downloadTaskId);

    if (DLCDownloader::TaskState::Finished == status.state)
    {
        bool allGood = status.error.curlErr == 0 && status.error.curlMErr == 0;
        if (allGood)
        {
            uint32 crc32 = CRC32::ForBuffer(reinterpret_cast<char*>(&initFooterOnServer.info), sizeof(initFooterOnServer.info));
            if (crc32 != initFooterOnServer.infoCrc32)
            {
                DAVA_THROW(DAVA::Exception, "on server bad superpack!!! Footer not match crc32");
            }
            usedPackFile.footer = initFooterOnServer;
            initState = InitState::LoadingRequestAskFileTable;
            log << "initState: " << ToString(initState) << std::endl;
        }
        else
        {
            initError = InitError::LoadingRequestFailed;
            initErrorMsg = "failed get footer from server, download error: ";
            log << initErrorMsg << std::endl;
        }
    }
}

void DLCManagerImpl::AskFileTable()
{
    //Logger::FrameworkDebug("pack manager ask_file_table");

    FileSystem* fs = FileSystem::Instance();
    if (fs->IsFile(localCacheFileTable))
    {
        uint32 crc = CRC32::ForFile(localCacheFileTable);
        if (crc == initFooterOnServer.info.filesTableCrc32)
        {
            initState = InitState::LoadingRequestGetFileTable;
            return;
        }
        fs->DeleteFile(localCacheFileTable);
    }

    buffer.resize(initFooterOnServer.info.filesTableSize);

    uint64 downloadOffset = fullSizeServerData - (sizeof(initFooterOnServer) + initFooterOnServer.info.filesTableSize);

    downloadTaskId = downloader->StartTask(urlToSuperPack, localCacheFileTable.GetAbsolutePathname(), DLCDownloader::TaskType::FULL, nullptr, downloadOffset, buffer.size());
    if (0 == downloadTaskId)
    {
        DAVA_THROW(DAVA::Exception, "can't start downloading into buffer");
    }
    initState = InitState::LoadingRequestGetFileTable;
    log << "initState: " << ToString(initState) << std::endl;
}

void DLCManagerImpl::GetFileTable()
{
    //Logger::FrameworkDebug("pack manager get_file_table");

    DLCDownloader::TaskStatus status = downloader->GetTaskStatus(downloadTaskId);

    if (DLCDownloader::TaskState::Finished == status.state)
    {
        bool allGood = status.error.curlErr == 0 && status.error.curlMErr == 0;
        if (allGood)
        {
            uint64 fileSize = 0;
            FileSystem* fs = FileSystem::Instance();

            fs->GetFileSize(localCacheFileTable, fileSize);

            buffer.resize(static_cast<size_t>(fileSize));

            {
                ScopedPtr<File> f(File::Create(localCacheFileTable, File::OPEN | File::READ));
                f->Read(&buffer[0], static_cast<uint32>(buffer.size()));
            }

            uint32 crc32 = CRC32::ForBuffer(&buffer[0], buffer.size());
            if (crc32 != initFooterOnServer.info.filesTableCrc32)
            {
                const char* err = "FileTable not match crc32";
                Logger::Error("%s", err);
                DAVA_THROW(DAVA::Exception, err);
            }

            uncompressedFileNames.clear();
            PackArchive::ExtractFileTableData(initFooterOnServer,
                                              buffer,
                                              uncompressedFileNames,
                                              usedPackFile.filesTable);

            // fill fileNamesIndexes
            startFileNameIndexesInUncompressedNames.clear();
            startFileNameIndexesInUncompressedNames.reserve(usedPackFile.filesTable.data.files.size());
            startFileNameIndexesInUncompressedNames.push_back(0); // first name, and skip last '\0' char
            for (uint32 index = 0, last = static_cast<uint32>(uncompressedFileNames.size()) - 1;
                 index < last; ++index)
            {
                if (uncompressedFileNames[index] == '\0')
                {
                    startFileNameIndexesInUncompressedNames.push_back(index + 1);
                }
            }

            initState = InitState::CalculateLocalDBHashAndCompare;
            log << "initState: " << ToString(initState) << std::endl;
        }
        else
        {
            DVASSERT(false);
            initError = InitError::LoadingRequestFailed;
            initErrorMsg = "failed get fileTable from server, download error: ";
        }
    }
}

void DLCManagerImpl::CompareLocalMetaWitnRemoteHash()
{
    //Logger::FrameworkDebug("pack manager calc_local_db_with_remote_crc32");

    FileSystem* fs = FileSystem::Instance();

    if (fs->IsFile(localCacheMeta))
    {
        const uint32 localCrc32 = CRC32::ForFile(localCacheMeta);
        if (localCrc32 != initFooterOnServer.metaDataCrc32)
        {
            DeleteLocalMetaFiles();
            // we have to download new localDB file from server!
            initState = InitState::LoadingRequestAskMeta;
        }
        else
        {
            // all good go to
            initState = InitState::LoadingPacksDataFromLocalMeta;
        }
    }
    else
    {
        DeleteLocalMetaFiles();

        initState = InitState::LoadingRequestAskMeta;
    }
    log << "initState: " << ToString(initState) << std::endl;
}

void DLCManagerImpl::AskServerMeta()
{
    //Logger::FrameworkDebug("pack manager ask_db");

    uint64 internalDataSize = initFooterOnServer.metaDataSize +
    initFooterOnServer.info.filesTableSize +
    sizeof(PackFormat::PackFile::FooterBlock);

    uint64 downloadOffset = fullSizeServerData - internalDataSize;
    uint64 downloadSize = initFooterOnServer.metaDataSize;

    buffer.resize(static_cast<size_t>(downloadSize));

    memBufWriter.reset(new MemoryBufferWriter(buffer.data(), buffer.size()));

    downloadTaskId = downloader->StartTask(urlToSuperPack, "", DLCDownloader::TaskType::FULL, memBufWriter.get(), downloadOffset, downloadSize);
    DVASSERT(0 != downloadTaskId);

    initState = InitState::LoadingRequestGetMeta;
    log << "initState: " << ToString(initState) << std::endl;
}

void DLCManagerImpl::GetServerMeta()
{
    //Logger::FrameworkDebug("pack manager get_db");

    DLCDownloader::TaskStatus status = downloader->GetTaskStatus(downloadTaskId);

    if (DLCDownloader::TaskState::Finished == status.state)
    {
        bool allGood = status.error.curlErr == 0 && status.error.curlMErr == 0;
        if (allGood)
        {
            initState = InitState::UnpakingDB;
            log << "initState: " << ToString(initState) << std::endl;
        }
        else
        {
            DVASSERT(false);
            initError = InitError::LoadingRequestFailed;
            initErrorMsg = "failed get meta from server, download error: ";
            log << initErrorMsg << std::endl;
        }
    }
}

void DLCManagerImpl::ParseMeta()
{
    //Logger::FrameworkDebug("pack manager unpacking_db");

    uint32 buffCrc32 = CRC32::ForBuffer(reinterpret_cast<char*>(buffer.data()), static_cast<uint32>(buffer.size()));

    if (buffCrc32 != initFooterOnServer.metaDataCrc32)
    {
        DAVA_THROW(DAVA::Exception, "on server bad superpack!!! Footer meta not match crc32");
    }
    try
    {
        WriteBufferToFile(buffer, localCacheMeta);
    }
    catch (DAVA::Exception& ex)
    {
        int32 localErrno = errno;
        log << ex.what();
        fileErrorOccured.Emit(localCacheMeta.GetAbsolutePathname(), localErrno);
        RetryInit();
        return;
    }

    buffer.clear();
    buffer.shrink_to_fit();

    initState = InitState::DeleteDownloadedPacksIfNotMatchHash;
    log << "initState: " << ToString(initState) << std::endl;
}

void DLCManagerImpl::StoreAllMountedPackNames()
{
}

void DLCManagerImpl::DeleteOldPacks()
{
    //Logger::FrameworkDebug("pack manager delete_old_packs");

    StoreAllMountedPackNames();

    initState = InitState::LoadingPacksDataFromLocalMeta;
    log << "initState: " << ToString(initState) << std::endl;
}

void DLCManagerImpl::LoadPacksDataFromMeta()
{
    //Logger::FrameworkDebug("pack manager load_packs_data_from_db");

    try
    {
        ScopedPtr<File> f(File::Create(localCacheMeta, File::OPEN | File::READ));

        uint32 size = static_cast<uint32>(f->GetSize());

        buffer.resize(size);

        uint32 readSize = f->Read(&buffer[0], size);

        if (size != readSize)
        {
            DAVA_THROW(Exception, "can't read localCacheMeta size not match");
        }

        uint32 buffHash = CRC32::ForBuffer(&buffer[0], size);

        if (initFooterOnServer.metaDataCrc32 != buffHash)
        {
            DAVA_THROW(Exception, "can't read localCacheMeta hash not match");
        }

        meta.reset(new PackMetaData(&buffer[0], buffer.size()));

        size_t numFiles = meta->GetFileCount();
        scanFileReady.resize(numFiles);

        // now user can do requests for local packs
        requestManager.reset(new RequestManager(*this));
    }
    catch (std::exception& ex)
    {
        Logger::Info("can't load pack data from meta: %s", ex.what());
        FileSystem::Instance()->DeleteFile(localCacheMeta);
        RetryInit();
        return;
    }

    metaDataLoadedSem.Post();

    initState = InitState::WaitScanThreadToFinish;
    log << "initState: " << ToString(initState) << std::endl;
}

void DLCManagerImpl::SwapPointers(PackRequest* userRequestObject, PackRequest* invalidPointer)
{
    auto it = find(begin(requests), end(requests), invalidPointer);
    DVASSERT(it != end(requests));
    // change old pointer (just deleted) to correct one
    *it = userRequestObject;
}

void DLCManagerImpl::SwapRequestAndUpdatePointers(PackRequest* userRequestObject, PackRequest* newRequestObject)
{
    // We want to give user same pointer, so we need move(swap) old object
    // with new object value
    *userRequestObject = std::move(*newRequestObject);
    delete newRequestObject; // now this pointer is invalid!
    PackRequest* invalidPointer = newRequestObject;
    // so we have to update all references to new swapped pointer value
    // find new pointer and change it to correct one
    SwapPointers(userRequestObject, invalidPointer);

    requestManager->SwapPointers(userRequestObject, invalidPointer);
}

void DLCManagerImpl::StartDelayedRequests()
{
    //Logger::FrameworkDebug("pack manager mount_downloaded_packs");
    if (scanThread != nullptr)
    {
        // scan thread should be finished already
        if (scanThread->IsJoinable())
        {
            scanThread->Join();
        }
        scanThread->Release();
        scanThread = nullptr;
    }

    // I want to create new requests then move its content to old
    // to save pointers for users, if user store pointer to IRequest
    Vector<PackRequest*> tmpRequests;
    delayedRequests.swap(tmpRequests);
    // first remove old request pointers from requestManager
    for (auto request : tmpRequests)
    {
        requestManager->Remove(request);
    }

    for (auto request : tmpRequests)
    {
        const String& requestedPackName = request->GetRequestedPackName();
        PackRequest* r = FindRequest(requestedPackName);

        if (r == nullptr)
        {
            PackRequest* newRequest = CreateNewRequest(requestedPackName);
            DVASSERT(newRequest != request);
            DVASSERT(newRequest != nullptr);

            SwapRequestAndUpdatePointers(request, newRequest);
        }
        else
        {
            DVASSERT(r != request);
            // if we come here, it means one of previous requests
            // create it's dependencies and this is it
            SwapRequestAndUpdatePointers(request, r);
        }
    }

    initState = InitState::Ready;
    log << "initState: " << ToString(initState) << std::endl;

    size_t numDownloaded = std::count(begin(scanFileReady), end(scanFileReady), true);

    initializeFinished.Emit(numDownloaded, meta->GetFileCount());
}

void DLCManagerImpl::DeleteLocalMetaFiles()
{
    FileSystem* fs = FileSystem::Instance();
    fs->DeleteFile(localCacheMeta);
}

bool DLCManagerImpl::IsPackDownloaded(const String& packName)
{
    DVASSERT(Thread::IsMainThread());

    if (!IsInitialized())
    {
        DVASSERT(false && "Initialization not finished. Files is scanning now.");
        log << "Initialization not finished. Files is scanning now." << std::endl;
        return false;
    }
    // client wants only assert on bad pack name or dependency name
    try
    {
        // check every file in requested pack and all it's dependencies
        Vector<uint32> packFileIndexes = meta->GetFileIndexes(packName);
        for (uint32 fileIndex : packFileIndexes)
        {
            if (!IsFileReady(fileIndex))
            {
                return false;
            }
        }

        Vector<uint32> deps = meta->GetPackDependencyIndexes(packName);

        for (uint32 dependencyPack : deps)
        {
            const String& depPackName = meta->GetPackInfo(dependencyPack).packName;
            if (!IsPackDownloaded(depPackName)) // recursive call
            {
                return false;
            }
        }
    }
    catch (DAVA::Exception& e)
    {
        log << "Exception at `" << e.file << "`: " << e.line << '\n';
        log << Debug::GetBacktraceString(e.callstack) << std::endl;
        DVASSERT(false && "check out log file");
        return false;
    }

    return true;
}

const DLCManager::IRequest* DLCManagerImpl::RequestPack(const String& packName)
{
    DVASSERT(Thread::IsMainThread());

    log << __FUNCTION__ << " packName: " << packName << std::endl;

    if (!IsInitialized())
    {
        PackRequest* request = AddDelayedRequest(packName);
        return request;
    }

    const PackRequest* request = FindRequest(packName);
    if (request == nullptr)
    {
        request = CreateNewRequest(packName);
    }
    return request;
}

void DLCManagerImpl::SetRequestPriority(const IRequest* request)
{
    DVASSERT(Thread::IsMainThread());

    if (request != nullptr)
    {
        const PackRequest* r = dynamic_cast<const PackRequest*>(request);

        log << __FUNCTION__ << " request: " << request->GetRequestedPackName() << std::endl;

        PackRequest* req = const_cast<PackRequest*>(r);
        if (IsInitialized())
        {
            requestManager->SetPriorityToRequest(req);
        }
        else
        {
            auto it = std::find(begin(delayedRequests), end(delayedRequests), request);
            if (it != end(delayedRequests))
            {
                delayedRequests.erase(it);
                delayedRequests.insert(delayedRequests.begin(), req);
            }
        }
    }
}

void DLCManagerImpl::RemovePack(const String& requestedPackName)
{
    DVASSERT(Thread::IsMainThread());

    PackRequest* request = FindRequest(requestedPackName);
    if (nullptr != request)
    {
        requestManager->Remove(request);

        auto it = find(begin(requests), end(requests), request);
        if (it != end(requests))
        {
            requests.erase(it);
        }

        it = find(begin(delayedRequests), end(delayedRequests), request);
        if (it != end(delayedRequests))
        {
            delayedRequests.erase(it);
        }

        delete request;
    }

    if (IsInitialized())
    {
        // remove all files for pack
        Vector<uint32> fileIndexes = meta->GetFileIndexes(requestedPackName);
        for (uint32 index : fileIndexes)
        {
            if (IsFileReady(index))
            {
                const String relFile = GetRelativeFilePath(index);
                FileSystem::Instance()->DeleteFile(dirToDownloadedPacks + relFile);
                scanFileReady[index] = false;
            }
        }
    }
}

DLCManager::Progress DLCManagerImpl::GetProgress() const
{
    using namespace DAVA;
    using namespace PackFormat;

    Progress progress;

    if (!IsInitialized())
    {
        return progress;
    }

    progress.isRequestingEnabled = true;

    const Vector<PackFile::FilesTableBlock::FilesData::Data>& files = usedPackFile.filesTable.data.files;
    const size_t size = files.size();
    for (size_t indexOfFile = 0; indexOfFile < size; ++indexOfFile)
    {
        const auto& fileData = files[indexOfFile];
        uint64 fullFileSize = fileData.compressedSize + sizeof(LitePack::Footer);
        progress.total += fullFileSize;

        if (IsFileReady(indexOfFile))
        {
            progress.alreadyDownloaded += fullFileSize;
        }
        else
        {
            // is current file is downloading in requestManager queue?
            const PackMetaData::PackInfo& packInfo = meta->GetPackInfo(fileData.metaIndex);
            if (requestManager->IsInQueue(packInfo.packName))
            {
                progress.inQueue += fullFileSize;
            }
        }
    }

    return progress;
}

bool DLCManagerImpl::IsRequestingEnabled() const
{
    DVASSERT(Thread::IsMainThread());
    return isProcessingEnabled;
}

void DLCManagerImpl::SetRequestingEnabled(bool value)
{
    DVASSERT(Thread::IsMainThread());

    if (value)
    {
        if (!isProcessingEnabled)
        {
            isProcessingEnabled = true;
            if (requestManager)
            {
                requestManager->Start();
            }
        }
    }
    else
    {
        if (isProcessingEnabled)
        {
            isProcessingEnabled = false;
            if (requestManager)
            {
                requestManager->Stop();
            }
        }
    }
}

PackRequest* DLCManagerImpl::FindRequest(const String& requestedPackName) const
{
    DVASSERT(Thread::IsMainThread());

    for (auto request : requests)
    {
        if (request->GetRequestedPackName() == requestedPackName)
        {
            return request;
        }
    }

    for (auto request : delayedRequests)
    {
        if (request->GetRequestedPackName() == requestedPackName)
        {
            return request;
        }
    }

    return nullptr;
}

bool DLCManagerImpl::IsPackInQueue(const String& packName)
{
    DVASSERT(Thread::IsMainThread());
    if (!IsInitialized())
    {
        return false;
    }

    return requestManager->IsInQueue(packName);
}

const FilePath& DLCManagerImpl::GetLocalPacksDirectory() const
{
    DVASSERT(Thread::IsMainThread());
    return dirToDownloadedPacks;
}

const String& DLCManagerImpl::GetSuperPackUrl() const
{
    DVASSERT(Thread::IsMainThread());
    return urlToSuperPack;
}

String DLCManagerImpl::GetRelativeFilePath(uint32 fileIndex)
{
    uint32 startOfFilePath = startFileNameIndexesInUncompressedNames.at(fileIndex);
    return &uncompressedFileNames.at(startOfFilePath);
}

void DLCManagerImpl::StartScanDownloadedFiles()
{
    if (ScanState::Wait == scanState)
    {
        scanState = ScanState::Starting;
        scanThread = Thread::Create(MakeFunction(this, &DLCManagerImpl::ThreadScanFunc));
        scanThread->Start();
    }
}

void DLCManagerImpl::RecursiveScan(const FilePath& baseDir, const FilePath& dir, Vector<LocalFileInfo>& files)
{
    ScopedPtr<FileList> fl(new FileList(dir, false));

    for (uint32 index = 0; index < fl->GetCount(); ++index)
    {
        const FilePath& path = fl->GetPathname(index);
        if (fl->IsNavigationDirectory(index))
        {
            continue;
        }
        if (fl->IsDirectory(index))
        {
            RecursiveScan(baseDir, path, files);
        }
        else
        {
            if (path.GetExtension() == ".dvpl")
            {
                LocalFileInfo info;
                info.relativeName = path.GetRelativePathname(baseDir);
                FILE* f = FileAPI::OpenFile(path.GetAbsolutePathname(), "rb");
                if (f == nullptr)
                {
                    Logger::Info("can't open file %s during scan", path.GetAbsolutePathname().c_str());
                }
                else
                {
                    int32 footerSize = sizeof(PackFormat::LitePack::Footer);
                    if (0 == fseek(f, -footerSize, SEEK_END)) // TODO check SEEK_END may not work on all platforms
                    {
                        PackFormat::LitePack::Footer footer;
                        if (footerSize == fread(&footer, 1, footerSize, f))
                        {
                            info.compressedSize = footer.sizeCompressed;
                            info.crc32Hash = footer.crc32Compressed;
                        }
                        else
                        {
                            Logger::Info("can't read footer in file: %s", path.GetAbsolutePathname().c_str());
                        }
                    }
                    else
                    {
                        Logger::Info("can't seek to dvpl footer in file: %s", path.GetAbsolutePathname().c_str());
                    }
                    FileAPI::Close(f);
                }
                files.push_back(info);
            }
        }
    }
}

void DLCManagerImpl::ScanFiles(const FilePath& dir, Vector<LocalFileInfo>& files)
{
    if (FileSystem::Instance()->IsDirectory(dir))
    {
        files.clear();
        files.reserve(hints.maxFilesToDownload);
        RecursiveScan(dir, dir, files);
    }
}

void DLCManagerImpl::ThreadScanFunc()
{
    Thread* thisThread = Thread::Current();
    // scan files in download dir
    int64 startTime = SystemTimer::GetMs();

    ScanFiles(dirToDownloadedPacks, localFiles);

    int64 finishScan = SystemTimer::GetMs() - startTime;

    Logger::Info("finish scan files for: %fsec total files: %ld", finishScan / 1000.f, localFiles.size());

    if (thisThread->IsCancelling())
    {
        return;
    }

    metaDataLoadedSem.Wait();

    if (thisThread->IsCancelling() || meta == nullptr)
    {
        return;
    }

    // merge with meta
    // Yes! is pack loaded before meta
    const PackFormat::PackFile& pack = GetPack();

    Vector<ResourceArchive::FileInfo> filesInfo;
    PackArchive::FillFilesInfo(pack, uncompressedFileNames, mapFileData, filesInfo);

    String relativeNameWithoutDvpl;

    if (thisThread->IsCancelling())
    {
        return;
    }

    for (const LocalFileInfo& info : localFiles)
    {
        relativeNameWithoutDvpl = info.relativeName.substr(0, info.relativeName.size() - 5);
        const PackFormat::FileTableEntry* entry = mapFileData[relativeNameWithoutDvpl];
        if (entry != nullptr)
        {
            if (entry->compressedCrc32 != info.crc32Hash || entry->compressedSize != info.compressedSize)
            {
                Logger::Info("hash not match for file: %s delete it", info.relativeName.c_str());
                FileSystem::Instance()->DeleteFile(dirToDownloadedPacks + info.relativeName);
            }
            else
            {
                size_t fileIndex = std::distance(&pack.filesTable.data.files[0], entry);
                scanFileReady[fileIndex] = true;
            }
        }
        else
        {
            // no such file on server, delete it
            FileSystem::Instance()->DeleteFile(dirToDownloadedPacks + info.relativeName);
        }
    }

    if (thisThread->IsCancelling())
    {
        return;
    }
#ifdef __DAVAENGINE_COREV2__
    DAVA::RunOnMainThreadAsync([this]()
                               {
                                   // finish thread
                                   scanState = ScanState::Done;
                               });
#endif
}

} // end namespace DAVA
