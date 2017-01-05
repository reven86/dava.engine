#include "Menu.h"

void ActionItem::OnActivate(DAVA::BaseObject* caller, void* param, void* callerData)
{
    if (parentMenu)
    {
        parentMenu->BackToMainMenu();
    }
    action(caller, callerData);
}
