#pragma once

#include "PackManager/Private/PacksDB.h"
#include "PackManager/Private/RequestManager.h"
#include "FileSystem/Private/PackFormatSpec.h"
#include "FileSystem/ResourceArchive.h"
#include "FileSystem/FileSystem.h"

namespace DAVA
{

class PackManagerImpl : public IPackManager
{
public:
    PackManagerImpl() = default;

    void InitLocalCommonPacks(const FilePath& readOnlyPacksDir_,
                              const FilePath& downloadPacksDir_,
                              const Hints& hints_) override;

    void InitLocalGpuPacks(const String& architecture_, const String& dbFileName) override;

    bool IsGpuPacksInitialized() const override;

    void InitRemotePacks(const String& urlToServerSuperpack) override;

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

    void Update() override;

    const String& FindPackName(const FilePath& relativePathInPack) const override;

    const Pack& RequestPack(const String& packName) override;

    const IRequest* FindRequest(const String& pack) const override;

    void SetRequestOrder(const String& packName, float newPriority) override;

    uint32 GetPackIndex(const String& packName) const;
    Pack& GetPack(const String& packName);

    const Pack& FindPack(const String& packName) const override;

    void MountPacks(const Set<FilePath>& basePacks);

    void DeletePack(const String& packName);

    uint32_t DownloadPack(const String& packName, const FilePath& packPath);

    const Vector<Pack>& GetPacks() const override;

    const FilePath& GetLocalPacksDirectory() const override;

    const String& GetSuperPackUrl() const;

    const PackFormat::PackFile::FooterBlock& GetInitFooter() const
    {
        return initFooterOnServer;
    }

    const Hints& GetHints() const
    {
        return hints;
    }

private:
    void ContinueInitialization();

    void FirstTimeInit();
    void InitStarting();
    void InitializePacks();
    void MountCommonBasePacks();
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

    FilePath localPacksDir;
    FilePath readOnlyPacksDir;
    String superPackUrl;
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
    InitState initState = InitState::FirstInit;
    InitError initError = InitError::AllGood;
    PackFormat::PackFile::FooterBlock initFooterOnServer; // tmp supperpack info for every new pack request or during initialization
    PackFormat::PackFile usedPackFile; // current superpack info
    Vector<uint8> buffer; // tmp buff
    UnorderedMap<String, const PackFormat::FileTableEntry*> initFileData;
    Vector<ResourceArchive::FileInfo> initfilesInfo;
    uint32 downloadTaskId = 0;
    uint64 fullSizeServerData = 0;
    bool initPaused = false;

    List<FilePath> mountedCommonPacks;

    Hints hints;
};

} // end namespace DAVA
