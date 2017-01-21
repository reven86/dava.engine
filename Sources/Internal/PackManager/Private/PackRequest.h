#pragma once

#include "Base/BaseTypes.h"
#include "PackManager/PackManager.h"

namespace DAVA
{
class DLCManagerImpl;

/**
	Downdload several files
*/

class PackRequest : public IDLCManager::IRequest
{
public:
    PackRequest(DLCManagerImpl& packManager_, const String& packName, Vector<uint32> fileIndexes_);
    PackRequest(DLCManagerImpl& packManager_, const String& requestedPackName);

    ~PackRequest() override;

    void Start();
    void Update();
    void Stop();

    const String& GetRequestedPackName() const override;
    /** recalculate full size with all dependencies */
    Vector<const IRequest*> GetDependencies() const override;
    /** return size of files within this request without dependencies */
    uint64 GetSize() const override;
    /** recalculate current downloaded size without dependencies */
    uint64 GetDownloadedSize() const override;
    /** return true when all files loaded and ready */
    bool IsDownloaded() const override;

private:
    struct FileRequest
    {
        FileRequest();
        ~FileRequest();

        enum Status : uint32
        {
            Wait = 0,
            CheckLocalFile,
            LoadingPackFile, // download manager thread, wait on main thread
            CheckHash, // on main thread (in future move to job manager)
            Ready, // on main thread

            Error
        };

        void Initialize(const uint32 fileIndex,
                        const String& fileName,
                        const uint32 hash,
                        const uint64 startLoadingPos,
                        const uint64 fileSize);
        void Update();

        bool IsDownloaded();

        FilePath localFile;
        String fileName;
        String errorMsg;
        uint32 hashFromMeta = 0;
        uint64 startLoadingPos = 0;
        uint64 sizeOfFile = 0;
        uint32 taskId = 0;
        Status status = Wait;
    };

    DLCManagerImpl& packManagerImpl;

    FileRequest fileRequest;
    Vector<uint32> fileIndexes;
    String requestedPackName;

    uint32 downloadTaskId = 0;
    uint32 currentFileIndex = 0;

    uint64 totalAllPacksSize = 0;
    uint64 downloadedSize = 0;
};

} // end namespace DAVA
