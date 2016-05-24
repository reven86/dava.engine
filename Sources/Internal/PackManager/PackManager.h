#pragma once

#include "Functional/Signal.h"
#include "DLC/Downloader/DownloaderCommon.h"

namespace DAVA
{
class PackManagerImpl;

class PackManager final
{
public:
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

        DownloadError downloadError = DLE_NO_ERROR;
        Status state = Status::NotRequested;

        bool isGPU = false;
    };

    PackManager();
    ~PackManager();

    // 1. open local database and read all packs info
    // 2. list all packs on filesystem
    // 3. mount all packs which found on filesystem and in database
    // throw exception if can't initialize with deteils
    void Initialize(const FilePath& dbFile,
                    const FilePath& downloadPacksDir,
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

    void Delete(const String& packName);

    // signal user about every pack state change
    Signal<const Pack&, Pack::Change> onPackStateChanged;

    const FilePath& GetLocalPacksDirectory() const;
    const String& GetRemotePacksUrl() const;

private:
    std::unique_ptr<PackManagerImpl> impl;
};

} // end namespace DAVA
