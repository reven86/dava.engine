#include "PackManager/Private/PackManagerImpl.h"
#include "FileSystem/FileList.h"
#include "DLC/Downloader/DownloadManager.h"
#include "Utils/CRC32.h"
#include "FileSystem/Private/PackArchive.h"

namespace DAVA
{
void PackManagerImpl::Initialize(const FilePath& dbFile_,
                                 const FilePath& localPacksDir_,
                                 const FilePath& readOnlyPacksDir_,
                                 const String& remotePacksURL_,
                                 const String& architecture_,
                                 PackManager* packManager_)
{
    dbFile = dbFile_;
    localPacksDir = localPacksDir_;
    readOnlyPacksDir = readOnlyPacksDir_;
    packsUrlCommon = remotePacksURL_;
    if (packsUrlCommon.empty() || packsUrlCommon.back() != '/')
    {
        throw std::runtime_error("incorrect common url");
    }
    architecture = architecture_;
    if (architecture.empty() || architecture.back() != '/')
    {
        throw std::runtime_error("incorrect gpu url");
    }
    requestManager.reset(new RequestManager(*this));

    packManager = packManager_;
    DVASSERT(packManager != nullptr);

    onPackChange = &packManager->packStateChanged;
    onRequestChange = &packManager->requestProgressChanged;
    packDownload = &packManager->packDownloadChanged;

    initState = PackManager::InitState::Starting;

    // TODO
    // open DB and load packs state then mount all archives to FileSystem
    //db.reset(new PacksDB(dbFile));
    //db->InitializePacks(packs);
    //MountDownloadedPacks(readOnlyPacksDir);
    //MountDownloadedPacks(localPacksDir);
}

// start PackManager::IInitialization //////////////////////////////////////
PackManager::InitState PackManagerImpl::GetState() const
{
    return initState;
}

PackManager::InitError PackManagerImpl::GetError() const
{
    return initError;
}

const String& PackManagerImpl::GetErrorMessage() const
{
    return initErrorMsg;
}

bool PackManagerImpl::CanRetry() const
{
    switch (initState)
    {
    case PackManager::InitState::FirstInit:
    case PackManager::InitState::Starting:
    case PackManager::InitState::MountingLocalPacks:
        return false;
    case PackManager::InitState::LoadingRequestAskFooter:
    case PackManager::InitState::LoadingRequestGetFooter:
    case PackManager::InitState::LoadingRequestAskFileTable:
    case PackManager::InitState::LoadingRequestGetFileTable:
        return true;
    case PackManager::InitState::CalculateLocalDBHashAndCompare:
        return false;
    case PackManager::InitState::LoadingRequestAskDB:
    case PackManager::InitState::LoadingRequestGetDB:
        return true;
    case PackManager::InitState::UnpakingDB:
    case PackManager::InitState::DeleteDownloadedPacksIfNotMatchHash:
    case PackManager::InitState::LoadingPacksDataFromLocalDB:
    case PackManager::InitState::MountingDownloadedPacks:
    case PackManager::InitState::Ready:
        return false;
    }
    DVASSERT(false && "add state");
    return false;
}

void PackManagerImpl::Retry()
{
    if (CanRetry())
    {
        // TODO clear error and move to prev state and unpause if needed
        throw std::runtime_error("implement it");
    }
    else
    {
        throw std::runtime_error("can't retry initialization from current state");
    }
}

bool PackManagerImpl::IsPaused() const
{
    return initPaused;
}

void PackManagerImpl::Pause()
{
    if (initState != PackManager::InitState::Ready)
    {
        initPaused = true;
    }
}
// end PackManager::IInitialization ////////////////////////////////////////

void PackManagerImpl::Update()
{
    if (initPaused)
    {
        if (initState != PackManager::InitState::Ready)
        {
            ContinueInitialization();
        }
        else if (isProcessingEnabled)
        {
            requestManager->Update();
        }
    }
}

void PackManagerImpl::ContinueInitialization()
{
    // TODO
    //        InitState
    //        Starting,
    //        MountingLocalPacks,
    //        LoadingRequestFooter,
    //        LoadingRequestFileTable,
    //        CalculateLocalDataBaseHashAndCompare,
    //        LoadingRequestDataBase,              // skip if hash match
    //        UnpakingkDataBase,                   // skip if hash match
    //        DeleteDownloadedPacksIfNotMatchHash, // skip if hash match
    //        LoadingPacksDataFromDataBase,
    //        MountingDownloadedPacks,
    //        Ready,
    //        Error
    if (PackManager::InitState::FirstInit == initState)
    {
        FirstTimeInit();
    }
    else if (PackManager::InitState::Starting == initState)
    {
        InitStarting();
    }
    else if (PackManager::InitState::MountingLocalPacks == initState)
    {
        MountLocalPacks();
    }
    else if (PackManager::InitState::LoadingRequestAskFooter == initState)
    {
        AskFooter();
    }
    else if (PackManager::InitState::LoadingRequestGetFooter == initState)
    {
        GetFooter();
    }
    else if (PackManager::InitState::LoadingRequestAskFileTable == initState)
    {
        AskFileTable();
    }
    else if (PackManager::InitState::LoadingRequestGetFileTable == initState)
    {
        GetFileTable();
    }
    else if (PackManager::InitState::CalculateLocalDBHashAndCompare == initState)
    {
        CalcLocalDBWitnRemoteCrc32();
    }
    else if (PackManager::InitState::LoadingRequestAskDB == initState)
    {
        AskDB();
    }
    else if (PackManager::InitState::LoadingRequestGetDB == initState)
    {
        GetDB();
    }
    else if (PackManager::InitState::UnpakingDB == initState)
    {
        UnpackDB();
    }
    else if (PackManager::InitState::DeleteDownloadedPacksIfNotMatchHash == initState)
    {
        DeleteOldPacks();
    }
    else if (PackManager::InitState::LoadingPacksDataFromLocalDB == initState)
    {
        LoadingPacksData();
    }
    else if (PackManager::InitState::MountingDownloadedPacks == initState)
    {
        MountDownloadedPacks();
    }
    else if (PackManager::InitState::Ready == initState)
    {
        // happy end
    }
}

void PackManagerImpl::FirstTimeInit()
{
    DVASSERT(initState == PackManager::InitState::FirstInit);
    initState = PackManager::InitState::Starting;
}

void PackManagerImpl::InitStarting()
{
    DVASSERT(initState != PackManager::InitState::FirstInit);
    // you can be in any state and user can start REinitialization
    initState = PackManager::InitState::MountingLocalPacks;
}

void PackManagerImpl::MountLocalPacks()
{
    MountPacks(localPacksDir);
    initState = PackManager::InitState::LoadingRequestAskFooter;
}

void PackManagerImpl::AskFooter()
{
    DownloadManager* dm = DownloadManager::Instance();

    if (0 == fullSizeServerData)
    {
        if (0 == downloadTaskId)
        {
            downloadTaskId = dm->Download(packsUrlCommon, "", GET_SIZE);
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
                            throw std::runtime_error("can't get size of file on server side");
                        }
                    }
                }
            }
        }
    }
    else
    {
        if (fullSizeServerData < sizeof(PackFormat::PackFile))
        {
            throw std::runtime_error("too small superpack on server");
        }

        uint64 downloadOffset = fullSizeServerData - sizeof(footerOnServer);
        downloadTaskId = dm->DownloadIntoBuffer(packsUrlCommon, &footerOnServer, downloadOffset, sizeof(footerOnServer));
        DVASSERT(0 != downloadTaskId);
        initState = PackManager::InitState::LoadingRequestGetFooter;
    }
}

