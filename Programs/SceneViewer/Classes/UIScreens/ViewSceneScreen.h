#pragma once

#include "SceneViewerApp.h"
#include "BaseScreen.h"
#include "UIControls/Menu.h"

#include <GridTest.h>

#include <Scene3D/Systems/Controller/RotationControllerSystem.h>
#include <Scene3D/Systems/Controller/WASDControllerSystem.h>
#include <Utils/FpsMeter.h>

class ViewSceneScreen
: public BaseScreen,
  public DAVA::UIFileSystemDialogDelegate,
  public GridTestListener
{
public:
    ViewSceneScreen(SceneViewerData& data);

    // UIScreen
    void LoadResources() override;
    void UnloadResources() override;

    // UIControl
    void Draw(const DAVA::UIGeometricData& geometricData) override;
    void Update(DAVA::float32 timeElapsed) override;

    // UIControl
    void Input(DAVA::UIEvent* currentInput) override;

private:
    // UIFileSystemDialogDelegate
    void OnFileSelected(DAVA::UIFileSystemDialog* forDialog, const DAVA::FilePath& pathToFile) override;
    void OnFileSytemDialogCanceled(DAVA::UIFileSystemDialog* forDialog) override;

    // GridTestListener
    void OnGridTestStateChanged() override;

    void AddSceneViewControl();
    void AddMenuControl();
    void AddFileDialogControl();
    void AddJoypadControl();
    void AddInfoTextControl();

    void AddControls();
    void RemoveControls();

    void OnButtonReloadShaders(DAVA::BaseObject* caller, void* param, void* callerData);
    void OnButtonPerformanceTest(DAVA::BaseObject* caller, void* param, void* callerData);
    void OnButtonSelectFromRes(DAVA::BaseObject* caller, void* param, void* callerData);
    void OnButtonSelectFromDoc(DAVA::BaseObject* caller, void* param, void* callerData);
    void OnButtonSelectFromExt(DAVA::BaseObject* caller, void* param, void* callerData);

    void UpdateInfo(DAVA::float32 timeElapsed);
    void UpdatePerformanceTest(DAVA::float32 timeElapsed);
    void ProcessUserInput(DAVA::float32 timeElapsed);

    void SetCameraAtCenter(DAVA::Camera* camera);

    void LoadScene();
    void UnloadScene();
    void ReloadScene();

    SceneViewerData& data;
    DAVA::FilePath& scenePath;
    DAVA::ScopedPtr<DAVA::Scene> scene;
    DAVA::ScopedPtr<DAVA::UI3DView> sceneView;

    DAVA::ScopedPtr<DAVA::UIStaticText> infoText;
    DAVA::ScopedPtr<DAVA::UIJoypad> moveJoyPAD;
    DAVA::ScopedPtr<DAVA::UIFileSystemDialog> fileSystemDialog;
    std::unique_ptr<Menu> menu;

    //     DAVA::uint64 drawTime = 0;
    //     DAVA::uint64 updateTime = 0;

    DAVA::RotationControllerSystem* rotationControllerSystem;
    DAVA::WASDControllerSystem* wasdSystem;

    //     Vector2 cursorPosition;
    //     float32 cursorSize = 0.1f;

    DAVA::FpsMeter fpsMeter;

    GridTest gridTest;
};
