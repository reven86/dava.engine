#pragma once

#include "Base/BaseTypes.h"
#include "PackManager/PackManager.h"

namespace DAVA
{
class PackManagerImpl;

class PackRequest
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

    const String& GetPackName() const
    {
        return pack->name;
    }

    float32 GetPriority() const
    {
        return pack->priority;
    }
    bool IsDone() const;
    bool IsError() const;
    const SubRequest& GetCurrentSubRequest() const;

private:
    void CollectDownlodbleDependency(const String& packName, Set<PackManager::Pack*>& dependency);

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
    Vector<SubRequest> dependencies; // first all dependencies then pack sub request
};
} // end namespace DAVA