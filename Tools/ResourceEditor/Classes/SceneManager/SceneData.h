#pragma once

#include "TArc/DataProcessing/DataNode.h"

#include "Reflection/Registrator.h"

class SceneEditor2;
class SceneData : public DAVA::TArc::DataNode
{
public:
    SceneEditor2* GetScene();

private:
    friend class SceneManagerModule;
    SceneEditor2* scene = nullptr;

    DAVA_VIRTUAL_REFLECTION(SceneData, DAVA::TArc::DataNode)
    {
        DAVA::ReflectionRegistrator<SceneData>::Begin()
        .Field("Scene", &SceneData::scene)
        .End();
    }
};