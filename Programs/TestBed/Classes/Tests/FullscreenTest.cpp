#include "Tests/FullscreenTest.h"
#include "Infrastructure/TestBed.h"

#include <Engine/Engine.h>
#include <Engine/Window.h>
#include <Input/InputSystem.h>

using namespace DAVA;

FullscreenTest::FullscreenTest(TestBed& app)
    : BaseScreen(app, "FullscreenTest")
{
}

void FullscreenTest::LoadResources()
{
    BaseScreen::LoadResources();

    inputHandlerToken = GetEngineContext()->inputSystem->AddHandler(eInputDevices::CLASS_KEYBOARD, MakeFunction(this, &FullscreenTest::OnToggleFullscreen));

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

    currentModeText = new UIStaticText(Rect(310, 10, 300, 20));
    currentModeText->SetFont(font);
    currentModeText->SetTextColor(Color::White);
    AddControl(currentModeText);

    // pinning mode
    btn.reset(new UIButton(Rect(10, 85, 300, 20)));
    btn->SetStateFont(0xFF, font);
    btn->SetStateText(0xFF, L"Mouse Visibility: false");
    btn->SetDebugDraw(true);
    btn->SetTag(0);
    btn->AddEvent(UIButton::EVENT_TOUCH_DOWN, Message(this, &FullscreenTest::OnPinningClick));
    AddControl(btn);

    btn.reset(new UIButton(Rect(10, 110, 300, 20)));
    btn->SetStateFont(0xFF, font);
    btn->SetStateText(0xFF, L"Mouse Capture Mode: Pinning");
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

    btn.reset(new UIButton(Rect(10, 135, 145, 30)));
    btn->SetStateFont(0xFF, font);
    btn->SetStateText(0xFF, L"Mul +0.1");
    btn->SetDebugDraw(true);
    btn->AddEvent(UIButton::EVENT_TOUCH_UP_INSIDE, Message(this, &FullscreenTest::OnMulUp));
    AddControl(btn);

    btn.reset(new UIButton(Rect(155, 135, 145, 30)));
    btn->SetStateFont(0xFF, font);
    btn->SetStateText(0xFF, L"Mul -0.1");
    btn->SetDebugDraw(true);
    btn->AddEvent(UIButton::EVENT_TOUCH_UP_INSIDE, Message(this, &FullscreenTest::OnMulDown));
    AddControl(btn);

    currentScaleText = new UIStaticText(Rect(310, 125, 300, 30));
    currentScaleText->SetFont(font);
    currentScaleText->SetTextColor(Color::White);
    currentScaleText->SetText(Format(L"%f", GetPrimaryWindow()->GetSurfaceScale()));
    AddControl(currentScaleText);

    // UI3DView test

    ui3dview = new UI3DView(Rect(10, 175, 320, 240));
    ui3dview->SetDebugDraw(true);

    ScopedPtr<Scene> scene(new Scene());
    scene->LoadScene("~res:/3d/Objects/monkey.sc2");

    ScopedPtr<Camera> camera(new Camera());
    VirtualCoordinatesSystem* vcs = DAVA::UIControlSystem::Instance()->vcs;
    float32 aspect = static_cast<float32>(vcs->GetVirtualScreenSize().dy) / vcs->GetVirtualScreenSize().dx;
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

    btn.reset(new UIButton(Rect(340, 175, 145, 20)));
    btn->SetStateFont(0xFF, font);
    btn->SetStateText(0xFF, L"3d Scale +0.1");
    btn->SetDebugDraw(true);
    btn->SetTag(0);
    btn->AddEvent(UIButton::EVENT_TOUCH_DOWN, Message(this, &FullscreenTest::On3DViewControllClick));
    AddControl(btn);

    btn.reset(new UIButton(Rect(340, 205, 145, 20)));
    btn->SetStateFont(0xFF, font);
    btn->SetStateText(0xFF, L"3d Scale -0.1");
    btn->SetDebugDraw(true);
    btn->SetTag(1);
    btn->AddEvent(UIButton::EVENT_TOUCH_DOWN, Message(this, &FullscreenTest::On3DViewControllClick));
    AddControl(btn);

    currentScaleText = new UIStaticText(Rect(340, 235, 145, 20));
    currentScaleText->SetFont(font);
    currentScaleText->SetTextColor(Color::White);
    currentScaleText->SetText(Format(L"%f", ui3dview->GetFrameBufferScaleFactor()));
    AddControl(currentScaleText);

    btn.reset(new UIButton(Rect(340, 265, 145, 20)));
    btn->SetStateFont(0xFF, font);
    btn->SetStateText(0xFF, L"On draw to FBO");
    btn->SetDebugDraw(true);
    btn->SetTag(2);
    btn->AddEvent(UIButton::EVENT_TOUCH_DOWN, Message(this, &FullscreenTest::On3DViewControllClick));
    AddControl(btn);

    btn.reset(new UIButton(Rect(340, 295, 145, 20)));
    btn->SetStateFont(0xFF, font);
    btn->SetStateText(0xFF, L"Off draw to FBO");
    btn->SetDebugDraw(true);
    btn->SetTag(3);
    btn->AddEvent(UIButton::EVENT_TOUCH_DOWN, Message(this, &FullscreenTest::On3DViewControllClick));
    AddControl(btn);

    auto update = [this](Window*, Size2f, Size2f) { UpdateMode(); };
    sizeChangedSigConn = GetPrimaryWindow()->sizeChanged.Connect(update);

    UpdateMode();
}

