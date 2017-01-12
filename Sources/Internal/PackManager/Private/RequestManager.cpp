#include "PackManager/Private/RequestManager.h"
#include "FileSystem/FileSystem.h"
#include "Utils/CRC32.h"
#include "Utils/StringFormat.h"
#include "DLC/Downloader/DownloadManager.h"
#include "PackManager/Private/DCLManagerImpl.h"
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
        if (request->IsDone())
        {
            Pop();
        }
        else if (request->IsError())
        {
            request->Start();
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

PackRequest* RequestManager::Top()
{
    if (requests.empty())
    {
        return nullptr;
    }
    return &requests[0];
}

PackRequest* RequestManager::Find(const String& requestedPackName)
{
    auto it = std::find_if(begin(requests), end(requests), [&requestedPackName](const PackRequest& r) -> bool
                           {
                               return r.GetRootPack().name == requestedPackName;
                           });
    if (it == end(requests))
    {
        return nullptr;
    }
    return (*it);
}

PackRequest* RequestManager::Push(const String& requestedPackName)
{
    PackRequest* request = Find(requestedPackName);
    if (request != nullptr)
    {
        Logger::Error("second time push same pack in queue, pack: %s", requestedPackName.c_str());
        return;
    }

    PackRequest* newRequest = new PackRequest(packManager, requestedPackName);

    requests.push_back(newRequest);

    return newRequest;
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
