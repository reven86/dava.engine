#include "SceneViewData.h"

void SceneViewData::SetScene(DAVA::Scene* scene_)
{
    DAVA::SafeRelease(scene);
    scene = scene_;
    DAVA::SafeRetain(scene);
}

DAVA::Scene* SceneViewData::GetScene() const
{
    return scene;
}
