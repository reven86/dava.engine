#pragma once

#include "Base/BaseTypes.h"
#include "PackManager/PackManager.h"
#include "FileSystem/Private/PackFormatSpec.h"

namespace DAVA
{
class PackManagerImpl;

class PackRequest : public IPackManager::IRequest
{
public:
    PackRequest(PackManagerImpl& packManager_, IPackManager::Pack& pack_);

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

        IPackManager::Pack* pack = nullptr;
        String errorMsg;
        uint32 taskId = 0;
        Status status = Wait;
    };

    const IPackManager::Pack& GetRootPack() const override
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
    const IPackManager::Pack& GetErrorPack() const override;
    const String& GetErrorMessage() const override;

private:
    void Restart();
    void SetErrorStatusAndFireSignal(SubRequest& subRequest, IPackManager::Pack& currentPack);

    void AskFooter();
    void GetFooter();
    void StartLoadingPackFile();
    bool IsLoadingPackFileFinished();
    void StartCheckHash();
    bool IsCheckingHashFinished();
    void MountPack();
    void GoToNextSubRequest();

    PackManagerImpl* packManagerImpl = nullptr;
    IPackManager::Pack* rootPack = nullptr;
    Vector<IPackManager::Pack*> dependencyList;
    Vector<SubRequest> dependencies; // first all dependencies then pack sub request
    uint64 totalAllPacksSize = 0;

    uint64 fullSizeServerData = 0;
    uint32 downloadTaskId = 0;
    PackFormat::PackFile::FooterBlock footerOnServer;
};
} // end namespace DAVA