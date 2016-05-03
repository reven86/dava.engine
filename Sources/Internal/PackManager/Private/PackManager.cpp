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
struct PackPriorityComparator
{
    bool operator()(const PackManager::Pack* lhs, const PackManager::Pack* rhs) const
    {
        return lhs->priority < rhs->priority;
    }
};

class PackManagerImpl
{
public:
    PackManagerImpl()
    {
    }

    void Initialize(PackManager* packManager_, const FilePath& dbFile_, const FilePath& localPacksDir_, const String& remotePacksURL_)
    {
        dbFile = dbFile_;
        localPacksDir = localPacksDir_;
        remotePacksUrl = remotePacksURL_;
        packManager = packManager_;
        queue.reset(new RequestQueue(*packManager));

        // open DB and load packs state then mount all archives to FileSystem
        db.reset(new PacksDB(dbFile));
        db->GetAllPacksState(packs);
        MountDownloadedPacks();
    }

    bool IsProcessingEnabled() const
    {
        return isProcessingEnabled;
    }

    void EnableProcessing()
    {
        if (!isProcessingEnabled)
        {
            isProcessingEnabled = true;
            queue->Start();
        }
    }

    void DisableProcessing()
    {
        if (isProcessingEnabled)
        {
            isProcessingEnabled = false;
            queue->Stop();
        }
    }

    void Update()
    {
        if (isProcessingEnabled)
        {
            queue->Update();
        }
    }

    const String& FindPack(const FilePath& relativePathInPack) const
    {
        return db->FindPack(relativePathInPack);
    }

    const PackManager::Pack& RequestPack(const String& packID, float32 priority)
    {
        priority = std::max(0.f, priority);
        priority = std::min(1.f, priority);

        auto& pack = GetPackState(packID);
        if (pack.state == PackManager::Pack::NotRequested)
        {
            queue->Push(packID, priority);
        }
        else
        {
            if (queue->IsInQueue(packID))
            {
                queue->UpdatePriority(packID, priority);
            }
        }
        return pack;
    }

    uint32 GetPackIndex(const String& packName)
    {
        auto it = packsIndex.find(packName);
        if (it != end(packsIndex))
        {
            return it->second;
        }
        throw std::runtime_error("can't find pack with name: " + packName);
    }

    PackManager::Pack& GetPackState(const String& packName)
    {
        uint32 index = GetPackIndex(packName);
        return packs[index];
    }

    void MountDownloadedPacks()
    {
        FileSystem* fs = FileSystem::Instance();

        // build packIndex
        for (uint32 packIndex = 0; packIndex < packs.size(); ++packIndex)
        {
            PackManager::Pack& pack = packs[packIndex];
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
                ScopedPtr<File> metaFile(File::Create(filePath + RequestQueue::crc32Postfix, File::OPEN | File::READ));
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
                    pack.crc32FromMeta = crc32;
                    if (pack.crc32FromDB != pack.crc32FromMeta)
                    {
                        // old Pack file with previous version crc32 - delete it
                        fs->DeleteFile(filePath);
                        fs->DeleteFile(filePath + RequestQueue::crc32Postfix);
                    }
                    else
                    {
                        try
                        {
                            fs->Mount(filePath, "Data/");
                            pack.state = PackManager::Pack::Mounted;
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

    void DeletePack(const String& packName)
    {
        auto& state = GetPackState(packName);
        if (state.state == PackManager::Pack::Mounted)
        {
            // first modify DB
            state.state = PackManager::Pack::NotRequested;
            state.priority = 0.0f;
            state.downloadProgress = 0.f;

            // now remove archive from filesystem
            FileSystem* fs = FileSystem::Instance();
            FilePath archivePath = localPacksDir + packName;
            fs->Unmount(archivePath);

            FilePath archiveCrc32Path = archivePath + RequestQueue::crc32Postfix;
            fs->DeleteFile(archiveCrc32Path);
        }
    }

    const Vector<PackManager::Pack>& GetAllState() const
    {
        return packs;
    }

    const FilePath& GetLocalPacksDir() const
    {
        return localPacksDir;
    }

    const String& GetRemotePacksURL() const
    {
        return remotePacksUrl;
    }

private:
    FilePath dbFile;
    FilePath localPacksDir;
    String remotePacksUrl;
    bool isProcessingEnabled = false;
    PackManager* packManager = nullptr;
    UnorderedMap<String, uint32> packsIndex;
    Vector<PackManager::Pack> packs;
    std::unique_ptr<RequestQueue> queue;
    std::unique_ptr<PacksDB> db;
};

PackManager::PackManager()
{
    impl.reset(new PackManagerImpl());
};

PackManager::~PackManager() = default;

void PackManager::Initialize(const FilePath& filesDB_, const FilePath& localPacksDir_, const String& remotePacksUrl)
{
    DVASSERT(FileSystem::Instance()->IsFile(filesDB_));
    DVASSERT(FileSystem::Instance()->IsDirectory(localPacksDir_));

    impl->Initialize(this, filesDB_, localPacksDir_, remotePacksUrl);
}

bool PackManager::IsProcessingEnabled() const
{
    return impl->IsProcessingEnabled();
}

void PackManager::EnableProcessing()
{
    impl->EnableProcessing();
}

void PackManager::DisableProcessing()
{
    impl->DisableProcessing();
}

void PackManager::Update()
{
    // first check if something downloading
    impl->Update();
}

const String& PackManager::FindPack(const FilePath& relativePathInPack) const
{
    return impl->FindPack(relativePathInPack);
}

const PackManager::Pack& PackManager::GetPack(const String& packID) const
{
    return impl->GetPackState(packID);
}

const PackManager::Pack& PackManager::RequestPack(const String& packID, float priority)
{
    return impl->RequestPack(packID, priority);
}

const Vector<PackManager::Pack>& PackManager::GetPacks() const
{
    return impl->GetAllState();
}

void PackManager::Delete(const String& packID)
{
    impl->DeletePack(packID);
}

const FilePath& PackManager::GetLocalPacksDirectory() const
{
    return impl->GetLocalPacksDir();
}

const String& PackManager::GetRemotePacksUrl() const
{
    return impl->GetRemotePacksURL();
}

} // end namespace DAVA