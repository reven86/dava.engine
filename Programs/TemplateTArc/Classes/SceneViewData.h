#pragma once

#include "Scene3D/Scene.h"
#include "DataProcessing/DataNode.h"

class SceneViewData : public DAVA::TArc::DataNode
{
public:
    void SetScene(DAVA::Scene* scene);
    DAVA::Scene* GetScene() const;

private:
    DAVA::Scene* scene = nullptr;
    DAVA_VIRTUAL_REFLECTION(SceneViewData)
    {
        DAVA::ReflectionRegistrator<SceneViewData>::Begin()
        .Field("scene", &SceneViewData::scene)
        .End();
    }
};