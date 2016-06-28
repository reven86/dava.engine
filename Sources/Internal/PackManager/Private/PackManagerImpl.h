#pragma once

#include "PackManager/Private/PacksDB.h"
#include "PackManager/Private/RequestManager.h"
#include "FileSystem/Private/PackFormatSpec.h"

namespace DAVA
{
struct PackPriorityComparator;

class PackManagerImpl : public PackManager::IInit
{
public:
    PackManagerImpl() = default;

    void Initialize(const String& dbFile_,
                    const FilePath& localPacksDir_,
                    const FilePath& readOnlyPacksDir_,
                    const String& packUrlCommon,
                    const String& packUrlGpu,
                    PackManager* packManager_);

    // start PackManager::IInitialization ///////////////////////////////
    PackManager::InitState GetState() const override;
    PackManager::InitError GetError() const override;
    const String& GetErrorMessage() const override;
    bool CanRetry() const override;
    void Retry() override;
    bool IsPaused() const override;
    void Pause() override; // if you need ask USER what to do, you can "Pause" initialization and wait some frames and later call "Retry"
    // end PackManager::IInitialization /////////////////////////////////

    bool IsProcessingEnabled() const;

    void EnableProcessing();

    void DisableProcessing();

    void Update();

    const String& FindPackName(const FilePath& relativePathInPack) const;

    const PackManager::Pack& RequestPack(const String& packName);

    void ChangePackPriority(const String& packName, float newPriority) const;

    uint32 GetPackIndex(const String& packName);

    PackManager::Pack& GetPack(const String& packName);

    void MountPacks(const FilePath&);

    void DeletePack(const String& packName);

    uint32_t DownloadPack(const String& packName, const FilePath& packPath);

    const Vector<PackManager::Pack>& GetAllState() const;

    const FilePath& GetLocalPacksDir() const;

    const String& GetSuperPackUrl() const;

    PackManager& GetPM()
    {
        return *packManager;
    }

    const PackFormat::PackFile::FooterBlock& GetInitFooter() const
    {
        return initFooterOnServer;
    }

private:
    void ContinueInitialization();

    void FirstTimeInit();
    void InitStarting();
    void MountLocalPacks();
    void AskFooter();
    void GetFooter();
    void AskFileTable();
    void GetFileTable();
    void CalcLocalDBWitnRemoteCrc32();
    void AskDB();
    void GetDB();
    void UnpackingDB();
    void DeleteOldPacks();
    void LoadPacksDataFromDB();
    void MountDownloadedPacks();

    FilePath localPacksDir;
    FilePath readOnlyPacksDir;
    String superPackUrl;
    String architecture;
    bool isProcessingEnabled = false;
    PackManager* packManager = nullptr;
    UnorderedMap<String, uint32> packsIndex;
    Vector<PackManager::Pack> packs;
    std::unique_ptr<RequestManager> requestManager;
    std::unique_ptr<PacksDB> db;

    FilePath dbZipInDoc;
    FilePath dbZipInData;
    FilePath dbInDoc;

    String initLocalDBFileName;
    String initErrorMsg;
    PackManager::InitState initState = PackManager::InitState::FirstInit;
    PackManager::InitError initError = PackManager::InitError::AllGood;
    PackFormat::PackFile::FooterBlock initFooterOnServer; // tmp supperpack info for every new pack request or during initialization
    PackFormat::PackFile usedPackFile; // current superpack info
    Vector<uint8> buffer; // tmp buff
    UnorderedMap<String, const PackFormat::FileTableEntry*> initFileData;
    Vector<ResourceArchive::FileInfo> initfilesInfo;
    uint32 downloadTaskId = 0;
    uint64 fullSizeServerData = 0;
    bool initPaused = false;
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

inline const String& PackManagerImpl::FindPackName(const FilePath& relativePathInPack) const
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
    return packs.at(index);
}

inline const Vector<PackManager::Pack>& PackManagerImpl::GetAllState() const
{
    return packs;
}

inline const FilePath& PackManagerImpl::GetLocalPacksDir() const
{
    return localPacksDir;
}

inline const String& PackManagerImpl::GetSuperPackUrl() const
{
    return superPackUrl;
}

} // end namespace DAVA