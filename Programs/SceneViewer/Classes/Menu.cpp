#include "Menu.h"

ActionItem::ActionItem(Menu* parentMenu, DAVA::Message& action)
    : parentMenu(parentMenu)
    , action(action)
{
}

void ActionItem::OnActivate(DAVA::BaseObject* caller, void* param, void* callerData)
{
    if (parentMenu)
    {
        parentMenu->BackToMainMenu();
    }
    action(caller, callerData);
}

namespace MenuDetails
{
    const DAVA::float32 SPACE_BETWEEN_BUTTONS = 10.0f;
}

Menu::Menu(Menu* parentMenu, DAVA::UIControl* bearerControl, DAVA::Font* font, DAVA::Rect& firstButtonRect)
    : parentMenu(parentMenu)
    , bearerControl(bearerControl)
    , font(font)
    , firstButtonRect(firstButtonRect)
    , nextButtonRect(firstButtonRect)
{
}

Menu::~Menu()
{
    for (auto& item : menuItems)
    {
        bearerControl->RemoveControl(item->button);
    }
}

void Menu::AddActionItem(const DAVA::WideString& text, DAVA::Message& action)
{
    ActionItem* actionItem = new ActionItem(this, action);
    menuItems.emplace_back(actionItem);
    actionItem->button = ConstructMenuButton(text, DAVA::Message(actionItem, &ActionItem::OnActivate));
}

Menu* Menu::AddSubMenuItem(const DAVA::WideString& text)
{
    SubMenuItem* subMenuItem = new SubMenuItem;
    menuItems.emplace_back(subMenuItem);

    subMenuItem->submenu.reset(new Menu(this, bearerControl, font, firstButtonRect));
    subMenuItem->button = ConstructMenuButton(text, DAVA::Message(subMenuItem->submenu.get(), &Menu::OnActivate));

    return subMenuItem->submenu.get();
}

void Menu::AddBackItem()
{
    menuItems.emplace_back(new MenuItem);
    menuItems.back()->button = ConstructMenuButton(L"Back", DAVA::Message(this, &Menu::OnBack));
}

void Menu::BackToMainMenu()
{
    if (parentMenu)
    {
        Show(false);
        parentMenu->BackToMainMenu();
    }
    else
    {
        Show(true);
    }
}

DAVA::ScopedPtr<DAVA::UIButton> Menu::ConstructMenuButton(const DAVA::WideString& text, const DAVA::Message& action)
{
    DAVA::ScopedPtr<DAVA::UIButton> button(new DAVA::UIButton(nextButtonRect));
    nextButtonRect.y += (nextButtonRect.dy + MenuDetails::SPACE_BETWEEN_BUTTONS);

    button->SetVisibilityFlag(IsFirstLevelMenu());
    button->SetStateText(DAVA::UIControl::STATE_NORMAL, text);
    button->SetStateTextAlign(DAVA::UIControl::STATE_NORMAL, DAVA::ALIGN_HCENTER | DAVA::ALIGN_VCENTER);
    button->SetStateFont(DAVA::UIControl::STATE_NORMAL, font);
    button->SetStateFontColor(DAVA::UIControl::STATE_NORMAL, DAVA::Color::White);
    button->SetStateFontColor(DAVA::UIControl::STATE_PRESSED_INSIDE, DAVA::Color(0.7f, 0.7f, 0.7f, 1.f));
    button->AddEvent(DAVA::UIControl::EVENT_TOUCH_UP_INSIDE, action);
    button->SetDebugDraw(true);
    bearerControl->AddControl(button);

    return button;
}

void Menu::Show(bool toShow)
{
    for (auto& menuItem : menuItems)
    {
        menuItem->button->SetVisibilityFlag(toShow);
    }
}

void Menu::OnBack(DAVA::BaseObject* caller, void* param, void* callerData)
{
    Show(false);

    if (parentMenu)
    {
        parentMenu->Show(true);
    }
}

void Menu::OnActivate(DAVA::BaseObject* caller, void* param, void* callerData)
{
    if (parentMenu)
    {
        parentMenu->Show(false);
    }

    Show(true);
}
