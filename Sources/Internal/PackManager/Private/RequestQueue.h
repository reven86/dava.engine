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
#include "PackManager/Private/PackManagerImpl.h"

namespace DAVA
{
class PackRequest
{
public:
    PackRequest(PackManagerImpl& packManager_, PackManager::Pack& pack_);

    void Start();
    void Update();
    void ChangePriority(float32 newPriority);
    void Pause();

    bool operator<(const PackRequest& other) const
    {
        return GetPriority() < other.GetPriority();
    }

    struct SubRequest
    {
        enum Status : uint32
        {
            Wait = 0,
            LoadingCRC32File = 1, // download manager thread, wait on main thread
            LoadingPackFile = 2, // download manager thread, wait on main thread
            CheckCRC32 = 3, // on main thread (in future move to job manager)
            Mounted = 4, // on main thread

            Error = 10
        };

        PackManager::Pack* pack = nullptr;
        String errorMsg;
        uint32 taskId = 0;
        Status status = Wait;
    };

    const String& GetPackName() const
    {
        return pack->name;
    }

    float32 GetPriority() const
    {
        return pack->priority;
    }
    bool IsDone() const;
    bool IsError() const;
    const SubRequest& GetCurrentSubRequest() const;

private:
    void CollectDownlodbleDependency(const String& packName, Set<PackManager::Pack*>& dependency);

    void StartLoadingCRC32File();
    bool DoneLoadingCRC32File();
    void StartLoadingPackFile();
    bool DoneLoadingPackFile();
    void StartCheckCRC32();
    bool DoneCheckingCRC32();
    void MountPack();
    void GoToNextSubRequest();

    PackManagerImpl* packManager = nullptr;
    PackManager::Pack* pack = nullptr;
    Vector<SubRequest> dependencies; // first all dependencies then pack sub request
};

class RequestManager
{
public:
    explicit RequestManager(PackManagerImpl& packManager_)
        : packManager(packManager_)
    {
    }

    void Start();
    void Stop();
    void Update();
    bool IsInQueue(const String& packName) const;
    bool Empty() const;
    size_type Size() const;
    PackRequest& Top();
    PackRequest& Find(const String& packName);
    void Push(const String& packName, float32 priority);
    void UpdatePriority(const String& packName, float32 newPriority);
    void Pop();

    static const String packPostfix;
    static const String hashPostfix;
    static const uint32 emptyZipArchiveHash = 0xD7CBC50E;

private:
    void CheckRestartLoading();

    PackManagerImpl& packManager;
    String currrentTopLoadingPack;
    Vector<PackRequest> items;
};
} // end namespace DAVA
