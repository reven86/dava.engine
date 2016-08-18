#include "Tests/KeyboardTest.h"
#include <Input/InputCallback.h>
#include <UI/Focus/UIFocusComponent.h>

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

UIControl* redBox = nullptr;

class CustomText : public UIStaticText
{
public:
    CustomText(const Rect& rect)
        : UIStaticText(rect)
    {
        UIStaticText::SetInputEnabled(true);

        InputCallback gamepadCallback(this, &CustomText::OnGamepadEvent, InputSystem::INPUT_DEVICE_JOYSTICK);
        InputSystem::Instance()->AddInputCallback(gamepadCallback);
    }
    ~CustomText()
    {
        InputCallback gamepadCallback(this, &CustomText::OnGamepadEvent, InputSystem::INPUT_DEVICE_JOYSTICK);
        InputSystem::Instance()->RemoveInputCallback(gamepadCallback);
    }

    bool SystemProcessInput(UIEvent* currentInput) override
    {
        bool result = false;
        if (currentInput->phase == UIEvent::Phase::GESTURE)
        {
            OnGestureEvent(currentInput);
        }
        else if (currentInput->device == UIEvent::Device::GAMEPAD)
        {
            // this code never happen
            DVASSERT(false);
            OnGamepadEvent(currentInput);
        }
        else
        {
            result = OnMouseTouchOrKeyboardEvent(currentInput);
        }
        return result;
    }

    void ResetCounters()
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
        lastMouseKey = L'\0';
        lastMouseX = 0;
        lastMouseY = 0;
        lastWheel = 0.f;
    }

private:
    void UpdateGamepadElement(String name, bool isVisible)
    {
        gamepadButtons[name]->SetVisibilityFlag(isVisible);
    }
    void UpdateGamepadStickX(String name, float axisValue)
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
    void UpdateGamepadStickY(String name, float axisValue)
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

    void OnGestureEvent(UIEvent* event)
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

    void OnGamepadEvent(UIEvent* event)
    {
        //Logger::Info("gamepad tid: %2d, x: %.3f, y:%.3f", event->tid, event->point.x, event->point.y);

        DVASSERT(event->device == UIEvent::Device::GAMEPAD);
        DVASSERT(event->phase == UIEvent::Phase::JOYSTICK);

        switch (event->element)
        {
        case GamepadDevice::GAMEPAD_ELEMENT_BUTTON_A:
            UpdateGamepadElement("button_a", event->point.x == 1);
            break;
        case GamepadDevice::GAMEPAD_ELEMENT_BUTTON_B:
            UpdateGamepadElement("button_b", event->point.x == 1);
            break;
        case GamepadDevice::GAMEPAD_ELEMENT_BUTTON_X:
            UpdateGamepadElement("button_x", event->point.x == 1);
            break;
        case GamepadDevice::GAMEPAD_ELEMENT_BUTTON_Y:
            UpdateGamepadElement("button_y", event->point.x == 1);
            break;
        case GamepadDevice::GAMEPAD_ELEMENT_BUTTON_LS:
            UpdateGamepadElement("shift_left", event->point.x == 1);
            break;
        case GamepadDevice::GAMEPAD_ELEMENT_BUTTON_RS:
            UpdateGamepadElement("shift_right", event->point.x == 1);
            break;
        case GamepadDevice::GAMEPAD_ELEMENT_LT:
            UpdateGamepadElement("triger_left", event->point.x > 0);
            break;
        case GamepadDevice::GAMEPAD_ELEMENT_RT:
            UpdateGamepadElement("triger_right", event->point.x > 0);
            break;
        case GamepadDevice::GAMEPAD_ELEMENT_AXIS_LX:
            UpdateGamepadStickX("stick_left", event->point.x);
            break;
        case GamepadDevice::GAMEPAD_ELEMENT_AXIS_LY:
            UpdateGamepadStickY("stick_left", event->point.x);
            break;
        case GamepadDevice::GAMEPAD_ELEMENT_AXIS_RX:
            UpdateGamepadStickX("stick_right", event->point.x);
            break;
        case GamepadDevice::GAMEPAD_ELEMENT_AXIS_RY:
            UpdateGamepadStickY("stick_right", event->point.x);
            break;
        case GamepadDevice::GAMEPAD_ELEMENT_DPAD_X:
            UpdateGamepadElement("button_left", event->point.x < 0);
            UpdateGamepadElement("button_right", event->point.x > 0);
            break;
        case GamepadDevice::GAMEPAD_ELEMENT_DPAD_Y:
            UpdateGamepadElement("button_up", event->point.x > 0);
            UpdateGamepadElement("button_down", event->point.x < 0);
            break;
        default:
            Logger::Error("not handled gamepad input event element: %d", event->element);
        }
    }

    bool OnMouseTouchOrKeyboardEvent(UIEvent* currentInput)
    {
        KeyboardDevice& keyboard = InputSystem::Instance()->GetKeyboard();

        if (currentInput->device == UIEvent::Device::KEYBOARD)
        {
            ++numKeyboardEvents;
        }
        else if (currentInput->device == UIEvent::Device::MOUSE)
        {
            ++numMouseEvents;
        }
        switch (currentInput->phase)
        {
        case UIEvent::Phase::BEGAN: //!<Screen touch or mouse button press is began.
            if (currentInput->device == UIEvent::Device::MOUSE)
            {
                ++numMouseDown;
                lastMouseX = static_cast<int32>(currentInput->point.x);
                lastMouseY = static_cast<int32>(currentInput->point.y);
                lastMouseKey = L'0' + static_cast<wchar_t>(currentInput->mouseButton);
            }
            if (currentInput->device == UIEvent::Device::TOUCH_SURFACE)
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
            if (currentInput->device == UIEvent::Device::MOUSE)
            {
                ++numDrag;
                lastMouseX = static_cast<int32>(currentInput->point.x);
                lastMouseY = static_cast<int32>(currentInput->point.y);
            }
            if (currentInput->device == UIEvent::Device::TOUCH_SURFACE)
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
            if (currentInput->device == UIEvent::Device::MOUSE)
            {
                ++numMouseUp;
                lastMouseX = static_cast<int32>(currentInput->point.x);
                lastMouseY = static_cast<int32>(currentInput->point.y);
                lastMouseKey = L'0' + static_cast<wchar_t>(currentInput->mouseButton);
            }
            if (currentInput->device == UIEvent::Device::TOUCH_SURFACE)
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
            if (currentInput->device == UIEvent::Device::TOUCH_SURFACE)
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
                    << L"Whl: " << numMouseWheel << L"\n"
                    << L"Cncl: " << numMouseCancel << L"\n"
                    << L"Btn: " << lastMouseKey << L"\n"
                    << L"X: " << lastMouseX << L"\n"
                    << L"Y: " << lastMouseY << L"\n"
                    << L"Whl: " << lastWheel;

        SetText(currentText.str());

        return false; // let pass event to other controls
    }

    uint32 numKeyboardEvents = 0;
    uint32 numKeyDown = 0;
    uint32 numKeyUp = 0;
    uint32 numKeyDownRepeat = 0;
    uint32 numChar = 0;
    uint32 numCharRepeat = 0;
    wchar_t lastChar = L'\0';
    WideString lastKey;

    uint32 numMouseEvents = 0;
    uint32 numDrag = 0;
    uint32 numMouseMove = 0;
    uint32 numMouseDown = 0;
    uint32 numMouseUp = 0;
    uint32 numMouseWheel = 0;
    uint32 numMouseCancel = 0;
    wchar_t lastMouseKey = L'\0';
    int32 lastMouseX = 0;
    int32 lastMouseY = 0;
    float32 lastWheel = 0.f;

    //gamepad state
};

