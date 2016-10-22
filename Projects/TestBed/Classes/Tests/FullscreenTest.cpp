#include "Tests/FullscreenTest.h"
#include "Infrastructure/TestBed.h"

#include <Engine/Engine.h>
#include <Engine/Window.h>

using namespace DAVA;

FullscreenTest::FullscreenTest(TestBed& app)
    : BaseScreen(app, "FullscreenTest")
    , primaryWindow(app.GetEngine().PrimaryWindow())
{
}

void FullscreenTest::LoadResources()
{
    BaseScreen::LoadResources();

    GetBackground()->SetColor(Color::White);

    ScopedPtr<Font> font(FTFont::Create("~res:/Fonts/korinna.ttf"));

    // Screen mode test
    ScopedPtr<UIButton> btn(new UIButton(Rect(10, 10, 300, 20)));
    btn->SetStateFont(0xFF, font);
    btn->SetStateText(0xFF, L"Refresh status");
    btn->SetDebugDraw(true);
    btn->SetTag(99);
    btn->AddEvent(UIButton::EVENT_TOUCH_DOWN, Message(this, &FullscreenTest::OnSelectModeClick));
    AddControl(btn);

    btn.reset(new UIButton(Rect(10, 35, 300, 20)));
    btn->SetStateFont(0xFF, font);
    btn->SetStateText(0xFF, L"Windowed");
    btn->SetDebugDraw(true);
    btn->SetTag(0);
    btn->AddEvent(UIButton::EVENT_TOUCH_DOWN, Message(this, &FullscreenTest::OnSelectModeClick));
    AddControl(btn);

    btn.reset(new UIButton(Rect(10, 60, 300, 20)));
    btn->SetStateFont(0xFF, font);
    btn->SetStateText(0xFF, L"Fullsreen");
    btn->SetDebugDraw(true);
    btn->SetTag(1);
    btn->AddEvent(UIButton::EVENT_TOUCH_DOWN, Message(this, &FullscreenTest::OnSelectModeClick));
    AddControl(btn);

    btn.reset(new UIButton(Rect(10, 85, 300, 20)));
    btn->SetStateFont(0xFF, font);
    btn->SetStateText(0xFF, L"Windowed fullscreen (borderless)");
    btn->SetDebugDraw(true);
    btn->SetTag(2);
    btn->AddEvent(UIButton::EVENT_TOUCH_DOWN, Message(this, &FullscreenTest::OnSelectModeClick));
    AddControl(btn);

    currentModeText = new UIStaticText(Rect(310, 10, 300, 20));
    currentModeText->SetFont(font);
    currentModeText->SetTextColor(Color::White);
    AddControl(currentModeText);

    // pinning mode
    btn.reset(new UIButton(Rect(10, 110, 300, 20)));
    btn->SetStateFont(0xFF, font);
    btn->SetStateText(0xFF, L"Mouse Capute: Frame");
    btn->SetDebugDraw(true);
    btn->SetTag(0);
    btn->AddEvent(UIButton::EVENT_TOUCH_DOWN, Message(this, &FullscreenTest::OnPinningClick));
    AddControl(btn);

    btn.reset(new UIButton(Rect(10, 135, 300, 20)));
    btn->SetStateFont(0xFF, font);
    btn->SetStateText(0xFF, L"Mouse Capute: Pining");
    btn->SetDebugDraw(true);
    btn->SetTag(1);
    btn->AddEvent(UIButton::EVENT_TOUCH_DOWN, Message(this, &FullscreenTest::OnPinningClick));
    AddControl(btn);

    pinningText = new UIStaticText(Rect(310, 30, 300, 35));
    pinningText->SetFont(font);
    pinningText->SetMultiline(true);
    pinningText->SetTextColor(Color::White);
    AddControl(pinningText);

    pinningMousePosText = new UIStaticText(Rect(310, 70, 300, 20));
    pinningMousePosText->SetFont(font);
    pinningMousePosText->SetTextColor(Color(0.5f, 0.5f, .0f, 1.0f));
    pinningMousePosText->SetVisibilityFlag(false);
    AddControl(pinningMousePosText);

    // Scale factor test

    btn.reset(new UIButton(Rect(10, 160, 145, 30)));
    btn->SetStateFont(0xFF, font);
    btn->SetStateText(0xFF, L"Mul +0.1");
    btn->SetDebugDraw(true);
    btn->AddEvent(UIButton::EVENT_TOUCH_DOWN, Message(this, &FullscreenTest::OnMulUp));
    AddControl(btn);

    btn.reset(new UIButton(Rect(155, 160, 145, 30)));
    btn->SetStateFont(0xFF, font);
    btn->SetStateText(0xFF, L"Mul -0.1");
    btn->SetDebugDraw(true);
    btn->AddEvent(UIButton::EVENT_TOUCH_DOWN, Message(this, &FullscreenTest::OnMulDown));
    AddControl(btn);

    currentScaleText = new UIStaticText(Rect(310, 150, 300, 30));
    currentScaleText->SetFont(font);
    currentScaleText->SetTextColor(Color::White);
    currentScaleText->SetText(Format(L"%f", primaryWindow->GetUserScale()));
    AddControl(currentScaleText);

    // UI3DView test

    ui3dview = new UI3DView(Rect(10, 200, 320, 240));
    ui3dview->SetDebugDraw(true);

    ScopedPtr<Scene> scene(new Scene());
    scene->LoadScene("~res:/3d/Objects/monkey.sc2");

    ScopedPtr<Camera> camera(new Camera());
    VirtualCoordinatesSystem* vcs = DAVA::VirtualCoordinatesSystem::Instance();
    float32 aspect = (float32)vcs->GetVirtualScreenSize().dy / (float32)vcs->GetVirtualScreenSize().dx;
    camera->SetupPerspective(70.f, aspect, 0.5f, 2500.f);
    camera->SetLeft(Vector3(1, 0, 0));
    camera->SetUp(Vector3(0, 0, 1.f));
    camera->SetTarget(Vector3(0, 0, 0));
    camera->SetPosition(Vector3(0, -10, 1));

    ScopedPtr<Entity> cameraEntity(new Entity());
    cameraEntity->AddComponent(new CameraComponent(camera));
    cameraEntity->AddComponent(new RotationControllerComponent());
    scene->AddNode(cameraEntity);

    rotationControllerSystem = new RotationControllerSystem(scene);
    scene->AddSystem(rotationControllerSystem,
                     MAKE_COMPONENT_MASK(Component::CAMERA_COMPONENT) | MAKE_COMPONENT_MASK(Component::ROTATION_CONTROLLER_COMPONENT),
                     Scene::SCENE_SYSTEM_REQUIRE_PROCESS | Scene::SCENE_SYSTEM_REQUIRE_INPUT);

    scene->AddCamera(camera);
    scene->SetCurrentCamera(camera);
    ui3dview->SetScene(scene);
    AddControl(ui3dview);

    btn.reset(new UIButton(Rect(340, 200, 145, 20)));
    btn->SetStateFont(0xFF, font);
    btn->SetStateText(0xFF, L"3d Scale +0.1");
    btn->SetDebugDraw(true);
    btn->SetTag(0);
    btn->AddEvent(UIButton::EVENT_TOUCH_DOWN, Message(this, &FullscreenTest::On3DViewControllClick));
    AddControl(btn);

    btn.reset(new UIButton(Rect(340, 230, 145, 20)));
    btn->SetStateFont(0xFF, font);
    btn->SetStateText(0xFF, L"3d Scale -0.1");
    btn->SetDebugDraw(true);
    btn->SetTag(1);
    btn->AddEvent(UIButton::EVENT_TOUCH_DOWN, Message(this, &FullscreenTest::On3DViewControllClick));
    AddControl(btn);

    currentScaleText = new UIStaticText(Rect(340, 260, 145, 20));
    currentScaleText->SetFont(font);
    currentScaleText->SetTextColor(Color::White);
    currentScaleText->SetText(Format(L"%f", ui3dview->GetFrameBufferScaleFactor()));
    AddControl(currentScaleText);

    btn.reset(new UIButton(Rect(340, 290, 145, 20)));
    btn->SetStateFont(0xFF, font);
    btn->SetStateText(0xFF, L"On draw to FBO");
    btn->SetDebugDraw(true);
    btn->SetTag(2);
    btn->AddEvent(UIButton::EVENT_TOUCH_DOWN, Message(this, &FullscreenTest::On3DViewControllClick));
    AddControl(btn);

    btn.reset(new UIButton(Rect(340, 320, 145, 20)));
    btn->SetStateFont(0xFF, font);
    btn->SetStateText(0xFF, L"Off draw to FBO");
    btn->SetDebugDraw(true);
    btn->SetTag(3);
    btn->AddEvent(UIButton::EVENT_TOUCH_DOWN, Message(this, &FullscreenTest::On3DViewControllClick));
    AddControl(btn);

    UpdateMode();
}

