#pragma once

#include "Functional/Signal.h"
#include "FileSystem/FilePath.h"

namespace DAVA
{
/**
 Interface for requesting packs from server.

 Topical work flow:
 1. connect to state change signal and to request update signal
 2. call Initialize to connect to server, wait for state become `Pack::Status::Ready`
 3. request pack from server or mount local automatically on request

 example:
 ```
 IDLCManager& pm = *engine.GetContext()->packManager;
 // if init failed we will know about it
 pm.networkReady.Connect(this, &PackManagerTest::OnNetworkReady);

 FilePath folderWithDownloadedPacks = "~doc:/FolderForPacks/";
 String urlToServerSuperpack = "http://server.net/superpack.3.7.0.mali.dvpk";
 IDLCManager::Hints hints;
 hints.retryConnectMilliseconds = 1000; // retry connect every second

 pm.Initialize(folderWithDownloadedPacks, urlToServerSuperpack, hints);

 // now we can connect to request signal, and start requesting packs

 ```
*/

class IDLCManager
{
public:
    virtual ~IDLCManager();

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
        /** recalculate full size with all dependencies */
        virtual Vector<String> GetDependencies() const = 0;
        /** return size of files within this request without dependencies */
        virtual uint64 GetSize() const = 0;
        /** recalculate current downloaded size without dependencies */
        virtual uint64 GetDownloadedSize() const = 0;
        /** return true when all files loaded and ready */
        virtual bool IsDownloaded() const = 0;
    };

    /** you have to subscribe to this signal before call `Initialize` */
    Signal<bool> networkReady;
    /** signal per user request with full size of all depended packs */
    Signal<const IRequest&> requestUpdated;

    struct Hints
    {
        uint32 retryConnectMilliseconds = 5000; //!< try to reconnect to server if `Offline` state
        uint32 checkLocalFileExistPerUpdate = 100; //!< how many file to check per Update call
    };

    /**
     Start complex initialization process. You can call it again if need.

     You also should subscribe to all signals especially state changes
     before you call Initialize.
     At least subscribe to `networkReady` signal
    */
    virtual void Initialize(const FilePath& dirToDownloadPacks,
                            const String& urlToServerSuperpack,
                            const Hints& hints) = 0;

    virtual bool IsInitialized() const = 0;

    virtual bool IsRequestingEnabled() const = 0;

    virtual void SetRequestingEnabled(bool value) = 0;

    /** return nullptr if can't find pack */
    virtual const IRequest* RequestPack(const String& packName) = 0;

    /** order - [0..N] - 0 - first, 1, 2, ... , N - last in queue */
    virtual void SetRequestOrder(const IRequest* request, uint32 orderIndex) = 0;
};

} // end namespace DAVA
