#pragma once

#include "Entity/SceneSystem.h"
#include "UI/UIEvent.h"
#include "Base/Introspection.h"

#include "Classes/Qt/Scene/System/EditorSceneSystem.h"

class SceneGridSystem : public DAVA::SceneSystem, public EditorSceneSystem
{
public:
    SceneGridSystem(DAVA::Scene* scene);

protected:
    void Draw() override;
};
