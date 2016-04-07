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

#include "SDLC/SmartDLC.h"
#include "SDLC/Private/PacksDB.h"

namespace DAVA
{
static bool PackCompare(const SmartDlc::PackState* lhs, SmartDlc::PackState* rhs)
{
    return lhs->priority < rhs->priority;
}

class SmartDlcImpl
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
    SmartDlc::PackState& GetPackState(const String& packName)
    {
        uint32 index = GetPackIndex(packName);
        return packs[index];
    }
    void AddToDownloadQueue(const String& packName)
    {
        uint32 index = GetPackIndex(packName);
        SmartDlc::PackState* packState = &packs[index];
        queue.push_back(packState);
        std::stable_sort(begin(queue), end(queue), PackCompare);
    }
    void UpdateQueuePriority(const String& packName, float priority)
    {
        uint32 index = GetPackIndex(packName);
        SmartDlc::PackState* packState = &packs[index];
        auto it = std::find(begin(queue), end(queue), packState);
        if (it != end(queue))
        {
            packState->priority = priority;
            std::stable_sort(begin(queue), end(queue), PackCompare);
        }
    }
    void UpdateCurrentDownload()
    {
        // TODO
        if (currentDownload != nullptr)
        {
        }
    }
    bool IsDownloading() const
    {
        return currentDownload != nullptr;
    }
    void StartNextPackDownloading()
    {
        if (!queue.empty())
        {
            currentDownload = queue.front();
            queue.erase(std::remove(begin(queue), end(queue), currentDownload));

            // TODO start downloading
            auto fullUrl = remotePacksUrl + currentDownload->name;
        }
    }
    void MountDownloadedPacks()
    {
        FileSystem* fs = FileSystem::Instance();
        for (auto& pack : packs)
        {
            // TODO mount pack
        }
    }

    FilePath packsDB;
    FilePath localPacksDir;
    String remotePacksUrl;
    bool isProcessingEnabled = false;
    SmartDlc* sdlcPublic = nullptr;
    UnorderedMap<String, uint32> packsIndex;
    Vector<SmartDlc::PackState> packs;
    Vector<SmartDlc::PackState*> queue;
    SmartDlc::PackState* currentDownload = nullptr;
    std::unique_ptr<PacksDB> packDB;
};

SmartDlc::SmartDlc(const FilePath& packsDB, const FilePath& localPacksDir, const String& remotePacksUrl)
{
    impl.reset(new SmartDlcImpl());
    impl->packsDB = packsDB;
    impl->localPacksDir = localPacksDir;
    impl->remotePacksUrl = remotePacksUrl;

    // TODO open DB and load packs state then mount all archives to FileSystem
    impl->packDB.reset(new PacksDB(packsDB));
    impl->packDB->GetAllPacksState(impl->packs);
    impl->MountDownloadedPacks();
}

SmartDlc::~SmartDlc() = default;

bool SmartDlc::IsProcessingEnabled() const
{
    return impl->isProcessingEnabled;
}

void SmartDlc::EnableProcessing()
{
    impl->isProcessingEnabled = true;
}

void SmartDlc::DisableProcessing()
{
    impl->isProcessingEnabled = false;
}

void SmartDlc::Update()
{
    // TODO first check if something downloading
    if (impl->isProcessingEnabled)
    {
        impl->UpdateCurrentDownload();

        if (!impl->IsDownloading())
        {
            impl->StartNextPackDownloading();
        }
    }
}

const SmartDlc::PackName& SmartDlc::FindPack(const FilePath& relativePathInPack) const
{
    // TODO execute sqlite query
    throw std::runtime_error("not implemented");
}

const SmartDlc::PackState& SmartDlc::GetPackState(const PackName& packID) const
{
    return impl->GetPackState(packID);
}

const SmartDlc::PackState& SmartDlc::RequestPack(const PackName& packID, float priority)
{
    priority = std::max(0.f, priority);
    priority = std::min(1.f, priority);

    auto& packState = impl->GetPackState(packID);
    if (packState.state == PackState::NotRequested)
    {
        packState.state = PackState::Queued;
        packState.priority = priority;
        impl->AddToDownloadQueue(packID);
    }
    else
    {
        impl->UpdateQueuePriority(packID, priority);
    }
    return packState;
}

Vector<SmartDlc::PackState*> SmartDlc::GetRequestedPacks() const
{
    return impl->queue;
}

void SmartDlc::DeletePack(const SmartDlc::PackName& packID)
{
    auto& state = GetPackState(packID);
    if (state.state == SmartDlc::PackState::Mounted)
    {
        // TODO Unmount archive
        // delete archive on file system
        // update DataBase about this pack
    }
}
} // end namespace DAVA