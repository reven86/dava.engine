/*==================================================================================
Copyright (c) 2008, binaryzebra
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

* Redistributions of source code must retain the above copyright
notice, this list of conditions and the following disclaimer.
* Redistributions in binary form must reproduce the above copyright
notice, this list of conditions and the following disclaimer in the
documentation and/or other materials provided with the distribution.
* Neither the name of the binaryzebra nor the
names of its contributors may be used to endorse or promote products
derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE binaryzebra AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL binaryzebra BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/

#include "PackManager/PackManager.h"
#include "PackManager/Private/PacksDB.h"
#include "FileSystem/FileList.h"
#include "DLC/Downloader/DownloadManager.h"
#include "PackManager/Private/DynamicPriorityQueue.h"

namespace DAVA
{
struct PackPriorityComparator
{
    bool operator()(const PackManager::PackState* lhs, const PackManager::PackState* rhs) const
    {
        return lhs->priority < rhs->priority;
    }
};

class ArchiveManagerImpl
{
public:
    uint32 GetPackIndex(const String& packName)
    {
        auto it = packsIndex.find(packName);
        if (it != end(packsIndex))
        {
            return it->second;
        }
        throw std::runtime_error("can't find pack with name: " + packName);
    }

    PackManager::PackState& GetPackState(const String& packName)
    {
        uint32 index = GetPackIndex(packName);
        return packs[index];
    }

    void AddToDownloadQueue(const String& packName)
    {
        uint32 index = GetPackIndex(packName);
        PackManager::PackState* packState = &packs[index];

        // TODO теперь нужно узнать виртуальный ли это пакет, и первыми поставить на закачку
        // так как у нас может быть несколько зависимых паков, тоже виртуальными, то
        // мы должны сначала сделать плоскую структуру всех зависимых паков, всем им
        // выставить одинаковый приоритет - текущего виртуального пака и добавить
        // в очередь на скачку в порядке, зависимостей
        Set<PackManager::PackState*> dependency;
        CollectAllDependencyForPack(packName, dependency); // TODO continue here

        queue.Push(packState);

        if (isProcessingEnabled && !IsDownloading())
        {
            StartNextPackDownloading();
        }
    }

    void UpdateQueuePriority(const String& packName, float priority)
    {
        uint32 index = GetPackIndex(packName);
        PackManager::PackState* packState = &packs[index];
        queue.UpdatePriority(packState);
    }

    void UpdateCurrentDownload()
    {
        if (currentDownload != nullptr)
        {
            DownloadManager* dm = DownloadManager::Instance();
            DownloadStatus status = DL_UNKNOWN;
            dm->GetStatus(downloadHandler, status);
            uint64 progress = 0;
            switch (status)
            {
            case DL_IN_PROGRESS:
                if (dm->GetProgress(downloadHandler, progress))
                {
                    uint64 total = 0;
                    if (dm->GetTotal(downloadHandler, total))
                    {
                        currentDownload->downloadProgress = static_cast<float>(progress) / total;
                        // fire event on update progress
                        sdlcPublic->onPackStateChanged.Emit(*currentDownload);
                    }
                }
                break;
            case DL_FINISHED:
            {
                // first test error code
                DownloadError downloadError = DLE_NO_ERROR;
                if (dm->GetError(downloadHandler, downloadError))
                {
                    switch (downloadError)
                    {
                    case DLE_CANCELLED: // download was cancelled by our side
                    case DLE_COULDNT_RESUME: // seems server doesn't supports download resuming
                    case DLE_COULDNT_RESOLVE_HOST: // DNS request failed and we cannot to take IP from full qualified domain name
                    case DLE_COULDNT_CONNECT: // we cannot connect to given adress at given port
                    case DLE_CONTENT_NOT_FOUND: // server replies that there is no requested content
                    case DLE_NO_RANGE_REQUEST: // Range requests is not supported. Use 1 thread without reconnects only.
                    case DLE_COMMON_ERROR: // some common error which is rare and requires to debug the reason
                    case DLE_INIT_ERROR: // any handles initialisation was unsuccessful
                    case DLE_FILE_ERROR: // file read and write errors
                    case DLE_UNKNOWN: // we cannot determine the error
                        // inform user about error
                        {
                            currentDownload->state = PackManager::PackState::ErrorLoading;
                            currentDownload->downloadError = downloadError;
                            sdlcPublic->onPackStateChanged.Emit(*currentDownload);
                            break;
                        }
                    case DLE_NO_ERROR:
                    {
                        // now mount archive
                        // validate it
                        FileSystem* fs = FileSystem::Instance();
                        FilePath archivePath = localPacksDir + currentDownload->name;
                        fs->Mount(archivePath, "Data");

                        currentDownload->state = PackManager::PackState::Mounted;
                    }
                    } // end switch downloadError
                    currentDownload = nullptr;
                    downloadHandler = 0;
                }
                else
                {
                    throw std::runtime_error(Format("can't get download error code for download job id: %d", downloadHandler));
                }
            }
            break;
            default:
                break;
            }
        }
    }

    bool IsDownloading() const
    {
        return currentDownload != nullptr;
    }

    void StartNextPackDownloading()
    {
        if (!queue.Empty())
        {
            currentDownload = queue.Pop();

            // start downloading
            auto fullUrl = remotePacksUrl + currentDownload->name;

            FilePath archivePath = localPacksDir + currentDownload->name;

            DownloadManager* dm = DownloadManager::Instance();
            downloadHandler = dm->Download(fullUrl, archivePath);
        }
    }

    void StopDownloading()
    {
        if (currentDownload != nullptr)
        {
            DownloadManager* dm = DownloadManager::Instance();
            dm->Cancel(downloadHandler);
        }
    }

    void MountDownloadedPacks()
    {
        FileSystem* fs = FileSystem::Instance();

        // build packIndex
        for (uint32 packIndex = 0; packIndex < packs.size(); ++packIndex)
        {
            PackManager::PackState& pack = packs[packIndex];
            packsIndex[pack.name] = packIndex;
        }

        ScopedPtr<FileList> fileList(new FileList(localPacksDir, false));

        uint32 numFilesAndDirs = fileList->GetCount();

        for (uint32 fileIndex = 0; fileIndex < numFilesAndDirs; ++fileIndex)
        {
            if (fileList->IsDirectory(fileIndex))
            {
                continue;
            }
            const FilePath& filePath = fileList->GetPathname(fileIndex);
            String fileName = filePath.GetFilename();
            auto it = packsIndex.find(fileName);
            if (it != end(packsIndex))
            {
                // check CRC32 meta and try mount this file
                ScopedPtr<File> metaFile(File::Create(filePath + ".crc32", File::OPEN | File::READ));
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

                    PackManager::PackState& pack = packs.at(it->second);
                    pack.crc32FromMeta = crc32;
                    if (pack.crc32FromDB != pack.crc32FromMeta)
                    {
                        // old Pack file with previous version crc32 - delete it
                        fs->DeleteFile(filePath);
                        fs->DeleteFile(filePath + ".crc32");
                    }
                    else
                    {
                        try
                        {
                            fs->Mount(filePath, "Data");
                            pack.state = PackManager::PackState::Mounted;
                        }
                        catch (std::exception& ex)
                        {
                            Logger::Error("%s", ex.what());
                        }
                    }
                }
            }
        } // end for fileIndex
    }

    FilePath packsDB;
    FilePath localPacksDir;
    String remotePacksUrl;
    bool isProcessingEnabled = false;
    PackManager* sdlcPublic = nullptr;
    UnorderedMap<String, uint32> packsIndex;
    Vector<PackManager::PackState> packs;
    DynamicPriorityQueue<PackManager::PackState, PackPriorityComparator> queue;
    PackManager::PackState* currentDownload = nullptr;
    std::unique_ptr<PacksDB> packDB;
    uint32 downloadHandler = 0;
};

PackManager::PackManager(const FilePath& packsDB, const FilePath& localPacksDir, const String& remotePacksUrl)
{
    if (!localPacksDir.IsDirectoryPathname())
    {
        throw std::runtime_error("not directory path_name:" + localPacksDir.GetStringValue());
    }

    impl.reset(new ArchiveManagerImpl());
    impl->packsDB = packsDB;
    impl->localPacksDir = localPacksDir;
    impl->remotePacksUrl = remotePacksUrl;
    impl->sdlcPublic = this;

    // open DB and load packs state then mount all archives to FileSystem
    impl->packDB.reset(new PacksDB(packsDB));
    impl->packDB->GetAllPacksState(impl->packs);
    impl->MountDownloadedPacks();
}

PackManager::~PackManager() = default;

bool PackManager::IsProcessingEnabled() const
{
    return impl->isProcessingEnabled;
}

void PackManager::EnableProcessing()
{
    if (!impl->isProcessingEnabled)
    {
        impl->isProcessingEnabled = true;
    }
}

void PackManager::DisableProcessing()
{
    if (impl->isProcessingEnabled)
    {
        impl->isProcessingEnabled = false;
        impl->StopDownloading();
        if (impl->currentDownload)
        {
            impl->AddToDownloadQueue(impl->currentDownload->name);
            impl->currentDownload = nullptr;
        }
    }
}

void PackManager::Update()
{
    // first check if something downloading
    if (impl->isProcessingEnabled)
    {
        impl->UpdateCurrentDownload();

        if (!impl->IsDownloading())
        {
            impl->StartNextPackDownloading();
        }
    }
}

const String& PackManager::FindPack(const FilePath& relativePathInPack) const
{
    return impl->packDB->FindPack(relativePathInPack);
}

const PackManager::PackState& PackManager::GetPackState(const String& packID) const
{
    return impl->GetPackState(packID);
}

const PackManager::PackState& PackManager::RequestPack(const String& packID, float priority)
{
    priority = std::max(0.f, priority);
    priority = std::min(1.f, priority);

    auto& packState = impl->GetPackState(packID);
    if (packState.state == PackState::NotRequested)
    {
        packState.state = PackState::Requested;
        packState.priority = priority;
        impl->AddToDownloadQueue(packID);
    }
    else
    {
        impl->UpdateQueuePriority(packID, priority);
    }
    return packState;
}

const Vector<PackManager::PackState*>& PackManager::GetAllState() const
{
    static Vector<PackManager::PackState*> allState;

    allState.clear();
    allState.reserve(impl->packs.size());

    for (auto& state : impl->packs)
    {
        allState.push_back(&state);
    }

    return allState;
}

void PackManager::DeletePack(const String& packID)
{
    auto& state = impl->GetPackState(packID);
    if (state.state == PackState::Mounted)
    {
        // first modify DB
        state.state = PackState::NotRequested;
        state.priority = 0.5f;
        state.downloadProgress = 0.f;

        impl->packDB->UpdatePackState(state);

        // now remove archive from filesystem
        FileSystem* fs = FileSystem::Instance();
        FilePath archivePath = impl->packsDB + packID;
        fs->Unmount(archivePath);
    }
}
} // end namespace DAVA