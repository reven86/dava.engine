#include "PackManager/Private/PackManagerImpl.h"
#include "FileSystem/FileList.h"
#include "FileSystem/Private/PackArchive.h"
#include "FileSystem/Private/ZipArchive.h"
#include "DLC/Downloader/DownloadManager.h"
#include "Utils/CRC32.h"
#include "Utils/StringUtils.h"
#include "Compression/LZ4Compressor.h"
#include "DLC/DLC.h"
#include "Base/Exception.h"
#include "Concurrency/Mutex.h"
#include "Concurrency/LockGuard.h"

#include <algorithm>

namespace DAVA
{
IPackManager::~IPackManager() = default;
IPackManager::IRequest::~IRequest() = default;

const String& IPackManager::ToString(IPackManager::InitState state)
{
    static Vector<String> states{
        "Starting",
        "LoadingRequestAskFooter",
        "LoadingRequestGetFooter",
        "LoadingRequestAskFileTable",
        "LoadingRequestGetFileTable",
        "CalculateLocalDBHashAndCompare",
        "LoadingRequestAskDB",
        "LoadingRequestGetDB",
        "UnpakingDB",
        "DeleteDownloadedPacksIfNotMatchHash",
        "LoadingPacksDataFromLocalDB",
        "MountingDownloadedPacks",
        "Ready"
    };
    DVASSERT(states.size() == 13);
    return states.at(static_cast<size_t>(state));
}

const String& IPackManager::ToString(IPackManager::InitError state)
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
PackManagerImpl::PackManagerImpl(Engine& engine_)
    : engine(engine_)
{
    DVASSERT(Thread::IsMainThread());
    sigConnectionUpdate = engine.update.Connect(this, &PackManagerImpl::Update);
}

PackManagerImpl::~PackManagerImpl()
{
    DVASSERT(Thread::IsMainThread());
    engine.update.Disconnect(sigConnectionUpdate);
}
#endif

void PackManagerImpl::Initialize(const String& architecture_,
                                 const FilePath& dirToDownloadPacks_,
                                 const FilePath& dbFileName_,
                                 const String& urlToServerSuperpack_,
                                 const Hints& hints_)
{
    DVASSERT(Thread::IsMainThread());
    // TODO check if signal asyncConnectStateChanged has any subscriber

    LockGuard<Mutex> lock(protectPM); // just paranoia

    dbPath = dbFileName_;
    dirToDownloadedPacks = dirToDownloadPacks_;

    FileSystem* fs = FileSystem::Instance();
    if (FileSystem::DIRECTORY_CANT_CREATE == fs->CreateDirectory(dirToDownloadedPacks, true))
    {
        DAVA_THROW(DAVA::Exception, "can't create directory for packs: " + dirToDownloadedPacks.GetStringValue());
    }

    urlToSuperPack = urlToServerSuperpack_;
    architecture = architecture_;
    hints = hints_;

    dbName = dbPath.GetFilename();

    dbLocalNameZipped = dirToDownloadedPacks + dbName;

    dbLocalName = dbLocalNameZipped;
    dbLocalName.ReplaceExtension("");

    initPaused = false;

    // if Initialize called second time
    fullSizeServerData = 0;
    if (0 != downloadTaskId)
    {
        DownloadManager::Instance()->Cancel(downloadTaskId);
        downloadTaskId = 0;
    }
    requestManager.reset();
    db.reset();
    // do not! packs.clear();
    // later we will need remember all mounted packs and remount it back
    packsIndex.clear();
    initFileData.clear();

    initError = InitError::AllGood;
    initPaused = false;

    initState = InitState::LoadingRequestAskFooter;
}

bool PackManagerImpl::IsInitialized() const
{
    LockGuard<Mutex> lock(protectPM);
    return InitState::Ready == initState;
}

// start ISync //////////////////////////////////////
IPackManager::InitState PackManagerImpl::GetInitState() const
{
    DVASSERT(Thread::IsMainThread());
    return initState;
}

IPackManager::InitError PackManagerImpl::GetInitError() const
{
    DVASSERT(Thread::IsMainThread());
    return initError;
}

const String& PackManagerImpl::GetInitErrorMessage() const
{
    DVASSERT(Thread::IsMainThread());
    return initErrorMsg;
}

void PackManagerImpl::RetryInit()
{
    DVASSERT(Thread::IsMainThread());

    // clear error state
    Initialize(architecture, dirToDownloadedPacks, dbPath, urlToSuperPack, hints);
}

bool PackManagerImpl::IsPausedInit() const
{
    DVASSERT(Thread::IsMainThread());
    return initPaused;
}

void PackManagerImpl::PauseInit()
{
    DVASSERT(Thread::IsMainThread());

    if (initState != InitState::Ready)
    {
        initPaused = true;
    }
}
// end IInitialization ////////////////////////////////////////

void PackManagerImpl::Update(float)
{
    DVASSERT(Thread::IsMainThread());

    try
    {
        if (InitState::Starting != initState)
        {
            if (!initPaused)
            {
                if (initState != InitState::Ready)
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
    catch (std::exception& ex)
    {
        Logger::Error("PackManager error: %s", ex.what());
        throw; // crush or let parent code decide
    }
}

void PackManagerImpl::ContinueInitialization()
{
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
        CompareLocalDBWitnRemoteHash();
    }
    else if (InitState::LoadingRequestAskDB == initState)
    {
        AskDB();
    }
    else if (InitState::LoadingRequestGetDB == initState)
    {
        GetDB();
    }
    else if (InitState::UnpakingDB == initState)
    {
        UnpackingDB();
    }
    else if (InitState::DeleteDownloadedPacksIfNotMatchHash == initState)
    {
        DeleteOldPacks();
    }
    else if (InitState::LoadingPacksDataFromLocalDB == initState)
    {
        LoadPacksDataFromDB();
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
        asyncConnectStateChanged.Emit(*this);
    }
}

void PackManagerImpl::InitializePacksAndBuildIndex()
{
    db->InitializePacks(packs);

    packsIndex.clear();
    uint32 packIndex = 0;
    for (const auto& pack : packs)
    {
        packsIndex[pack.name] = packIndex++;
    }
}

static void ListPacksInDirAndCopyIfNecessary(const FilePath& copyToDir, const FilePath& fromDir, bool copy, Set<FilePath>& resultSet)
{
    ScopedPtr<FileList> common(new FileList(fromDir));
    for (uint32 i = 0u; i < common->GetCount(); ++i)
    {
        FilePath path = common->GetPathname(i);
        if (path.GetExtension() == RequestManager::packPostfix)
        {
            if (copy)
            {
                FilePath docPath(copyToDir + "/" + path.GetFilename());
                if (!FileSystem::Instance()->Exists(docPath))
                {
                    bool result = FileSystem::Instance()->CopyFile(path, docPath);
                    if (!result)
                    {
                        Logger::Error("can't copy pack from assets to pack dir");
                        DAVA_THROW(DAVA::Exception, "can't copy pack from assets to pack dir");
                    }
                }
                resultSet.insert(docPath);
            }
            else
            {
                resultSet.insert(path);
            }
        }
    }
};

void PackManagerImpl::AskFooter()
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
                    initErrorMsg = "failed get superpack size on server, download error: " + DLC::ToString(error);
                }
            }
        }
    }
}

