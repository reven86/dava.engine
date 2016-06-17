#include "PackManager/Private/PackManagerImpl.h"
#include "FileSystem/FileList.h"
#include "DLC/Downloader/DownloadManager.h"
#include "Utils/CRC32.h"

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

    state = PackManager::InitState::Starting;

    // open DB and load packs state then mount all archives to FileSystem
    //db.reset(new PacksDB(dbFile));
    //db->InitializePacks(packs);
    //MountDownloadedPacks(readOnlyPacksDir);
    //MountDownloadedPacks(localPacksDir);
}

void PackManagerImpl::Update()
{
    if (state != PackManager::InitState::Ready &&
        state != PackManager::InitState::Error)
    {
        ContinueInitialization();
    }
    else if (isProcessingEnabled)
    {
        requestManager->Update();
    }
}

void PackManagerImpl::ContinueInitialization()
{
    // TODO
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
    if (PackManager::InitState::Starting == state)
    {
        state = PackManager::InitState::MountingLocalPacks;
    }
    else if (PackManager::InitState::MountingLocalPacks == state)
    {
        MountDownloadedPacks(localPacksDir);
        state = PackManager::InitState::LoadingRequestFooter;
    }
    else if (PackManager::InitState::LoadingRequestFooter == state)
    {
        if (IsFinishingLoadingFooter())
        {
            state = PackManager::InitState::LoadingRequestFileTable;
        }
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

void PackManagerImpl::MountDownloadedPacks(const FilePath& packsDir)
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