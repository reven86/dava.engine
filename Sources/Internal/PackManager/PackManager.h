#pragma once

#include "Functional/Signal.h"
#include "DLC/Downloader/DownloaderCommon.h"

namespace DAVA
{
class PackManagerImpl;

class PackManager final
{
public:
    enum class InitializeState : uint32
    {
        Starting,
        LoadingDB,
        UnpakingkDB,
        LoadingPacksData,
        MountingLocalPacks,
        MountingDownloadedPacks,
        Ready,
        Error
    };

    struct Pack
    {
        enum class Status : uint32
        {
            NotRequested = 0,
            Requested = 1,
            Downloading = 2,
            Mounted = 3,
            ErrorLoading = 4, // downloadError - value returned from DLC DownloadManager
            OtherError = 5 // mount failed, check hash failed, file IO failed see otherErrorMsg
        };

        enum class Change : uint32
        {
            State = 1,
            DownloadProgress = 2,
            Priority = 4,
        };

        Vector<String> dependency; // names of dependency packs or empty

        String name; // unique pack name
        String remoteUrl; // url used for download archive or empty
        String otherErrorMsg;

        float32 downloadProgress = 0.f; // 0.0f to 1.0f
        float32 priority = 0.f; // 0.0f to 1.0f

        uint32 hashFromMeta = 0; // example: tanks.pak -> tanks.pak.hash
        uint32 hashFromDB = 0;

        uint64 downloadedSize = 0;
        uint64 totalSize = 0;
        uint64 totalSizeFromDB = 0;

        DownloadError downloadError = DLE_NO_ERROR;
        Status state = Status::NotRequested;

        bool isGPU = false;
    };

    // proxy interface to easily check pack request progress
    class IRequest
    {
    public:
        virtual ~IRequest();

        virtual const String& GetPackName() const = 0;

        virtual uint64 GetFullSizeWithDependencies() const = 0;

        virtual uint64 GetDownloadedSize() const = 0;

        virtual bool IsError() const = 0;

        virtual const String& GetErrorMessage() const = 0;
    };

    PackManager();
    ~PackManager();

    // 0. connect to remote server and download DB with info about all packs and files
    // 1. unpack DB to local write dir
    // 2. open local database and read all packs info
    // 3. list all packs on filesystem
    // 4. mount all packs which found on filesystem and in database
    // throw exception if can't initialize with deteils
    void Initialize(const String& dbFileName,
                    const FilePath& downloadPacksDir,
                    const FilePath& readOnlyPacksDir, // can be empty
                    const String& packsUrlCommon,
                    const String& packsUrlGpu);

    bool IsProcessingEnabled() const;
    // enable user request processing
    void EnableProcessing();
    // disalbe user request processing
    void DisableProcessing();

    // internal method called per frame in framework (can thow exception)
    void Update();

    // return unique pack name or empty string
    const String& FindPack(const FilePath& relativePathInArchive) const;

    // thow exception if can't find pack
    const Pack& RequestPack(const String& packName, float priority = 0.0f);

    // all packs state, valid till next call Update()
    const Vector<Pack>& GetPacks() const;

    void DeletePack(const String& packName);

    // user have to wait till InitializationState become Ready
    // second argument - status text usfull for loging
    Signal<InitializeState, const String&> onInitializationStatusChanged;
    // signal user about every pack state change
    Signal<const Pack&, Pack::Change> onPackStateChanged;
    // signal per user request with complete size of all depended packs
    Signal<const IRequest&> onRequestProgressChanged;

    const FilePath& GetLocalPacksDirectory() const;
    const String& GetRemotePacksUrl(bool isGPU) const;

private:
    std::unique_ptr<PackManagerImpl> impl;
    InitializeState state = InitializeState::Starting;
};

} // end namespace DAVA