void PackManagerImpl::GetFooter()
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
                initErrorMsg = "failed get footer from server, download error: " + DLC::ToString(error);
            }
        }
    }
    else
    {
        DAVA_THROW(DAVA::Exception, "can't get status for download task");
    }
}

void PackManagerImpl::AskFileTable()
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

void PackManagerImpl::GetFileTable()
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

                String fileNames;
                PackArchive::ExtractFileTableData(initFooterOnServer, buffer, fileNames, usedPackFile.filesTable);
                PackArchive::FillFilesInfo(usedPackFile, fileNames, initFileData, initfilesInfo);

                initState = InitState::CalculateLocalDBHashAndCompare;
            }
            else
            {
                initError = InitError::LoadingRequestFailed;
                initErrorMsg = "failed get fileTable from server, download error: " + DLC::ToString(error);
            }
        }
    }
    else
    {
        DAVA_THROW(DAVA::Exception, "can't get status for download task");
    }
}

void PackManagerImpl::CompareLocalDBWitnRemoteHash()
{
    //Logger::FrameworkDebug("pack manager calc_local_db_with_remote_crc32");

    FileSystem* fs = FileSystem::Instance();

    if (fs->IsFile(dbLocalName) && fs->IsFile(dbLocalNameZipped))
    {
        const uint32 localCrc32 = CRC32::ForFile(dbLocalNameZipped);
        auto it = initFileData.find(dbName);
        if (it != end(initFileData))
        {
            // on server side we not compress
            if (localCrc32 != it->second->originalCrc32)
            {
                DeleteLocalDBFiles();
                // we have to download new localDB file from server!
                initState = InitState::LoadingRequestAskDB;
            }
            else
            {
                // all good go to
                initState = InitState::LoadingPacksDataFromLocalDB;
            }
        }
        else
        {
            DAVA_THROW(DAVA::Exception, "can't find DB at server superpack: " + dbName);
        }
    }
    else
    {
        DeleteLocalDBFiles();

        initState = InitState::LoadingRequestAskDB;
    }
}