CustomText* customText = nullptr;

KeyboardTest::KeyboardTest(GameCore* g)
    : BaseScreen(g, "KeyboardTest")
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
    previewText->SetDebugDraw(true);
    previewText->SetTextAlign(ALIGN_LEFT | ALIGN_TOP);
    AddControl(previewText);

    customText = new CustomText(Rect(20, 230, 200, 400));
    customText->SetDebugDraw(true);
    customText->SetTextColor(Color::White);
    customText->SetFont(font);
    customText->GetOrCreateComponent<UIFocusComponent>();
    customText->SetSpriteAlign(ALIGN_LEFT | ALIGN_TOP);
    customText->SetTextAlign(ALIGN_LEFT | ALIGN_TOP);
    customText->SetMultiline(true);
    customText->SetMultilineType(UIStaticText::MULTILINE_ENABLED_BY_SYMBOL);
    AddControl(customText);

    resetButton = new UIButton(Rect(420, 30, 50, 30));
    resetButton->SetDebugDraw(true);
    resetButton->SetStateFont(0xFF, font);
    resetButton->SetStateFontColor(0xFF, Color::White);
    resetButton->SetStateText(0xFF, L"Reset");
    resetButton->AddEvent(UIButton::EVENT_TOUCH_DOWN, Message(this, &KeyboardTest::OnResetClick));
    AddControl(resetButton);

    for (auto& touch : touches)
    {
        touch.img = new UIButton(Rect(0, 0, 50, 50));
        touch.img->SetDebugDraw(true);
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

    UIButton* box = new UIButton(Rect(512, 512, 128, 128));
    box->SetPivotPoint(Vector2(64.f, 64.f));
    box->SetInputEnabled(false);
    box->SetDebugDraw(true);
    auto boxBack = box->GetBackground();
    boxBack->SetDrawColor(Color(1.f, 0.f, 0.f, 1.f));
    boxBack->SetColor(Color(1.f, 0.f, 0.f, 1.f));
    boxBack->SetDrawType(UIControlBackground::eDrawType::DRAW_FILL);
    redBox = box;
    AddControl(redBox);

    gamepad = new UIControl(gamepadPos);
    auto pathToBack = FilePath("~res:/Gfx/GamepadTest/gamepad");
    gamepad->GetBackground()->SetModification(ESM_VFLIP | ESM_HFLIP);
    gamepad->SetSprite(pathToBack, 0);
    AddControl(gamepad);

    for (auto& buttonOrAxisName : gamepadButtonsNames)
    {
        UIControl* img = new UIControl(gamepadPos);
        auto path = FilePath("~res:/Gfx/GamepadTest/") + buttonOrAxisName;
        img->GetBackground()->SetModification(ESM_VFLIP | ESM_HFLIP);
        img->SetSprite(path, 0);
        gamepadButtons[buttonOrAxisName] = img;
        AddControl(img);
        img->SetVisibilityFlag(false);
    }
}

void KeyboardTest::UnloadResources()
{
    SafeRelease(customText);
    SafeRelease(previewText);
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
    if (customText)
    {
        customText->ResetCounters();
        customText->SetText(L"");
    }
}
