#include "UI/UIScreenManager.h"

#if defined(__DAVAENGINE_MACOS__) || defined(__DAVAENGINE_WINDOWS__) || defined(__DAVAENGINE_LINUX__)

#include "Base/BaseObject.h"

namespace DAVA
{
UIScreenManager::UIScreenManager()
{
    glControllerId = -1;
    activeControllerId = -1;
    activeScreenId = -1;
}

UIScreenManager::~UIScreenManager()
{
    Vector<Screen> releaseBuf;
    for (Map<int, Screen>::const_iterator it = screens.begin(); it != screens.end(); it++)
    {
        if (it->second.type == Screen::TYPE_SCREEN)
        {
            (static_cast<UIScreen*>(it->second.value))->UnloadGroup();
            //			it->second.type == Screen::TYPE_NULL;
            releaseBuf.push_back(it->second);
        }
    }
    for (Vector<Screen>::const_iterator it = releaseBuf.begin(); it != releaseBuf.end(); it++)
    {
        (static_cast<UIScreen*>(it->value))->Release();
    }
}

void UIScreenManager::SetFirst(int screenId)
{
    DVASSERT(activeScreenId == -1 && "[UIScreenManager::SetFirst] called twice");

    Screen& screen = screens[screenId];
    if (screen.type == Screen::TYPE_SCREEN)
    {
        activeScreenId = screenId;
        UIControlSystem::Instance()->SetScreen(static_cast<UIScreen*>(screen.value));
    }
    else
    {
        Logger::Error("ScreenManager::SetFirst wrong type of screen");
    }
}

void UIScreenManager::SetScreen(int screenId, UIScreenTransition* transition)
{
    Screen& screen = screens[screenId];
    if (screen.type == Screen::TYPE_SCREEN)
    {
        activeScreenId = screenId;
        UIControlSystem::Instance()->SetScreen(static_cast<UIScreen*>(screen.value), transition);
    }
}

void UIScreenManager::ResetScreen()
{
    activeScreenId = -1;
    UIControlSystem::Instance()->Reset();
}

void UIScreenManager::RegisterScreen(int screenId, UIScreen* screen)
{
    DVASSERT(screen != 0);
    screens[screenId] = Screen(Screen::TYPE_SCREEN, SafeRetain(screen));
}

UIScreen* UIScreenManager::GetScreen(int screenId)
{
    Screen& screen = screens[screenId];
    if (screen.type == Screen::TYPE_SCREEN)
    {
        return static_cast<UIScreen*>(screen.value);
    }
    return NULL;
}

UIScreen* UIScreenManager::GetScreen()
{
    return GetScreen(activeScreenId);
}
int32 UIScreenManager::GetScreenId()
{
    return activeScreenId;
}
}

#endif // defined(__DAVAENGINE_MACOS__) || defined(__DAVAENGINE_WINDOWS__) || defined(__DAVAENGINE_LINUX__)
