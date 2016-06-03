#pragma once

#include "PackManager/Private/PackRequest.h"

namespace DAVA
{
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
