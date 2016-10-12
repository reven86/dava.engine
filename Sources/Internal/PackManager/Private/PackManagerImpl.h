#pragma once

#include "PackManager/PackManager.h"
#include "PackManager/Private/PacksDB.h"
#include "PackManager/Private/RequestManager.h"
#include "FileSystem/Private/PackFormatSpec.h"
#include "FileSystem/ResourceArchive.h"
#include "FileSystem/FileSystem.h"

#ifdef __DAVAENGINE_COREV2__
#include "Engine/Engine.h"
#endif

namespace DAVA
{
class PackManagerImpl : public IPackManager
{
public:
#ifdef __DAVAENGINE_COREV2__
    explicit PackManagerImpl(Engine& engine_);
    ~PackManagerImpl();
    Engine& engine;
    SigConnectionID sigConnectionUpdate = 0;
#else
    PackManagerImpl() = default;
    ~PackManagerImpl() = default;
#endif

    void Initialize(const String& architecture_,
                    const FilePath& dirToDownloadPacks_,
                    const FilePath& pathToBasePacksDB_,
                    const String& urlToServerSuperpack_,
                    const Hints& hints_) override;

    bool IsInitialized() const override;

    InitState GetInitState() const override;

    InitError GetInitError() const override;

    const String& GetInitErrorMessage() const override;

    bool CanRetryInit() const override;

    void RetryInit() override;

    bool IsPausedInit() const override;

    void PauseInit() override;

    bool IsRequestingEnabled() const override;

    void EnableRequesting() override;

    void DisableRequesting() override;

    void Update(float) override;

    const String& FindPackName(const FilePath& relativePathInPack) const override;

    const Pack& RequestPack(const String& packName) override;

    void ListFilesInPacks(const FilePath& relativePathDir, const Function<void(const FilePath&, const String&)>& fn) override;

    const IRequest* FindRequest(const String& pack) const override;

    void SetRequestOrder(const String& packName, float newPriority) override;

    uint32 GetPackIndex(const String& packName) const;
    Pack& GetPack(const String& packName);

    const Pack& FindPack(const String& packName) const override;

    void MountPacks(const Set<FilePath>& basePacks);

    void DeletePack(const String& packName) override;

    uint32_t DownloadPack(const String& packName, const FilePath& packPath);

    const Vector<Pack>& GetPacks() const override;

    const FilePath& GetLocalPacksDirectory() const override;

    const String& GetSuperPackUrl() const override;

    const PackFormat::PackFile::FooterBlock& GetInitFooter() const
    {
        return initFooterOnServer;
    }

    const Hints& GetHints() const
    {
        return hints;
    }

    static void CollectDownloadableDependency(PackManagerImpl& pm, const String& packName, Vector<Pack*>& dependency);

private:
    // initialization state functions
    void AskFooter();
    void GetFooter();
    void AskFileTable();
    void GetFileTable();
    void CompareLocalDBWitnRemoteHash();
    void AskDB();
    void GetDB();
    void UnpackingDB();
    void DeleteOldPacks();
    void LoadPacksDataFromDB();
    void MountDownloadedPacks();
    // helper functions
    void DeleteLocalDBFiles();
    void ContinueInitialization();
    void InitializePacksAndBuildIndex();
    void UnmountAllPacks();
    void MountPackWithDependencies(Pack& pack, const FilePath& path);

    mutable Mutex protectPM;

    FilePath dirToDownloadedPacks;
    FilePath pathToBasePacksDB;
    String urlToSuperPack;
    String architecture;
    bool isProcessingEnabled = false;
    UnorderedMap<String, uint32> packsIndex;
    Vector<Pack> packs;
    std::unique_ptr<RequestManager> requestManager;
    std::unique_ptr<PacksDB> db;

    FilePath dbZipInDoc;
    FilePath dbZipInData;
    FilePath dbInDoc;

    String initLocalDBFileName;
    String initErrorMsg;
    InitState initState = InitState::Starting;
    InitError initError = InitError::AllGood;
    PackFormat::PackFile::FooterBlock initFooterOnServer; // tmp supperpack info for every new pack request or during initialization
    PackFormat::PackFile usedPackFile; // current superpack info
    Vector<uint8> buffer; // tmp buff
    UnorderedMap<String, const PackFormat::FileTableEntry*> initFileData;
    Vector<ResourceArchive::FileInfo> initfilesInfo;
    uint32 downloadTaskId = 0;
    uint64 fullSizeServerData = 0;
    bool initPaused = false;

    Hints hints;
};

} // end namespace DAVA
