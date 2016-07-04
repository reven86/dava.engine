#include "PackManager/Private/PackManagerImpl.h"
#include "FileSystem/FileList.h"
#include "FileSystem/Private/PackArchive.h"
#include "FileSystem/Private/ZipArchive.h"
#include "DLC/Downloader/DownloadManager.h"
#include "Utils/CRC32.h"
#include "Compression/LZ4Compressor.h"
#include "DLC/DLC.h"

namespace DAVA
{
static void WriteBufferToFile(const Vector<uint8>& outDB, const FilePath& path)
{
    ScopedPtr<File> f(File::Create(path, File::WRITE | File::CREATE));
    if (!f)
    {
        throw std::runtime_error("can't create file for local DB: " + path.GetStringValue());
    }

    uint32 written = f->Write(outDB.data(), static_cast<uint32>(outDB.size()));
    if (written != outDB.size())
    {
        throw std::runtime_error("can't write file for local DB: " + path.GetStringValue());
    }
}

void PackManagerImpl::Initialize(const String& dbFile_,
                                 const FilePath& readOnlyPacksDir_,
                                 const String& architecture_,
                                 PackManager* packManager_)
{
    readOnlyPacksDir = readOnlyPacksDir_;

    architecture = architecture_;

    if (architecture.empty())
    {
        throw std::runtime_error("empty gpu architecture");
    }

    packManager = packManager_;
    DVASSERT(packManager != nullptr);

    initLocalDBFileName = dbFile_;

    dbZipInDoc = FilePath("~doc:/" + initLocalDBFileName);
    dbZipInData = FilePath(readOnlyPacksDir_ + initLocalDBFileName);

    dbInDoc = FilePath("~doc:/" + initLocalDBFileName);
    dbInDoc.ReplaceExtension("");

    initState = PackManager::InitState::Starting;

    // now init all pack in local read only dir
    FirstTimeInit();
    InitStarting();
    MountLocalPacks();
}

void PackManagerImpl::SyncWithServer(const String& urlToServerSuperpack, const FilePath& downloadPacksDir)
{
    superPackUrl = urlToServerSuperpack;
    localPacksDir = downloadPacksDir;

    if (initState != PackManager::InitState::ReadOnlyPacksReady)
    {
        throw std::runtime_error("first call Initialize");
    }

    initState = PackManager::InitState::LoadingRequestAskFooter;
}

// start PackManager::ISync //////////////////////////////////////
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
    case PackManager::InitState::MountingReadOnlyPacks:
    case PackManager::InitState::ReadOnlyPacksReady:
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
        // for now just go to server check
        initState = PackManager::InitState::LoadingRequestAskFooter;
        // clear error state
        initError = PackManager::InitError::AllGood;
        if (initPaused)
        {
            initPaused = false;
        }
    }
    else
    {
        throw std::runtime_error("can't retry initialization from current state: ");
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
    if (PackManager::InitState::FirstInit != initState)
    {
        if (!initPaused)
        {
            if (initState != PackManager::InitState::Ready)
            {
                ContinueInitialization();
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
}

void PackManagerImpl::ContinueInitialization()
{
    const PackManager::InitState beforeState = initState;

    if (PackManager::InitState::LoadingRequestAskFooter == initState)
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
        UnpackingDB();
    }
    else if (PackManager::InitState::DeleteDownloadedPacksIfNotMatchHash == initState)
    {
        DeleteOldPacks();
    }
    else if (PackManager::InitState::LoadingPacksDataFromLocalDB == initState)
    {
        LoadPacksDataFromDB();
    }
    else if (PackManager::InitState::MountingDownloadedPacks == initState)
    {
        MountDownloadedPacks();
    }
    else if (PackManager::InitState::Ready == initState)
    {
        // happy end
    }

    const PackManager::InitState newState = initState;

    if (newState != beforeState)
    {
        packManager->asyncConnectStateChanged.Emit(*this);
    }
}

void PackManagerImpl::FirstTimeInit()
{
    initState = PackManager::InitState::Starting;
}

void PackManagerImpl::InitStarting()
{
    Logger::FrameworkDebug("pack manager init_starting");

    DVASSERT(initState != PackManager::InitState::FirstInit);

    // copy localPackDB from Data to ~doc:/ if not exist
    FileSystem* fs = FileSystem::Instance();

    if (!fs->IsFile(dbInDoc) || !fs->IsFile(dbZipInDoc))
    {
        fs->DeleteFile(dbInDoc);
        fs->DeleteFile(dbZipInDoc);
        // 0. copy db file from assets (or Data) to doc
        if (!fs->CopyFile(dbZipInData, dbZipInDoc, true))
        {
            throw std::runtime_error("can't copy local zipped DB from Data to Doc: " + dbZipInDoc.GetStringValue());
        }
        // 1. extract db file from zip
        ZipArchive zip(dbZipInDoc);
        // only one file in archive
        const String& fileName = zip.GetFilesInfo().at(0).relativeFilePath;
        Vector<uint8> fileData;
        if (!zip.LoadFile(fileName, fileData))
        {
            throw std::runtime_error("can't unzip: " + fileName + " from: " + dbZipInData.GetStringValue());
        }
        // 2. write to ~doc:/file unpacked file
        ScopedPtr<File> f(File::Create(dbInDoc, File::WRITE | File::CREATE));
        if (!f)
        {
            throw std::runtime_error("can't create file: " + dbInDoc.GetStringValue());
        }
        uint32 written = f->Write(fileData.data(), static_cast<uint32>(fileData.size()));
        if (written != fileData.size())
        {
            throw std::runtime_error("can't write file: " + dbInDoc.GetStringValue());
        }
        if (!fs->IsFile(dbInDoc))
        {
            throw std::runtime_error("no local DB file");
        }
    }
    initState = PackManager::InitState::MountingReadOnlyPacks;
}

void PackManagerImpl::MountLocalPacks()
{
    Logger::FrameworkDebug("pack manager mount_local_packs");
    // now build all packs from localDB, later after request to server
    // we can delete localDB and replace with new from server if needed
    db.reset(new PacksDB(dbInDoc));
    db->InitializePacks(packs);

    MountPacks(readOnlyPacksDir + "common/");
    MountPacks(readOnlyPacksDir + architecture + "/");
    // now user can do requests for local packs
    requestManager.reset(new RequestManager(*this));
    initState = PackManager::InitState::ReadOnlyPacksReady;
}

void PackManagerImpl::AskFooter()
{
    Logger::FrameworkDebug("pack manager ask_footer");

    DownloadManager* dm = DownloadManager::Instance();

    if (0 == fullSizeServerData)
    {
        if (0 == downloadTaskId)
        {
            downloadTaskId = dm->Download(superPackUrl, "", GET_SIZE);
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
                    else
                    {
                        initError = PackManager::InitError::LoadingRequestFailed;
                        initErrorMsg = "failed get superpack size on server, download error: " + DLC::ToString(error);

                        packManager->asyncConnectStateChanged.Emit(*this);
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

        uint64 downloadOffset = fullSizeServerData - sizeof(initFooterOnServer);
        uint32 sizeofFooter = static_cast<uint32>(sizeof(initFooterOnServer));
        downloadTaskId = dm->DownloadIntoBuffer(superPackUrl, &initFooterOnServer, sizeofFooter, downloadOffset, sizeofFooter);
        initState = PackManager::InitState::LoadingRequestGetFooter;
    }
}

void PackManagerImpl::GetFooter()
{
    Logger::FrameworkDebug("pack manager get_footer");

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
                    throw std::runtime_error("on server bad superpack!!! Footer not match crc32");
                }
                usedPackFile.footer = initFooterOnServer;
                initState = PackManager::InitState::LoadingRequestAskFileTable;
            }
            else
            {
                initError = PackManager::InitError::LoadingRequestFailed;
                initErrorMsg = "failed get footer from server, download error: " + DLC::ToString(error);

                Logger::FrameworkDebug("%s", initErrorMsg.c_str());

                packManager->asyncConnectStateChanged.Emit(*this);
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
    Logger::FrameworkDebug("pack manager ask_file_table");

    DownloadManager* dm = DownloadManager::Instance();
    buffer.resize(initFooterOnServer.info.filesTableSize);

    uint64 downloadOffset = fullSizeServerData - (sizeof(initFooterOnServer) + initFooterOnServer.info.filesTableSize);

    downloadTaskId = dm->DownloadIntoBuffer(superPackUrl, buffer.data(), static_cast<uint32>(buffer.size()), downloadOffset, buffer.size());
    DVASSERT(0 != downloadTaskId);
    initState = PackManager::InitState::LoadingRequestGetFileTable;
}

void PackManagerImpl::GetFileTable()
{
    Logger::FrameworkDebug("pack manager get_file_table");

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
                    throw std::runtime_error("on server bad superpack!!! FileTable not match crc32");
                }

                String fileNames;
                PackArchive::ExtractFileTableData(initFooterOnServer, buffer, fileNames, usedPackFile.filesTable);
                PackArchive::FillFilesInfo(usedPackFile, fileNames, initFileData, initfilesInfo);

                initState = PackManager::InitState::CalculateLocalDBHashAndCompare;
            }
            else
            {
                initError = PackManager::InitError::LoadingRequestFailed;
                initErrorMsg = "failed get fileTable from server, download error: " + DLC::ToString(error);

                packManager->asyncConnectStateChanged.Emit(*this);
            }
        }
    }
    else
    {
        throw std::runtime_error("can't get status for download task");
    }
}

