#pragma once

#include "Base/BaseTypes.h"
#include "DLCManager/DLCManager.h"
#include "Compression/Compressor.h"

namespace DAVA
{
class DLCManagerImpl;

/**
	Downdload several files
*/
class PackRequest : public DLCManager::IRequest
{
public:
    PackRequest(DLCManagerImpl& packManager_, const String& packName, Vector<uint32> fileIndexes_);
    void CancelCurrentsDownloads();
    PackRequest(DLCManagerImpl& packManager_, const String& requestedPackName);

    ~PackRequest() override;

    void Start();
    bool Update();
    void Stop();

    const String& GetRequestedPackName() const override;
    /** recalculate full size with all dependencies */
    Vector<String> GetDependencies() const override;
    /** return size of files within this request without dependencies */
    uint64 GetSize() const override;
    /** recalculate current downloaded size without dependencies */
    uint64 GetDownloadedSize() const override;
    /** return true when all files loaded and ready */
    bool IsDownloaded() const override;

    void SetFileIndexes(Vector<uint32> fileIndexes_);

    /** this request depends on other, so other should be downloaded first */
    bool IsSubRequest(const PackRequest* other) const;

    bool IsDelayed() const;

    PackRequest& operator=(PackRequest&& other);

private:
    void InitializeFileRequests();

    enum Status : uint32
    {
        Wait = 0,
        CheckLocalFile,
        LoadingPackFile, // download manager thread, wait on main thread
        CheckHash, // on main thread (in future move to job manager)
        Ready, // on main thread

        Error
    };

    struct FileRequest
    {
        FilePath localFile;
        String errorMsg;
        String url;
        uint32 fileIndex = 0;
        uint32 hashFromMeta = 0;
        uint64 startLoadingPos = 0;
        uint64 sizeOfCompressedFile = 0;
        uint64 sizeOfUncompressedFile = 0;
        uint64 downloadedFileSize = 0;
        uint32 taskId = 0;
        Compressor::Type compressionType = Compressor::Type::Lz4HC;
        Status status = Wait;
    };

    void InitializeFileRequest(const uint32 fileIndex,
                               const FilePath& file,
                               const uint32 hash,
                               const uint64 startLoadingPos,
                               const uint64 fileCompressedSize,
                               const uint64 fileUncompressedSize,
                               const String& url,
                               const Compressor::Type compressionType_,
                               FileRequest& fileRequest);

    static void DeleteJustDownloadedFileAndStartAgain(FileRequest& fileRequest);
    void DisableRequestingAndFireSignalNoSpaceLeft(PackRequest::FileRequest& fileRequest);
    bool UpdateFileRequests();

    DLCManagerImpl* packManagerImpl = nullptr;

    Vector<FileRequest> requests;
    Vector<uint32> fileIndexes;
    String requestedPackName;
    mutable Vector<String> dependencyCache;

    uint32 numOfDownloadedFile = 0;

    // if this fild is false, you can check fileIndexes
    // else fileIndexes maybe empty and wait initialization
    bool delayedRequest = true;
};

inline bool PackRequest::IsDelayed() const
{
    return delayedRequest;
}

} // end namespace DAVA
