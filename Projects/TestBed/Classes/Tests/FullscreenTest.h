#ifndef __FULLSCREENTEST_TEST_H__
#define __FULLSCREENTEST_TEST_H__

#include "DAVAEngine.h"
#include "Infrastructure/BaseScreen.h"
#include "Scene3D/Systems/Controller/RotationControllerSystem.h"

using namespace DAVA;

class FullscreenTest : public BaseScreen
{
public:
    FullscreenTest();

protected:
    void LoadResources() override;
    void UnloadResources() override;

    bool SystemInput(UIEvent* currentInput) override;

private:
    void UpdateMode();
    void OnSelectModeClick(BaseObject* sender, void* data, void* callerData);
    void OnMulUp(BaseObject* sender, void* data, void* callerData);
    void OnMulDown(BaseObject* sender, void* data, void* callerData);
    void On3DViewControllClick(BaseObject* sender, void* data, void* callerData);
    void OnPinningClick(BaseObject* sender, void* data, void* callerData);

    UIStaticText* currentModeText;
    UIStaticText* currentScaleText;
    UI3DView* ui3dview = nullptr;
    RotationControllerSystem* rotationControllerSystem = nullptr;
    UIStaticText* currect3dScaleText = nullptr;
    UIStaticText* pinningText = nullptr;
    UIStaticText* pinningMousePosText = nullptr;
};

#endif //__FULLSCREENTEST_TEST_H__
