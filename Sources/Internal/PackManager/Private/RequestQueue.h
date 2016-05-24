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
    void Pause();

    bool operator<(const PackRequest& other) const
    {
        return GetPriority() < other.GetPriority();
    }

    struct SubRequest
    {
        enum Status : uint32
        {
            Wait = 0,
            LoadingCRC32File = 1, // download manager thread, wait on main thread
            LoadingPackFile = 2, // download manager thread, wait on main thread
            CheckCRC32 = 3, // on main thread (in future move to job manager)
            Mounted = 4, // on main thread

            Error = 10
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

    void StartLoadingCRC32File();
    bool DoneLoadingCRC32File();
    void StartLoadingPackFile();
    bool DoneLoadingPackFile();
    void StartCheckCRC32();
    bool DoneCheckingCRC32();
    void MountPack();
    void GoToNextSubRequest();

    PackManagerImpl* packManager = nullptr;
    PackManager::Pack* pack = nullptr;
    Vector<SubRequest> dependencies; // first all dependencies then pack sub request
};

class RequestManager
{
public:
    explicit RequestManager(PackManagerImpl& packManager_)
        : packManager(packManager_)
    {
    }

    void Start();
    void Stop();
    void Update();
    bool IsInQueue(const String& packName) const;
    bool Empty() const;
    size_type Size() const;
    PackRequest& Top();
    PackRequest& Find(const String& packName);
    void Push(const String& packName, float32 priority);
    void UpdatePriority(const String& packName, float32 newPriority);
    void Pop();

    static const String packPostfix;
    static const String hashPostfix;
    static const uint32 emptyZipArchiveHash = 0xD7CBC50E;

private:
    void CheckRestartLoading();

    PackManagerImpl& packManager;
    String currrentTopLoadingPack;
    Vector<PackRequest> items;
};
} // end namespace DAVA
