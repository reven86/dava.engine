#ifndef __VIEW_SCENE_SCREEN_H__
#define __VIEW_SCENE_SCREEN_H__

#include "BaseScreen.h"

namespace DAVA
{
class RotationControllerSystem;
class WASDControllerSystem;
};

class ViewSceneScreen : public BaseScreen
{
protected:
    virtual ~ViewSceneScreen()
    {
    }

public:
    virtual void LoadResources();
    virtual void UnloadResources();

    virtual void Draw(const UIGeometricData& geometricData);
    virtual void Update(float32 timeElapsed);

    virtual void DidAppear();

    virtual void Input(UIEvent* currentInput);

protected:
    void OnBack(BaseObject* caller, void* param, void* callerData);
    void OnReloadShaders(BaseObject* caller, void* param, void* callerData);
    void UpdateInfo(float32 timeElapsed);

    DAVA::UIStaticText* info = nullptr;
    DAVA::UIJoypad* moveJoyPAD = nullptr;

    DAVA::float32 framesTime = 0.0f;
    DAVA::uint32 frameCounter = 0;

    DAVA::uint64 drawTime = 0;
    DAVA::uint64 updateTime = 0;

    DAVA::Scene* scene = nullptr;
    DAVA::RotationControllerSystem* rotationControllerSystem = nullptr;
    DAVA::WASDControllerSystem* wasdSystem = nullptr;

    Vector2 cursorPosition;
    float32 cursorSize = 0.1f;
};

#endif //__VIEW_SCENE_SCREEN_H__