void PackManagerImpl::GetFooter()
{
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
                uint32 crc32 = CRC32::ForBuffer(reinterpret_cast<char*>(&footerOnServer.info), sizeof(footerOnServer.info));
                if (crc32 != footerOnServer.infoCrc32)
                {
                    throw std::runtime_error("on server bad superpack!!! Footer not match crc32");
                }
                usedPackFile.footer = footerOnServer;
                initState = PackManager::InitState::LoadingRequestAskFileTable;
            }
            else
            {
                // TODO ask what to do from Client?
                throw std::runtime_error("not implemented");
            }
        }
    }
    else
    {
        throw std::runtime_error("can't get status for download task");
    }
}

void PackManagerImpl::AskFileTable()
{
    DownloadManager* dm = DownloadManager::Instance();
    tmpFileTable.resize(footerOnServer.info.filesTableSize);

    uint64 downloadOffset = fullSizeServerData - (sizeof(footerOnServer) + footerOnServer.info.filesTableSize);

    downloadTaskId = dm->DownloadIntoBuffer(packsUrlCommon, tmpFileTable.data(), downloadOffset, static_cast<uint32>(tmpFileTable.size()));
    DVASSERT(0 != downloadTaskId);
    initState = PackManager::InitState::LoadingRequestGetFileTable;
}

