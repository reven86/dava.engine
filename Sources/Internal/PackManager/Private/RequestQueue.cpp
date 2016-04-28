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

#include "PackManager/Private/RequestQueue.h"

namespace DAVA
{
PackRequest::PackRequest(PackManager& packManager_, const String& name, float32 priority)
    : packManager(&packManager_)
{
    // find all dependenciec
    // put it all into vector and put final pack into vector too

    const PackManager::PackState& rootPack = packManager->GetPackState(name);

    // теперь нужно узнать виртуальный ли это пакет, и первыми поставить на закачку
    // так как у нас может быть несколько зависимых паков, тоже виртуальными, то
    // мы должны сначала сделать плоскую структуру всех зависимых паков, всем им
    // выставить одинаковый приоритет - текущего виртуального пака и добавить
    // в очередь на скачку в порядке, зависимостей
    Set<const PackManager::PackState*> dependency;
    CollectDownlodbleDependency(name, dependency);

    dependencies.reserve(dependency.size() + 1);

    for (const PackManager::PackState* pack : dependency)
    {
        SubRequest subRequest;

        subRequest.packName = pack->name;
        subRequest.status = SubRequest::Wait;
        subRequest.taskId = 0;

        dependencies.push_back(subRequest);
    }

    // last step download pack itself (if it not virtual)
    if (rootPack.crc32FromDB != 0)
    {
        SubRequest subRequest;

        subRequest.packName = rootPack.name;
        subRequest.status = SubRequest::Wait;
        subRequest.taskId = 0;
        dependencies.push_back(subRequest);
    }
}

void PackRequest::CollectDownlodbleDependency(const String& packName, Set<const PackManager::PackState*>& dependency)
{
    const PackManager::PackState& packState = packManager->GetPackState(packName);
    for (const String& dependName : packState.dependency)
    {
        const PackManager::PackState& dependPack = packManager->GetPackState(dependName);
        if (dependPack.crc32FromDB != 0 && dependPack.state != PackManager::PackState::Mounted)
        {
            dependency.insert(&dependPack);
        }

        CollectDownlodbleDependency(dependName, dependency);
    }
}

void PackRequest::Start()
{
    throw std::runtime_error("not implemented");
}

void PackRequest::Update(PackManager& packManager)
{
    if (!IsDone() && !IsError())
    {
        SubRequest& subRequest = dependencies.at(0);

        switch (subRequest.status)
        {
        case SubRequest::Wait:
            StartLoadingCRC32File();
            break;
        case SubRequest::LoadingCRC32File:
            if (DoneLoadingCRC32File())
            {
                StartLoadingPackFile();
            }
            break;
        case SubRequest::LoadingPackFile:
            if (DoneLoadingPackFile())
            {
                StartCheckCRC32();
            }
            break;
        case SubRequest::CheckCRC32:
            if (DoneCheckingCRC32())
            {
                MountPack();
            }
            break;
        case SubRequest::Mounted:
            GoToNextSubRequest();
            break;
        } // end switch status
    }
    //DownloadManager* dm = DownloadManager::Instance();
    //DownloadStatus status = DL_UNKNOWN;
    //dm->GetStatus(downloadHandler, status);
    //uint64 progress = 0;
    //switch (status)
    //{
    //case DL_IN_PROGRESS:
    //	if (dm->GetProgress(downloadHandler, progress))
    //	{
    //		uint64 total = 0;
    //		if (dm->GetTotal(downloadHandler, total))
    //		{
    //			currentDownload->downloadProgress = static_cast<float>(progress) / total;
    //			// fire event on update progress
    //			packMngrPublic->onPackStateChanged.Emit(*currentDownload);
    //		}
    //	}
    //	break;
    //case DL_FINISHED:
    //{
    //	// first test error code
    //	DownloadError downloadError = DLE_NO_ERROR;
    //	if (dm->GetError(downloadHandler, downloadError))
    //	{
    //		switch (downloadError)
    //		{
    //		case DLE_CANCELLED: // download was cancelled by our side
    //		case DLE_COULDNT_RESUME: // seems server doesn't supports download resuming
    //		case DLE_COULDNT_RESOLVE_HOST: // DNS request failed and we cannot to take IP from full qualified domain name
    //		case DLE_COULDNT_CONNECT: // we cannot connect to given adress at given port
    //		case DLE_CONTENT_NOT_FOUND: // server replies that there is no requested content
    //		case DLE_NO_RANGE_REQUEST: // Range requests is not supported. Use 1 thread without reconnects only.
    //		case DLE_COMMON_ERROR: // some common error which is rare and requires to debug the reason
    //		case DLE_INIT_ERROR: // any handles initialisation was unsuccessful
    //		case DLE_FILE_ERROR: // file read and write errors
    //		case DLE_UNKNOWN: // we cannot determine the error
    //						  // inform user about error
    //		{
    //			PackManager::PackState& pack = *currentDownload;

    //			pack.state = PackManager::PackState::ErrorLoading;
    //			pack.downloadError = downloadError;

    //			packMngrPublic->onPackStateChanged.Emit(pack);
    //			break;
    //		}
    //		case DLE_NO_ERROR:
    //		{
    //			// check crc32 of just downloaded file
    //			FilePath archivePath = localPacksDir + currentDownload->name;
    //			uint32 crc32 = CRC32::ForFile(archivePath);
    //			if (crc32 != currentDownload->crc32FromMeta)
    //			{
    //				throw std::runtime_error("crc32 not match");
    //			}
    //			// now mount archive
    //			// validate it
    //			FileSystem* fs = FileSystem::Instance();
    //			try
    //			{
    //				fs->Mount(archivePath, "Data/");
    //			}
    //			catch (std::exception& ex)
    //			{
    //				Logger::Error("%s", ex.what());
    //				throw;
    //			}

    //			currentDownload->state = PackManager::PackState::Mounted;
    //		}
    //		} // end switch downloadError
    //		currentDownload = nullptr;
    //		queue.Pop();
    //		downloadHandler = 0;
    //	}
    //	else
    //	{
    //		throw std::runtime_error(Format("can't get download error code for download job id: %d", downloadHandler));
    //	}
    //}
    //break;
    //default:
    //	break;
    //}
}

void PackRequest::ChangePriority(float32 newPriority)
{
    throw std::runtime_error("not implemented");
}

void PackRequest::Pause()
{
    throw std::runtime_error("not implemented");
}

bool PackRequest::IsDone() const
{
    return dependencies.empty();
}

bool PackRequest::IsError() const
{
    if (!dependencies.empty())
    {
        return GetCurrentSubRequest().status == SubRequest::Error;
    }
    return false;
}

const PackRequest::SubRequest& PackRequest::GetCurrentSubRequest() const
{
    return dependencies.at(0); // at check index
}

void RequestQueue::Start()
{
    if (packManager.IsProcessingEnabled())
    {
    }
}
void RequestQueue::Stop()
{
    throw std::runtime_error("implement me");
}

void RequestQueue::Update()
{
    if (!Empty())
    {
        PackRequest& request = Top();
        request.Update(packManager);
        if (request.IsDone())
        {
            Pop();
        }
        else if (request.IsError())
        {
            const PackRequest::SubRequest& subRequest = request.GetCurrentSubRequest();
            PackManager::PackState& rootPack = const_cast<PackManager::PackState&>(packManager.GetPackState(request.GetPackName()));
            if (rootPack.name != subRequest.packName)
            {
                rootPack.state = PackManager::PackState::OtherError;
                rootPack.otherErrorMsg = Format("can't load (%s) pack becouse dependent (%s) pack error: %s",
                                                rootPack.name.c_str(), subRequest.packName.c_str(), subRequest.errorMsg.c_str());

                Pop(); // first pop current request and only then inform user

                packManager.onPackStateChanged.Emit(rootPack);
            }
            else
            {
                // we already inform client about error in subRequest during Update()
            }
        }
    }
}

bool RequestQueue::IsInQueue(const String& packName) const
{
    auto it = std::find_if(begin(items), end(items), [packName](const PackRequest& r) -> bool
                           {
                               return r.GetPackName() == packName;
                           });
    return it != end(items);
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

    items.emplace_back(PackRequest{ packManager, packName, priority });
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
