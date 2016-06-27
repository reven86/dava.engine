#pragma once

#include "Base/BaseTypes.h"
#include "PackManager/PackManager.h"

namespace DAVA
{
class PackManagerImpl;

class PackRequest : public PackManager::IRequest
{
public:
    PackRequest(PackManagerImpl& packManager_, PackManager::Pack& pack_);

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
            LoadingPackFile, // download manager thread, wait on main thread
            CheckHash, // on main thread (in future move to job manager)
            Mounted, // on main thread

            Error
        };

        PackManager::Pack* pack = nullptr;
        String errorMsg;
        uint32 taskId = 0;
        Status status = Wait;
    };

    const PackManager::Pack& GetRootPack() const override
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
    const PackManager::Pack& GetErrorPack() const override;
    const String& GetErrorMessage() const override;

private:
    void CollectDownlodbleDependency(const String& packName, Set<PackManager::Pack*>& dependency);
    void SetErrorStatusAndFireSignal(SubRequest& subRequest, PackManager::Pack& currentPack);

    void StartLoadingPackFile();
    bool IsLoadingPackFileFinished();
    void StartCheckHash();
    bool IsCheckingHashFinished();
    void MountPack();
    void GoToNextSubRequest();

    PackManagerImpl* packManagerImpl = nullptr;
    PackManager::Pack* rootPack = nullptr;
    Set<PackManager::Pack*> dependencySet;
    Vector<SubRequest> dependencies; // first all dependencies then pack sub request
    uint64 totalAllPacksSize = 0;
};
} // end namespace DAVA