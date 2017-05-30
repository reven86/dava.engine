#include "UI/UIPopup.h"
#include "UI/UIControlSystem.h"

namespace DAVA
{
UIPopup::UIPopup(const Rect& rect)
    : UIScreen(rect)
    , isTransparent(true)
{
}

void UIPopup::Show()
{
    UIControlSystem::Instance()->AddPopup(this);
}

void UIPopup::Hide()
{
    if (IsActive())
        UIControlSystem::Instance()->RemovePopup(this);
}
};
