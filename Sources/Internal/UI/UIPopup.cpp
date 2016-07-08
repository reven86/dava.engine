#include "UI/UIPopup.h"
#include "UI/UIControlSystem.h"

namespace DAVA
{
UIPopup::UIPopup(const Rect& rect)
    : UIScreen(rect)
    , isTransparent(true)
{
    SetFillBorderOrder(UIScreen::FILL_BORDER_NONE);
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
