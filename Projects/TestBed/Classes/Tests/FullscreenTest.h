#pragma once

#include "DAVAEngine.h"
#include "Infrastructure/BaseScreen.h"
#include "Scene3D/Systems/Controller/RotationControllerSystem.h"

namespace DAVA
{
class Window;
}

class TestBed;
class FullscreenTest : public BaseScreen
{
public:
    FullscreenTest(TestBed& app);

protected:
    void LoadResources() override;
    void UnloadResources() override;

    bool SystemInput(DAVA::UIEvent* currentInput) override;

private:
    void UpdateMode();
    void OnSelectModeClick(DAVA::BaseObject* sender, void* data, void* callerData);
    void OnMulUp(DAVA::BaseObject* sender, void* data, void* callerData);
    void OnMulDown(DAVA::BaseObject* sender, void* data, void* callerData);
    void On3DViewControllClick(DAVA::BaseObject* sender, void* data, void* callerData);
    void OnPinningClick(DAVA::BaseObject* sender, void* data, void* callerData);
    void FocusChanged(DAVA::Window* window, bool hasFocus);

    DAVA::Window* primaryWindow = nullptr;

    DAVA::UIStaticText* currentModeText;
    DAVA::UIStaticText* currentScaleText;
    DAVA::UI3DView* ui3dview = nullptr;
    DAVA::RotationControllerSystem* rotationControllerSystem = nullptr;
    DAVA::UIStaticText* currect3dScaleText = nullptr;
    DAVA::UIStaticText* pinningText = nullptr;
    DAVA::UIStaticText* pinningMousePosText = nullptr;

    bool mouseCaptured = false;
    bool mouseVisible = true;
    bool isInit = false;
};
