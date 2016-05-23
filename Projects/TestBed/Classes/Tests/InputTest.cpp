#include "Tests/InputTest.h"
#include "Platform/DeviceInfo.h"

using namespace DAVA;

InputTest::InputTest()
    : BaseScreen("InputTest")
{
    DeviceInfo::GetHIDConnectionSignal(DAVA::DeviceInfo::eHIDType::HID_MOUSE_TYPE).Connect(this, &InputTest::OnInputChanged);
    DeviceInfo::GetHIDConnectionSignal(DAVA::DeviceInfo::eHIDType::HID_KEYBOARD_TYPE).Connect(this, &InputTest::OnInputChanged);
    DeviceInfo::GetHIDConnectionSignal(DAVA::DeviceInfo::eHIDType::HID_POINTER_TYPE).Connect(this, &InputTest::OnInputChanged);
}

void InputTest::OnInputChanged(DAVA::DeviceInfo::eHIDType hidType, bool connected)
{
    input[hidType] = DeviceInfo::IsHIDConnected(hidType);
}

void InputTest::MousePressed(BaseObject* obj, void* data, void* callerData)
{
    bool res = input[DeviceInfo::eHIDType::HID_MOUSE_TYPE] = DeviceInfo::IsHIDConnected(DeviceInfo::eHIDType::HID_MOUSE_TYPE);
    if (res)
    {
        mouse->SetStateText(0xFF, L"Mouse is presented, click for update.");
    }
    else
    {
        mouse->SetStateText(0xFF, L"Mouse is not presented, click for update.");
    }
}

void InputTest::TouchPressed(BaseObject* obj, void* data, void* callerData)
{
    bool res = input[DeviceInfo::eHIDType::HID_TOUCH_TYPE] = DeviceInfo::IsHIDConnected(DeviceInfo::eHIDType::HID_TOUCH_TYPE);
    if (res)
    {
        touch->SetStateText(0xFF, L"Touch is presented, click for update.");
    }
    else
    {
        touch->SetStateText(0xFF, L"Touch is not presented, click for update.");
    }
}

void InputTest::KeyboardPressed(BaseObject* obj, void* data, void* callerData)
{
    bool res = input[DeviceInfo::eHIDType::HID_KEYBOARD_TYPE] = DeviceInfo::IsHIDConnected(DeviceInfo::eHIDType::HID_KEYBOARD_TYPE);
    if (res)
    {
        keyboard->SetStateText(0xFF, L"Keyboard is presented, click for update.");
    }
    else
    {
        keyboard->SetStateText(0xFF, L"Keyboard is not presented, click for update.");
    }
}

void InputTest::LoadResources()
{
    Font* font = FTFont::Create("~res:/Fonts/korinna.ttf");
    DVASSERT(font);
    font->SetSize(17);
    Size2i screenSize = VirtualCoordinatesSystem::Instance()->GetVirtualScreenSize();
    float32 startX = 0.f, startY = 0.f, w = 0.f, h = 0.f;
    w = screenSize.dx * 0.5f;
    h = screenSize.dy * 0.05f;
    startX = screenSize.dx * 0.5f - w * 0.5f;
    startY = h;
    // button mouse
    mouse = new UIButton(Rect(startX, startY, w, h));
    mouse->SetStateFont(0xFF, font);
    mouse->SetDebugDraw(true);
    mouse->SetStateFontColor(0xFF, Color(1.0, 0.0, 0.0, 0.75));
    mouse->SetStateText(0xFF, L"mouse input");
    mouse->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &InputTest::MousePressed));
    AddControl(mouse);
    startY += h + h * 0.5f;
    // button touch
    touch = new UIButton(Rect(startX, startY, w, h));
    touch->SetStateFont(0xFF, font);
    touch->SetDebugDraw(true);
    touch->SetStateFontColor(0xFF, Color(1.0, 0.0, 0.0, 0.75));
    touch->SetStateText(0xFF, L"touch input");
    touch->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &InputTest::TouchPressed));
    AddControl(touch);
    startY += h + h * 0.5f;
    // button keyboard
    keyboard = new UIButton(Rect(startX, startY, w, h));
    keyboard->SetStateFont(0xFF, font);
    keyboard->SetDebugDraw(true);
    keyboard->SetStateFontColor(0xFF, Color(1.0, 0.0, 0.0, 0.75));
    keyboard->SetStateText(0xFF, L"keyboard input");
    keyboard->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &InputTest::KeyboardPressed));
    AddControl(keyboard);

    SafeRelease(font);
    BaseScreen::LoadResources();
}

void InputTest::UnloadResources()
{
    BaseScreen::UnloadResources();
    RemoveAllControls();

    SafeRelease(mouse);
    SafeRelease(touch);
    SafeRelease(keyboard);
}
