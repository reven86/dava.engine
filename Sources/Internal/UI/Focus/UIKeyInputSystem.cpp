#include "UIKeyInputSystem.h"

#include "UI/UIControl.h"
#include "UI/UIControlSystem.h"
#include "UI/UIEvent.h"

#include "UI/Focus/UIFocusSystem.h"

#include "Input/InputSystem.h"

namespace DAVA
{
UIKeyInputSystem::UIKeyInputSystem(UIFocusSystem* focusSystem_)
    : focusSystem(focusSystem_)
{
}

UIKeyInputSystem::~UIKeyInputSystem()
{
    focusSystem = nullptr;
}

void UIKeyInputSystem::HandleKeyEvent(UIEvent* event)
{
    bool processed = false;

    UIEvent::Phase phase = event->phase;
    DVASSERT(phase == UIEvent::Phase::KEY_DOWN || phase == UIEvent::Phase::KEY_UP || phase == UIEvent::Phase::KEY_DOWN_REPEAT || phase == UIEvent::Phase::CHAR || phase == UIEvent::Phase::CHAR_REPEAT);

    UIControl* focusedControl = focusSystem->GetFocusedControl();
    UIControl* rootControl = focusSystem->GetRoot();

    if (!processed && focusedControl)
    {
        UIControl* c = focusedControl;
        while (c != nullptr && c != rootControl && !processed)
        {
            processed = c->SystemProcessInput(event);
            c = c->GetParent();
        }
    }

    if (!processed && rootControl)
    {
        processed = rootControl->SystemProcessInput(event);
    }

    if (!processed && (phase == UIEvent::Phase::KEY_DOWN || phase == UIEvent::Phase::KEY_DOWN_REPEAT))
    {
        KeyboardDevice& keyboard = InputSystem::Instance()->GetKeyboard();
        switch (event->key)
        {
        case Key::LEFT:
            focusSystem->MoveFocusLeft();
            break;

        case Key::RIGHT:
            focusSystem->MoveFocusRight();
            break;

        case Key::UP:
            focusSystem->MoveFocusUp();
            break;

        case Key::DOWN:
            focusSystem->MoveFocusDown();
            break;

        case Key::TAB:
            if (keyboard.IsKeyPressed(Key::LSHIFT) || keyboard.IsKeyPressed(Key::RSHIFT))
            {
                focusSystem->MoveFocusBackward();
            }
            else
            {
                focusSystem->MoveFocusForward();
            }
            break;

        default:
            // do nothing
            break;
        }
    }
}
}
