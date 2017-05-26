#include "Infrastructure/TestBed.h"
#include "Tests/KeyboardTest.h"

#include <Engine/Engine.h>
#include <Input/InputCallback.h>
#include <UI/Focus/UIFocusComponent.h>
#include <UI/Render/UIDebugRenderComponent.h>
#include <Render/2D/Sprite.h>

using namespace DAVA;

struct Finger
{
    int32 index = 0;
    UIControl* img = nullptr;
    bool isActive = false;
};

Array<Finger, 10> touches;
Vector2 hiddenPos(-100, -100);

auto gamepadButtonsNames =
{ "button_a", "button_b", "button_x", "button_y", "button_left", "button_right",
  "button_up", "button_down", "button_select", "button_start",
  "shift_left", "shift_right", "triger_left", "triger_right",
  "stick_left", "stick_right" };

Map<String, UIControl*> gamepadButtons;

Rect gamepadPos(500, 000, 800, 450);
float32 gamepadStickDeltaMove = 20.f; // 20 pixels

KeyboardTest::KeyboardTest(TestBed& app)
    : BaseScreen(app, "KeyboardTest")
    , app(app)
{
}

void KeyboardTest::LoadResources()
{
    BaseScreen::LoadResources();

    ScopedPtr<FTFont> font(FTFont::Create("~res:/Fonts/korinna.ttf"));

    previewText = new UIStaticText(Rect(20, 30, 400, 200));
    previewText->SetFont(font);
    previewText->SetTextColor(Color::White);
    previewText->SetMultiline(true);
    previewText->SetText(L"Press (Hold) and Unpress keys\nOn MacOS test gestures magnify/rotate/swipe");
    previewText->GetOrCreateComponent<UIDebugRenderComponent>();
    previewText->SetTextAlign(ALIGN_LEFT | ALIGN_TOP);
    AddControl(previewText);

    descriptionText = new UIStaticText(Rect(20, 230, 200, 400));
    descriptionText->GetOrCreateComponent<UIDebugRenderComponent>();
    descriptionText->SetTextColor(Color::White);
    descriptionText->SetFont(font);
    descriptionText->GetOrCreateComponent<UIFocusComponent>();
    UIControlBackground* descriptionTextBg = descriptionText->GetOrCreateComponent<UIControlBackground>();
    descriptionTextBg->SetAlign(ALIGN_LEFT | ALIGN_TOP);
    descriptionText->SetTextAlign(ALIGN_LEFT | ALIGN_TOP);
    descriptionText->SetMultiline(true);
    descriptionText->SetMultilineType(UIStaticText::MULTILINE_ENABLED_BY_SYMBOL);
    AddControl(descriptionText);

    InputSystem* inputSystem = app.GetEngine().GetContext()->inputSystem;
    pointerInputToken = inputSystem->AddHandler(eInputDevices::CLASS_POINTER, MakeFunction(this, &KeyboardTest::OnPointerEvent));
    keyboardInputToken = inputSystem->AddHandler(eInputDevices::CLASS_KEYBOARD, MakeFunction(this, &KeyboardTest::OnKeyboardEvent));
    gamepadInputToken = inputSystem->AddHandler(eInputDevices::CLASS_GAMEPAD, MakeFunction(this, &KeyboardTest::OnGamepadEvent));

    resetButton = new UIButton(Rect(420, 30, 50, 30));
    resetButton->GetOrCreateComponent<UIDebugRenderComponent>();
    resetButton->SetStateFont(0xFF, font);
    resetButton->SetStateFontColor(0xFF, Color::White);
    resetButton->SetStateText(0xFF, L"Reset");
    resetButton->AddEvent(UIButton::EVENT_TOUCH_UP_INSIDE, Message(this, &KeyboardTest::OnResetClick));
    AddControl(resetButton);

    for (auto& touch : touches)
    {
        touch.img = new UIButton(Rect(0, 0, 50, 50));
        touch.img->GetOrCreateComponent<UIDebugRenderComponent>();
        touch.img->SetInputEnabled(false);
        touch.img->SetPosition(hiddenPos);
        auto back = touch.img->GetBackground();
        int red = std::rand() % 256;
        int green = std::rand() % 256;
        int blue = std::rand() % 256;
        int color = 0xFF000000 | (blue << 16) | (green << 8) | (red);
        back->SetDrawColor(color);
        back->SetColor(color);
        back->SetDrawType(UIControlBackground::eDrawType::DRAW_FILL);

        AddControl(touch.img);
    }

    redBox = new UIButton(Rect(512, 512, 128, 128));
    redBox->SetPivotPoint(Vector2(64.f, 64.f));
    redBox->SetInputEnabled(false);
    redBox->GetOrCreateComponent<UIDebugRenderComponent>();
    auto boxBack = redBox->GetBackground();
    boxBack->SetDrawColor(Color(1.f, 0.f, 0.f, 1.f));
    boxBack->SetColor(Color(1.f, 0.f, 0.f, 1.f));
    boxBack->SetDrawType(UIControlBackground::eDrawType::DRAW_FILL);
    AddControl(redBox);

    gamepad = new UIControl(gamepadPos);
    FilePath pathToBack("~res:/TestData/GamepadTest/gamepad.png");
    ScopedPtr<Sprite> gamepadSprite(Sprite::CreateFromSourceFile(pathToBack));
    UIControlBackground* bg = gamepad->GetOrCreateComponent<UIControlBackground>();
    bg->SetModification(ESM_VFLIP | ESM_HFLIP);
    bg->SetSprite(gamepadSprite, 0);
    AddControl(gamepad);

    for (auto& buttonOrAxisName : gamepadButtonsNames)
    {
        UIControl* img = new UIControl(gamepadPos);
        auto path = FilePath("~res:/TestData/GamepadTest/") + buttonOrAxisName + ".png";
        UIControlBackground* bg = img->GetOrCreateComponent<UIControlBackground>();
        bg->SetModification(ESM_VFLIP | ESM_HFLIP);

        ScopedPtr<Sprite> sprite(Sprite::CreateFromSourceFile(path));
        bg->SetSprite(sprite, 0);
        gamepadButtons[buttonOrAxisName] = img;
        AddControl(img);
        img->SetVisibilityFlag(false);
    }
}