void PackManagerImpl::CalcLocalDBWitnRemoteCrc32()
{
    Logger::FrameworkDebug("pack manager calc_local_db_with_remote_crc32");

    FileSystem* fs = FileSystem::Instance();

    if (fs->IsFile(dbInDoc) && fs->IsFile(dbZipInDoc))
    {
        const uint32 localCrc32 = CRC32::ForFile(dbZipInDoc);
        auto it = initFileData.find(initLocalDBFileName);
        if (it != end(initFileData))
        {
            // on server side we not compress
            if (localCrc32 != it->second->originalCrc32)
            {
                // we have to download new localDB file from server!
                initState = PackManager::InitState::LoadingRequestAskDB;
            }
            else
            {
                // all good go to
                initState = PackManager::InitState::MountingDownloadedPacks;
            }
        }
        else
        {
            throw std::runtime_error("can't find local DB in server superpack: " + initLocalDBFileName);
        }
    }
    else
    {
        throw std::runtime_error("no local DB file");
    }
}

void PackManagerImpl::AskDB()
{
    Logger::FrameworkDebug("pack manager ask_db");

    DownloadManager* dm = DownloadManager::Instance();

    auto it = initFileData.find(initLocalDBFileName);
    if (it == end(initFileData))
    {
        throw std::runtime_error("can't find local DB file on server in superpack: " + initLocalDBFileName);
    }

    const PackFormat::FileTableEntry& fileData = *(it->second);

    uint64 downloadOffset = fileData.startPosition;
    uint64 downloadSize = fileData.compressedSize;

    buffer.resize(static_cast<uint32>(downloadSize));

    downloadTaskId = dm->DownloadIntoBuffer(superPackUrl, buffer.data(), static_cast<uint32>(buffer.size()), downloadOffset, downloadSize);
    DVASSERT(0 != downloadTaskId);

    initState = PackManager::InitState::LoadingRequestGetDB;
}

