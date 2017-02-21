#include "DLCManager/Private/RequestManager.h"
#include "DLCManager/Private/DLCManagerImpl.h"
#include "Debug/DVAssert.h"

namespace DAVA
{
void RequestManager::Start()
{
    if (packManager.IsRequestingEnabled())
    {
        PackRequest* request = Top();
        if (request != nullptr)
        {
            request->Start();
        }
    }
}

void RequestManager::Stop()
{
    PackRequest* request = Top();
    if (request != nullptr)
    {
        request->Start();
    }
}

void RequestManager::Update()
{
    if (!Empty())
    {
        PackRequest* request = Top();
        request->Update();
        if (request->IsDownloaded())
        {
            Pop();
        }
    }
}

bool RequestManager::Empty() const
{
    return requests.empty();
}

size_t RequestManager::GetNumRequests() const
{
    return requests.size();
}

PackRequest* RequestManager::Top() const
{
    if (requests.empty())
    {
        return nullptr;
    }
    return requests.front();
}

PackRequest* RequestManager::Find(const String& requestedPackName) const
{
    auto it = std::find_if(begin(requests), end(requests), [&requestedPackName](const PackRequest* r) -> bool
                           {
                               return r->GetRequestedPackName() == requestedPackName;
                           });
    if (it == end(requests))
    {
        return nullptr;
    }
    return (*it);
}

void RequestManager::Push(PackRequest* request_)
{
    for (PackRequest* r : requests)
    {
        if (r == request_)
        {
            return;
        }
    }

    requests.push_back(request_);
}

void RequestManager::SetPriorityToRequest(PackRequest* request)
{
    DVASSERT(Thread::IsMainThread());
    DVASSERT(request != nullptr);

    PackRequest* prevTop = Top();
    if (prevTop == nullptr)
    {
        return;
    }

    auto it = find(begin(requests), end(requests), request);
    if (it != end(requests))
    {
        // 1. collect all request not sub request for "request"
        Vector<PackRequest*> removeFromBeg;
        for (PackRequest* r : requests)
        {
            if (r == request)
            {
                break;
            }
            if (!request->IsSubRequest(r))
            {
                removeFromBeg.push_back(r);
            }
        }
        // 2. remove all NOT sub request from begining queue
        for (PackRequest* r : removeFromBeg)
        {
            requests.erase(find(begin(requests), end(requests), r));
        }
        // 3. find position after "request"
        it = find(begin(requests), end(requests), request);
        it++;
        // 4. insert all previously removed request after preserve order
        for (PackRequest* r : removeFromBeg)
        {
            it = requests.insert(it, r);
            it++;
        }
    }

    PackRequest* newTop = Top();
    if (newTop != prevTop)
    {
        prevTop->Stop();
        newTop->Start();
    }
}

void RequestManager::Pop()
{
    if (!requests.empty())
    {
        requests.erase(begin(requests));
    }
}

void RequestManager::Remove(PackRequest* request)
{
    DVASSERT(request != nullptr);

    auto it = find(begin(requests), end(requests), request);
    if (it != end(requests))
    {
        requests.erase(it);
    }
}

} // end namespace DAVA
