#ifndef __DAVAENGINE_UI_NAVIGATION_COMPONENT_H__
#define __DAVAENGINE_UI_NAVIGATION_COMPONENT_H__

#include "Base/BaseTypes.h"

#include "UI/Components/UIComponent.h"
#include "UI/Focus/FocusHelpers.h"

namespace DAVA
{
class UINavigationComponent : public UIBaseComponent<UIComponent::NAVIGATION_COMPONENT>
{
public:
    enum Direction
    {
        LEFT = 0,
        RIGHT,
        UP,
        DOWN,

        DIRECTION_COUNT
    };

    UINavigationComponent();
    UINavigationComponent(const UINavigationComponent& src);

protected:
    virtual ~UINavigationComponent();

private:
    UINavigationComponent& operator=(const UINavigationComponent&) = delete;

public:
    UINavigationComponent* Clone() const override;

    const String& GetNextFocusLeft() const;
    void SetNextFocusLeft(const String& val);

    const String& GetNextFocusRight() const;
    void SetNextFocusRight(const String& val);

    const String& GetNextFocusUp() const;
    void SetNextFocusUp(const String& val);

    const String& GetNextFocusDown() const;
    void SetNextFocusDown(const String& val);

    const String& GetNextControlPathInDirection(Direction dir);

private:
    String nextFocusPath[DIRECTION_COUNT];

public:
    INTROSPECTION_EXTEND(UINavigationComponent, UIComponent,
                         PROPERTY("left", "Next Focus Left", GetNextFocusLeft, SetNextFocusLeft, I_SAVE | I_VIEW | I_EDIT)
                         PROPERTY("right", "Next Focus Right", GetNextFocusRight, SetNextFocusRight, I_SAVE | I_VIEW | I_EDIT)
                         PROPERTY("up", "Next Focus Up", GetNextFocusUp, SetNextFocusUp, I_SAVE | I_VIEW | I_EDIT)
                         PROPERTY("down", "Next Focus Down", GetNextFocusDown, SetNextFocusDown, I_SAVE | I_VIEW | I_EDIT));
};
}


#endif //__DAVAENGINE_UI_NAVIGATION_COMPONENT_H__
