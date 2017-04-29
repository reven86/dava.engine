#pragma once

#include "Classes/Qt/Scene/System/EditorSceneSystem.h"

#include <Entity/SceneSystem.h>
#include <Base/BaseTypes.h>

class OwnersSignatureSystem : public DAVA::SceneSystem, public EditorSceneSystem
{
public:
    OwnersSignatureSystem(DAVA::Scene* scene, const DAVA::String& userName);

    void AddEntity(DAVA::Entity* entity) override;
    void Process(DAVA::float32 timeElapsed) override;

private:
    void UpdateOwner(DAVA::Entity* entity);

    DAVA::String currentUserName = "nobody";
};
