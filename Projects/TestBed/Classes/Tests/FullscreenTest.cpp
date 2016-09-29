#include "Tests/FullscreenTest.h"
#include "Engine/EngineModule.h"

using namespace DAVA;

FullscreenTest::FullscreenTest(GameCore* g)
    : BaseScreen(g, "FullscreenTest")
{
}

void FullscreenTest::LoadResources()
{
#if defined(__DAVAENGINE_COREV2__)
    if (!isInit)
    {
        isInit = true;
        Engine::Instance()->PrimaryWindow()->focusChanged.Connect(this, &FullscreenTest::FocusChanged);
    }
#endif // defined(__DAVAENGINE_COREV2__)
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
#if !defined(__DAVAENGINE_COREV2__)
    btn->SetStateText(0xFF, L"Mouse Capute: Frame");
#else
    btn->SetStateText(0xFF, L"Mouse Visibility: false");
#endif // !defined(__DAVAENGINE_COREV2__)
    btn->SetDebugDraw(true);
    btn->SetTag(0);
    btn->AddEvent(UIButton::EVENT_TOUCH_DOWN, Message(this, &FullscreenTest::OnPinningClick));
    AddControl(btn);

    btn.reset(new UIButton(Rect(10, 135, 300, 20)));
    btn->SetStateFont(0xFF, font);
#if !defined(__DAVAENGINE_COREV2__)
    btn->SetStateText(0xFF, L"Mouse Mode: Pinning");
#else
    btn->SetStateText(0xFF, L"Mouse Capture Mode: Pinning");
#endif // !defined(__DAVAENGINE_COREV2__)
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
#if !defined(__DAVAENGINE_COREV2__)
    currentScaleText->SetText(Format(L"%f", Core::Instance()->GetScreenScaleMultiplier()));
#endif
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
#if !defined(__DAVAENGINE_COREV2__)
    UIButton* btn = static_cast<UIButton*>(sender);
    switch (btn->GetTag())
    {
    case 0:
        Core::Instance()->SetScreenMode(Core::eScreenMode::WINDOWED);
        break;
    case 1:
        Core::Instance()->SetScreenMode(Core::eScreenMode::FULLSCREEN);
        break;
    case 2:
        Core::Instance()->SetScreenMode(Core::eScreenMode::WINDOWED_FULLSCREEN);
        break;
    case 99:
        UpdateMode();
        break;
    }
#endif
}

void FullscreenTest::OnMulUp(BaseObject* sender, void* data, void* callerData)
{
#if !defined(__DAVAENGINE_COREV2__)
    float32 mul = Core::Instance()->GetScreenScaleMultiplier();
    if (mul < 2.0f)
    {
        mul += 0.1f;
    }

    Core::Instance()->SetScreenScaleMultiplier(mul);

    currentScaleText->SetText(Format(L"%f", mul));
#endif
}

void FullscreenTest::OnMulDown(BaseObject* sender, void* data, void* callerData)
{
#if !defined(__DAVAENGINE_COREV2__)
    float32 mul = Core::Instance()->GetScreenScaleMultiplier();
    if (mul > 0.2f)
    {
        mul -= 0.1f;
    }

    Core::Instance()->SetScreenScaleMultiplier(mul);

    currentScaleText->SetText(Format(L"%f", mul));
#endif
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
#if !defined(__DAVAENGINE_COREV2__)
        InputSystem::Instance()->GetMouseDevice().SetMode(eCaptureMode::FRAME);
#else
        mouseVisible = !Engine::Instance()->PrimaryWindow()->SetMouseVisibility(false);
#endif // !defined(__DAVAENGINE_COREV2__)
        break;

    case 1:
#if !defined(__DAVAENGINE_COREV2__)
        InputSystem::Instance()->GetMouseDevice().SetMode(eCaptureMode::PINING);
#else
        mouseCaptured = Engine::Instance()->PrimaryWindow()->SetCaptureMode(eCaptureMode::PINNING);
        mouseCaptured &= Engine::Instance()->PrimaryWindow()->SetMouseVisibility(false);
#endif // !defined(__DAVAENGINE_COREV2__)
        break;

    default:
        break;
    }

    UpdateMode();
}

