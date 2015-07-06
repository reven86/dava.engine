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


#include "SceneProcessor.h"

using namespace DAVA;

namespace
{

typedef DAVA::Set<String> StringSet;

}

SceneProcessor::SceneProcessor(EntityProcessorBase *_entityProcessor /*= NULL*/)
    : entityProcessor(SafeRetain(_entityProcessor))
{
}

SceneProcessor::~SceneProcessor()
{
    SafeRelease(entityProcessor);
}

void SceneProcessor::SetEntityProcessor(EntityProcessorBase *_entityProcessor)
{
    SafeRelease(entityProcessor);

    entityProcessor = SafeRetain(_entityProcessor);
}

bool SceneProcessor::Execute(DAVA::Scene *currentScene)
{
    if (!entityProcessor)
    {
        Logger::Warning("%s need to set EntityProcessor", __FUNCTION__);
        return false;
    }

    entityProcessor->Init();

    int32 childrenCount = currentScene->GetChildrenCount();

    StringSet refToOwnerSet;

    const bool needProcessExternal = entityProcessor->NeedProcessExternal();
    bool sceneModified = false;

    for (int32 index = 0; index < childrenCount; index++)
    {
        Entity *currentEntity = currentScene->GetChild(index);

        bool entityModified = entityProcessor->ProcessEntity(currentEntity, currentEntity->GetName(), false);
        sceneModified = sceneModified || entityModified;
        if (entityModified && needProcessExternal)
        {
            KeyedArchive *props = GetCustomPropertiesArchieve(currentEntity);
            
            if (!props)
            {
                Logger::Warning("%s %s custom properties not found", __FUNCTION__, currentEntity->GetName().c_str());
                continue;
            }
            
            if (!props->IsKeyExists("editor.referenceToOwner"))
            {
                Logger::Error("%s editor.referenceToOwner not found for %s", __FUNCTION__, currentEntity->GetName().c_str());
                continue;
            }

            const String referenceToOwner = props->GetString("editor.referenceToOwner");
            std::pair<StringSet::iterator, bool> insertResult = refToOwnerSet.insert(referenceToOwner);

            if (insertResult.second)
            {
                Scene *newScene = new Scene();
                newScene->LoadScene(referenceToOwner);
                DVASSERT(newScene->GetChildrenCount() == 1);
                entityProcessor->ProcessEntity(newScene, currentEntity->GetName(), true);
                newScene->SaveScene(referenceToOwner);
                SafeRelease(newScene);
            }
        }
    }

    entityProcessor->Finalize();
    return sceneModified;
}
