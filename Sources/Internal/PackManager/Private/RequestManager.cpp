#include "PackManager/Private/RequestManager.h"
#include "FileSystem/FileSystem.h"
#include "Utils/CRC32.h"
#include "Utils/StringFormat.h"
#include "DLC/Downloader/DownloadManager.h"
#include "PackManager/Private/DLCManagerImpl.h"
#include "Base/Exception.h"

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
    auto it = std::find_if(begin(requests), end(requests), [&requestedPackName](const PackRequest& r) -> bool
                           {
                               return r.GetRequestedPackName() == requestedPackName;
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

void RequestManager::UpdateOrder(PackRequest* request, uint32 orderIndex)
{
    DVASSERT(request != nullptr);

    PackRequest* prevTop = Top();
    if (prevTop == nullptr)
    {
        return;
    }

    auto it = find(begin(requests), end(requests), request);
    if (it != end(requests))
    {
        requests.erase(it);
        if (orderIndex >= requests.size())
        {
            requests.push_back(request);
        }
        else
        {
            auto insertIt = begin(requests) + orderIndex;
            requests.insert(insertIt, request);
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

} // end namespace DAVA
