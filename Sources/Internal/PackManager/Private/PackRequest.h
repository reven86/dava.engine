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
            LoadingHaskFile, // download manager thread, wait on main thread
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

    const String& GetPackName() const override
    {
        return pack->name;
    }

    float32 GetPriority() const
    {
        return pack->priority;
    }
    bool IsDone() const;
    bool IsError() const override;
    const SubRequest& GetCurrentSubRequest() const;

    uint64 GetFullSizeWithDependencies() const override;

    uint64 GetDownloadedSize() const override;

    const String& GetErrorMessage() const override;

private:
    void CollectDownlodbleDependency(const String& packName, Set<PackManager::Pack*>& dependency);
    void SetErrorStatusAndFireSignal(SubRequest& subRequest, PackManager::Pack& currentPack);

    void StartLoadingHashFile();
    bool IsLoadingHashFileFinished();
    void StartLoadingPackFile();
    bool IsLoadingPackFileFinished();
    void StartCheckHash();
    bool IsCheckingHashFinished();
    void MountPack();
    void GoToNextSubRequest();

    PackManagerImpl* packManager = nullptr;
    PackManager::Pack* pack = nullptr;
    Set<PackManager::Pack*> dependencySet;
    Vector<SubRequest> dependencies; // first all dependencies then pack sub request
    uint64 totalAllPacksSize = 0;
};
} // end namespace DAVA