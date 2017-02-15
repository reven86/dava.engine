#include "DLCManager/Private/DLCManagerImpl.h"
#include "FileSystem/FileList.h"
#include "FileSystem/File.h"
#include "FileSystem/Private/PackArchive.h"
#include "FileSystem/Private/PackMetaData.h"
#include "FileSystem/FileAPIHelper.h"
#include "DLC/Downloader/DownloadManager.h"
#include "Utils/CRC32.h"
#include "DLC/DLC.h"
#include "Logger/Logger.h"
#include "Base/Exception.h"
#include "Concurrency/Mutex.h"
#include "Concurrency/LockGuard.h"
#include "Time/SystemTimer.h"

namespace DAVA
{
DLCManager::~DLCManager() = default;
DLCManager::IRequest::~IRequest() = default;

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

DLCManagerImpl::DLCManagerImpl(Engine* engine_)
    : engine(*engine_)
{
    DVASSERT(Thread::IsMainThread());
    sigConnectionUpdate = engine.update.Connect(this, &DLCManagerImpl::Update);
}

DLCManagerImpl::~DLCManagerImpl()
{
    DVASSERT(Thread::IsMainThread());

    if (scanThread)
    {
        scanThread->Cancel();
        metaDataLoadedSem.Post();
        if (scanThread->IsJoinable())
        {
            scanThread->Join();
            scanThread = nullptr;
        }
    }

    engine.update.Disconnect(sigConnectionUpdate);

    for (auto request : requests)
    {
        delete request;
    }

    for (auto request : delayedRequests)
    {
        delete request;
    }
}

void DLCManagerImpl::Initialize(const FilePath& dirToDownloadPacks_,
                                const String& urlToServerSuperpack_,
                                const Hints& hints_)
{
    DVASSERT(Thread::IsMainThread());
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
            Logger::Error("%s", err.c_str());
            DAVA_THROW(DAVA::Exception, err);
        }

        urlToSuperPack = urlToServerSuperpack_;
        hints = hints_;
    }

    // if Initialize called second time
    fullSizeServerData = 0;
    if (0 != downloadTaskId)
    {
        DownloadManager::Instance()->Cancel(downloadTaskId);
        downloadTaskId = 0;
    }

    initError = InitError::AllGood;
    initState = InitState::LoadingRequestAskFooter;

    StartScanDownloadedFiles(); // safe to call several times, only first will work
}

bool DLCManagerImpl::IsInitialized() const
{
    // current inputState can be in differect states becouse of
    // offline mode
    LockGuard<Mutex> lock(protectDM);

    bool requestManagerCreated = requestManager != nullptr;

    return requestManagerCreated;
}

// start ISync //////////////////////////////////////
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
        StartDeleyedRequests();
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

