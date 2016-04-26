/*==================================================================================
    Copyright (c) 2008, binaryzebra
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    * Neither the name of the binaryzebra nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE binaryzebra AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL binaryzebra BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/

#pragma once

#include "Base/BaseTypes.h"
#include "PackManager/PackManager.h"
#include "DLC/Downloader/DownloadManager.h"

namespace DAVA
{
class PackRequest
{
public:
    PackRequest(const String& name, float32 priority);

    void Start();
    void Update(PackManager& packManager);
    void ChangePriority(float32 newPriority);
    void Pause();

    enum Status : uint32
    {
        WaitInQueue = 0,
        LoadingDependencies = 1,
        LoadingPack = 2,
        Mounting = 3,
    };

    bool operator<(const PackRequest& other) const
    {
        return priority < other.priority;
    }

    struct SubRequest
    {
        String packName;
        uint32 taskId;
    };

    Status GetStatus() const
    {
        return status;
    }
    const String& GetPackName() const
    {
        return packName;
    }
    float32 GetPriority() const
    {
        return priority;
    }

private:
    Status status;
    String packName;
    float32 priority;
    uint32 currentDependencySubRequest;
    Vector<SubRequest> dependencies;
    SubRequest finalPackRequest;
};

class RequestQueue
{
public:
    RequestQueue() = default;

    inline bool IsInQueue(const String& packName) const;
    inline bool Empty() const;
    inline uint32 Size() const;
    inline PackRequest& Top();
    inline PackRequest& Find(const String& packName);
    inline void Push(const String& packName, float32 priority);
    inline void UpdatePriority(const String& packName, float32 newPriority);
    inline void Pop();

private:
    inline void CheckRestartLoading();

    String currrentTopLoadingPack;
    Vector<PackRequest> items;
};

bool RequestQueue::IsInQueue(const String& packName) const
{
    auto it = std::find_if(begin(items), end(items), [packName](const PackRequest& r) -> bool
                           {
                               return r.GetPackName() == packName;
                           });
    return items.empty();
}

bool RequestQueue::Empty() const
{
    return items.empty();
}

uint32 RequestQueue::Size() const
{
    return static_cast<uint32>(items.size());
}

PackRequest& RequestQueue::Top()
{
    PackRequest& topItem = items.front();
    return topItem;
}

PackRequest& RequestQueue::Find(const String& packName)
{
    auto it = std::find_if(begin(items), end(items), [packName](const PackRequest& r) -> bool
                           {
                               return r.GetPackName() == packName;
                           });
    if (it == end(items))
    {
        throw std::runtime_error("can't fined pack by name: " + packName);
    }
    return *it;
}

void RequestQueue::CheckRestartLoading()
{
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
        prevTopRequest.Pause();
        currrentTopLoadingPack = top.GetPackName();
        top.Start();
    }
}

void RequestQueue::Push(const String& packName, float32 priority)
{
    if (IsInQueue(packName))
    {
        throw std::runtime_error("second time push same pack in queue, pack: " + packName);
    }

    items.emplace_back(PackRequest{ packName, priority });
    std::push_heap(begin(items), end(items));

    CheckRestartLoading();
}

void RequestQueue::UpdatePriority(const String& packName, float32 newPriority)
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

void RequestQueue::Pop()
{
    std::pop_heap(begin(items), end(items));
    items.pop_back();
}
} // end namespace DAVA
