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

    bool IsProcessingEnabled() const;

    void EnableProcessing();

    void DisableProcessing();

    void Update();

    const String& FindPack(const FilePath& relativePathInPack) const;

    const PackManager::Pack& RequestPack(const String& packName, float32 priority);

    uint32 GetPackIndex(const String& packName);

    PackManager::Pack& GetPack(const String& packName);

    void MountDownloadedPacks();

    void DeletePack(const String& packName);

    const Vector<PackManager::Pack>& GetAllState() const;

    const FilePath& GetLocalPacksDir() const;

    const String& GetRemotePacksURL() const;

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

inline bool PackManagerImpl::IsProcessingEnabled() const
{
    return isProcessingEnabled;
}

inline void PackManagerImpl::EnableProcessing()
{
    if (!isProcessingEnabled)
    {
        isProcessingEnabled = true;
        requestManager->Start();
    }
}

inline void PackManagerImpl::DisableProcessing()
{
    if (isProcessingEnabled)
    {
        isProcessingEnabled = false;
        requestManager->Stop();
    }
}

inline void PackManagerImpl::Update()
{
    if (isProcessingEnabled)
    {
        requestManager->Update();
    }
}

inline const String& PackManagerImpl::FindPack(const FilePath& relativePathInPack) const
{
    return db->FindPack(relativePathInPack);
}

inline uint32 PackManagerImpl::GetPackIndex(const String& packName)
{
    auto it = packsIndex.find(packName);
    if (it != end(packsIndex))
    {
        return it->second;
    }
    throw std::runtime_error("can't find pack with name: " + packName);
}

inline PackManager::Pack& PackManagerImpl::GetPack(const String& packName)
{
    uint32 index = GetPackIndex(packName);
    return packs[index];
}

inline const Vector<PackManager::Pack>& PackManagerImpl::GetAllState() const
{
    return packs;
}

inline const FilePath& PackManagerImpl::GetLocalPacksDir() const
{
    return localPacksDir;
}

inline const String& PackManagerImpl::GetRemotePacksURL() const
{
    return packsUrlCommon;
}

} // end namespace DAVA