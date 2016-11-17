#pragma once

#include "TArc/DataProcessing/DataNode.h"

#include "Reflection/Registrator.h"

class SceneEditor2;
class SceneData : public DAVA::TArc::DataNode
{
public:
    SceneEditor2* GetScene();

    bool IsSceneChanged() const;
    DAVA::FilePath GetScenePath() const;

    static const char* scenePropertyName;
    static const char* sceneChangedPropertyName;
    static const char* scenePathPropertyName;

private:
    friend class SceneManagerModule;
    SceneEditor2* scene = nullptr;

    DAVA_VIRTUAL_REFLECTION(SceneData, DAVA::TArc::DataNode)
    {
        DAVA::ReflectionRegistrator<SceneData>::Begin()
        .Field(scenePropertyName, &SceneData::scene)
        .Field(sceneChangedPropertyName, &SceneData::IsSceneChanged, nullptr)
        .Field(scenePathPropertyName, &SceneData::GetScenePath, nullptr)
        .End();
    }
};