void FullscreenTest::UnloadResources()
{
    GetEngineContext()->inputSystem->RemoveHandler(inputHandlerToken);

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

    // TODO: UIControls and others should be deleted when window is destroyed, not later
    if (GetPrimaryWindow() != nullptr)
    {
        GetPrimaryWindow()->sizeChanged.Disconnect(sizeChangedSigConn);
        sizeChangedSigConn = SigConnectionID();
    }

    BaseScreen::UnloadResources();
}

void FullscreenTest::OnSelectModeClick(BaseObject* sender, void* data, void* callerData)
{
    UIButton* btn = static_cast<UIButton*>(sender);
    switch (btn->GetTag())
    {
    case 0:
        GetPrimaryWindow()->SetFullscreenAsync(eFullscreen::Off);
        break;
    case 1:
        GetPrimaryWindow()->SetFullscreenAsync(eFullscreen::On);
        break;
    case 99:
        UpdateMode();
        break;
    }
}

void FullscreenTest::OnMulUp(BaseObject* sender, void* data, void* callerData)
{
    float32 mul = GetPrimaryWindow()->GetSurfaceScale();
    mul += 0.1f;

    GetPrimaryWindow()->SetSurfaceScaleAsync(mul);
    currentScaleText->SetText(Format(L"%f", mul));
}

void FullscreenTest::OnMulDown(BaseObject* sender, void* data, void* callerData)
{
    float32 mul = GetPrimaryWindow()->GetSurfaceScale();
    mul -= 0.1f;

    GetPrimaryWindow()->SetSurfaceScaleAsync(mul);
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
        GetPrimaryWindow()->SetCursorVisibility(false);
        break;
    case 1:
        GetPrimaryWindow()->SetCursorCapture(eCursorCapture::PINNING);
        break;
    default:
        break;
    }

    UpdateMode();
}

bool FullscreenTest::OnToggleFullscreen(DAVA::UIEvent* uie)
{
    if (uie->phase == UIEvent::Phase::KEY_UP)
    {
        Window* window = GetPrimaryWindow();
        if ((uie->key == Key::ENTER || uie->key == Key::NUMPADENTER) && (uie->modifiers & eModifierKeys::ALT) == eModifierKeys::ALT)
        {
            eFullscreen mode = window->GetFullscreen();
            mode = mode == eFullscreen::On ? eFullscreen::Off : eFullscreen::On;
            window->SetFullscreenAsync(mode);
        }
        else if (uie->key == Key::KEY_P && uie->modifiers == eModifierKeys::NONE)
        {
            eCursorCapture mode = window->GetCursorCapture();
            mode = mode == eCursorCapture::OFF ? eCursorCapture::PINNING : eCursorCapture::OFF;
            window->SetCursorCapture(mode);
        }
    }
    return false;
}

void FullscreenTest::UpdateMode()
{
    Window* w = GetPrimaryWindow();
    if (w->GetFullscreen() == eFullscreen::On)
    {
        currentModeText->SetText(L"Fullscreen");
    }
    else
    {
        currentModeText->SetText(L"Windowed");
    }

    WideString outStr;
    if (w->GetCursorCapture() == eCursorCapture::PINNING)
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
        outStr += L"Mouse Capture Mode mode: OFF";
        outStr += L"\n";
        if (w->GetCursorVisibility())
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
}

bool FullscreenTest::SystemInput(UIEvent* currentInput)
{
    if (currentInput->device == eInputDevices::MOUSE)
    {
        Window* w = GetPrimaryWindow();
        switch (currentInput->phase)
        {
        case UIEvent::Phase::BEGAN:
            if (currentInput->mouseButton == eMouseButtons::MIDDLE)
            {
                w->SetCursorCapture(eCursorCapture::OFF);
                w->SetCursorVisibility(true);
            }
            break;

        case UIEvent::Phase::MOVE:
        case UIEvent::Phase::DRAG:
            pinningMousePosText->SetText(Format(L"dx: %f, dy: %f, rel=%d", currentInput->physPoint.dx, currentInput->physPoint.dy, currentInput->isRelative));
            break;

        default:
            break;
        }

        UpdateMode();
    }

    return BaseScreen::SystemInput(currentInput);
}
