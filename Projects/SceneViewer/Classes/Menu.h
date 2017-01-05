#pragma once

#include "Base/BaseTypes.h"
#include "Base/Message.h"
#include "Base/ScopedPtr.h"
#include "Base/BaseObject.h"
#include "Math/Rect.h"
#include "Math/Color.h"
#include "UI/UIButton.h"
#include "UI/UIControl.h"

struct MenuItem
{
    DAVA::ScopedPtr<DAVA::UIButton> button;
};

class Menu;

struct ActionItem : public MenuItem
{
    explicit ActionItem(Menu* parentMenu, DAVA::Message& action)
        : parentMenu(parentMenu)
        , action(action)
    {
    }

    void OnActivate(DAVA::BaseObject* caller, void* param, void* callerData);

private:
    Menu* parentMenu = nullptr;
    DAVA::Message action;
};

struct SubMenuItem : public MenuItem
{
    std::unique_ptr<Menu> submenu;
};

class Menu
{
public:
    explicit Menu(Menu* parentMenu, DAVA::UIControl* bearerControl, DAVA::Font* font, DAVA::Rect& firstButtonRect)
        : parentMenu(parentMenu)
        , bearerControl(bearerControl)
        , font(font)
        , firstButtonRect(firstButtonRect)
        , nextButtonRect(firstButtonRect)
    {
    }

    void AddActionItem(const DAVA::WideString& text, DAVA::Message& action)
    {
        ActionItem* actionItem = new ActionItem(this, action);
        menuItems.emplace_back(actionItem);
        actionItem->button = ConstructMenuButton(text, DAVA::Message(actionItem, &ActionItem::OnActivate));
    }

    Menu* AddSubMenuItem(const DAVA::WideString& text)
    {
        SubMenuItem* subMenuItem = new SubMenuItem;
        menuItems.emplace_back(subMenuItem);

        subMenuItem->submenu.reset(new Menu(this, bearerControl, font, firstButtonRect));
        subMenuItem->button = ConstructMenuButton(text, DAVA::Message(subMenuItem->submenu.get(), &Menu::OnActivate));

        return subMenuItem->submenu.get();
    }

    void AddBackItem()
    {
        menuItems.emplace_back(new MenuItem);
        menuItems.back()->button = ConstructMenuButton(L"Back", DAVA::Message(this, &Menu::OnBack));
    }

    void BackToMainMenu()
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

private:
    const DAVA::float32 SPACE_BETWEEN_BUTTONS = 10.0f;

    DAVA::ScopedPtr<DAVA::UIButton> ConstructMenuButton(const DAVA::WideString& text, const DAVA::Message& action)
    {
        DAVA::ScopedPtr<DAVA::UIButton> button(new DAVA::UIButton(nextButtonRect));
        nextButtonRect.y += (nextButtonRect.dy + SPACE_BETWEEN_BUTTONS);

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

private:
    void Show(bool toShow)
    {
        for (auto& menuItem : menuItems)
        {
            menuItem->button->SetVisibilityFlag(toShow);
        }
    }

    void OnBack(DAVA::BaseObject* caller, void* param, void* callerData)
    {
        Show(false);

        if (parentMenu)
        {
            parentMenu->Show(true);
        }
    }

    void OnActivate(DAVA::BaseObject* caller, void* param, void* callerData)
    {
        if (parentMenu)
        {
            parentMenu->Show(false);
        }

        Show(true);
    }

    bool IsFirstLevelMenu() const
    {
        return (parentMenu == nullptr);
    }

private:
    Menu* parentMenu = nullptr;
    DAVA::UIControl* bearerControl = nullptr;
    DAVA::Font* font = nullptr;

    DAVA::Vector<std::unique_ptr<MenuItem>> menuItems;

    DAVA::Rect firstButtonRect;
    DAVA::Rect nextButtonRect;
};