PackRequest* DLCManagerImpl::AddDeleyedRequest(const String& requestedPackName)
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

    Vector<uint32> packIndexes = meta->GetFileIndexes(requestedPackName);

    // check all requested files already downloaded
    auto isFileDownloaded = [&](uint32 index) { return IsFileReady(index); };
    auto removeIt = remove_if(begin(packIndexes), end(packIndexes), isFileDownloaded);
    if (removeIt != end(packIndexes))
    {
        packIndexes.erase(removeIt, end(packIndexes));
    }

    PackRequest* request = new PackRequest(*this, requestedPackName, std::move(packIndexes));

    Vector<String> deps = request->GetDependencies();

    for (const String& dependent : deps)
    {
        PackRequest* r = FindRequest(dependent);
        if (nullptr == r)
        {
            // recursive call
            PackRequest* dependentRequest = CreateNewRequest(dependent);
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

    DownloadManager* dm = DownloadManager::Instance();

    DVASSERT(0 == fullSizeServerData);

    if (0 == downloadTaskId)
    {
        downloadTaskId = dm->Download(urlToSuperPack, "", GET_SIZE);
    }
    else
    {
        DownloadStatus status = DL_UNKNOWN;
        if (dm->GetStatus(downloadTaskId, status))
        {
            if (DL_FINISHED == status)
            {
                DownloadError error = DLE_NO_ERROR;
                dm->GetError(downloadTaskId, error);
                if (DLE_NO_ERROR == error)
                {
                    if (!dm->GetTotal(downloadTaskId, fullSizeServerData))
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
                    downloadTaskId = dm->DownloadIntoBuffer(urlToSuperPack, &initFooterOnServer, sizeofFooter, downloadOffset, sizeofFooter);
                    initState = InitState::LoadingRequestGetFooter;
                }
                else
                {
                    initError = InitError::LoadingRequestFailed;
                    initErrorMsg = "failed get superpack size on server, download error: " + DLC::ToString(error) + " " + std::to_string(retryCount);
                    Logger::Error("%s", initErrorMsg.c_str());
                }
            }
        }
    }
}

void DLCManagerImpl::GetFooter()
{
    //Logger::FrameworkDebug("pack manager get_footer");

    DownloadManager* dm = DownloadManager::Instance();
    DownloadStatus status = DL_UNKNOWN;
    if (dm->GetStatus(downloadTaskId, status))
    {
        if (DL_FINISHED == status)
        {
            DownloadError error = DLE_NO_ERROR;
            dm->GetError(downloadTaskId, error);
            if (DLE_NO_ERROR == error)
            {
                uint32 crc32 = CRC32::ForBuffer(reinterpret_cast<char*>(&initFooterOnServer.info), sizeof(initFooterOnServer.info));
                if (crc32 != initFooterOnServer.infoCrc32)
                {
                    DAVA_THROW(DAVA::Exception, "on server bad superpack!!! Footer not match crc32");
                }
                usedPackFile.footer = initFooterOnServer;
                initState = InitState::LoadingRequestAskFileTable;
            }
            else
            {
                initError = InitError::LoadingRequestFailed;
                initErrorMsg = "failed get footer from server, download error: " + DLC::ToString(error) + " " + std::to_string(retryCount);
            }
        }
    }
    else
    {
        DAVA_THROW(DAVA::Exception, "can't get status for download task");
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

    DownloadManager* dm = DownloadManager::Instance();
    buffer.resize(initFooterOnServer.info.filesTableSize);

    uint64 downloadOffset = fullSizeServerData - (sizeof(initFooterOnServer) + initFooterOnServer.info.filesTableSize);

    downloadTaskId = dm->DownloadRange(urlToSuperPack, localCacheFileTable, downloadOffset, buffer.size());
    if (0 == downloadTaskId)
    {
        DAVA_THROW(DAVA::Exception, "can't start downloading into buffer");
    }
    initState = InitState::LoadingRequestGetFileTable;
}

void DLCManagerImpl::GetFileTable()
{
    //Logger::FrameworkDebug("pack manager get_file_table");

    DownloadManager* dm = DownloadManager::Instance();
    DownloadStatus status = DL_UNKNOWN;
    if (dm->GetStatus(downloadTaskId, status))
    {
        if (DL_FINISHED == status)
        {
            DownloadError error = DLE_NO_ERROR;
            dm->GetError(downloadTaskId, error);
            if (DLE_NO_ERROR == error)
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
            }
            else
            {
                initError = InitError::LoadingRequestFailed;
                initErrorMsg = "failed get fileTable from server, download error: " + DLC::ToString(error) + " " + std::to_string(retryCount);
            }
        }
    }
    else
    {
        const char* err = "can't get status for download task";
        Logger::Error("%s", err);
        DAVA_THROW(DAVA::Exception, err);
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
}

void DLCManagerImpl::AskServerMeta()
{
    //Logger::FrameworkDebug("pack manager ask_db");

    DownloadManager* dm = DownloadManager::Instance();

    uint64 internalDataSize = initFooterOnServer.metaDataSize +
    initFooterOnServer.info.filesTableSize +
    sizeof(PackFormat::PackFile::FooterBlock);

    uint64 downloadOffset = fullSizeServerData - internalDataSize;
    uint64 downloadSize = initFooterOnServer.metaDataSize;

    buffer.resize(static_cast<size_t>(downloadSize));

    downloadTaskId = dm->DownloadIntoBuffer(urlToSuperPack, buffer.data(), static_cast<uint32>(buffer.size()), downloadOffset, downloadSize);
    DVASSERT(0 != downloadTaskId);

    initState = InitState::LoadingRequestGetMeta;
}

void DLCManagerImpl::GetServerMeta()
{
    //Logger::FrameworkDebug("pack manager get_db");

    DownloadManager* dm = DownloadManager::Instance();
    DownloadStatus status = DL_UNKNOWN;
    if (dm->GetStatus(downloadTaskId, status))
    {
        if (DL_FINISHED == status)
        {
            DownloadError error = DLE_NO_ERROR;
            dm->GetError(downloadTaskId, error);
            if (DLE_NO_ERROR == error)
            {
                initState = InitState::UnpakingDB;
            }
            else
            {
                initError = InitError::LoadingRequestFailed;
                initErrorMsg = "failed get meta from server, download error: " + DLC::ToString(error) + " " + std::to_string(retryCount);
            }
        }
    }
    else
    {
        DAVA_THROW(DAVA::Exception, "can't get status for download task");
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

    WriteBufferToFile(buffer, localCacheMeta);

    buffer.clear();
    buffer.shrink_to_fit();

    initState = InitState::DeleteDownloadedPacksIfNotMatchHash;
}

void DLCManagerImpl::StoreAllMountedPackNames()
{
}

void DLCManagerImpl::DeleteOldPacks()
{
    //Logger::FrameworkDebug("pack manager delete_old_packs");

    StoreAllMountedPackNames();

    initState = InitState::LoadingPacksDataFromLocalMeta;
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

        // now user can do requests for local packs
        requestManager.reset(new RequestManager(*this));
    }
    catch (std::exception& ex)
    {
        Logger::Error("can't load pack data from meta: %s", ex.what());
        FileSystem::Instance()->DeleteFile(localCacheMeta);
        RetryInit();
        return;
    }

    metaDataLoadedSem.Post();

    initState = InitState::WaitScanThreadToFinish;
}

void DLCManagerImpl::StartDeleyedRequests()
{
    //Logger::FrameworkDebug("pack manager mount_downloaded_packs");
    if (scanThread != nullptr)
    {
        // scan thread should be finished already
        if (scanThread->IsJoinable())
        {
            scanThread->Join();
        }
        scanThread = nullptr;
    }

    for (auto request : delayedRequests)
    {
        const String& packName = request->GetRequestedPackName();
        Vector<uint32> fileIndexes = meta->GetFileIndexes(packName);
        request->SetFileIndexes(std::move(fileIndexes));

        requests.push_back(request);
        requestManager->Push(request);
    }

    delayedRequests.clear();

    initState = InitState::Ready;
}

void DLCManagerImpl::DeleteLocalMetaFiles()
{
    FileSystem* fs = FileSystem::Instance();
    fs->DeleteFile(localCacheMeta);
}

const DLCManager::IRequest* DLCManagerImpl::RequestPack(const String& packName)
{
    DVASSERT(Thread::IsMainThread());

    if (!IsInitialized())
    {
        PackRequest* request = AddDeleyedRequest(packName);
        return request;
    }

    const PackRequest* request = FindRequest(packName);
    if (request == nullptr)
    {
        request = CreateNewRequest(packName);
    }
    return request;
}

void DLCManagerImpl::SetRequestOrder(const IRequest* request, uint32 orderIndex)
{
    DVASSERT(Thread::IsMainThread());

    if (request != nullptr)
    {
        const PackRequest* r = dynamic_cast<const PackRequest*>(request);
        PackRequest* req = const_cast<PackRequest*>(r);
        if (IsInitialized())
        {
            requestManager->UpdateOrder(req, orderIndex);
        }
        else
        {
            auto it = std::find(begin(delayedRequests), end(delayedRequests), request);
            if (it != end(delayedRequests))
            {
                delayedRequests.erase(it);
                if (delayedRequests.size() > orderIndex)
                {
                    delayedRequests.insert(delayedRequests.begin() + orderIndex, req);
                }
                else
                {
                    delayedRequests.push_back(req);
                }
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
                scanFileReady.reset(index);
            }
        }
    }
}

bool DLCManagerImpl::IsRequestingEnabled() const
{
    DVASSERT(Thread::IsMainThread());
    return isProcessingEnabled;
}

void DLCManagerImpl::SetRequestingEnabled(bool value)
{
    DVASSERT(Thread::IsMainThread());

    LockGuard<Mutex> lock(protectDM);

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
                    Logger::Error("can't open file %s during scan", path.GetAbsolutePathname().c_str());
                }
                else
                {
                    int32 footerSize = sizeof(PackFormat::LitePack::Footer);
                    if (0 == fseek(f, -footerSize, SEEK_END)) // TODO check SEEK_END mey not work on all platforms
                    {
                        PackFormat::LitePack::Footer footer;
                        if (footerSize == fread(&footer, 1, footerSize, f))
                        {
                            info.compressedSize = footer.sizeCompressed;
                            info.crc32Hash = footer.crc32Compressed;
                        }
                        else
                        {
                            Logger::Error("can't read footer in file: %s", path.GetAbsolutePathname().c_str());
                        }
                    }
                    else
                    {
                        Logger::Error("can't seek to dvpl footer in file: %s", path.GetAbsolutePathname().c_str());
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
                scanFileReady.set(fileIndex);
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

    DAVA::RunOnMainThreadAsync([this]()
                               {
                                   // finish thread
                                   scanState = ScanState::Done;
                               });
}

} // end namespace DAVA
