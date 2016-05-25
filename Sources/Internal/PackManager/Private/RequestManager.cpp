#include "PackManager/Private/RequestManager.h"
#include "FileSystem/FileSystem.h"
#include "Utils/CRC32.h"
#include "DLC/Downloader/DownloadManager.h"
#include "PackManager/Private/PackManagerImpl.h"

namespace DAVA
{
const String RequestManager::packPostfix = ".pack";
const String RequestManager::hashPostfix = ".hash";

void RequestManager::Start()
{
    if (packManager.IsProcessingEnabled())
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
            if (request.GetPackName() != subRequest.pack->name)
            {
                PackManager::Pack& rootPack = packManager.GetPack(request.GetPackName());
                rootPack.state = PackManager::Pack::Status::OtherError;
                rootPack.otherErrorMsg = Format("can't load (%s) pack becouse dependent (%s) pack error: %s",
                                                rootPack.name.c_str(), subRequest.pack->name.c_str(), subRequest.errorMsg.c_str());

                Pop(); // first pop current request and only then inform user

                packManager.onPackChange->Emit(rootPack, PackManager::Pack::Change::State);
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
                               return r.GetPackName() == packName;
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
                               return r.GetPackName() == packName;
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
        currrentTopLoadingPack = top.GetPackName();
        top.Start();
    }
    else if (!currrentTopLoadingPack.empty() && top.GetPackName() != currrentTopLoadingPack)
    {
        // we have to cancel current pack request and start new with higher priority
        PackRequest& prevTopRequest = Find(currrentTopLoadingPack);
        prevTopRequest.Stop();
        currrentTopLoadingPack = top.GetPackName();
        top.Start();
    }
}

void RequestManager::Push(const String& packName, float32 priority)
{
    if (IsInQueue(packName))
    {
        throw std::runtime_error("second time push same pack in queue, pack: " + packName);
    }

    PackManager::Pack& pack = packManager.GetPack(packName);

    pack.state = PackManager::Pack::Status::Requested;
    pack.priority = priority;

    items.emplace_back(packManager, pack);
    std::push_heap(begin(items), end(items));

    packManager.onPackChange->Emit(pack, PackManager::Pack::Change::State);
    packManager.onPackChange->Emit(pack, PackManager::Pack::Change::Priority);

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
            std::sort_heap(begin(items), end(items));

            CheckRestartLoading();
        }
    }
}

void RequestManager::Pop()
{
    DVASSERT(!items.empty());

    std::pop_heap(begin(items), end(items));
    items.pop_back();
}

} // end namespace DAVA