void FullscreenTest::UnloadResources()
{
    if (ui3dview->GetScene())
    {
        ui3dview->GetScene()->RemoveSystem(rotationControllerSystem);
    }
    SafeDelete(rotationControllerSystem);
    SafeRelease(ui3dview);
    SafeRelease(currect3dScaleText);
    SafeRelease(currentModeText);
    SafeRelease(pinningText);
    SafeRelease(pinningMousePosText);
    BaseScreen::UnloadResources();
}

void FullscreenTest::OnSelectModeClick(BaseObject* sender, void* data, void* callerData)
{
    UIButton* btn = static_cast<UIButton*>(sender);
    switch (btn->GetTag())
    {
    case 0:
        // TODO: implement window mode switch in engine and testbed
        // Core::Instance()->SetScreenMode(Core::eScreenMode::WINDOWED);
        break;
    case 1:
        // TODO: implement window mode switch in engine and testbed
        // Core::Instance()->SetScreenMode(Core::eScreenMode::FULLSCREEN);
        break;
    case 2:
        // TODO: implement window mode switch in engine and testbed
        // Core::Instance()->SetScreenMode(Core::eScreenMode::WINDOWED_FULLSCREEN);
        break;
    case 99:
        UpdateMode();
        break;
    }
}

void FullscreenTest::OnMulUp(BaseObject* sender, void* data, void* callerData)
{
    float32 mul = primaryWindow->GetUserScale();
    if (mul < 2.0f)
    {
        mul += 0.1f;
    }

    // TODO: implement window user scale factor in engine and testbed
    // Core::Instance()->SetScreenScaleMultiplier(mul);

    mul = primaryWindow->GetUserScale();
    currentScaleText->SetText(Format(L"%f", mul));
}