void PackManagerImpl::AskDB()
{
    //Logger::FrameworkDebug("pack manager ask_db");

    DownloadManager* dm = DownloadManager::Instance();

    auto it = initFileData.find(dbName);
    if (it == end(initFileData))
    {
        DAVA_THROW(DAVA::Exception, "can't find local DB file on server in superpack: " + dbName);
    }

    const PackFormat::FileTableEntry& fileData = *(it->second);

    uint64 downloadOffset = fileData.startPosition;
    uint64 downloadSize = fileData.compressedSize > 0 ? fileData.compressedSize : fileData.originalSize;

    buffer.resize(static_cast<size_t>(downloadSize));

    downloadTaskId = dm->DownloadIntoBuffer(urlToSuperPack, buffer.data(), static_cast<uint32>(buffer.size()), downloadOffset, downloadSize);
    DVASSERT(0 != downloadTaskId);

    initState = InitState::LoadingRequestGetDB;
}

void PackManagerImpl::GetDB()
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
                initErrorMsg = "failed get DB file from server, download error: " + DLC::ToString(error);
            }
        }
    }
    else
    {
        DAVA_THROW(DAVA::Exception, "can't get status for download task");
    }
}

void PackManagerImpl::UnpackingDB()
{
    //Logger::FrameworkDebug("pack manager unpacking_db");

    uint32 buffCrc32 = CRC32::ForBuffer(reinterpret_cast<char*>(buffer.data()), static_cast<uint32>(buffer.size()));

    auto it = initFileData.find(dbName);
    if (it == end(initFileData))
    {
        DAVA_THROW(DAVA::Exception, "can't find local DB file on server in superpack: " + dbName);
    }

    const PackFormat::FileTableEntry& fileData = *it->second;

    if (buffCrc32 != fileData.originalCrc32)
    {
        DAVA_THROW(DAVA::Exception, "on server bad superpack!!! Footer not match crc32");
    }

    if (Compressor::Type::None != fileData.type)
    {
        DAVA_THROW(DAVA::Exception, "can't decompress buffer with local DB");
    }

    WriteBufferToFile(buffer, dbLocalNameZipped);

    RefPtr<File> fDb(File::Create(dbLocalNameZipped, File::OPEN | File::READ));

    ZipArchive zip(fDb, dbLocalNameZipped);
    const auto& fileInfos = zip.GetFilesInfo();
    if (fileInfos.empty() || !zip.LoadFile(fileInfos.front().relativeFilePath, buffer))
    {
        DAVA_THROW(DAVA::Exception, "can't unpack db from zip: " + dbLocalNameZipped.GetStringValue());
    }

    WriteBufferToFile(buffer, dbLocalName);

    buffer.clear();
    buffer.shrink_to_fit();

    initState = InitState::DeleteDownloadedPacksIfNotMatchHash;
}

void PackManagerImpl::DeleteOldPacks()
{
    //Logger::FrameworkDebug("pack manager delete_old_packs");

    // list all packs (dvpk files) downloaded
    // for each file calculate CRC32
    // check CRC32 with value in FileTable
    // we can unmount only packs with new crc32

    ScopedPtr<FileList> fileList(new FileList(dirToDownloadedPacks));

    for (uint32 i = 0; i < fileList->GetCount(); ++i)
    {
        const FilePath& path = fileList->GetPathname(i);

        if (path.GetExtension() == RequestManager::packPostfix)
        {
            uint32 crc32 = CRC32::ForFile(path);

            String fileName = path.GetBasename();

            for (auto it = initFileData.begin(); it != initFileData.end(); ++it)
            {
                const String& relativeFilePath = it->first;
                if (StringUtils::EndsWith(relativeFilePath, fileName))
                {
                    const PackFormat::FileTableEntry* fileEntry = it->second;
                    if (crc32 != fileEntry->originalCrc32)
                    {
                        FileSystem::Instance()->Unmount(path);
                        // delete old packfile
                        if (!FileSystem::Instance()->DeleteFile(path))
                        {
                            DAVA_THROW(DAVA::Exception, "can't delete old packfile: " + path.GetStringValue());
                        }
                    }
                }
                else
                {
                    // this pack not exist in current superpack just delete it.
                    // To leave more room for
                    FileSystem::Instance()->DeleteFile(path);
                }
            }
        }
    }

    initState = InitState::LoadingPacksDataFromLocalDB;
}

