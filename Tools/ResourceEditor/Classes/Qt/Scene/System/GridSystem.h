#ifndef __SCENE_GRID_SYSTEM_H__
#define __SCENE_GRID_SYSTEM_H__

#include "Commands2/Base/Command2.h"

#include "Entity/SceneSystem.h"
#include "UI/UIEvent.h"
#include "Base/Introspection.h"

class SceneGridSystem : public DAVA::SceneSystem
{
    friend class SceneEditor2;

public:
    SceneGridSystem(DAVA::Scene* scene);
    ~SceneGridSystem();

    void Process(DAVA::float32 timeElapsed) override;

protected:
    void Draw();
};

#endif