#pragma once

#include "Functional/Signal.h"
#include "DLC/Downloader/DownloaderCommon.h"

namespace DAVA
{
class PackManagerImpl;

class PackManager final
{
public:
    enum class InitState : uint32
    {
        FirstInit,
        Starting, // if not exist local DB in ~doc copy it from local resources (on first start)

        MountingReadOnlyPacks, // mount all local readonly packs (mount all founded in pointed directory)
        // now you can load files from base common packs
        CommonReadOnlyPacksReady,
        // now you can load files from base gpu packs
        GpuReadOnlyPacksReady,

        LoadingRequestAskFooter, // if no connection goto LoadingPacksDataFromDB try using only local packs
        LoadingRequestGetFooter,
        LoadingRequestAskFileTable,
        LoadingRequestGetFileTable,
        CalculateLocalDBHashAndCompare, // go to MountingDownloadedPacks if match
        LoadingRequestAskDB, // skip if hash match
        LoadingRequestGetDB, // skip if hash match
        UnpakingDB, // skip if hash match
        DeleteDownloadedPacksIfNotMatchHash, // skip if hash match
        LoadingPacksDataFromLocalDB,
        MountingDownloadedPacks,
        Ready
    };

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

    struct Pack
    {
        enum class Status : uint32
        {
            NotRequested,
            Requested,
            Downloading,
            Mounted,
            ErrorLoading, // downloadError - value returned from DLC DownloadManager
            OtherError // mount failed, check hash failed, file IO failed see otherErrorMsg
        };

        Vector<String> dependency; // names of dependency packs or empty

        String name; // unique pack name
        String otherErrorMsg;

        float32 downloadProgress = 0.f; // 0.0f to 1.0f
        float32 priority = 0.f; // 0.0f to 1.0f

        uint32 hashFromDB = 0;

        uint64 downloadedSize = 0;
        uint64 totalSize = 0;
        uint64 totalSizeFromDB = 0;

        DownloadError downloadError = DLE_NO_ERROR;
        Status state = Status::NotRequested;

        bool isGPU = false; // depends on architecture
        bool isReadOnly = false; // find in build readonly dir assets
    };

    class ISync
    {
    public:
        virtual ~ISync();

        virtual InitState GetState() const = 0;
        virtual InitError GetError() const = 0;
        virtual const String& GetErrorMessage() const = 0;
        virtual bool CanRetry() const = 0;
        virtual void Retry() = 0;
        virtual bool IsPaused() const = 0;
        virtual void Pause() = 0; // if you need ask USER what to do, you can "Pause" initialization and wait some frames and later call "Retry"
    };

    // proxy interface to easily check pack request progress
    class IRequest
    {
    public:
        virtual ~IRequest();

        virtual const Pack& GetRootPack() const = 0;
        virtual uint64 GetFullSizeWithDependencies() const = 0;
        virtual uint64 GetDownloadedSize() const = 0;
        virtual bool IsError() const = 0;
        virtual const Pack& GetErrorPack() const = 0;
        virtual const String& GetErrorMessage() const = 0;
    };

    // user have to wait till InitializationState become Ready
    // second argument - status text usfull for loging
    Signal<ISync&> asyncConnectStateChanged;
    // signal user about every pack state change
    Signal<const Pack&> packStateChanged;
    Signal<const Pack&> packDownloadChanged;
    // signal per user request with complete size of all depended packs
    Signal<const IRequest&> requestProgressChanged;

    PackManager();
    ~PackManager();

    struct Hints
    {
        bool dbInMemory = true; // on PC, Mac, Android preffer true RAM
        bool copyBasePacksToDocs = true; // on Android true improve perfomance (need play with it with different pack size and compression and loading order)
        // on PC, Mac, iOS - better false
    };

    // you can call it first line in FrameworkDidLaunched (throw exception on error)
    void InitCommonPacks(const FilePath& readOnlyPacksDir,
                         const FilePath& downloadPacksDir,
                         const Hints& hints);

    // you can call after InitCommonPacks in GameCore::OnAppStarted (throw exception on error)
    void InitGpuPacks(const String& architecture, const String& dbFileName);

    // complex async connect to server
    void SyncWithServer(const String& urlToServerSuperpack);

    bool IsGpuPacksInitialized() const;

    ISync& GetISync();

    bool IsRequestingEnabled() const;
    // enable user request processing
    void EnableRequesting();
    // disable user request processing
    void DisableRequesting();

    // internal method called per frame in framework (can thow exception)
    void Update();

    // return unique pack name or empty string
    const String& FindPackName(const FilePath& relativePathInArchive) const;

    // fast find using index
    const Pack& FindPack(const String& packName) const;

    // thow exception if can't find pack
    const Pack& RequestPack(const String& packName);

    // return request contains pack or nullptr
    // requestedPackName - previous requested pack currently in downloading
    // or present in wait queue
    const IRequest* FindRequest(const String& requestedPackName) const;

    // order - [0..1] - 0 - first, 1 - last
    void ChangeDownloadOrder(const String& packName, float order);

    // all packs state, valid till next call Update()
    const Vector<Pack>& GetPacks() const;

    void DeletePack(const String& packName);

    const FilePath& GetLocalPacksDirectory() const;
    const String& GetSuperPackUrl() const;

private:
    std::unique_ptr<PackManagerImpl> impl;
};

} // end namespace DAVA
