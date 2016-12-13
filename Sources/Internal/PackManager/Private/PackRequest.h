#pragma once

#include "Base/BaseTypes.h"
#include "PackManager/PackManager.h"
#include "FileSystem/Private/PackFormatSpec.h"

namespace DAVA
{
class DCLManagerImpl;

class PackRequest : public DLCManager::IRequest
{
public:
    PackRequest(DCLManagerImpl& packManager_, DLCManager::Pack& pack_);

    void Start();
    void Update();
    void ChangePriority(float32 newPriority);
    void Stop();

    bool operator<(const PackRequest& other) const
    {
        return GetPriority() < other.GetPriority();
    }

    struct SubRequest
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

        DLCManager::Pack* pack = nullptr;
        String errorMsg;
        uint32 taskId = 0;
        Status status = Wait;
    };

    const DLCManager::Pack& GetRootPack() const override
    {
        return *rootPack;
    }

    float32 GetPriority() const
    {
        return rootPack->priority;
    }
    bool IsDone() const;
    bool IsError() const override;
    const SubRequest& GetCurrentSubRequest() const;

    uint64 GetFullSizeWithDependencies() const override;

    uint64 GetDownloadedSize() const override;
    const DLCManager::Pack& GetErrorPack() const override;
    const String& GetErrorMessage() const override;

private:
    void Restart();
    void SetErrorStatusAndFireSignal(SubRequest& subRequest, DLCManager::Pack& currentPack);

    void AskFooter();
    void GetFooter();
    void StartLoadingPackFile();
    bool IsLoadingPackFileFinished();
    void StartCheckHash();
    bool IsCheckingHashFinished();
    void MountPack();
    void GoToNextSubRequest();

    DCLManagerImpl* packManagerImpl = nullptr;
    DLCManager::Pack* rootPack = nullptr;
    Vector<DLCManager::Pack*> dependencyList;
    Vector<SubRequest> dependencies; // first all dependencies then pack sub request
    uint64 totalAllPacksSize = 0;

    uint64 fullSizeServerData = 0;
    uint32 downloadTaskId = 0;
    PackFormat::PackFile::FooterBlock footerOnServer;
};
} // end namespace DAVA