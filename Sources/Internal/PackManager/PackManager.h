#pragma once

#include "Functional/Signal.h"
#include "DLC/Downloader/DownloaderCommon.h"
#include "Functional/Function.h"

namespace DAVA
{
/**
 Interface for requesting packs from server.

 tipical workflow:
 1. connect to state change signal and to request update signal
 2. call Initialize to connect to server, wait for state become `Pack::Status::Ready`
 3. request pack from server or mount local automaticaly on request
 4. findout which pack has file by filePath

 example:
 ```
 IPackManager& pm = *engine.GetContext()->packManager;
 // if init failed we will know about it
 pm.initStateChanged.Connect(this, &PackManagerTest::OnInitChange);

 String gpuArchitecture = "mali";
 FilePath folderWithDownloadedPacks = "~doc:/FolderForPacks/";
 String urlToServerSuperpack = "http://server.net/superpack.dvpk";
 IPackManager::Hints hints;
 hints.retryConnectMilliseconds = 1000; // retry connect every second
 hints.dbInMemory = true; // load DB in memory for perfomance

 pm.Initialize(gpuArchitecture, folderWithDownloadedPacks, dbFile, urlToServerSuperpack, hints);

 // now we can connect to request signal, and start requesting packs

 ```
*/
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
        Ready, //!< starting from this state client can call any method, second initialize will work too
        Offline //!< server not acceseble, retry initialization after Hints::retryConnectMilliseconds
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

    /**
     Pack with all details.
     Read only data, you don't need to create or destroy Pack's.
     During initialization all packs are created simultaneously.
    */
    struct Pack
    {
        enum class Status : uint32
        {
            NotRequested, //!< initial state
            Requested, //!< client made request and pack in queue
            Downloading, //!< loading pack data from server superpack
            Mounted, //!< if all good after loading and checking hash
            ErrorLoading, //!< downloadError - value returned from DLC DownloadManager
            OtherError //!< mount failed, check hash failed, file IO failed see otherErrorMsg
        };

        Vector<String> dependency; //!< names of dependency packs or empty

        String name; //!< unique pack name
        String otherErrorMsg; //!< extended error message

        float32 downloadProgress = 0.f; //!< [0.0f to 1.0f]
        float32 priority = 0.f; //!< [0.0f to 1.0f]

        uint32 hashFromDB = 0; //!< crc32 hash from DB file

        uint64 downloadedSize = 0; //!< current downloaded size, used only during loading
        uint64 totalSize = 0; //!< current total size on file system
        uint64 totalSizeFromDB = 0; //!< size from DB

        DownloadError downloadError = DLE_NO_ERROR; //!< download manager error
        Status state = Status::NotRequested; //!< current pack state

        bool isGPU = false; //!< value from DB
    };

    /**
     Proxy interface to easily check pack request progress
     to use it interface, for download progress you need to
     connect to `requestProgressChanged` signal and then
     call `RequestPack`. Also you can find reques by pack name
     with `FindRequest`.
    */
    class IRequest
    {
    public:
        virtual ~IRequest();

        /** return requested pack name */
        virtual const Pack& GetRootPack() const = 0;
        /** recalculate fullsize with all dependencies */
        virtual uint64 GetFullSizeWithDependencies() const = 0;
        /** recalculate current downloaded size */
        virtual uint64 GetDownloadedSize() const = 0;
        /** return true in case error loading */
        virtual bool IsError() const = 0;
        /** return pack during loading which error happend */
        virtual const Pack& GetErrorPack() const = 0;
        /** detailed error message */
        virtual const String& GetErrorMessage() const = 0;
    };

    /** you have to sibscribe to this signal before call `Initialize` */
    Signal<IPackManager&> initStateChanged;
    /** signal user about every pack state change */
    Signal<const Pack&> packStateChanged;
    /** you can track every pack download progress with this signal */
    Signal<const Pack&> packDownloadChanged;
    /** signal per user request with complete size of all depended packs */
    Signal<const IRequest&> requestProgressChanged;

    struct Hints
    {
        uint32 retryConnectMilliseconds = 5000; //!< try to recconect to server if Offline state
    };

    /**
     Start complex initialization process. You can call it again if need.

     You also should subscribe to all signals especially state changes
     before you call Initialize.
     At least subscribe to `initStateChanged` signal
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

    virtual bool IsRequestingEnabled() const = 0;
    /** enable user request processing */
    virtual void EnableRequesting() = 0;
    /** disable user request processing */
    virtual void DisableRequesting() = 0;

    /** return unique pack name or empty string on error */
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
    /**
     Unmount pack and then delete it, throw exception on error,
     then collect from DB all dependent packs and set it's state to
     `Pack::State::NotRequested`. Normaly you don't need to delete packs
     manually
     */
    virtual void DeletePack(const String& packName) = 0;

    virtual const FilePath& GetLocalPacksDirectory() const = 0;
    virtual const String& GetSuperPackUrl() const = 0;
};

} // end namespace DAVA
