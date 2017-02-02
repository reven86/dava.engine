#include "Classes/SceneManager/SceneData.h"

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

bool SceneData::IsSavingAllowed(QString* message /*= nullptr*/) const
{
    DVASSERT(scene.Get() != nullptr);
    QString warningMessage;
    if (scene->GetEnabledTools() != 0)
    {
        warningMessage = "Disable landscape editing before save!";
    }
    else if (scene->wayEditSystem->IsWayEditEnabled())
    {
        warningMessage = "Disable path editing before save!";
    }

    if (warningMessage.isEmpty())
    {
        return true;
    }
    if (message != nullptr)
    {
        *message = warningMessage;
    }
    return false;
}

const char* SceneData::scenePropertyName = "Scene";
const char* SceneData::sceneChangedPropertyName = "IsSceneChanged";
const char* SceneData::scenePathPropertyName = "ScenePath";
const char* SceneData::sceneLandscapeToolsPropertyName = "EnabledLandscapeTools";