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
    PackRequest(DLCManagerImpl& packManager_, const String& requestedPackName);

    ~PackRequest() override;

    void Start();
    void Update();
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
        uint64 prevDownloadedSize = 0;
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
    void UpdateFileRequests();

    DLCManagerImpl& packManagerImpl;

    Vector<FileRequest> requests;
    Vector<uint32> fileIndexes;
    String requestedPackName;

    uint32 numOfDownloadedFile = 0;
    uint64 downloadedSize = 0;

    // if this fild is false, you can check fileIndexes
    // else fileIndexes maybe empty and wait initialization
    bool delayedRequest = true;
};

} // end namespace DAVA
