#pragma once

#include "PackManager/Private/PacksDB.h"
#include "PackManager/Private/RequestManager.h"

namespace DAVA
{
struct PackPriorityComparator;

class PackManagerImpl
{
public:
    PackManagerImpl() = default;

    void Initialize(const FilePath& dbFile_,
                    const FilePath& localPacksDir_,
                    const String& packUrlCommon,
                    const String& packUrlGpu,
                    Signal<const PackManager::Pack&, PackManager::Pack::Change>& signal);

    inline bool IsProcessingEnabled() const;

    inline void EnableProcessing();

    inline void DisableProcessing();

    inline void Update();

    const String& FindPack(const FilePath& relativePathInPack) const;

    const PackManager::Pack& RequestPack(const String& packName, float32 priority);

    inline uint32 GetPackIndex(const String& packName);

    inline PackManager::Pack& GetPack(const String& packName);

    void MountDownloadedPacks();

    void DeletePack(const String& packName);

    inline const Vector<PackManager::Pack>& GetAllState() const;

    inline const FilePath& GetLocalPacksDir() const;

    inline const String& GetRemotePacksURL() const;

    Signal<const PackManager::Pack&, PackManager::Pack::Change>* onPackChange;

private:
    FilePath dbFile;
    FilePath localPacksDir;
    String packsUrlCommon;
    bool isProcessingEnabled = false;
    PackManager* packManager = nullptr;
    UnorderedMap<String, uint32> packsIndex;
    Vector<PackManager::Pack> packs;
    std::unique_ptr<RequestManager> requestManager;
    std::unique_ptr<PacksDB> db;
};

struct PackPriorityComparator
{
    bool operator()(const PackManager::Pack* lhs, const PackManager::Pack* rhs) const
    {
        return lhs->priority < rhs->priority;
    }
};

bool PackManagerImpl::IsProcessingEnabled() const
{
    return isProcessingEnabled;
}

void PackManagerImpl::EnableProcessing()
{
    if (!isProcessingEnabled)
    {
        isProcessingEnabled = true;
        requestManager->Start();
    }
}

void PackManagerImpl::DisableProcessing()
{
    if (isProcessingEnabled)
    {
        isProcessingEnabled = false;
        requestManager->Stop();
    }
}

void PackManagerImpl::Update()
{
    if (isProcessingEnabled)
    {
        requestManager->Update();
    }
}

const String& PackManagerImpl::FindPack(const FilePath& relativePathInPack) const
{
    return db->FindPack(relativePathInPack);
}

uint32 PackManagerImpl::GetPackIndex(const String& packName)
{
    auto it = packsIndex.find(packName);
    if (it != end(packsIndex))
    {
        return it->second;
    }
    throw std::runtime_error("can't find pack with name: " + packName);
}

PackManager::Pack& PackManagerImpl::GetPack(const String& packName)
{
    uint32 index = GetPackIndex(packName);
    return packs[index];
}

const Vector<PackManager::Pack>& PackManagerImpl::GetAllState() const
{
    return packs;
}

const FilePath& PackManagerImpl::GetLocalPacksDir() const
{
    return localPacksDir;
}

const String& PackManagerImpl::GetRemotePacksURL() const
{
    return packsUrlCommon;
}

} // end namespace DAVA