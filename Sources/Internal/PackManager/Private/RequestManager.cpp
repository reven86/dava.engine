#include "PackManager/Private/RequestManager.h"
#include "FileSystem/FileSystem.h"
#include "Utils/CRC32.h"
#include "Utils/StringFormat.h"
#include "DLC/Downloader/DownloadManager.h"
#include "PackManager/Private/PackManagerImpl.h"

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
    auto it = std::find_if(begin(items), end(items), [packName](const PackRequest& r) -> bool
                           {
                               return r.GetRootPack().name == packName;
                           });
    return it != end(items);
}

bool RequestManager::Empty() const
{
    return items.empty();
}

size_type RequestManager::Size() const
{
    return static_cast<size_type>(items.size());
}

PackRequest& RequestManager::Top()
{
    DVASSERT(!items.empty());

    PackRequest& topItem = items.front();
    return topItem;
}

PackRequest& RequestManager::Find(const String& packName)
{
    auto it = std::find_if(begin(items), end(items), [&packName](const PackRequest& r) -> bool
                           {
                               return r.GetRootPack().name == packName;
                           });
    if (it == end(items))
    {
        throw std::runtime_error("can't fined pack by name: " + packName);
    }
    return *it;
}

void RequestManager::CheckRestartLoading()
{
    DVASSERT(!items.empty());

    PackRequest& top = Top();

    if (Size() == 1)
    {
        currentTopLoadingPack = top.GetRootPack().name;
        top.Start();
    }
    else if (!currentTopLoadingPack.empty() && top.GetRootPack().name != currentTopLoadingPack)
    {
        // we have to cancel current pack request and start new with higher priority
        if (IsInQueue(currentTopLoadingPack))
        {
            PackRequest& prevTopRequest = Find(currentTopLoadingPack);
            prevTopRequest.Stop();
        }
        currentTopLoadingPack = top.GetRootPack().name;
        top.Start();
    }
}

void RequestManager::Push(const String& packName, float32 priority)
{
    if (IsInQueue(packName))
    {
        throw std::runtime_error("second time push same pack in queue, pack: " + packName);
    }

    IPackManager::Pack& pack = packManager.GetPack(packName);

    pack.state = IPackManager::Pack::Status::Requested;
    pack.priority = priority;

    items.emplace_back(packManager, pack);
    stable_sort(begin(items), end(items));

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
            stable_sort(begin(items), end(items));

            CheckRestartLoading();
        }
    }
}

void RequestManager::Pop()
{
    DVASSERT(!items.empty());

    items.erase(items.begin());
}

} // end namespace DAVA
