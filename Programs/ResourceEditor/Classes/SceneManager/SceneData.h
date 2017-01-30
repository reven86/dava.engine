#pragma once

#include "Classes/Qt/Scene/SceneEditor2.h"

#include "TArc/DataProcessing/DataNode.h"

#include "Reflection/ReflectionRegistrator.h"
#include "Base/RefPtr.h"

#include <QString>

class SceneData : public DAVA::TArc::DataNode
{
public:
    // use this alias in constructions like Any::CanCast, Any::Cast, Any::CanGet and Any::Get
    using TSceneType = DAVA::RefPtr<SceneEditor2>;
    TSceneType GetScene();

    bool IsSceneChanged() const;
    DAVA::FilePath GetScenePath() const;
    DAVA::uint32 GetEnabledLandscapeTools() const;

    bool IsSavingAllowed(QString* message = nullptr) const;

    static const char* scenePropertyName;
    static const char* sceneChangedPropertyName;
    static const char* scenePathPropertyName;
    static const char* sceneLandscapeToolsPropertyName;

private:
    friend class SceneManagerModule;

    DAVA::RefPtr<SceneEditor2> scene;

    DAVA_VIRTUAL_REFLECTION_IN_PLACE(SceneData, DAVA::TArc::DataNode)
    {
        DAVA::ReflectionRegistrator<SceneData>::Begin()
        .Field(scenePropertyName, &SceneData::scene)
        .Field(sceneChangedPropertyName, &SceneData::IsSceneChanged, nullptr)
        .Field(scenePathPropertyName, &SceneData::GetScenePath, nullptr)
        .Field(sceneLandscapeToolsPropertyName, &SceneData::GetEnabledLandscapeTools, nullptr)
        .End();
    }
};