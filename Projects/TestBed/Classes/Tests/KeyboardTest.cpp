/*==================================================================================
Copyright (c) 2008, binaryzebra
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

* Redistributions of source code must retain the above copyright
notice, this list of conditions and the following disclaimer.
* Redistributions in binary form must reproduce the above copyright
notice, this list of conditions and the following disclaimer in the
documentation and/or other materials provided with the distribution.
* Neither the name of the binaryzebra nor the
names of its contributors may be used to endorse or promote products
derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE binaryzebra AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL binaryzebra BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/

#include "Tests/KeyboardTest.h"
#include <Input/InputCallback.h>

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

Map<std::string, UIControl*> gamepadButtons;

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

    bool SystemInput(UIEvent* currentInput) override
    {
    	bool result = false;
    	if (currentInput->device == UIEvent::Device::GAMEPAD)
    	{
    		// this code never happen
    		DVASSERT(false);
    		OnGamepadEvent(currentInput);
    	} else
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
        lastKey = 0;

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
    void OnGamepadEvent(UIEvent* event)
    {
    	Logger::Info("gamepad tid: %2d, x: %.3f, y:%.3f", event->tid, event->point.x, event->point.y);

    	DVASSERT(event->device == UIEvent::Device::GAMEPAD);
    	DVASSERT(event->phase == UIEvent::Phase::JOYSTICK);

        switch (event->tid)
        {
        case GamepadDevice::GAMEPAD_ELEMENT_BUTTON_A:
            gamepadButtons["button_a"]->SetVisible(true);
            break;
        case GamepadDevice::GAMEPAD_ELEMENT_BUTTON_B:
            gamepadButtons["button_b"]->SetVisible(true);
            break;
        case GamepadDevice::GAMEPAD_ELEMENT_BUTTON_X:
            gamepadButtons["button_x"]->SetVisible(true);
            break;
        case GamepadDevice::GAMEPAD_ELEMENT_BUTTON_Y:
            gamepadButtons["button_y"]->SetVisible(true);
            break;
        case GamepadDevice::GAMEPAD_ELEMENT_BUTTON_LS:
            gamepadButtons["shift_left"]->SetVisible(true);
            break;
        case GamepadDevice::GAMEPAD_ELEMENT_BUTTON_RS:
            break;
        case GamepadDevice::GAMEPAD_ELEMENT_LT:
            break;
        case GamepadDevice::GAMEPAD_ELEMENT_RT:
            break;
        case GamepadDevice::GAMEPAD_ELEMENT_DPAD_X:
            break;
        case GamepadDevice::GAMEPAD_ELEMENT_DPAD_Y:
            break;
        case GamepadDevice::GAMEPAD_ELEMENT_AXIS_LX:
            break;
        case GamepadDevice::GAMEPAD_ELEMENT_AXIS_LY:
            break;
        case GamepadDevice::GAMEPAD_ELEMENT_AXIS_RX:
            break;
        case GamepadDevice::GAMEPAD_ELEMENT_AXIS_RY:
            break;
        }
    }

    bool OnMouseTouchOrKeyboardEvent(UIEvent* currentInput)
    {
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
                lastMouseKey = currentInput->tid;
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
                    it->index = currentInput->tid;
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
                int32 index = currentInput->tid;
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
                lastMouseKey = currentInput->tid;
            }
            if (currentInput->device == UIEvent::Device::TOUCH_SURFACE)
            {
                int32 index = currentInput->tid;
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
            lastMouseKey = currentInput->tid;
            break;
        case UIEvent::Phase::WHEEL: //!<Mouse wheel event. MacOS & Win32 only
            ++numMouseWheel;
            lastWheel = currentInput->physPoint.y;
            break;
        case UIEvent::Phase::CANCELLED: //!<Event was cancelled by the platform or by the control system for the some reason.
            ++numMouseCancel;
            if (currentInput->device == UIEvent::Device::TOUCH_SURFACE)
            {
                int32 index = currentInput->tid;
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
            lastChar = currentInput->keyChar;
            break;
        case UIEvent::Phase::CHAR_REPEAT:
            ++numCharRepeat;
            break;
        case UIEvent::Phase::KEY_DOWN:
            ++numKeyDown;
            lastKey = currentInput->tid;
            break;
        case UIEvent::Phase::KEY_DOWN_REPEAT:
            ++numKeyDownRepeat;
            lastKey = currentInput->tid;
            break;
        case UIEvent::Phase::KEY_UP:
            ++numKeyUp;
            lastKey = currentInput->tid;
            break;
        default:
            break;
        };

        std::wstringstream currentText;
        currentText << L"Keyboards: " << numKeyboardEvents << L"\n"
                    << L"KD: " << numKeyDown << L"\n"
                    << L"KU: " << numKeyUp << L"\n"
                    << L"KDR: " << numKeyDownRepeat << L"\n"
                    << L"Ch: " << numChar << L"\n"
                    << L"ChR: " << numCharRepeat << L"\n"
                    << L"ch: \'" << lastChar << L"\'\n"
                    << L"k: " << lastKey << L"\n"
                    << L"Mouse: " << numMouseEvents << L"\n"
                    << L"Drg: " << numDrag << L"\n"
                    << L"Mv: " << numMouseMove << L"\n"
                    << L"Dn: " << numMouseDown << L"\n"
                    << L"Up: " << numMouseUp << L"\n"
                    << L"Whl: " << numMouseWheel << L"\n"
                    << L"Cncl: " << numMouseCancel << L"\n"
                    << L"Key: " << lastMouseKey << L"\n"
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
    uint32 lastKey = 0;

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

KeyboardTest::KeyboardTest()
    : BaseScreen("KeyboardTest")
{
}

void KeyboardTest::LoadResources()
{
    BaseScreen::LoadResources();

    ScopedPtr<FTFont> font(FTFont::Create("~res:/Fonts/korinna.ttf"));

    previewText = new UIStaticText(Rect(20, 30, 400, 200));
    previewText->SetFont(font);
    previewText->SetTextColor(Color::White);
    previewText->SetText(L"Press (Hold) and Unpress keys");
    previewText->SetDebugDraw(true);
    previewText->SetTextAlign(ALIGN_LEFT | ALIGN_TOP);
    AddControl(previewText);

    customText = new CustomText(Rect(20, 230, 200, 400));
    customText->SetDebugDraw(true);
    customText->SetTextColor(Color::White);
    customText->SetFont(font);
    customText->SetFocusEnabled(true);
    customText->SetAlign(ALIGN_LEFT | ALIGN_TOP);
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

    UIControlSystem::Instance()->SetFocusedControl(customText, true);

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

	UIControl* gamepad = new UIControl(Rect(500, 0, 800, 450));
	auto pathToBack = FilePath("~res:/Gfx/GamepadTest/gamepad");
	gamepad->SetSprite(pathToBack, 0);
	AddControl(gamepad);

    for (auto& buttonOrAxisName : gamepadButtonsNames)
    {
    	UIControl* img = new UIControl(Rect(500, 000, 800, 450));
    	auto path = FilePath("~res:/Gfx/GamepadTest/") + buttonOrAxisName;
    	img->SetSprite(path, 0);
    	gamepadButtons[buttonOrAxisName] = img;
    	AddControl(img);
    }
}

void KeyboardTest::UnloadResources()
{
    SafeRelease(customText);
    SafeRelease(previewText);
    SafeRelease(resetButton);

    for (auto& touch : touches)
    {
        SafeRelease(touch.img);
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
