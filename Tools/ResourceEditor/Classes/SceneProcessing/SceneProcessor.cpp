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
                Entity *root = newScene->GetRootNode(referenceToOwner);
                newScene->AddNode(root);
                DVASSERT(root->GetChildrenCount() == 1);
                entityProcessor->ProcessEntity(root->GetChild(0), currentEntity->GetName(), true);
                newScene->SaveScene(referenceToOwner);
                SafeRelease(newScene);
            }
        }
    }

    entityProcessor->Finalize();
    return sceneModified;
}
