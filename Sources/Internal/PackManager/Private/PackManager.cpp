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
#include "PackManager/Private/RequestQueue.h"
#include "Utils/CRC32.h"

namespace DAVA
{
const String PackManager::crc32Postfix = ".hash";

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

    void UpdateCurrentDownload()
    {

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
    PackManager* packMngrPublic = nullptr;
    UnorderedMap<String, uint32> packsIndex;
    Vector<PackManager::PackState> packs;
    RequestQueue queue;
    std::unique_ptr<PacksDB> packDB;
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
    impl->packMngrPublic = this;

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
        impl->queue.Start();
    }
}

void PackManager::DisableProcessing()
{
    if (impl->isProcessingEnabled)
    {
        impl->isProcessingEnabled = false;
        impl->queue.Stop();
    }
}

void PackManager::Update()
{
    // first check if something downloading
    if (impl->isProcessingEnabled)
    {
        impl->queue.Update();
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

        impl->queue.Push(packID, priority);
    }
    else
    {
        if (impl->queue.IsInQueue(packID))
        {
            impl->queue.UpdatePriority(packID, priority);
        }
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
        state.priority = 0.0f;
        state.downloadProgress = 0.f;

        // now remove archive from filesystem
        FileSystem* fs = FileSystem::Instance();
        FilePath archivePath = impl->packsDB + packID;
        fs->Unmount(archivePath);

        FilePath archiveCrc32Path = impl->localPacksDir + packID + crc32Postfix;
        fs->Unmount(archiveCrc32Path);
    }
}

const FilePath& PackManager::GetLocalPacksDirectory() const
{
    return impl->localPacksDir;
}

const String& PackManager::GetRemotePacksUrl() const
{
    return impl->remotePacksUrl;
}

} // end namespace DAVA