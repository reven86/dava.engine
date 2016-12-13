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
    size_t CountRequests() const;
    PackRequest& Top();
    PackRequest& Find(const String& packName);
    void Push(const String& packName, float32 priority);
    void Pop();
    void UpdatePriority(const String& packName, float32 newPriority);

    static const String packPostfix;
    static const uint32 emptyZipArchiveHash = 0xD7CBC50E;
    static const uint32 emptyLZ4HCArchiveCrc32 = 0xA4324938;

    const Vector<PackRequest>& GetRequests() const
    {
        return requests;
    }
    const String& GetLoadingPackName() const
    {
        return loadingPackName;
    }

private:
    void CheckRestartLoading();

    PackManagerImpl& packManager;
    String loadingPackName;
    Vector<PackRequest> requests;
};
} // end namespace DAVA