void PackManagerImpl::LoadPacksDataFromDB()
{
    //Logger::FrameworkDebug("pack manager load_packs_data_from_db");

    // now build all packs from localDB, later after request to server
    // we can delete localDB and replace with new from server if needed
    db.reset(new PacksDB(dbLocalName, hints.dbInMemory));

    InitializePacksAndBuildIndex();

    // now user can do requests for local packs
    requestManager.reset(new RequestManager(*this));

    initState = InitState::MountingDownloadedPacks;
}

void PackManagerImpl::MountDownloadedPacks()
{
    //Logger::FrameworkDebug("pack manager mount_downloaded_packs");

    FileSystem* fs = FileSystem::Instance();

    // now mount all downloaded packs
    // we have to mount all packs togather it's a client requerement
    ScopedPtr<FileList> packFiles(new FileList(dirToDownloadedPacks, false));
    for (unsigned i = 0; i < packFiles->GetCount(); ++i)
    {
        if (packFiles->IsDirectory(i))
        {
            continue;
        }
        const FilePath& packPath = packFiles->GetPathname(i);
        if (packPath.GetExtension() == RequestManager::packPostfix)
        {
            try
            {
                Pack& pack = GetPack(packPath.GetBasename());
                if (!fs->IsMounted(packPath))
                {
                    fs->Mount(packPath, "Data/");
                }
                // do not do! pack.state = Pack::Status::Mounted;
                // client code should request pack first
                // we need it for consistensy with pack dependency
            }
            catch (std::exception& ex)
            {
                Logger::Error("can't auto mount pack on init: %s, cause: %s, so delete it", packPath.GetAbsolutePathname().c_str(), ex.what());
                fs->DeleteFile(packPath);
            }
        }
    }

    initState = InitState::Ready;
}

void PackManagerImpl::DeleteLocalDBFiles()
{
    FileSystem* fs = FileSystem::Instance();
    fs->DeleteFile(dbLocalName);
    fs->DeleteFile(dbLocalNameZipped);
}

void PackManagerImpl::UnmountAllPacks()
{
    for (auto& pack : packs)
    {
        if (pack.state == Pack::Status::Mounted)
        {
            FileSystem::Instance()->Unmount(pack.name);
        }
    }
}

static void CheckPackCrc32(const FilePath& path, const uint32 hashFromDB)
{
    uint32 crc32ForFile = CRC32::ForFile(path);
    if (crc32ForFile != hashFromDB)
    {
        FileSystem::Instance()->DeleteFile(path);
        const char* str = path.GetStringValue().c_str();

        String msg = Format(
        "crc32 not match for pack %s, crc32 from DB 0x%X crc32 from file 0x%X",
        str, hashFromDB, crc32ForFile);

        DAVA_THROW(DAVA::Exception, msg.c_str());
    }
}

void PackManagerImpl::MountPackWithDependencies(Pack& pack, const FilePath& path)
{
    FileSystem* fs = FileSystem::Instance();
    // first check all dependencies already mounted and mount if not
    // 1. collect dependencies
    Vector<Pack*> dependencies;
    dependencies.reserve(64);
    CollectDownloadableDependency(*this, pack.name, dependencies);
    // 2. check every dependency
    Vector<Pack*> notMounted;
    notMounted.reserve(dependencies.size());
    for (Pack* packItem : dependencies)
    {
        if (packItem->state != Pack::Status::Mounted)
        {
            notMounted.push_back(packItem);
        }
    }
    // 3. mount packs
    const FilePath packsDir = path.GetDirectory();
    for (Pack* packItem : notMounted)
    {
        try
        {
            const FilePath packPath = packsDir.GetStringValue() + packItem->name + RequestManager::packPostfix;

            CheckPackCrc32(packPath, packItem->hashFromDB);

            fs->Mount(packPath, "Data/");
            packItem->state = Pack::Status::Mounted;
        }
        catch (std::exception& ex)
        {
            Logger::Error("can't mount dependent pack: %s cause: %s", packItem->name.c_str(), ex.what());
            throw;
        }
    }

    CheckPackCrc32(path, pack.hashFromDB);
    fs->Mount(path, "Data/");
    pack.state = Pack::Status::Mounted;
}

