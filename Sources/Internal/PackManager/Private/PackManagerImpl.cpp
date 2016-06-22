#include "PackManager/Private/PackManagerImpl.h"
#include "FileSystem/FileList.h"
#include "FileSystem/Private/PackArchive.h"
#include "DLC/Downloader/DownloadManager.h"
#include "Utils/CRC32.h"
#include "Compression/LZ4Compressor.h"

namespace DAVA
{
void PackManagerImpl::Initialize(const String& dbFile_,
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

    initLocalDBFileName = architecture + ".db";
    initState = PackManager::InitState::Starting;
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
    const PackManager::InitState beforeState = initState;

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
        packManager->initStateChanged.Emit(*this);
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
    // TODO copy localPackDB from Data to ~doc:/ if not exist
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
        uint32 sizeofFooter = static_cast<uint32>(sizeof(footerOnServer));
        downloadTaskId = dm->DownloadIntoBuffer(packsUrlCommon, &footerOnServer, sizeofFooter, downloadOffset, sizeofFooter);
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
    buffer.resize(footerOnServer.info.filesTableSize);

    uint64 downloadOffset = fullSizeServerData - (sizeof(footerOnServer) + footerOnServer.info.filesTableSize);

    downloadTaskId = dm->DownloadIntoBuffer(packsUrlCommon, buffer.data(), static_cast<uint32>(downloadOffset), static_cast<uint32>(buffer.size()));
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
                uint32 crc32 = CRC32::ForBuffer(buffer.data(), static_cast<uint32>(buffer.size()));
                if (crc32 != footerOnServer.info.filesTableCrc32)
                {
                    throw std::runtime_error("on server bad superpack!!! FileTable not match crc32");
                }

                String fileNames;
                PackArchive::ExtractFileTableData(footerOnServer, buffer, fileNames, usedPackFile.filesTable);
                PackArchive::FillFilesInfo(usedPackFile, fileNames, initFileData, initfilesInfo);

                initState = PackManager::InitState::CalculateLocalDBHashAndCompare;
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

void PackManagerImpl::CalcLocalDBWitnRemoteCrc32()
{
    const FilePath localDB("~doc:/" + initLocalDBFileName);
    FileSystem* fs = FileSystem::Instance();

    if (fs->IsFile(localDB))
    {
        const uint32 localCrc32 = CRC32::ForFile(localDB);
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
        initState = PackManager::InitState::MountingDownloadedPacks;
    }
}

void PackManagerImpl::AskDB()
{
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

    downloadTaskId = dm->DownloadIntoBuffer(packsUrlCommon, buffer.data(), static_cast<uint32>(buffer.size()), downloadOffset, downloadSize);
    DVASSERT(0 != downloadTaskId);

    initState = PackManager::InitState::LoadingRequestGetDB;
}

void PackManagerImpl::GetDB()
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
                initState = PackManager::InitState::UnpakingDB;
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

void PackManagerImpl::UnpackingDB()
{
    uint32 compressedCrc32 = CRC32::ForBuffer(reinterpret_cast<char*>(buffer.data()), static_cast<uint32>(buffer.size()));

    auto it = initFileData.find(initLocalDBFileName);
    if (it == end(initFileData))
    {
        throw std::runtime_error("can't find local DB file on server in superpack: " + initLocalDBFileName);
    }

    const PackFormat::FileTableEntry& fileData = *it->second;

    if (compressedCrc32 != fileData.compressedCrc32)
    {
        throw std::runtime_error("on server bad superpack!!! Footer not match crc32");
    }

    Vector<uint8> outDB;
    outDB.resize(fileData.originalSize);

    if (LZ4HCCompressor().Decompress(buffer, outDB))
    {
        buffer.clear();
        buffer.shrink_to_fit();

        File* f = File::Create("~doc:/" + initLocalDBFileName, File::WRITE | File::CREATE);
        if (nullptr == f)
        {
            throw std::runtime_error("can't create file for local DB");
        }
        else
        {
            uint32 written = f->Write(outDB.data(), static_cast<uint32>(outDB.size()));
            if (written != outDB.size())
            {
                throw std::runtime_error("can't write file for local DB");
            }
        }

        initState = PackManager::InitState::UnpakingDB;
    }
    else
    {
        throw std::runtime_error("can't decompress buffer with local DB");
    }
}

void PackManagerImpl::DeleteOldPacks()
{
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
    // open DB and load packs state then mount all archives to FileSystem
    FilePath path("~doc:/" + dbFile);
    if (FileSystem::Instance()->IsFile(path))
    {
        db.reset(new PacksDB(dbFile));
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
    MountPacks(localPacksDir);
    initState = PackManager::InitState::Ready;
}

const PackManager::Pack& PackManagerImpl::RequestPack(const String& packName)
{
    auto& pack = GetPack(packName);
    if (pack.state == PackManager::Pack::Status::NotRequested)
    {
        float priority = 0.0f;
        requestManager->Push(packName, priority);
    }
    return pack;
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