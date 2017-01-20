#pragma once

#include "Base/BaseTypes.h"

namespace DAVA
{
class DLCManagerImpl;

class PackRequest : public IDLCManager::IRequest
{
public:
    PackRequest(DLCManagerImpl& packManager_, const String& packName, const Vector<uint32> fileIndexes);
    PackRequest(DLCManagerImpl& packManager_, const String& requestedPackName);

    void Start();
    void Update();
    void Stop();

    /** return requested pack name */
    const String& GetRequestedPackName() const override;
    /** recalculate full size with all dependencies */
    uint64 GetFullSizeWithDependencies() const override;
    /** recalculate current downloaded size */
    uint64 GetDownloadedSize() const override;
    /** return true when all files loaded and ready */
    bool IsDowndloaded() const override;

private:
    struct FileRequest
    {
        enum Status : uint32
        {
            Wait = 0,
            CheckLocalFile,
            LoadingPackFile, // download manager thread, wait on main thread
            CheckHash, // on main thread (in future move to job manager)
            Mounted, // on main thread

            Error
        };

        void Initialize(const String& fileName,
                        const uint32 hash, const uint64 startLoadingPos,
                        const uint64 fileSize);
        void Update();

        bool Error() const;
        bool IsDone() const;

    private:
        FilePath localFile;
        String fileName;
        String errorMsg;
        uint32 hashFromMeta = 0;
        uint64 startLoadingPos = 0;
        uint64 sizeOfFile = 0;
        uint32 taskId = 0;
        Status status = Wait;
    };

    void Restart();
    void StartLoadingPackFile();
    bool IsLoadingPackFileFinished();
    void StartCheckHash();
    bool IsCheckingHashFinished();
    void MountPack();
    void GoToNextSubRequest();

    DLCManagerImpl& packManagerImpl;

    String requestedPackName;
    FileRequest currentFileRequest;
    uint64 totalAllPacksSize = 0;
    uint64 downloadedSize = 0;

    uint32 downloadTaskId = 0;
};

} // end namespace DAVA
