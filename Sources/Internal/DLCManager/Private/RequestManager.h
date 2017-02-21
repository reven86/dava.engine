#pragma once

#include "DLCManager/Private/PackRequest.h"

namespace DAVA
{
class RequestManager
{
public:
    explicit RequestManager(DLCManager& packManager_)
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
    void SetPriorityToRequest(PackRequest* request);
    void Remove(PackRequest* request);

private:
    DLCManager& packManager;
    Vector<PackRequest*> requests;
};
} // end namespace DAVA
