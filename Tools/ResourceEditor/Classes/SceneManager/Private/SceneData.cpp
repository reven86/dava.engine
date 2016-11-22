#include "Classes/SceneManager/SceneData.h"
#include "Classes/Qt/Scene/SceneEditor2.h"

DAVA::RefPtr<SceneEditor2> SceneData::GetScene()
{
    return scene;
}

bool SceneData::IsSceneChanged() const
{
    if (scene.Get() == nullptr)
    {
        return false;
    }

    return scene->IsChanged();
}

DAVA::FilePath SceneData::GetScenePath() const
{
    if (scene.Get() == nullptr)
    {
        return DAVA::FilePath();
    }

    return scene->GetScenePath();
}

DAVA::uint32 SceneData::GetEnabledLandscapeTools() const
{
    if (scene.Get() == nullptr)
    {
        return 0;
    }

    return scene->GetEnabledTools();
}

const char* SceneData::scenePropertyName = "Scene";
const char* SceneData::sceneChangedPropertyName = "IsSceneChanged";
const char* SceneData::scenePathPropertyName = "ScenePath";
const char* SceneData::sceneLandscapeToolsPropertyName = "EnabledLandscapeTools";