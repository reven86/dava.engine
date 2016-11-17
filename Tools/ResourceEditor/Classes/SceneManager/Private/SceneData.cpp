#include "Classes/SceneManager/SceneData.h"
#include "Classes/Qt/Scene/SceneEditor2.h"

SceneEditor2* SceneData::GetScene()
{
    return scene;
}

bool SceneData::IsSceneChanged() const
{
    if (scene == nullptr)
    {
        return false;
    }

    return scene->IsChanged();
}

DAVA::FilePath SceneData::GetScenePath() const
{
    if (scene == nullptr)
    {
        return DAVA::FilePath();
    }

    return scene->GetScenePath();
}

const char* SceneData::scenePropertyName = "Scene";
const char* SceneData::sceneChangedPropertyName = "IsSceneChanged";
const char* SceneData::scenePathPropertyName = "ScenePath";