#include "SceneViewData.h"

void SceneViewData::SetScene(DAVA::Scene* scene_)
{
    DAVA::SafeRetain(scene_);
    DAVA::SafeRelease(scene);
    scene = scene_;
}

DAVA::Scene* SceneViewData::GetScene() const
{
    return scene;
}
