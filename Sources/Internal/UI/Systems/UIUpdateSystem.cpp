#include "UIUpdateSystem.h"
#include "UI/UIControlSystem.h"
#include "Platform/SystemTimer.h"

namespace DAVA
{

UIUpdateSystem::UIUpdateSystem()
{
}

UIUpdateSystem::~UIUpdateSystem()
{
}

uint32 UIUpdateSystem::GetRequiredComponents() const
{
    return 0;
}

uint32 UIUpdateSystem::GetType() const
{
    return TYPE;
}

void UIUpdateSystem::Process()
{
    float32 timeElapsed = SystemTimer::FrameDelta();

    if (RenderManager::Instance()->GetOptions()->IsOptionEnabled(RenderOptions::UPDATE_UI_CONTROL_SYSTEM))
    {
        UIControl* currentScreen = UIControlSystem::Instance()->GetScreen();
        if (currentScreen)
        {
            SystemUpdate(currentScreen, timeElapsed);
        }

        UIControl* popupContainer = UIControlSystem::Instance()->GetPopupContainer();
        if (popupContainer)
        {
            SystemUpdate(popupContainer, timeElapsed);
        }

    }
}

void UIUpdateSystem::SystemUpdate(UIControl* control, float32 timeElapsed)
{
    UIControlSystem::Instance()->updateCounter++;

    if (control->customNeedUpdateCheck == (int)NULL || control->customNeedUpdateCheck())
    {
        control->Update(timeElapsed);
    }
    control->isUpdated = true;

    List<UIControl*>::iterator it = control->childs.begin();
    for (; it != control->childs.end(); ++it)
    {
        (*it)->isUpdated = false;
    }

    it = control->childs.begin();
    while (it != control->childs.end())
    {
        control->isIteratorCorrupted = false;
        UIControl *current = *it;
        if (current->customNeedSystemUpdateCheck != (int)NULL && !current->customNeedSystemUpdateCheck())
        {
            continue;
        }
        if (!current->isUpdated)
        {
            current->Retain();
            SystemUpdate(current, timeElapsed);
            current->Release();
            if (control->isIteratorCorrupted)
            {
                it = control->childs.begin();
                continue;
            }
        }
        ++it;
    }
}

}
