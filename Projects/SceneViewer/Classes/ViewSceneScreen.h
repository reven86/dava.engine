#ifndef __VIEW_SCENE_SCREEN_H__
#define __VIEW_SCENE_SCREEN_H__

#include "BaseScreen.h"
#include "Menu.h"

#include "Scene3D/Systems/Controller/RotationControllerSystem.h"
#include "Scene3D/Systems/Controller/WASDControllerSystem.h"

class ViewSceneScreen : public BaseScreen, public UIFileSystemDialogDelegate
{
public:
    // UIScreen
    void LoadResources() override;
    void UnloadResources() override;

    // UIControl
    void Draw(const UIGeometricData& geometricData) override;
    void Update(float32 timeElapsed) override;

    // UIControl
    void Input(UIEvent* currentInput) override;

private:
    struct Sample
    {
        Vector3 pos;
        float32 angle = 0.0f;
        float32 sine = 0.0f;
        float32 cos = 0.0f;

        float32 fps = 0.0f;
    };

    class ScreenshotSaver
    {
    public:
        explicit ScreenshotSaver(FilePath& path, Sample& sample)
            : savePath(path)
            , sample(sample)
        {
        }

        void SaveTexture(Texture* screenshot)
        {
            ScopedPtr<Image> image(screenshot->CreateImageFromMemory());
            const Size2i& size = UIControlSystem::Instance()->vcs->GetPhysicalScreenSize();
            image->ResizeCanvas(uint32(size.dx), uint32(size.dy));
            image->Save(savePath);
            saved = true;
        }

        Sample sample;
        FilePath savePath;
        bool saved = false;
    };

    // UIFileSystemDialogDelegate
    void OnFileSelected(UIFileSystemDialog* forDialog, const FilePath& pathToFile) override;
    void OnFileSytemDialogCanceled(UIFileSystemDialog* forDialog) override;

    void OnButtonReloadShaders(BaseObject* caller, void* param, void* callerData);
    void OnButtonPerformanceTest(BaseObject* caller, void* param, void* callerData);
    void OnButtonSelectFromRes(BaseObject* caller, void* param, void* callerData);
    void OnButtonSelectFromDoc(BaseObject* caller, void* param, void* callerData);
    void OnButtonSelectFromExt(BaseObject* caller, void* param, void* callerData);

    void UpdateInfo(float32 timeElapsed);
    void UpdatePerformanceTest(float32 timeElapsed);
    void ProcessUserInput(float32 timeElapsed);

    void MakeScreenshot(ScreenshotSaver&);

    void SetCameraAtCenter(Camera* camera);

    void SetSamplePosition(Sample& sample);

    void ReloadScene();

    ScopedPtr<UI3DView> sceneView;

    DAVA::ScopedPtr<DAVA::UIStaticText> info;
    //DAVA::UIStaticText* dbg = nullptr;
    DAVA::ScopedPtr<DAVA::UIJoypad> moveJoyPAD;

    DAVA::float32 framesTime = 0.0f;
    DAVA::uint32 frameCounter = 0;

    DAVA::float32 framesTimePT = 0.0f;
    DAVA::uint32 frameCounterPT = 0;
    DAVA::uint32 fpsSum = 0;
    DAVA::uint32 sampleIndex = 0;

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

    enum class PT_State
    {
        Running,
        MakingScreenshots,
        Finished
    };
    PT_State performanceTestState = PT_State::Finished;

    Vector<Sample> samples;
    List<ScreenshotSaver> screenshotsToStart;
    List<ScreenshotSaver> screenshotsToSave;
    bool isEvenFrame = false;
    FilePath reportFolderPath;
    ScopedPtr<File> reportFile;

    std::unique_ptr<Menu> menu;
};

#endif //__VIEW_SCENE_SCREEN_H__
