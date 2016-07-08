#ifndef __SCENE_PREVIEW_CONTROL_H__
#define __SCENE_PREVIEW_CONTROL_H__

#include "DAVAEngine.h"
#include "Scene3D/Systems/Controller/RotationControllerSystem.h"

class ScenePreviewControl : public DAVA::UI3DView
{
public:
    enum eError
    {
        ERROR_WRONG_EXTENSION = 100,
        ERROR_CANNOT_OPEN_FILE = 101
    };

public:
    ScenePreviewControl(const DAVA::Rect& rect);
    virtual ~ScenePreviewControl();

    virtual void Input(DAVA::UIEvent* touch);
    virtual void Update(DAVA::float32 timeElapsed);

    DAVA::int32 OpenScene(const DAVA::FilePath& pathToFile);
    void ReleaseScene();
    void RecreateScene();

private:
    void CreateCamera();
    void SetupCamera();

private:
    //scene controls
    DAVA::Scene* editorScene = nullptr;
    DAVA::RotationControllerSystem* rotationSystem = nullptr;
    DAVA::FilePath currentScenePath;
    bool needSetCamera = false;
};

#endif // __SCENE_PREVIEW_CONTROL_H__
