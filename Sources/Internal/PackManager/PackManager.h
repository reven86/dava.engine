#pragma once

#include "Functional/Signal.h"
#include "FileSystem/FilePath.h"
#include "Engine/EngineContext.h"

namespace DAVA
{
/**
 Interface for requesting packs from server.

 Topical work flow:
 1. connect to state change signal and to request update signal
 2. call Initialize to connect to server, wait for state become `Pack::Status::Ready`
 3. request pack from server or mount local automatically on request
 4. find out which pack has file by filePath

 example:
 ```
 DLCManager& pm = *engine.GetContext()->packManager;
 // if init failed we will know about it
 pm.networkReady.Connect(this, &PackManagerTest::OnInitChange);

 String gpuArchitecture = "mali";
 FilePath folderWithDownloadedPacks = "~doc:/FolderForPacks/";
 String urlToServerSuperpack = "http://server.net/superpack.dvpk";
 DLCManager::Hints hints;
 hints.retryConnectMilliseconds = 1000; // retry connect every second
 hints.dbInMemory = true; // load DB in memory for performance

 pm.Initialize(gpuArchitecture, folderWithDownloadedPacks, dbFile, urlToServerSuperpack, hints);

 // now we can connect to request signal, and start requesting packs

 ```
*/
class DCLManagerImpl;

class DLCManager final
{
public:
#ifdef __DAVAENGINE_COREV2__
    explicit DLCManager(const EngineContext* ctx);
#else
    DLCManager();
#endif
    ~DLCManager();

    /**
     Proxy interface to easily check pack request progress
     to use it interface, for download progress you need to
     connect to `requestProgressChanged` signal and then
     call `RequestPack`.
    */
    class IRequest
    {
    public:
        virtual ~IRequest();

        /** return requested pack name */
        virtual const String& GetRequestedPackName() const = 0;
        /** recalculate full size with all dependencies */
        virtual uint64 GetFullSizeWithDependencies() const = 0;
        /** recalculate current downloaded size */
        virtual uint64 GetDownloadedSize() const = 0;
        /** return true when all files loaded and ready */
        virtual bool IsDowndloaded() const = 0;
    };

    /** you have to subscribe to this signal before call `Initialize` */
    Signal<bool> networkReady;
    /** signal per user request with full size of all depended packs */
    Signal<const IRequest&> requestProgressChanged;

    struct Hints
    {
        uint32 retryConnectMilliseconds = 5000; //!< try to reconnect to server if `Offline` state
    };

    /**
     Start complex initialization process. You can call it again if need.

     You also should subscribe to all signals especially state changes
     before you call Initialize.
     At least subscribe to `networkReady` signal
    */
    void Initialize(const String& architecture,
                    const FilePath& dirToDownloadPacks,
                    const FilePath& pathToBasePacksDB,
                    const String& urlToServerSuperpack,
                    const Hints& hints);

    bool IsInitialized() const;

    bool IsRequestingEnabled() const;

    void SetRequestingEnabled(bool value);

    /** return nullptr if can't find pack */
    const IRequest* RequestPack(const String& packName);

    /** order - [0..N] - 0 - first, 1, 2, ... , N - last in queue */
    void SetRequestOrder(const IRequest* request, unsigned orderIndex);

private:
    std::unique_ptr<DCLManagerImpl> impl;
};

} // end namespace DAVA