void KeyboardTest::UnloadResources()
{
    InputSystem* inputSystem = app.GetEngine().GetContext()->inputSystem;
    inputSystem->RemoveHandler(pointerInputToken);
    inputSystem->RemoveHandler(keyboardInputToken);
    inputSystem->RemoveHandler(gamepadInputToken);

    SafeRelease(previewText);
    SafeRelease(descriptionText);
    SafeRelease(resetButton);
    SafeRelease(redBox);

    for (auto& touch : touches)
    {
        SafeRelease(touch.img);
    }

    SafeRelease(gamepad);

    for (auto& gamepadButton : gamepadButtons)
    {
        SafeRelease(gamepadButton.second);
    }

    BaseScreen::UnloadResources();
}

void KeyboardTest::OnResetClick(BaseObject* sender, void* data, void* callerData)
{
    ResetCounters();
    descriptionText->SetText(L"");
}

void KeyboardTest::ResetCounters()
{
    numKeyboardEvents = 0;
    numKeyDown = 0;
    numKeyUp = 0;
    numKeyDownRepeat = 0;
    numChar = 0;
    numCharRepeat = 0;
    lastChar = L'\0';

    numMouseEvents = 0;
    numDrag = 0;
    numMouseMove = 0;
    numMouseDown = 0;
    numMouseUp = 0;
    numMouseWheel = 0;
    numMouseCancel = 0;
    numMouseDblUp = 0;
    numMouseDblDown = 0;
    lastMouseKey = L'\0';
    lastMouseX = 0;
    lastMouseY = 0;
    lastWheel = 0.f;
}