void PackManagerImpl::GetDB()
{
    Logger::FrameworkDebug("pack manager get_db");

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
                initState = PackManager::InitState::UnpakingDB;
            }
            else
            {
                initError = PackManager::InitError::LoadingRequestFailed;
                initErrorMsg = "failed get DB file from server, download error: " + DLC::ToString(error);

                packManager->asyncConnectStateChanged.Emit(*this);
            }
        }
    }
    else
    {
        throw std::runtime_error("can't get status for download task");
    }
}

void PackManagerImpl::UnpackingDB()
{
    Logger::FrameworkDebug("pack manager unpacking_db");

    uint32 buffCrc32 = CRC32::ForBuffer(reinterpret_cast<char*>(buffer.data()), static_cast<uint32>(buffer.size()));

    auto it = initFileData.find(initLocalDBFileName);
    if (it == end(initFileData))
    {
        throw std::runtime_error("can't find local DB file on server in superpack: " + initLocalDBFileName);
    }

    const PackFormat::FileTableEntry& fileData = *it->second;

    if (buffCrc32 != fileData.originalCrc32)
    {
        throw std::runtime_error("on server bad superpack!!! Footer not match crc32");
    }

    if (Compressor::Type::None != fileData.type)
    {
        throw std::runtime_error("can't decompress buffer with local DB");
    }

    WriteBufferToFile(buffer, dbZipInDoc);

    ZipArchive zip(dbZipInDoc);
    if (!zip.LoadFile(zip.GetFilesInfo().at(0).relativeFilePath, buffer))
    {
        throw std::runtime_error("can't unpack db from zip: " + dbZipInDoc.GetStringValue());
    }

    WriteBufferToFile(buffer, dbInDoc);

    buffer.clear();
    buffer.shrink_to_fit();

    initState = PackManager::InitState::DeleteDownloadedPacksIfNotMatchHash;
}

