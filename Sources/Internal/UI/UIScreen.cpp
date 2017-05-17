#include "UI/UIScreen.h"
#include "UI/UIControlSystem.h"
#include "Render/RenderHelper.h"
#include "Render/2D/Systems/RenderSystem2D.h"
#include "Render/RHI/Common/PreProcess.h"
#include "Reflection/ReflectionRegistrator.h"
#include "Time/SystemTimer.h"

namespace DAVA
{
List<UIScreen*> UIScreen::appScreens;
int32 UIScreen::groupIdCounter = -1;

DAVA_VIRTUAL_REFLECTION_IMPL(UIScreen)
{
    ReflectionRegistrator<UIScreen>::Begin()
    .ConstructorByPointer()
    .DestructorByPointer([](UIScreen* o) { o->Release(); })
    .End();
}

UIScreen::UIScreen(const Rect& rect)
    : UIControl(rect)
    , groupId(groupIdCounter)
{
    // add screen to list
    appScreens.push_back(this);
    groupIdCounter--;
    isLoaded = false;
}

UIScreen::~UIScreen()
{
    // remove screen from list
    for (List<UIScreen*>::iterator t = appScreens.begin(); t != appScreens.end(); ++t)
    {
        if (*t == this)
        {
            appScreens.erase(t);
            break;
        }
    }
}

void UIScreen::SystemScreenSizeChanged(const Rect& newFullScreenRect)
{
    SetSize(newFullScreenRect.GetSize());
    UIControl::SystemScreenSizeChanged(newFullScreenRect);
}

void UIScreen::LoadGroup()
{
    //Logger::FrameworkDebug("load group started");
    //uint64 loadGroupStart = SystemTimer::GetMs();
    ShaderPreprocessScope preprocessScope;

    if (groupId < 0)
    {
        if (isLoaded)
            return;

        LoadResources();
        isLoaded = true;
    }
    else
    {
        int32 screenGroupId = groupId;
        for (List<UIScreen*>::iterator t = appScreens.begin(); t != appScreens.end(); ++t)
        {
            UIScreen* screen = *t;
            if ((screen->groupId == screenGroupId) && (!screen->isLoaded))
            {
                screen->LoadResources();
                screen->isLoaded = true;
            }
        }
    }
    //uint64 loadGroupEnd = SystemTimer::GetMs();
    //Logger::FrameworkDebug("load group finished: %lld", loadGroupEnd - loadGroupStart);
}

void UIScreen::UnloadGroup()
{
    if (groupId < 0)
    {
        if (!isLoaded)
            return;

        UnloadResources();
        isLoaded = false;
    }
    else
    {
        int32 screenGroupId = groupId;
        for (List<UIScreen*>::iterator t = appScreens.begin(); t != appScreens.end(); ++t)
        {
            UIScreen* screen = *t;
            if ((screen->groupId == screenGroupId) && (screen->isLoaded))
            {
                screen->UnloadResources();
                screen->isLoaded = false;
            }
        }
    }
}

bool UIScreen::IsLoaded()
{
    return isLoaded;
}

void UIScreen::AddToGroup(int32 _groupId)
{
    groupId = _groupId;
}

void UIScreen::RemoveFromGroup()
{
    groupId = groupIdCounter--;
}

int32 UIScreen::GetGroupId()
{
    return groupId;
}
};