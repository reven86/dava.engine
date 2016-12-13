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
class DCLManagerImpl final
{
public:
    enum class InitState : uint32
    {
        Starting, //!< before any initialization code state
        LoadingRequestAskFooter, //!< connect to server superpack.dvpk for footer block
        LoadingRequestGetFooter, //!< download footer and parse it, find out filetable block size and position
        LoadingRequestAskFileTable, //!< start loading filetable block from superpack.dvpk
        LoadingRequestGetFileTable, //!< download filetable and fill info about every file on server superpack.dvpk
        CalculateLocalDBHashAndCompare, //!< check if existing local DB hash match with remote DB on server, go to LoadingPacksDataFromLocalDB if match
        LoadingRequestAskDB, //!< start loading DB from server
        LoadingRequestGetDB, //!< download DB and check it's hash
        UnpakingDB, //!< unpack DB from zip
        DeleteDownloadedPacksIfNotMatchHash, //!< go throw all local packs and unmount it if hash not match then delete
        LoadingPacksDataFromLocalDB, //!< open local DB and build pack index for all packs
        MountingDownloadedPacks, //!< mount all local packs downloaded and not mounted later
        Ready, //!< starting from this state client can call any method, second initialize will work too
        Offline //!< server not accessible, retry initialization after Hints::retryConnectMilliseconds
    };

    static const String& ToString(InitState state);

    enum class InitError : uint32
    {
        AllGood,
        CantCopyLocalDB,
        CantMountLocalPacks,
        LoadingRequestFailed,
        UnpackingDBFailed,
        DeleteDownloadedPackFailed,
        LoadingPacksDataFailed,
        MountingDownloadedPackFailed
    };

    static const String& ToString(InitError state);

#ifdef __DAVAENGINE_COREV2__
    explicit DCLManagerImpl(Engine& engine_);
    ~DCLManagerImpl();
    Engine& engine;
    SigConnectionID sigConnectionUpdate = 0;
#else
    DCLManagerImpl() = default;
    ~DCLManagerImpl() = default;
#endif

    void Initialize(const String& architecture_,
                    const FilePath& dirToDownloadPacks_,
                    const FilePath& pathToBasePacksDB_,
                    const String& urlToServerSuperpack_,
                    const DLCManager::Hints& hints_);

    void RetryInit();

    bool IsInitialized() const;

    InitState GetInitState() const;

    InitError GetInitError() const;

    const String& GetLastErrorMessage() const;

    bool IsRequestingEnabled() const;

    void EnableRequesting();

    void DisableRequesting();

    void Update(float frameDelta);

    const String& FindPackName(const FilePath& relativePathInPack) const;

    const DLCManager::IRequest* RequestPack(const String& packName);

    const DLCManager::IRequest* FindRequest(const String& pack) const;

    void SetRequestOrder(const DLCManager::IRequest*, unsigned orderIndex);

    uint32_t DownloadPack(const String& packName, const FilePath& packPath);

    const FilePath& GetLocalPacksDirectory() const;

    const String& GetSuperPackUrl() const;

    uint32 GetServerFooterCrc32() const
    {
        return initFooterOnServer.infoCrc32;
    }

    const DLCManager::Hints& GetHints() const
    {
        DVASSERT(hints != nullptr);
        return *hints;
    }

    static void CollectDownloadableDependency(DCLManagerImpl& pm, const String& packName, Vector<DLCManager::IRequest*>& dependency);

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
    void StoreAllMountedPackNames();
    void DeleteOldPacks();
    void ReloadState();
    void LoadPacksDataFromDB();
    void MountDownloadedPacks();
    // helper functions
    void DeleteLocalDBFiles();
    void ContinueInitialization(float frameDelta);
    void UnmountAllPacks();

    mutable Mutex protectPM;

    FilePath dirToDownloadedPacks;
    FilePath dbPath;
    String urlToSuperPack;
    String architecture;
    bool isProcessingEnabled = false;
    std::unique_ptr<RequestManager> requestManager;
    std::unique_ptr<PacksDB> db;

    FilePath dbLocalNameZipped;
    FilePath dbLocalName;

    String dbName;
    String initErrorMsg;
    InitState initState = InitState::Starting;
    InitError initError = InitError::AllGood;
    PackFormat::PackFile::FooterBlock initFooterOnServer; // tmp supperpack info for every new pack request or during initialization
    PackFormat::PackFile usedPackFile; // current superpack info
    Vector<uint8> buffer; // tmp buff
    // first - relative file name in archive, second - file properties
    UnorderedMap<String, const PackFormat::FileTableEntry*> initFileData;
    Vector<ResourceArchive::FileInfo> initfilesInfo;
    uint32 downloadTaskId = 0;
    uint64 fullSizeServerData = 0;

    DLCManager::Hints* hints = nullptr;

    float32 timeWaitingNextInitializationAttempt = 0;
    uint32 retryCount = 0; // count every initialization error during session
    Vector<String> tmpOldMountedPackNames;
};

} // end namespace DAVA
