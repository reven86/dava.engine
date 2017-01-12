#pragma once

#include "Base/BaseTypes.h"
#include "PackManager/PackManager.h"
#include "FileSystem/Private/PackFormatSpec.h"

namespace DAVA
{
class DCLManagerImpl;

class PackRequest : public IDLCManager::IRequest
{
public:
    PackRequest(DCLManagerImpl& packManager_, const String& packName);

    void Start();
    void Update();
    void Stop();

    struct FileRequest
    {
        enum Status : uint32
        {
            Wait = 0,
            AskFooter,
            GetFooter,
            LoadingPackFile, // download manager thread, wait on main thread
            CheckHash, // on main thread (in future move to job manager)
            Mounted, // on main thread

            Error
        };

        String fileName;
        String errorMsg;
        uint32 taskId = 0;
        Status status = Wait;
    };

    const String& GetRootPack() const override
    {
        return *rootPack;
    }

    float32 GetPriority() const
    {
        return rootPack->priority;
    }
    bool IsDone() const;
    const FileRequest& GetCurrentSubRequest() const;

    uint64 GetFullSizeWithDependencies() const override;

    uint64 GetDownloadedSize() const override;
private:
    void Restart();
    void SetErrorStatusAndFireSignal(FileRequest& subRequest, IDLCManager::Pack& currentPack);

    void AskFooter();
    void GetFooter();
    void StartLoadingPackFile();
    bool IsLoadingPackFileFinished();
    void StartCheckHash();
    bool IsCheckingHashFinished();
    void MountPack();
    void GoToNextSubRequest();

    DCLManagerImpl* packManagerImpl = nullptr;
    String rootPack;
    Vector<PackRequest*> dependencyPacks;
    Vector<FileRequest> dependencies; // first all dependencies then pack sub request
    uint64 totalAllPacksSize = 0;

    uint64 fullSizeServerData = 0;
    uint32 downloadTaskId = 0;
    PackFormat::PackFile::FooterBlock footerOnServer;
};
} // end namespace DAVA