void FullscreenTest::FocusChanged(Window& window, bool hasFocus)
{
    if (!hasFocus)
    {
        mouseCaptured = false;
        mouseVisible = true;
    }
    UpdateMode();
}

void FullscreenTest::UpdateMode()
{
#if !defined(__DAVAENGINE_COREV2__)
    switch (Core::Instance()->GetScreenMode())
    {
    case Core::eScreenMode::WINDOWED:
        currentModeText->SetText(L"Windowed");
        break;
    case Core::eScreenMode::WINDOWED_FULLSCREEN:
        currentModeText->SetText(L"Windowed fullscreen");
        break;
    case Core::eScreenMode::FULLSCREEN:
        currentModeText->SetText(L"Fullscreen");
        break;
    default:
        currentModeText->SetText(L"Unknown");
        break;
    }

    eCaptureMode captureMode = InputSystem::Instance()->GetMouseDevice().GetMode();
    switch (captureMode)
    {
    case eCaptureMode::OFF:
        pinningText->SetText(L"Mouse capture mode: OFF");
        pinningMousePosText->SetVisibilityFlag(false);
        break;

    case eCaptureMode::FRAME:
        pinningText->SetText(L"Mouse Capture = FRAME, press Mouse Button to turn off");
        pinningMousePosText->SetVisibilityFlag(true);
        break;

    case eCaptureMode::PINING:
        pinningText->SetText(L"Mouse Capture = PINING, press Mouse Button to turn off");
        pinningMousePosText->SetVisibilityFlag(true);
        break;
    }
#else
    WideString outStr;
    if (mouseCaptured)
    {
        outStr += L"Mouse Capture Mode = PINNING";
        outStr += L"\n";
        outStr += L"Mouse visibility = false";
        outStr += L"\n";
        outStr += L"press Middle Mouse Button to turn off";
        pinningMousePosText->SetVisibilityFlag(true);
    }
    else
    {
        outStr += L"Mouse Capture Mode mode: DEFAULT";
        outStr += L"\n";
        if (mouseVisible)
        {
            outStr += L"Mouse visibility = true";
            pinningMousePosText->SetVisibilityFlag(false);
        }
        else
        {
            outStr += L"Mouse visibility = false";
            outStr += L"\n";
            outStr += L"press Middle Mouse Button to turn off";
            pinningMousePosText->SetVisibilityFlag(true);
        }
    }
    pinningText->SetText(outStr.c_str());
#endif
}

bool FullscreenTest::SystemInput(UIEvent* currentInput)
{
#if !defined(__DAVAENGINE_COREV2__)
    if ((InputSystem::Instance()->GetMouseDevice().GetMode() != eCaptureMode::OFF) && (currentInput->device == UIEvent::Device::MOUSE))
    {
        switch (currentInput->phase)
        {
        case UIEvent::Phase::BEGAN:
        {
            InputSystem::Instance()->GetMouseDevice().SetMode(eCaptureMode::OFF);
            break;
        }
        case UIEvent::Phase::MOVE:
            pinningMousePosText->SetText(Format(L"dx: %f, dy: %f", currentInput->physPoint.dx, currentInput->physPoint.dy));
            break;

        default:
            break;
        }

        UpdateMode();
    }

    return BaseScreen::SystemInput(currentInput);
#else
    if (currentInput->device == UIEvent::Device::MOUSE)
    {
        Window* primWind = Engine::Instance()->PrimaryWindow();
        switch (currentInput->phase)
        {
        case UIEvent::Phase::BEGAN:
            if (currentInput->mouseButton == UIEvent::MouseButton::MIDDLE)
            {
                mouseCaptured = !primWind->SetCaptureMode(eCaptureMode::DEFAULT);
                mouseVisible = primWind->SetMouseVisibility(true);
            }
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
#endif
}
