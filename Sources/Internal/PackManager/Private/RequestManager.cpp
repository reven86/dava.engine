#include "PackManager/Private/RequestManager.h"
#include "FileSystem/FileSystem.h"
#include "Utils/CRC32.h"
#include "DLC/Downloader/DownloadManager.h"
#include "PackManager/Private/PackManagerImpl.h"
#include "Base/Exception.h"

namespace DAVA
{
const String RequestManager::packPostfix = ".dvpk";

void RequestManager::Start()
{
    if (packManager.IsRequestingEnabled())
    {
        if (!Empty())
        {
            Top().Start();
        }
    }
}

void RequestManager::Stop()
{
    if (!Empty())
    {
        Top().Stop();
    }
}

void RequestManager::Update()
{
    if (!Empty())
    {
        PackRequest& request = Top();
        request.Update();
        if (request.IsDone())
        {
            Pop();
        }
        else if (request.IsError())
        {
            const PackRequest::SubRequest& subRequest = request.GetCurrentSubRequest();
            if (request.GetRootPack().name != subRequest.pack->name)
            {
                IPackManager::Pack& rootPack = packManager.GetPack(request.GetRootPack().name);
                rootPack.state = IPackManager::Pack::Status::OtherError;
                rootPack.otherErrorMsg = Format("can't load (%s) pack becouse dependent (%s) pack error: %s",
                                                rootPack.name.c_str(), subRequest.pack->name.c_str(), subRequest.pack->otherErrorMsg.c_str());

                Pop(); // first pop current request and only then inform user

                packManager.packStateChanged.Emit(rootPack);
            }
            else
            {
                // we already inform client about error in subRequest during Update()
            }
        }
    }
}

bool RequestManager::IsInQueue(const String& packName) const
{
    auto it = std::find_if(begin(requests), end(requests), [packName](const PackRequest& r) -> bool
                           {
                               return r.GetRootPack().name == packName;
                           });
    return it != end(requests);
}

bool RequestManager::Empty() const
{
    return requests.empty();
}

size_t RequestManager::CountRequests() const
{
    return requests.size();
}

PackRequest& RequestManager::Top()
{
    DVASSERT(!requests.empty());

    PackRequest& topItem = requests.front();
    return topItem;
}

PackRequest& RequestManager::Find(const String& packName)
{
    auto it = std::find_if(begin(requests), end(requests), [&packName](const PackRequest& r) -> bool
                           {
                               return r.GetRootPack().name == packName;
                           });
    if (it == end(requests))
    {
        DAVA_THROW(DAVA::Exception, "can't fined pack by name: " + packName);
    }
    return *it;
}

void RequestManager::CheckRestartLoading()
{
    DVASSERT(!requests.empty());

    PackRequest& top = Top();

    if (CountRequests() == 1)
    {
        loadingPackName = top.GetRootPack().name;
        top.Start();
    }
    else if (!loadingPackName.empty() && top.GetRootPack().name != loadingPackName)
    {
        // we have to cancel current pack request and start new with higher priority
        if (IsInQueue(loadingPackName))
        {
            PackRequest& prevTopRequest = Find(loadingPackName);
            prevTopRequest.Stop();
        }
        loadingPackName = top.GetRootPack().name;
        top.Start();
    }
}

void RequestManager::Push(const String& packName, float32 priority)
{
    if (IsInQueue(packName))
    {
        DAVA_THROW(DAVA::Exception, "second time push same pack in queue, pack: " + packName);
    }

    IPackManager::Pack& pack = packManager.GetPack(packName);

    pack.state = IPackManager::Pack::Status::Requested;
    pack.priority = priority;

    requests.emplace_back(packManager, pack);
    stable_sort(begin(requests), end(requests));

    packManager.packStateChanged.Emit(pack);

    CheckRestartLoading();
}

void RequestManager::UpdatePriority(const String& packName, float32 newPriority)
{
    if (IsInQueue(packName))
    {
        PackRequest& packRequest = Find(packName);
        if (packRequest.GetPriority() != newPriority)
        {
            packRequest.ChangePriority(newPriority);
            stable_sort(begin(requests), end(requests));

            CheckRestartLoading();
        }
    }
}

void RequestManager::Pop()
{
    DVASSERT(!requests.empty());

    requests.erase(requests.begin());
}

} // end namespace DAVA
