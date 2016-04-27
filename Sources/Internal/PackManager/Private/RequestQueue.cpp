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
        subRequest.status = Wait;
        subRequest.taskId = 0;

        dependencies.push_back(subRequest);
    }

    // last step download pack itself (if it not virtual)
    if (rootPack.crc32FromDB != 0)
    {
        SubRequest subRequest;

        subRequest.packName = rootPack.name;
        subRequest.status = Wait;
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
    throw std::runtime_error("not implemented");
}

void PackRequest::ChangePriority(float32 newPriority)
{
    throw std::runtime_error("not implemented");
}

void PackRequest::Pause()
{
    throw std::runtime_error("not implemented");
}
} // end namespace DAVA