void FullscreenTest::OnMulDown(BaseObject* sender, void* data, void* callerData)
{
    float32 mul = primaryWindow->GetUserScale();
    if (mul > 0.2f)
    {
        mul -= 0.1f;
    }

    // TODO: implement window user scale factor in engine and testbed
    // Core::Instance()->SetScreenScaleMultiplier(mul);

    mul = primaryWindow->GetUserScale();
    currentScaleText->SetText(Format(L"%f", mul));
}

void FullscreenTest::On3DViewControllClick(BaseObject* sender, void* data, void* callerData)
{
    UIButton* btn = static_cast<UIButton*>(sender);
    switch (btn->GetTag())
    {
    case 0: // + scale
    {
        float32 mul = ui3dview->GetFrameBufferScaleFactor();
        if (mul < 2.0f)
        {
            mul += 0.1f;
        }
        ui3dview->SetFrameBufferScaleFactor(mul);
        currentScaleText->SetText(Format(L"%f", mul));
        break;
    }
    case 1: // - scale
    {
        float32 mul = ui3dview->GetFrameBufferScaleFactor();
        if (mul > 0.2f)
        {
            mul -= 0.1f;
        }
        ui3dview->SetFrameBufferScaleFactor(mul);
        currentScaleText->SetText(Format(L"%f", mul));
        break;
    }
    case 2: // turn on
        ui3dview->SetDrawToFrameBuffer(true);
        break;
    case 3: // turn off
        ui3dview->SetDrawToFrameBuffer(false);
        break;
    }
}

void FullscreenTest::OnPinningClick(DAVA::BaseObject* sender, void* data, void* callerData)
{
    UIButton* btn = static_cast<UIButton*>(sender);
    switch (btn->GetTag())
    {
    case 0:
        InputSystem::Instance()->GetMouseDevice().SetMode(eCaptureMode::FRAME);
        break;

    case 1:
        InputSystem::Instance()->GetMouseDevice().SetMode(eCaptureMode::PINING);
        break;

    default:
        break;
    }

    UpdateMode();
}

void FullscreenTest::UpdateMode()
{
    // TODO: implement window mode switch in engine and testbed

    // switch (Core::Instance()->GetScreenMode())
    // {
    // case Core::eScreenMode::WINDOWED:
    //     currentModeText->SetText(L"Windowed");
    //     break;
    // case Core::eScreenMode::WINDOWED_FULLSCREEN:
    //     currentModeText->SetText(L"Windowed fullscreen");
    //     break;
    // case Core::eScreenMode::FULLSCREEN:
    //     currentModeText->SetText(L"Fullscreen");
    //     break;
    // default:
    //     currentModeText->SetText(L"Unknown");
    //     break;
    // }
    //
    // eCaptureMode captureMode = InputSystem::Instance()->GetMouseDevice().GetMode();
    // switch (captureMode)
    // {
    // case eCaptureMode::OFF:
    //     pinningText->SetText(L"Mouse capture mode: OFF");
    //     pinningMousePosText->SetVisibilityFlag(false);
    //     break;
    //
    // case eCaptureMode::FRAME:
    //     pinningText->SetText(L"Mouse Capture = FRAME, press Mouse Button to turn off");
    //     pinningMousePosText->SetVisibilityFlag(true);
    //     break;
    //
    // case eCaptureMode::PINING:
    //     pinningText->SetText(L"Mouse Capture = PINING, press Mouse Button to turn off");
    //     pinningMousePosText->SetVisibilityFlag(true);
    //     break;
    // }
}

bool FullscreenTest::SystemInput(UIEvent* currentInput)
{
    if ((InputSystem::Instance()->GetMouseDevice().GetMode() != eCaptureMode::OFF) && (currentInput->device == UIEvent::Device::MOUSE))
    {
        switch (currentInput->phase)
        {
        case UIEvent::Phase::BEGAN:
            InputSystem::Instance()->GetMouseDevice().SetMode(eCaptureMode::OFF);
            break;

        case UIEvent::Phase::MOVE:
            pinningMousePosText->SetText(Format(L"dx: %f, dy: %f", currentInput->physPoint.dx, currentInput->physPoint.dy));
            break;

        default:
            break;
        }

        UpdateMode();
    }

    return BaseScreen::SystemInput(currentInput);
}
