#pragma once

#include "PackManager/Private/PackRequest.h"

namespace DAVA
{
class RequestManager
{
public:
    explicit RequestManager(IDLCManager& packManager_)
        : packManager(packManager_)
    {
    }

    void Start();
    void Stop();
    void Update();
    bool Empty() const;
    size_t GetNumRequests() const;
    PackRequest* Find(const String& requestedPackName) const;
    PackRequest* Top() const;
    void Push(PackRequest*);
    void Pop();
    void UpdateOrder(PackRequest* request, uint32 newOrderIndex);

private:
    IDLCManager& packManager;
    Vector<PackRequest*> requests;
};
} // end namespace DAVA