void PackManagerImpl::DeleteOldPacks()
{
    Logger::FrameworkDebug("pack manager delete_old_packs");
    // list all packs (dvpk files) downloaded
    // for each file calculate CRC32
    // check CRC32 with value in FileTable

    ScopedPtr<FileList> fileList(new FileList(localPacksDir));

    for (uint32 i = 0; i > fileList->GetCount(); ++i)
    {
        const FilePath& path = fileList->GetPathname(i);

        if (path.GetExtension() == ".dvpk")
        {
            uint32 crc32 = CRC32::ForFile(path);

            String fileName = path.GetBasename();

            for (auto it = initFileData.begin(); it != initFileData.end(); ++it)
            {
                if (String::npos != it->first.find(fileName))
                {
                    if (crc32 != it->second->originalCrc32)
                    {
                        // delete old packfile
                        if (!FileSystem::Instance()->DeleteFile(path))
                        {
                            throw std::runtime_error("can't delete old packfile: " + path.GetStringValue());
                        }
                    }
                }
            }
        }
    }

    initState = PackManager::InitState::LoadingPacksDataFromLocalDB;
}

void PackManagerImpl::LoadPacksDataFromDB()
{
    Logger::FrameworkDebug("pack manager load_packs_data_from_db");

    // open DB and load packs state then mount all archives to FileSystem
    FilePath path(dbInDoc);
    if (FileSystem::Instance()->IsFile(path))
    {
        db.reset(new PacksDB(path));
        db->InitializePacks(packs);

        initState = PackManager::InitState::MountingDownloadedPacks;
    }
    else
    {
        throw std::runtime_error("no local DB file: " + path.GetStringValue());
    }
}

void PackManagerImpl::MountDownloadedPacks()
{
    Logger::FrameworkDebug("pack manager mount_downloaded_packs");

    MountPacks(localPacksDir);
    initState = PackManager::InitState::Ready;
}

const PackManager::Pack& PackManagerImpl::RequestPack(const String& packName)
{
    if (requestManager)
    {
        PackManager::Pack& pack = GetPack(packName);
        if (pack.state == PackManager::Pack::Status::NotRequested)
        {
            float priority = 0.0f;
            requestManager->Push(packName, priority);
        }
        return pack;
    }

    throw std::runtime_error("can't process request initialization not finished");
}

void PackManagerImpl::ChangePackPriority(const String& packName, float newPriority) const
{
    if (requestManager->IsInQueue(packName))
    {
        requestManager->UpdatePriority(packName, newPriority);
    }
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
            PackManager::Pack& pack = packs.at(packIndex);
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
            FilePath packPath(filePath);
            PackManager::Pack& pack = packs.at(it->second);

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
    }
}

uint32_t PackManagerImpl::DownloadPack(const String& packName, const FilePath& packPath)
{
    PackManager::Pack& pack = GetPack(packName);
    String packFile = packName + RequestManager::packPostfix;

    if (pack.isGPU)
    {
        packFile = architecture + "/" + packFile;
    }
    else
    {
        packFile = "common/" + packFile;
    }

    auto it = initFileData.find(packFile);

    if (it == end(initFileData))
    {
        throw std::runtime_error("can't find pack file: " + packFile);
    }

    uint64 downloadOffset = it->second->startPosition;
    uint64 downloadSize = it->second->originalSize;

    DownloadManager* dm = DownloadManager::Instance();
    const String& url = GetSuperPackUrl();
    uint32 result = dm->DownloadRange(url, packPath, downloadOffset, downloadSize);
    return result;
}

} // end namespace DAVA