bool KeyboardTest::OnPointerEvent(UIEvent* e)
{
    if (e->phase == UIEvent::Phase::GESTURE)
    {
        OnGestureEvent(e);
    }
    else
    {
        OnMouseTouchOrKeyboardEvent(e);
    }
    return false;
}

bool KeyboardTest::OnKeyboardEvent(UIEvent* e)
{
    OnMouseTouchOrKeyboardEvent(e);
    return false;
}

bool KeyboardTest::OnGamepadEvent(UIEvent* event)
{
    //Logger::Info("gamepad tid: %2d, x: %.3f, y:%.3f", event->tid, event->point.x, event->point.y);

    DVASSERT(event->device == eInputDevices::GAMEPAD);
    DVASSERT(event->phase == UIEvent::Phase::JOYSTICK);

    switch (event->element)
    {
    case eGamepadElements::A:
        UpdateGamepadElement("button_a", event->point.x == 1);
        break;
    case eGamepadElements::B:
        UpdateGamepadElement("button_b", event->point.x == 1);
        break;
    case eGamepadElements::X:
        UpdateGamepadElement("button_x", event->point.x == 1);
        break;
    case eGamepadElements::Y:
        UpdateGamepadElement("button_y", event->point.x == 1);
        break;
    case eGamepadElements::LEFT_SHOULDER:
        UpdateGamepadElement("shift_left", event->point.x == 1);
        break;
    case eGamepadElements::RIGHT_SHOULDER:
        UpdateGamepadElement("shift_right", event->point.x == 1);
        break;
    case eGamepadElements::LEFT_TRIGGER:
        UpdateGamepadElement("triger_left", event->point.x > 0);
        break;
    case eGamepadElements::RIGHT_TRIGGER:
        UpdateGamepadElement("triger_right", event->point.x > 0);
        break;
    case eGamepadElements::LEFT_THUMBSTICK_X:
        UpdateGamepadStickX("stick_left", event->point.x);
        break;
    case eGamepadElements::LEFT_THUMBSTICK_Y:
        UpdateGamepadStickY("stick_left", event->point.x);
        break;
    case eGamepadElements::RIGHT_THUMBSTICK_X:
        UpdateGamepadStickX("stick_right", event->point.x);
        break;
    case eGamepadElements::RIGHT_THUMBSTICK_Y:
        UpdateGamepadStickY("stick_right", event->point.x);
        break;
    case eGamepadElements::DPAD_X:
        UpdateGamepadElement("button_left", event->point.x < 0);
        UpdateGamepadElement("button_right", event->point.x > 0);
        break;
    case eGamepadElements::DPAD_Y:
        UpdateGamepadElement("button_up", event->point.x > 0);
        UpdateGamepadElement("button_down", event->point.x < 0);
        break;
    default:
        Logger::Error("not handled gamepad input event element: %d", event->element);
    }
    return false;
}

