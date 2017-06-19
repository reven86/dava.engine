#pragma once

#include "FileSystem/FilePath.h"
#include "Functional/Signal.h"

namespace DAVA
{
/**
 Interface for requesting packs from server.

 Typical work flow:
 1. connect to state change signal and to request update signal
 2. call Initialize to connect to server, wait for state become `Pack::Status::Ready`
 3. request pack from server or mount local automatically on request

 example:
 ```
 DLCManager& pm = *engine.GetContext()->dlcManager;
 // if initialize failed we will know about it
 pm.networkReady.Connect(this, &DLCManagerTest::OnNetworkReady);

 FilePath folderWithDownloadedPacks = "~doc:/FolderForPacks/";
 String urlToServerSuperpack = "http://server.net/superpack.3.7.0.mali.dvpk";
 DLCManager::Hints hints;
 hints.logFilePath = "~doc:/UberGame/dlc_manager.log";
 hints.retryConnectMilliseconds = 1000; // retry connect every second
 hints.maxFilesToDownload = 22456; // help download manager to reserve memory better

 pm.Initialize(folderWithDownloadedPacks, urlToServerSuperpack, hints);

 // now we can connect to request signal, and start requesting packs

 ```
*/

class DLCManager
{
public:
    virtual ~DLCManager();

    /**
     Proxy interface to easily check pack request progress
     to use it interface, for download progress you need to
     connect to `requestUpdated` signal and then
     call `RequestPack`.
    */
    struct IRequest
    {
        virtual ~IRequest();

        /** return requested pack name */
        virtual const String& GetRequestedPackName() const = 0;
        /** return size of files within this request without dependencies */
        virtual uint64 GetSize() const = 0;
        /** recalculate current downloaded size without dependencies */
        virtual uint64 GetDownloadedSize() const = 0;
        /** return true when all files loaded and ready */
        virtual bool IsDownloaded() const = 0;
    };

    /**
	   You have to subscribe to this signal before calling `Initialize`, it helps
	   to know whether connection to server works or connection lost.
	   Note: if download attempt is failed, DLC Manager will retry every `Hints::retryConnectMilliseconds` ms.
	   */
    Signal<bool> networkReady;
    /**
	    Tells that dlcmanager is initialized.
	    First parameter is a number of already downloaded files.
	    Second parameter is total number of files in server superpack.
	    After this signal you can use ```bool IsPackDownloaded(const String& packName);```
		*/
    Signal<size_t, size_t> initializeFinished;
    /** signal per user request */
    Signal<const IRequest&> requestUpdated;
    /** signal just before start loading request */
    Signal<const IRequest&> requestStartLoading;
    /**
	    Tells that some file error occurred during downloading process.
	    First parameter is a full path to the file which couldn't be created or written,
		second parameter is an error code
		(example: ENOSPC - No space left on device (POSIX.1).)
		DLCManager requesting disabled before signal.
		If you receive this signal first check available space on device.
		*/
    Signal<const String&, int32> fileErrorOccured;

    /**
	    User fills hints to internal implementation.
		Used for initialization.
	*/
    struct Hints
    {
        String logFilePath = "~doc:/dlc_manager.log"; //!< path for separate log file
        uint32 retryConnectMilliseconds = 5000; //!< try to reconnect to server if `Offline` state default every 5 seconds
        uint32 maxFilesToDownload = 0; //!< user should fill this value default value average files count in Data
        uint32 timeoutForDownload = 30; //!< this value passed to DownloadManager
        uint32 retriesCountForDownload = 3; //!< this value passed to DownloadManager
        uint32 downloaderMaxHandles = 8; //!< play with any values you like from 1 to max open file per process
        uint32 downloaderChankBufSize = 512 * 1024; //!< 512Kb RAM buffer for one handle, you can set any value in bytes
    };

    /** Start complex initialization process. You can call it again if need.

		If you want to know what is going on during initialization you
		have to subscribe to any signals you want before call `Initialize(..)`
    */
    virtual void Initialize(const FilePath& dirToDownloadPacks,
                            const String& urlToServerSuperpack,
                            const Hints& hints) = 0;
    /**
	 Stop all operations and free all resources. Useful during unit tests.
	*/
    virtual void Deinitialize() = 0;

    virtual bool IsInitialized() const = 0;

    virtual bool IsRequestingEnabled() const = 0;

    /** Return true if pack is already downloaded. */
    virtual bool IsPackDownloaded(const String& packName) = 0;

    virtual void SetRequestingEnabled(bool value) = 0;

    /** return nullptr if can't find pack */
    virtual const IRequest* RequestPack(const String& packName) = 0;

    virtual bool IsPackInQueue(const String& packName) = 0;

    /** Update request queue to first download dependency of selected request
        and then request itself */
    virtual void SetRequestPriority(const IRequest* request) = 0;

    /** Remove request with downloaded content with dependent packs */
    virtual void RemovePack(const String& packName) = 0;

    struct Progress
    {
        uint64 total = 0; //!< in bytes
        uint64 alreadyDownloaded = 0; //!< in bytes
        uint64 inQueue = 0; //!< in bytes
        bool isRequestingEnabled = false; //!< current state of requesting
    };

    /** Calculate statistic about downloading progress */
    virtual Progress GetProgress() const = 0;
};

} // end namespace DAVA