void PackManagerImpl::GetFileTable()
{
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
                uint32 crc32 = CRC32::ForBuffer(tmpFileTable.data(), static_cast<uint32>(tmpFileTable.size()));
                if (crc32 != footerOnServer.info.filesTableCrc32)
                {
                    throw std::runtime_error("on server bad superpack!!! FileTable not match crc32");
                }

                String fileNames;
                PackArchive::ExtractFileTableData(footerOnServer, tmpFileTable, fileNames, usedPackFile.filesTable);

                initState = PackManager::InitState::LoadingRequest;
            }
            else
            {
                // TODO ask what to do from Client?
                throw std::runtime_error("not implemented");
            }
        }
    }
    else
    {
        throw std::runtime_error("can't get status for download task");
    }
}

const PackManager::Pack& PackManagerImpl::RequestPack(const String& packName, float32 priority)
{
    priority = std::max(0.f, priority);
    priority = std::min(1.f, priority);

    auto& pack = GetPack(packName);
    if (pack.state == PackManager::Pack::Status::NotRequested)
    {
        requestManager->Push(packName, priority);
    }
    else
    {
        if (requestManager->IsInQueue(packName))
        {
            requestManager->UpdatePriority(packName, priority);
        }
    }
    return pack;
}

void PackManagerImpl::MountPacks(const FilePath& packsDir)
{
    if (packsDir.IsEmpty())
    {
        return;
    }
    FileSystem* fs = FileSystem::Instance();

    // build packIndex
    if (packsIndex.empty())
    {
        for (uint32 packIndex = 0; packIndex < packs.size(); ++packIndex)
        {
            PackManager::Pack& pack = packs[packIndex];
            packsIndex[pack.name] = packIndex;
        }
    }

    ScopedPtr<FileList> fileList(new FileList(packsDir, false));

    uint32 numFilesAndDirs = fileList->GetCount();

    for (uint32 fileIndex = 0; fileIndex < numFilesAndDirs; ++fileIndex)
    {
        if (fileList->IsDirectory(fileIndex))
        {
            continue;
        }
        const FilePath& filePath = fileList->GetPathname(fileIndex);
        String fileName = filePath.GetBasename();
        auto it = packsIndex.find(fileName);
        if (it != end(packsIndex) && filePath.GetExtension() == RequestManager::packPostfix)
        {
            // check CRC32 meta and try mount this file
            FilePath packPath(filePath);
            FilePath hashPath(filePath);
            hashPath.ReplaceFilename(fileName + RequestManager::hashPostfix);

            ScopedPtr<File> metaFile(File::Create(hashPath, File::OPEN | File::READ));
            if (metaFile)
            {
                String content;
                metaFile->ReadString(content);

                unsigned int crc32 = 0;
                {
                    StringStream ss;
                    ss << std::hex << content;
                    ss >> crc32;
                }

                PackManager::Pack& pack = packs.at(it->second);
                pack.hashFromMeta = crc32;
                if (pack.hashFromDB != pack.hashFromMeta)
                {
                    // old Pack file with previous version crc32 - delete it
                    fs->DeleteFile(packPath);
                    fs->DeleteFile(hashPath);
                }
                else
                {
                    try
                    {
                        fs->Mount(packPath, "Data/");
                        pack.state = PackManager::Pack::Status::Mounted;
                    }
                    catch (std::exception& ex)
                    {
                        Logger::Error("%s", ex.what());
                    }
                }
            }
        }
        else
        {
            // TODO write what to do with this file? delete it? is it .hash?
        }
    } // end for fileIndex
}

void PackManagerImpl::DeletePack(const String& packName)
{
    auto& pack = GetPack(packName);
    if (pack.state == PackManager::Pack::Status::Mounted)
    {
        // first modify DB
        pack.state = PackManager::Pack::Status::NotRequested;
        pack.priority = 0.0f;
        pack.downloadProgress = 0.f;

        // now remove archive from filesystem
        FileSystem* fs = FileSystem::Instance();
        FilePath archivePath = localPacksDir + packName + RequestManager::packPostfix;
        fs->Unmount(archivePath);

        fs->DeleteFile(archivePath);
        FilePath archiveCrc32Path = archivePath + RequestManager::hashPostfix;
        fs->DeleteFile(archiveCrc32Path);
    }
}

} // end namespace DAVA