bool KeyboardTest::OnMouseTouchOrKeyboardEvent(UIEvent* currentInput)
{
    KeyboardDevice& keyboard = InputSystem::Instance()->GetKeyboard();

    if (currentInput->device == eInputDevices::KEYBOARD)
    {
        ++numKeyboardEvents;
    }
    else if (currentInput->device == eInputDevices::MOUSE)
    {
        ++numMouseEvents;
    }
    switch (currentInput->phase)
    {
    case UIEvent::Phase::BEGAN: //!<Screen touch or mouse button press is began.
        if (currentInput->device == eInputDevices::MOUSE)
        {
            ++numMouseDown;
            lastMouseX = static_cast<int32>(currentInput->point.x);
            lastMouseY = static_cast<int32>(currentInput->point.y);
            lastMouseKey = L'0' + static_cast<wchar_t>(currentInput->mouseButton);

            if (currentInput->tapCount > 1)
            {
                numMouseDblDown++;
            }
        }
        if (currentInput->device == eInputDevices::TOUCH_SURFACE)
        {
            auto FindFirstEmptyImage = [](::Finger& t) {
                return !t.isActive;
            };
            auto it = std::find_if(begin(touches), end(touches), FindFirstEmptyImage);
            if (it != touches.end())
            {
                it->isActive = true;
                it->img->SetPosition(currentInput->point);
                it->index = currentInput->touchId;
            }
        }
        break;
    case UIEvent::Phase::DRAG: //!<User moves mouse with presset button or finger over the screen.
        if (currentInput->device == eInputDevices::MOUSE)
        {
            ++numDrag;
            lastMouseX = static_cast<int32>(currentInput->point.x);
            lastMouseY = static_cast<int32>(currentInput->point.y);
        }
        if (currentInput->device == eInputDevices::TOUCH_SURFACE)
        {
            int32 index = currentInput->touchId;
            auto FindTouchById = [index](::Finger& t) {
                return index == t.index;
            };
            auto it = std::find_if(begin(touches), end(touches), FindTouchById);
            if (it != touches.end())
            {
                it->img->SetPosition(currentInput->point);
            }
            else
            {
                DVASSERT(false);
            }
        }
        break;
    case UIEvent::Phase::ENDED: //!<Screen touch or mouse button press is ended.
        if (currentInput->device == eInputDevices::MOUSE)
        {
            ++numMouseUp;
            lastMouseX = static_cast<int32>(currentInput->point.x);
            lastMouseY = static_cast<int32>(currentInput->point.y);
            lastMouseKey = L'0' + static_cast<wchar_t>(currentInput->mouseButton);

            if (currentInput->tapCount > 1)
            {
                numMouseDblUp++;
            }
        }
        if (currentInput->device == eInputDevices::TOUCH_SURFACE)
        {
            int32 index = currentInput->touchId;
            auto FindTouchById = [index](::Finger& t) {
                return index == t.index;
            };
            auto it = std::find_if(begin(touches), end(touches), FindTouchById);
            if (it != touches.end())
            {
                it->img->SetPosition(hiddenPos);
                it->isActive = false;
            }
            else
            {
                DVASSERT(false);
            }
        }
        break;
    case UIEvent::Phase::MOVE: //!<Mouse move event. Mouse moves without pressing any buttons. Works only with mouse controller.
        ++numMouseMove;
        lastMouseX = static_cast<int32>(currentInput->point.x);
        lastMouseY = static_cast<int32>(currentInput->point.y);
        lastMouseKey = L'0' + static_cast<wchar_t>(currentInput->mouseButton);
        break;
    case UIEvent::Phase::WHEEL: //!<Mouse wheel event. MacOS & Win32 only
        ++numMouseWheel;
        lastWheel = currentInput->wheelDelta.y;
        break;
    case UIEvent::Phase::CANCELLED: //!<Event was cancelled by the platform or by the control system for the some reason.
        ++numMouseCancel;
        if (currentInput->device == eInputDevices::TOUCH_SURFACE)
        {
            int32 index = currentInput->touchId;
            auto FindTouchById = [index](::Finger& t) {
                return index == t.index;
            };
            auto it = std::find_if(begin(touches), end(touches), FindTouchById);
            if (it != touches.end())
            {
                it->img->SetPosition(hiddenPos);
                it->isActive = false;
            }
            else
            {
                DVASSERT(false);
            }
        }
        break;
    case UIEvent::Phase::CHAR:
        ++numChar;
        lastChar = static_cast<wchar_t>(currentInput->keyChar);
        break;
    case UIEvent::Phase::CHAR_REPEAT:
        ++numCharRepeat;
        break;
    case UIEvent::Phase::KEY_DOWN:
        ++numKeyDown;
        lastKey = UTF8Utils::EncodeToWideString(keyboard.GetKeyName(currentInput->key));
        break;
    case UIEvent::Phase::KEY_DOWN_REPEAT:
        ++numKeyDownRepeat;
        lastKey = UTF8Utils::EncodeToWideString(keyboard.GetKeyName(currentInput->key));
        break;
    case UIEvent::Phase::KEY_UP:
        ++numKeyUp;
        lastKey = UTF8Utils::EncodeToWideString(keyboard.GetKeyName(currentInput->key));
        break;
    default:
        break;
    };

    std::wstringstream currentText;
    currentText << L"Keys: " << numKeyboardEvents << L"\n"
                << L"D: " << numKeyDown << L"\n"
                << L"U: " << numKeyUp << L"\n"
                << L"DR: " << numKeyDownRepeat << L"\n"
                << L"C: " << numChar << L"\n"
                << L"CR: " << numCharRepeat << L"\n"
                << L"c: \'" << lastChar << L"\'\n"
                << L"k: " << lastKey << L"\n"
                << L"Mouse: " << numMouseEvents << L"\n"
                << L"Drg: " << numDrag << L"\n"
                << L"Mv: " << numMouseMove << L"\n"
                << L"Dn: " << numMouseDown << L"\n"
                << L"Up: " << numMouseUp << L"\n"
                << L"DblDn: " << numMouseDblDown << L"\n"
                << L"DblUp: " << numMouseDblUp << L"\n"
                << L"Whl: " << numMouseWheel << L"\n"
                << L"Cncl: " << numMouseCancel << L"\n"
                << L"Btn: " << lastMouseKey << L"\n"
                << L"X: " << lastMouseX << L"\n"
                << L"Y: " << lastMouseY << L"\n"
                << L"Whl: " << lastWheel;

    descriptionText->SetText(currentText.str());

    return false; // let pass event to other controls
}

