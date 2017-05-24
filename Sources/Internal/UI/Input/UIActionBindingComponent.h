#ifndef __DAVAENGINE_UI_ACTION_BINDING_COMPONENT_H__
#define __DAVAENGINE_UI_ACTION_BINDING_COMPONENT_H__

#include "Base/BaseTypes.h"

#include "UI/Components/UIComponent.h"
#include "Input/KeyboardShortcut.h"
#include "Functional/Signal.h"
#include "UIActionMap.h"
#include "UIInputMap.h"
#include "Reflection/Reflection.h"

namespace DAVA
{
class UIControl;

class UIActionBindingComponent : public UIComponent
{
    DAVA_VIRTUAL_REFLECTION(UIActionBindingComponent, UIComponent);
    IMPLEMENT_UI_COMPONENT(UIActionBindingComponent);

public:
    UIActionBindingComponent();
    UIActionBindingComponent(const UIActionBindingComponent& src);

protected:
    virtual ~UIActionBindingComponent();

private:
    UIActionBindingComponent& operator=(const UIActionBindingComponent&) = delete;

public:
    UIActionBindingComponent* Clone() const override;

    const UIActionMap& GetActionMap() const;
    UIActionMap& GetActionMap();

    const UIInputMap& GetInputMap() const;
    UIInputMap& GetInputMap();

    bool IsBlockOtherKeyboardShortcuts() const;
    void SetBlockOtherKeyboardShortcuts(bool block);

private:
    String GetActionsAsString() const;
    void SetActionsFromString(const String& value);

    struct Action
    {
        FastName action;
        KeyboardShortcut shortcut1;

        Action(const FastName& action_, const KeyboardShortcut& shortcut_)
            : action(action_)
            , shortcut1(shortcut_)
        {
        }
    };

    Vector<Action> actions;

    UIActionMap actionMap;
    UIInputMap inputMap;
    bool blockOtherKeyboardShortcuts = true;
};
}


#endif //__DAVAENGINE_UI_ACTION_BINDING_COMPONENT_H__
