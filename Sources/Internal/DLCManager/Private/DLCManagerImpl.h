#pragma once

#include <fstream>

#include "DLCManager/DLCManager.h"
#include "DLCManager/DLCDownloader.h"
#include "DLCManager/Private/RequestManager.h"
#include "FileSystem/Private/PackFormatSpec.h"
#include "FileSystem/Private/PackMetaData.h"
#include "Concurrency/Semaphore.h"
#include "Concurrency/Thread.h"

#ifdef __DAVAENGINE_COREV2__
#include "Engine/Engine.h"
#endif

namespace DAVA
{
class MemoryBufferWriter final : public DLCDownloader::IWriter
{
public:
    MemoryBufferWriter(void* buff, size_t size)
    {
        DVASSERT(buff != nullptr);
        DVASSERT(size > 0);

        start = static_cast<char*>(buff);
        current = start;
        end = start + size;
    }

    uint64 Save(const void* ptr, uint64 size) override
    {
        uint64 space = SpaceLeft();

        if (size > space)
        {
            memcpy(current, ptr, static_cast<size_t>(space));
            current += space;
            return space;
        }

        memcpy(current, ptr, static_cast<size_t>(size));
        current += size;
        return size;
    }

    uint64 GetSeekPos() override
    {
        return current - start;
    }

    bool Truncate() override
    {
        current = start;
        return true;
    }

    void Close() override
    {
        start = nullptr;
        current = nullptr;
        end = nullptr;
    }

    bool IsClosed() const override
    {
        return current == nullptr;
    }

    uint64 SpaceLeft() const
    {
        return end - current;
    }

private:
    char* start = nullptr;
    char* current = nullptr;
    char* end = nullptr;
};

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
        WaitScanThreadToFinish, //!< wait till finish scanning of downloaded .dvpl files
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

    void Deinitialize() override;

    void RetryInit();

    bool IsInitialized() const override;

    InitState GetInitState() const;

    InitError GetInitError() const;

    const String& GetLastErrorMessage() const;

    bool IsRequestingEnabled() const override;

    void SetRequestingEnabled(bool value) override;

    void Update(float frameDelta);

    bool IsPackDownloaded(const String& packName) override;

    const IRequest* RequestPack(const String& requestedPackName) override;

    PackRequest* FindRequest(const String& requestedPackName) const;

    bool IsPackInQueue(const String& packName) override;

    void SetRequestPriority(const IRequest* request) override;

    void RemovePack(const String& packName) override;

    Progress GetProgress() const override;

    const FilePath& GetLocalPacksDirectory() const;

    const String& GetSuperPackUrl() const;

    uint32 GetServerFooterCrc32() const;

    String GetRelativeFilePath(uint32 fileIndex);

    const Hints& GetHints() const;

    const PackMetaData& GetMeta() const;

    const PackFormat::PackFile& GetPack() const;

    // use only after initialization
    bool IsFileReady(size_t fileIndex) const;

    void SetFileIsReady(size_t fileIndex);

    bool IsInQueue(const PackRequest* request) const;

    bool IsTop(const PackRequest* request) const;

    std::ostream& GetLog() const;

    DLCDownloader* GetDownloader() const
    {
        return downloader.get();
    }

private:
    // initialization state functions
    void AskFooter();
    void PrintErrMsgAndGpuToLog();
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
    void StartDelayedRequests();
    // helper functions
    void DeleteLocalMetaFiles();
    void ContinueInitialization(float frameDelta);
    void ReadContentAndExtractFileNames();

    void SwapRequestAndUpdatePointers(PackRequest* request, PackRequest* newRequest);
    void SwapPointers(PackRequest* userRequestObject, PackRequest* newRequestObject);
    PackRequest* AddDelayedRequest(const String& requestedPackName);
    PackRequest* CreateNewRequest(const String& requestedPackName);

    void ClearResouces();

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
        uint32 compressedSize = std::numeric_limits<uint32>::max(); // file size can be 0 so use max value default
        uint32 crc32Hash = std::numeric_limits<uint32>::max();
    };
    // fill during scan local pack files, empty after finish scan
    Vector<LocalFileInfo> localFiles;
    // every bit mean file exist and size match with meta
    Vector<bool> scanFileReady;
    Thread* scanThread = nullptr;
    ScanState scanState{ ScanState::Wait };
    Semaphore metaDataLoadedSem;

    void StartScanDownloadedFiles();
    void ThreadScanFunc();
    void ScanFiles(const FilePath& dir, Vector<LocalFileInfo>& files);
    void RecursiveScan(const FilePath& baseDir, const FilePath& dir, Vector<LocalFileInfo>& files);

    mutable std::ofstream log;

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
    std::unique_ptr<MemoryBufferWriter> memBufWriter;
    PackFormat::PackFile::FooterBlock initFooterOnServer; // temp superpack info for every new pack request or during initialization
    PackFormat::PackFile usedPackFile; // current superpack info
    Vector<uint8> buffer; // temp buff
    String uncompressedFileNames;
    UnorderedMap<String, const PackFormat::FileTableEntry*> mapFileData;
    Vector<uint32> startFileNameIndexesInUncompressedNames;
    DLCDownloader::Task* downloadTaskId = nullptr;
    uint64 fullSizeServerData = 0;
    mutable Progress lastProgress;

    Hints hints{};

    float32 timeWaitingNextInitializationAttempt = 0;
    uint32 retryCount = 0; // count every initialization error during session

    std::unique_ptr<DLCDownloader> downloader;
};

inline uint32 DLCManagerImpl::GetServerFooterCrc32() const
{
    return initFooterOnServer.infoCrc32;
}

inline const DLCManagerImpl::Hints& DLCManagerImpl::GetHints() const
{
    return hints;
}

inline const PackMetaData& DLCManagerImpl::GetMeta() const
{
    return *meta;
}

inline const PackFormat::PackFile& DLCManagerImpl::GetPack() const
{
    return usedPackFile;
}

inline bool DLCManagerImpl::IsFileReady(size_t fileIndex) const
{
    DVASSERT(fileIndex < scanFileReady.size());
    return scanFileReady[fileIndex];
}

inline void DLCManagerImpl::SetFileIsReady(size_t fileIndex)
{
    scanFileReady.at(fileIndex) = true;
}

inline bool DLCManagerImpl::IsInQueue(const PackRequest* request) const
{
    DVASSERT(request != nullptr);
    return requestManager->IsInQueue(request->GetRequestedPackName());
}

inline bool DLCManagerImpl::IsTop(const PackRequest* request) const
{
    DVASSERT(request != nullptr);
    if (!requestManager->Empty())
    {
        return request == requestManager->Top();
    }
    return false;
}

} // end namespace DAVA