void PackManagerImpl::CollectDownloadableDependency(PackManagerImpl& pm, const String& packName, Vector<Pack*>& dependency)
{
    const Pack& packState = pm.FindPack(packName);
    for (const String& dependName : packState.dependency)
    {
        Pack* dependPack = nullptr;
        try
        {
            dependPack = &pm.GetPack(dependName);
        }
        catch (std::exception& ex)
        {
            Logger::Error("pack \"%s\" has dependency to base pack \"%s\", error: %s", packName.c_str(), dependName.c_str(), ex.what());
            continue;
        }

        if (dependPack->state != Pack::Status::Mounted)
        {
            if (find(begin(dependency), end(dependency), dependPack) == end(dependency))
            {
                dependency.push_back(dependPack);
            }

            CollectDownloadableDependency(pm, dependName, dependency);
        }
    }
}

const IPackManager::Pack& PackManagerImpl::RequestPack(const String& packName)
{
    DVASSERT(Thread::IsMainThread());

    if (requestManager)
    {
        Pack& pack = GetPack(packName);
        if (pack.state == Pack::Status::NotRequested)
        {
            // first try mount pack in it exist on local dounload dir
            FilePath path = dirToDownloadedPacks + "/" + packName + RequestManager::packPostfix;
            FileSystem* fs = FileSystem::Instance();
            if (fs->Exists(path))
            {
                try
                {
                    MountPackWithDependencies(pack, path);
                }
                catch (std::exception& ex)
                {
                    Logger::Error("%s", ex.what());
                    requestManager->Push(packName, 1.0f); // 1.0f last order by default
                }
            }
            else
            {
                requestManager->Push(packName, 1.0f); // 1.0f last order by default
            }
        }
        else if (pack.state == Pack::Status::Mounted)
        {
            // pass
        }
        else if (pack.state == Pack::Status::Downloading)
        {
            // pass
        }
        else if (pack.state == Pack::Status::ErrorLoading)
        {
            requestManager->Push(packName, 1.0f);
        }
        else if (pack.state == Pack::Status::OtherError)
        {
            requestManager->Push(packName, 1.0f);
        }
        else if (pack.state == Pack::Status::Requested)
        {
            // pass
        }
        return pack;
    }
    DAVA_THROW(DAVA::Exception, "can't process request initialization not finished");
}

void PackManagerImpl::ListFilesInPacks(const FilePath& relativePathDir, const Function<void(const FilePath&, const String&)>& fn)
{
    DVASSERT(Thread::IsMainThread());

    if (!relativePathDir.IsDirectoryPathname())
    {
        Logger::Error("can't list not directory path: %s", relativePathDir.GetStringValue().c_str());
        return;
    }

    Set<String> addedDirectory;

    const String relative = relativePathDir.GetRelativePathname("~res:/");

    auto filterMountedPacks = [&](const String& path, const String& pack)
    {
        try
        {
            const Pack& p = FindPack(pack);
            if (p.state == Pack::Status::Mounted)
            {
                size_type index = path.find_first_of("/", relative.size());
                if (String::npos != index)
                {
                    String directoryName = path.substr(relative.size(), index - relative.size());
                    if (addedDirectory.find(directoryName) == end(addedDirectory))
                    {
                        addedDirectory.insert(directoryName);
                        fn("~res:/" + relative + "/" + directoryName + "/", pack);
                    }
                }
                else
                {
                    fn("~res:/" + path, pack);
                }
            }
        }
        catch (std::exception& ex)
        {
            Logger::Error("error while list files in pack: %s", ex.what());
        }
    };

    db->ListFiles(relative, filterMountedPacks);
}

const IPackManager::IRequest* PackManagerImpl::FindRequest(const String& packName) const
{
    DVASSERT(Thread::IsMainThread());
    try
    {
        return &requestManager->Find(packName);
    }
    catch (std::exception&)
    {
        return nullptr;
    }
}

