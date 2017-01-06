#pragma once

#include "BaseScreen.h"
#include "Menu.h"

#include <GridTest.h>

#include <Scene3D/Systems/Controller/RotationControllerSystem.h>
#include <Scene3D/Systems/Controller/WASDControllerSystem.h>

class ViewSceneScreen
: public BaseScreen
  ,
  public UIFileSystemDialogDelegate
  ,
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

    void AddMenuControl();
    void AddFileDialogControl();
    void AddJoypadControl();
    void AddInfoTextControl();

    void OnButtonReloadShaders(BaseObject* caller, void* param, void* callerData);
    void OnButtonPerformanceTest(BaseObject* caller, void* param, void* callerData);
    void OnButtonSelectFromRes(BaseObject* caller, void* param, void* callerData);
    void OnButtonSelectFromDoc(BaseObject* caller, void* param, void* callerData);
    void OnButtonSelectFromExt(BaseObject* caller, void* param, void* callerData);

    void UpdateInfo(float32 timeElapsed);
    void UpdatePerformanceTest(float32 timeElapsed);
    void ProcessUserInput(float32 timeElapsed);

    void SetCameraAtCenter(Camera* camera);

    void ReloadScene();

    ScopedPtr<UI3DView> sceneView;

    DAVA::ScopedPtr<DAVA::UIStaticText> info;
    DAVA::ScopedPtr<DAVA::UIJoypad> moveJoyPAD;

    DAVA::float32 framesTime = 0.0f;
    DAVA::uint32 frameCounter = 0;

    DAVA::uint64 drawTime = 0;
    DAVA::uint64 updateTime = 0;

    DAVA::ScopedPtr<DAVA::Scene> scene;
    std::unique_ptr<DAVA::RotationControllerSystem> rotationControllerSystem;
    std::unique_ptr<DAVA::WASDControllerSystem> wasdSystem;

    Vector2 cursorPosition;
    float32 cursorSize = 0.1f;

    bool selectSceneExtendedMode = false;

    ScopedPtr<UIFileSystemDialog> fileSystemDialog;
    FilePath selectedScenePath;

    FpsMeter fpsMeter;

    std::unique_ptr<Menu> menu;

    GridTest gridTest;
};
