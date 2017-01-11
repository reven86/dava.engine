#pragma once

#include "BaseScreen.h"
#include "Menu.h"

#include <GridTest.h>

#include <Scene3D/Systems/Controller/RotationControllerSystem.h>
#include <Scene3D/Systems/Controller/WASDControllerSystem.h>
#include <Utils/FpsMeter.h>

class ViewSceneScreen
: public BaseScreen,
  public UIFileSystemDialogDelegate,
  public GridTestListener
{
public:
    ViewSceneScreen(DAVA::Engine& engine);

    // UIScreen
    void LoadResources() override;
    void UnloadResources() override;

    // UIControl
    void Draw(const UIGeometricData& geometricData) override;
    void Update(float32 timeElapsed) override;

    // UIControl
    void Input(UIEvent* currentInput) override;

private:
    // UIFileSystemDialogDelegate
    void OnFileSelected(UIFileSystemDialog* forDialog, const FilePath& pathToFile) override;
    void OnFileSytemDialogCanceled(UIFileSystemDialog* forDialog) override;

    // GridTestListener
    void OnGridTestStateChanged() override;

    void AddSceneViewControl();
    void AddMenuControl();
    void AddFileDialogControl();
    void AddJoypadControl();
    void AddInfoTextControl();

    void AddControls();
    void RemoveControls();

    void OnButtonReloadShaders(BaseObject* caller, void* param, void* callerData);
    void OnButtonPerformanceTest(BaseObject* caller, void* param, void* callerData);
    void OnButtonSelectFromRes(BaseObject* caller, void* param, void* callerData);
    void OnButtonSelectFromDoc(BaseObject* caller, void* param, void* callerData);
    void OnButtonSelectFromExt(BaseObject* caller, void* param, void* callerData);

    void UpdateInfo(float32 timeElapsed);
    void UpdatePerformanceTest(float32 timeElapsed);
    void ProcessUserInput(float32 timeElapsed);

    void SetCameraAtCenter(Camera* camera);

    void LoadScene();
    void UnloadScene();
    void ReloadScene();

    ScopedPtr<UI3DView> sceneView;
    ScopedPtr<UIStaticText> infoText;
    ScopedPtr<UIJoypad> moveJoyPAD;
    ScopedPtr<UIFileSystemDialog> fileSystemDialog;
    std::unique_ptr<Menu> menu;

    DAVA::uint64 drawTime = 0;
    DAVA::uint64 updateTime = 0;

    DAVA::ScopedPtr<DAVA::Scene> scene;
    DAVA::RotationControllerSystem* rotationControllerSystem;
    DAVA::WASDControllerSystem* wasdSystem;

    Vector2 cursorPosition;
    float32 cursorSize = 0.1f;

    FilePath selectedScenePath;

    FpsMeter fpsMeter;

    GridTest gridTest;
};
