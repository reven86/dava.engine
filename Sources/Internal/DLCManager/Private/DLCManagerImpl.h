#pragma once

#include "DLCManager/DLCManager.h"
#include "DLCManager/Private/RequestManager.h"
#include "FileSystem/Private/PackFormatSpec.h"
#include "FileSystem/Private/PackMetaData.h"
#include "Concurrency/Mutex.h"
#include "Concurrency/Semaphore.h"
#include "Concurrency/Thread.h"

#ifdef __DAVAENGINE_COREV2__
#include "Engine/Engine.h"
#endif

namespace DAVA
{
class DLCManagerImpl final : public DLCManager
{
public:
    enum class InitState : uint32
    {
        Starting, //!< before any initialization code state
        LoadingRequestAskFooter, //!< connect to server superpack.dvpk for footer block
        LoadingRequestGetFooter, //!< download footer and parse it, find out filetable block size and position
        LoadingRequestAskFileTable, //!< start loading filetable block from superpack.dvpk
        LoadingRequestGetFileTable, //!< download filetable and fill info about every file on server superpack.dvpk
        CalculateLocalDBHashAndCompare, //!< check if existing local DB hash match with remote DB on server, go to LoadingPacksDataFromLocalMeta if match
        LoadingRequestAskMeta, //!< start loading DB from server
        LoadingRequestGetMeta, //!< download DB and check it's hash
        UnpakingDB, //!< unpack DB from zip
        DeleteDownloadedPacksIfNotMatchHash, //!< go throw all local packs and unmount it if hash not match then delete
        LoadingPacksDataFromLocalMeta, //!< open local DB and build pack index for all packs
        WaitScanThreadToFinish, //!< wait till finish scaning of downloaded .dvpl files
        MoveDeleyedRequestsToQueue, //!< mount all local packs downloaded and not mounted later
        Ready, //!< starting from this state client can call any method, second initialize will work too
        Offline, //!< server not accessible, retry initialization after Hints::retryConnectMilliseconds

        State_COUNT
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
        MountingDownloadedPackFailed,

        Error_COUNT
    };

    static const String& ToString(InitError state);
#ifdef __DAVAENGINE_COREV2__
    explicit DLCManagerImpl(Engine* engine_);
    Engine& engine;
#else
    DLCManagerImpl() = default; // TODO remove it later (fix for client UnitTests)
#endif
    ~DLCManagerImpl();

    void Initialize(const FilePath& dirToDownloadPacks_,
                    const String& urlToServerSuperpack_,
                    const Hints& hints_) override;

    void RetryInit();

    bool IsInitialized() const override;

    InitState GetInitState() const;

    InitError GetInitError() const;

    const String& GetLastErrorMessage() const;

    bool IsRequestingEnabled() const override;

    void SetRequestingEnabled(bool value) override;

    void Update(float frameDelta);

    const IRequest* RequestPack(const String& requestedPackName) override;

    PackRequest* FindRequest(const String& requestedPackName) const;

    void SetRequestOrder(const IRequest* request, uint32 orderIndex) override;

    void RemovePack(const String& packName) override;

    const FilePath& GetLocalPacksDirectory() const;

    const String& GetSuperPackUrl() const;

    uint32 GetServerFooterCrc32() const
    {
        return initFooterOnServer.infoCrc32;
    }

    String GetRelativeFilePath(uint32 fileIndex);

    const Hints& GetHints() const
    {
        return hints;
    }

    const PackMetaData& GetMeta() const
    {
        return *meta;
    }

    const PackFormat::PackFile& GetPack() const
    {
        return usedPackFile;
    }

    // use only after initialization
    bool IsFileReady(uint32 fileIndex) const
    {
        return fileIndex < scanFileReady.size() && scanFileReady.test(fileIndex);
    }

private:
    // initialization state functions
    void AskFooter();
    void GetFooter();
    void AskFileTable();
    void GetFileTable();
    void CompareLocalMetaWitnRemoteHash();
    void AskServerMeta();
    void GetServerMeta();
    void ParseMeta();
    void StoreAllMountedPackNames();
    void DeleteOldPacks();
    void LoadPacksDataFromMeta();
    void WaitScanThreadToFinish();
    void StartDeleyedRequests();
    // helper functions
    void DeleteLocalMetaFiles();
    void ContinueInitialization(float frameDelta);
    PackRequest* AddDeleyedRequest(const String& requestedPackName);
    PackRequest* CreateNewRequest(const String& requestedPackName);

    enum class ScanState : uint32
    {
        Wait,
        Starting,
        Done
    };
    // info to scan local pack files
    struct LocalFileInfo
    {
        String relativeName;
        uint32 compressedSize = 0;
        uint32 crc32Hash = 0;
    };
    // fill during scan local pack files, emtpy after finish scan
    Vector<LocalFileInfo> localFiles;
    // every bit mean file exist and size match with meta
    std::bitset<32000> scanFileReady;
    Thread* scanThread = nullptr;
    ScanState scanState{ ScanState::Wait };
    Semaphore metaDataLoadedSem;

    void StartScanDownloadedFiles();
    void ThreadScanFunc();
    void ScanFiles(const FilePath& dir, Vector<LocalFileInfo>& files);
    void RecursiveScan(const FilePath& baseDir, const FilePath& dir, Vector<LocalFileInfo>& files);

    mutable Mutex protectDM;

    FilePath localCacheMeta;
    FilePath localCacheFileTable;
    FilePath dirToDownloadedPacks;
    String urlToSuperPack;
    bool isProcessingEnabled = false;
    bool isNetworkReadyLastState = false;
    std::unique_ptr<RequestManager> requestManager;
    std::unique_ptr<PackMetaData> meta;

    Vector<PackRequest*> requests; // not forget to delete in destructor
    Vector<PackRequest*> delayedRequests; // move to requests after initialization finished

    String initErrorMsg;
    InitState initState = InitState::Starting;
    InitError initError = InitError::AllGood;
    PackFormat::PackFile::FooterBlock initFooterOnServer; // tmp supperpack info for every new pack request or during initialization
    PackFormat::PackFile usedPackFile; // current superpack info
    Vector<uint8> buffer; // tmp buff
    String uncompressedFileNames;
    UnorderedMap<String, const PackFormat::FileTableEntry*> mapFileData;
    Vector<uint32> startFileNameIndexesInUncompressedNames;
    // first - relative file name in archive, second - file properties
    // DO I NEED IT? UnorderedMap<String, const PackFormat::FileTableEntry*> initFileData;
    // DO I NEED IN? Vector<ResourceArchive::FileInfo> initfilesInfo;
    uint32 downloadTaskId = 0;
    uint64 fullSizeServerData = 0;

    Hints hints{};

    float32 timeWaitingNextInitializationAttempt = 0;
    uint32 retryCount = 0; // count every initialization error during session
};

} // end namespace DAVA
