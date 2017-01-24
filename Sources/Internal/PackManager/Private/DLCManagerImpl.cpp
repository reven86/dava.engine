#include "PackManager/Private/DLCManagerImpl.h"
#include "FileSystem/FileList.h"
#include "FileSystem/Private/PackArchive.h"
#include "FileSystem/Private/PackMetaData.h"
#include "DLC/Downloader/DownloadManager.h"
#include "Utils/CRC32.h"
#include "Utils/StringUtils.h"
#include "Utils/StringFormat.h"
#include "DLC/DLC.h"
#include "Logger/Logger.h"
#include "Base/Exception.h"
#include "Concurrency/Mutex.h"
#include "Concurrency/LockGuard.h"

#include <algorithm>

namespace DAVA
{
IDLCManager::~IDLCManager() = default;
IDLCManager::IRequest::~IRequest() = default;

const String& DLCManagerImpl::ToString(DLCManagerImpl::InitState state)
{
    static Vector<String> states{
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
        "MountingDownloadedPacks",
        "Ready",
        "Offline"
    };
    DVASSERT(states.size() == 14);
    return states.at(static_cast<size_t>(state));
}

const String& DLCManagerImpl::ToString(DLCManagerImpl::InitError state)
{
    static Vector<String> states{
        "AllGood",
        "CantCopyLocalDB",
        "CantMountLocalPacks",
        "LoadingRequestFailed",
        "UnpackingDBFailed",
        "DeleteDownloadedPackFailed",
        "LoadingPacksDataFailed",
        "MountingDownloadedPackFailed"
    };
    DVASSERT(states.size() == 8);
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

#ifdef __DAVAENGINE_COREV2__
DLCManagerImpl::DLCManagerImpl(Engine* engine_)
    : engine(*engine_)
{
    DVASSERT(Thread::IsMainThread());
    sigConnectionUpdate = engine.update.Connect(this, &DLCManagerImpl::Update);
}

DLCManagerImpl::~DLCManagerImpl()
{
    DVASSERT(Thread::IsMainThread());
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
#endif

void DLCManagerImpl::Initialize(const FilePath& dirToDownloadPacks_,
                                const String& urlToServerSuperpack_,
                                const Hints& hints_)
{
    DVASSERT(Thread::IsMainThread());
    // TODO check if signal asyncConnectStateChanged has any subscriber

    if (!IsInitialized())
    {
        dirToDownloadedPacks = dirToDownloadPacks_;
        metaLocalCache = dirToDownloadPacks_ + "local_copy_server_meta.meta";

        FileSystem* fs = FileSystem::Instance();
        if (FileSystem::DIRECTORY_CANT_CREATE == fs->CreateDirectory(dirToDownloadedPacks, true))
        {
            DAVA_THROW(DAVA::Exception, "can't create directory for packs: " + dirToDownloadedPacks.GetStringValue());
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
}

bool DLCManagerImpl::IsInitialized() const
{
    // current inputState can be in differect states becouse of
    // offline mode
    LockGuard<Mutex> lock(protectPM);

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
    else if (InitState::MountingDownloadedPacks == initState)
    {
        MountDownloadedPacks();
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
            networkReady.Emit(false);
            RetryInit();
        }
        else
        {
            networkReady.Emit(true);
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

    PackRequest* request = new PackRequest(*this, requestedPackName, std::move(packIndexes));
    requests.push_back(request);

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

    DownloadManager* dm = DownloadManager::Instance();
    buffer.resize(initFooterOnServer.info.filesTableSize);

    uint64 downloadOffset = fullSizeServerData - (sizeof(initFooterOnServer) + initFooterOnServer.info.filesTableSize);

    downloadTaskId = dm->DownloadIntoBuffer(urlToSuperPack, buffer.data(), static_cast<uint32>(buffer.size()), downloadOffset, buffer.size());
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
                uint32 crc32 = CRC32::ForBuffer(buffer.data(), buffer.size());
                if (crc32 != initFooterOnServer.info.filesTableCrc32)
                {
                    DAVA_THROW(DAVA::Exception, "on server bad superpack!!! FileTable not match crc32");
                }

                uncompressedFileNames.clear();
                PackArchive::ExtractFileTableData(initFooterOnServer,
                                                  buffer,
                                                  uncompressedFileNames,
                                                  usedPackFile.filesTable);

                // fill fileNamesIndexes
                startFileNameIndexes.clear();
                startFileNameIndexes.reserve(usedPackFile.filesTable.data.files.size());
                startFileNameIndexes.push_back(0); // first name, and skip last '\0' char
                for (uint32 index = 0, last = static_cast<uint32>(uncompressedFileNames.size()) - 1;
                     index < last; ++index)
                {
                    if (uncompressedFileNames[index] == '\0')
                    {
                        startFileNameIndexes.push_back(index + 1);
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
        DAVA_THROW(DAVA::Exception, "can't get status for download task");
    }
}

void DLCManagerImpl::CompareLocalMetaWitnRemoteHash()
{
    //Logger::FrameworkDebug("pack manager calc_local_db_with_remote_crc32");

    FileSystem* fs = FileSystem::Instance();

    if (fs->IsFile(metaLocalCache))
    {
        const uint32 localCrc32 = CRC32::ForFile(metaLocalCache);
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

    WriteBufferToFile(buffer, metaLocalCache);

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
        ScopedPtr<File> f(File::Create(metaLocalCache, File::OPEN | File::READ));

        uint32 size = static_cast<uint32>(f->GetSize());

        buffer.resize(size);

        uint32 readSize = f->Read(&buffer[0], size);

        if (size != readSize)
        {
            DAVA_THROW(Exception, "can't read metaLocalCache size not match");
        }

        uint32 buffHash = CRC32::ForBuffer(&buffer[0], size);

        if (initFooterOnServer.metaDataCrc32 != buffHash)
        {
            DAVA_THROW(Exception, "can't read metaLocalCache hash not match");
        }

        meta.reset(new PackMetaData(&buffer[0], buffer.size()));

        // now user can do requests for local packs
        requestManager.reset(new RequestManager(*this));
    }
    catch (std::exception& ex)
    {
        Logger::Error("can't load pack data from meta: %s", ex.what());
        FileSystem::Instance()->DeleteFile(metaLocalCache);
        RetryInit();
        return;
    }

    initState = InitState::MountingDownloadedPacks;
}

void DLCManagerImpl::MountDownloadedPacks()
{
    //Logger::FrameworkDebug("pack manager mount_downloaded_packs");

    initState = InitState::Ready;
}

void DLCManagerImpl::DeleteLocalMetaFiles()
{
    FileSystem* fs = FileSystem::Instance();
    fs->DeleteFile(metaLocalCache);
}

const IDLCManager::IRequest* DLCManagerImpl::RequestPack(const String& packName)
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
        requestManager->Push(const_cast<PackRequest*>(request));
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
        requestManager->UpdateOrder(req, orderIndex);
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

    LockGuard<Mutex> lock(protectPM);

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
    uint32 startOfFilePath = startFileNameIndexes.at(fileIndex);
    return &uncompressedFileNames.at(startOfFilePath);
}

} // end namespace DAVA
