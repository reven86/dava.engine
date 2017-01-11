#pragma once

#include "PackManager/Private/PackRequest.h"

namespace DAVA
{
class RequestManager
{
public:
    explicit RequestManager(DCLManagerImpl& packManager_)
        : packManager(packManager_)
    {
    }

    void Start();
    void Stop();
    void Update();
    bool Empty() const;
    size_t GetNumRequests() const;
    PackRequest* Find(const String& requestedPackName) const;
    PackRequest* Top();
    PackRequest* Push(const String& requestedPackName);
    PackRequest* Pop();
    void UpdatePriority(const String& requestedPackName, uint32 orderIndex);

    static const String packPostfix;
    static const uint32 emptyZipArchiveHash = 0xD7CBC50E;
    static const uint32 emptyLZ4HCArchiveCrc32 = 0xA4324938;

private:
    void CheckRestartLoading();

    DCLManagerImpl& packManager;
    Vector<PackRequest> requests;
};
} // end namespace DAVA
