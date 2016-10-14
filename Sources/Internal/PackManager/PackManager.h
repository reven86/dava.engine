#pragma once

#include "Functional/Signal.h"
#include "DLC/Downloader/DownloaderCommon.h"
#include "Functional/Function.h"

namespace DAVA
{
class IPackManager
{
public:
    virtual ~IPackManager();
    enum class InitState : uint32
    {
        Starting, //!< before any initialization code state
        LoadingRequestAskFooter, //!< connect to server superpack.dvpk for footer block
        LoadingRequestGetFooter, //!< download footer and parse it, findout filetable block size and position
        LoadingRequestAskFileTable, //!< start loading filetable block from superpack.dvpk
        LoadingRequestGetFileTable, //!< download filetable and fill info about every file on server superpack.dvpk
        CalculateLocalDBHashAndCompare, //!< check if existing local DB hash match with remote DB on server, go to LoadingPacksDataFromLocalDB if match
        LoadingRequestAskDB, //!< start loading DB from server
        LoadingRequestGetDB, //!< download DB and check it's hash
        UnpakingDB, //!< unpack DB from zip
        DeleteDownloadedPacksIfNotMatchHash, //!< go throw all local packs and unmount it if hash not match then delete
        LoadingPacksDataFromLocalDB, //!< open local DB and build pack index for all packs
        MountingDownloadedPacks, //!< mount all local packs downloaded and not mounted later
        Ready //!< starting from this state client can call any method, second initialize will work too
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

    struct Pack
    {
        enum class Status : uint32
        {
            NotRequested,
            Requested,
            Downloading,
            Mounted,
            ErrorLoading, //!< downloadError - value returned from DLC DownloadManager
            OtherError //!< mount failed, check hash failed, file IO failed see otherErrorMsg
        };

        Vector<String> dependency; /** names of dependency packs or empty */

        String name; /** unique pack name */
        String otherErrorMsg;

        float32 downloadProgress = 0.f; /** 0.0f to 1.0f */
        float32 priority = 0.f; /** 0.0f to 1.0f */

        uint32 hashFromDB = 0;

        uint64 downloadedSize = 0;
        uint64 totalSize = 0;
        uint64 totalSizeFromDB = 0;

        DownloadError downloadError = DLE_NO_ERROR;
        Status state = Status::NotRequested;

        bool isGPU = false; /** depends on architecture */
    };

    /** proxy interface to easily check pack request progress */
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

    /** User have to wait till InitState become Ready */
    Signal<IPackManager&> asyncConnectStateChanged;
    /** signal user about every pack state change */
    Signal<const Pack&> packStateChanged;
    Signal<const Pack&> packDownloadChanged;
    /** signal per user request with complete size of all depended packs */
    Signal<const IRequest&> requestProgressChanged;

    struct Hints
    {
        bool dbInMemory = true; // on PC, Mac, Android preffer true RAM
    };

    /**
     You can call after in GameCore::OnAppStarted (throw exception on error)
     complex async connect to server

     You also should subscribe to all signals espesialy error states
     before you call Initialize.
     At least subscribe to `asyncConnectStateChanged` signal
    */
    virtual void Initialize(const String& architecture,
                            const FilePath& dirToDownloadPacks,
                            const FilePath& pathToBasePacksDB,
                            const String& urlToServerSuperpack,
                            const Hints& hints) = 0;

    virtual bool IsInitialized() const = 0;

    virtual InitState GetInitState() const = 0;

    virtual InitError GetInitError() const = 0;

    virtual const String& GetInitErrorMessage() const = 0;

    virtual void RetryInit() = 0;

    virtual bool IsPausedInit() const = 0;

    /** if you need ask USER what to do, you can "Pause" initialization and wait some frames and later call "RetryInit" */
    virtual void PauseInit() = 0;

    virtual bool IsRequestingEnabled() const = 0;
    /** enable user request processing */
    virtual void EnableRequesting() = 0;
    /** disable user request processing */
    virtual void DisableRequesting() = 0;

    /** return unique pack name or empty string */
    virtual const String& FindPackName(const FilePath& relativePathInArchive) const = 0;

    /** thow exception if can't find pack */
    virtual const Pack& FindPack(const String& packName) const = 0;

    /** thow exception if can't find pack */
    virtual const Pack& RequestPack(const String& packName) = 0;

    /**
     List all files from DB for every pack, and call callback function
     for every found path with
     two params first - relative file path, second pack name
    */
    virtual void ListFilesInPacks(const FilePath& relativePathDir, const Function<void(const FilePath&, const String&)>& fn) = 0;

    /**
     return request contains pack or nullptr
     requestedPackName - previous requested pack currently in downloading
     or present in wait queue
    */
    virtual const IRequest* FindRequest(const String& requestedPackName) const = 0;

    /** order - [0..1] - 0 - first, 1 - last */
    virtual void SetRequestOrder(const String& packName, float order) = 0;

    /** all packs state, valid till next call frame update */
    virtual const Vector<Pack>& GetPacks() const = 0;

    virtual void DeletePack(const String& packName) = 0;

    virtual const FilePath& GetLocalPacksDirectory() const = 0;
    virtual const String& GetSuperPackUrl() const = 0;
};

} // end namespace DAVA