void KeyboardTest::OnGestureEvent(UIEvent* event)
{
    float32 magnification = event->gesture.magnification;
    Vector2 newSize = redBox->GetSize();
    if (magnification > -1.f && magnification < 1.f)
    {
        newSize.x *= (1.0f + magnification);
        newSize.y *= (1.0f + magnification);
    }
    redBox->SetSize(newSize);

    float32 angleDegrees = event->gesture.rotation;
    if (angleDegrees != 0.f)
    {
        angleDegrees *= -1.f;
    }
    float32 newAngle = redBox->GetAngle() + ((angleDegrees / 180) * 3.14f);
    redBox->SetAngle(newAngle);

    float swipeStep = 50.f;
    Vector2 newPos = redBox->GetPosition() +
    Vector2(swipeStep * event->gesture.dx,
            swipeStep * event->gesture.dy);
    redBox->SetPosition(newPos);
}

void KeyboardTest::UpdateGamepadElement(String name, bool isVisible)
{
    gamepadButtons[name]->SetVisibilityFlag(isVisible);
}

void KeyboardTest::UpdateGamepadStickX(String name, float axisValue)
{
    UIControl* ctrl = gamepadButtons[name];
    Vector2 pos = ctrl->GetPosition();
    if (std::abs(axisValue) >= 0.05f)
    {
        pos.x = gamepadPos.GetPosition().x + (axisValue * gamepadStickDeltaMove);
    }
    else
    {
        pos.x = gamepadPos.GetPosition().x;
    }
    ctrl->SetPosition(pos);
    UpdateGamepadElement(name, pos != gamepadPos.GetPosition());
}

void KeyboardTest::UpdateGamepadStickY(String name, float axisValue)
{
    UIControl* ctrl = gamepadButtons[name];
    Vector2 pos = ctrl->GetPosition();
    if (std::abs(axisValue) >= 0.05f)
    {
        pos.y = gamepadPos.GetPosition().y + (axisValue * gamepadStickDeltaMove * -1); // -1 y axis from up to down
    }
    else
    {
        pos.y = gamepadPos.GetPosition().y;
    }
    ctrl->SetPosition(pos);
    UpdateGamepadElement(name, pos != gamepadPos.GetPosition());
}