void PackManagerImpl::SetRequestOrder(const String& packName, float newPriority)
{
    DVASSERT(Thread::IsMainThread());
    if (requestManager->IsInQueue(packName))
    {
        requestManager->UpdatePriority(packName, newPriority);
    }
}

void PackManagerImpl::MountPacks(const Set<FilePath>& basePacks)
{
    for_each(begin(basePacks), end(basePacks), [this](const FilePath& filePath)
             {
                 String fileName = filePath.GetBasename();
                 auto it = packsIndex.find(fileName);
                 if (it == end(packsIndex))
                 {
                     DAVA_THROW(DAVA::Exception, "can't find pack: " + fileName + " in packIndex");
                 }

                 Pack& pack = packs.at(it->second);

                 try
                 {
                     FileSystem* fs = FileSystem::Instance();
                     fs->Mount(filePath, "Data/");
                     pack.state = Pack::Status::Mounted;
                 }
                 catch (std::exception& ex)
                 {
                     Logger::Error("%s", ex.what());
                 }
             });
}

void PackManagerImpl::DeletePack(const String& packName)
{
    DVASSERT(Thread::IsMainThread());

    auto& pack = GetPack(packName);
    if (pack.state == Pack::Status::Mounted)
    {
        // first modify DB
        pack.state = Pack::Status::NotRequested;
        pack.priority = 0.0f;
        pack.downloadProgress = 0.f;

        // now remove archive from filesystem
        FileSystem* fs = FileSystem::Instance();
        FilePath archivePath = dirToDownloadedPacks + packName + RequestManager::packPostfix;
        fs->Unmount(archivePath);

        fs->DeleteFile(archivePath);
    }
}

uint32_t PackManagerImpl::DownloadPack(const String& packName, const FilePath& packPath)
{
    Pack& pack = GetPack(packName);
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
        DAVA_THROW(DAVA::Exception, "can't find pack file: " + packFile);
    }

    uint64 downloadOffset = it->second->startPosition;
    uint64 downloadSize = it->second->originalSize;

    DownloadManager* dm = DownloadManager::Instance();
    const String& url = GetSuperPackUrl();
    uint32 result = dm->DownloadRange(url, packPath, downloadOffset, downloadSize);
    return result;
}

bool PackManagerImpl::IsRequestingEnabled() const
{
    DVASSERT(Thread::IsMainThread());
    return isProcessingEnabled;
}

void PackManagerImpl::EnableRequesting()
{
    DVASSERT(Thread::IsMainThread());

    if (!isProcessingEnabled)
    {
        isProcessingEnabled = true;
        if (requestManager)
        {
            requestManager->Start();
        }
    }
}

void PackManagerImpl::DisableRequesting()
{
    DVASSERT(Thread::IsMainThread());

    if (isProcessingEnabled)
    {
        isProcessingEnabled = false;
        if (requestManager)
        {
            requestManager->Stop();
        }
    }
}

const String& PackManagerImpl::FindPackName(const FilePath& relativePathInPack) const
{
    LockGuard<Mutex> lock(protectPM);
    const String& result = db->FindPack(relativePathInPack);
    return result;
}

uint32 PackManagerImpl::GetPackIndex(const String& packName) const
{
    DVASSERT(Thread::IsMainThread());

    auto it = packsIndex.find(packName);
    if (it != end(packsIndex))
    {
        return it->second;
    }
    DAVA_THROW(DAVA::Exception, "can't find pack with name: " + packName);
}

IPackManager::Pack& PackManagerImpl::GetPack(const String& packName)
{
    DVASSERT(Thread::IsMainThread());

    uint32 index = GetPackIndex(packName);
    return packs.at(index);
}

const IPackManager::Pack& PackManagerImpl::FindPack(const String& packName) const
{
    DVASSERT(Thread::IsMainThread());

    uint32 index = GetPackIndex(packName);
    return packs.at(index);
}

const Vector<IPackManager::Pack>& PackManagerImpl::GetPacks() const
{
    DVASSERT(Thread::IsMainThread());
    return packs;
}

const FilePath& PackManagerImpl::GetLocalPacksDirectory() const
{
    DVASSERT(Thread::IsMainThread());
    return dirToDownloadedPacks;
}

const String& PackManagerImpl::GetSuperPackUrl() const
{
    DVASSERT(Thread::IsMainThread());
    return urlToSuperPack;
}

} // end namespace DAVA
