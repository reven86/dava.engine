#pragma once

#include "Infrastructure/BaseScreen.h"

class TestBed;
class KeyboardTest : public BaseScreen
{
public:
    KeyboardTest(TestBed& app);

protected:
    void LoadResources() override;
    void UnloadResources() override;

private:
    void OnResetClick(DAVA::BaseObject* sender, void* data, void* callerData);

    void ResetCounters();

    bool OnPointerEvent(DAVA::UIEvent* e);
    bool OnKeyboardEvent(DAVA::UIEvent* e);
    bool OnGamepadEvent(DAVA::UIEvent* event);

    bool OnMouseTouchOrKeyboardEvent(DAVA::UIEvent* currentInput);
    void OnGestureEvent(DAVA::UIEvent* event);

    void UpdateGamepadElement(DAVA::String name, bool isVisible);
    void UpdateGamepadStickX(DAVA::String name, float axisValue);
    void UpdateGamepadStickY(DAVA::String name, float axisValue);

    TestBed& app;
    DAVA::UIStaticText* previewText = nullptr;
    DAVA::UIStaticText* descriptionText = nullptr;
    DAVA::UIButton* resetButton = nullptr;
    DAVA::UIControl* redBox = nullptr;
    DAVA::UIControl* gamepad = nullptr;

    DAVA::uint32 pointerInputToken = 0;
    DAVA::uint32 keyboardInputToken = 0;
    DAVA::uint32 gamepadInputToken = 0;

    DAVA::uint32 numKeyboardEvents = 0;
    DAVA::uint32 numKeyDown = 0;
    DAVA::uint32 numKeyUp = 0;
    DAVA::uint32 numKeyDownRepeat = 0;
    DAVA::uint32 numChar = 0;
    DAVA::uint32 numCharRepeat = 0;
    wchar_t lastChar = L'\0';
    DAVA::WideString lastKey;

    DAVA::uint32 numMouseEvents = 0;
    DAVA::uint32 numDrag = 0;
    DAVA::uint32 numMouseMove = 0;
    DAVA::uint32 numMouseDown = 0;
    DAVA::uint32 numMouseUp = 0;
    DAVA::uint32 numMouseDblUp = 0;
    DAVA::uint32 numMouseDblDown = 0;
    DAVA::uint32 numMouseWheel = 0;
    DAVA::uint32 numMouseCancel = 0;
    wchar_t lastMouseKey = L'\0';
    DAVA::int32 lastMouseX = 0;
    DAVA::int32 lastMouseY = 0;
    DAVA::float32 lastWheel = 0.f;
};
