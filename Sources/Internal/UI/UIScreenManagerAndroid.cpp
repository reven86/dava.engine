#include "Base/Platform.h"

#if defined(__DAVAENGINE_ANDROID__)

#include "Base/BaseObject.h"
#include "UI/UIScreenManager.h"

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
            ((UIScreen*)it->second.value)->UnloadGroup();
            //			it->second.type == Screen::TYPE_NULL;
            releaseBuf.push_back(it->second);
        }
    }
    for (Vector<Screen>::const_iterator it = releaseBuf.begin(); it != releaseBuf.end(); it++)
    {
        ((UIScreen*)it->value)->Release();
    }
}

void UIScreenManager::SetFirst(int screenId)
{
    DVASSERT(activeScreenId == -1 && "[UIScreenManager::SetFirst] called twice");

    Screen& screen = screens[screenId];
    if (screen.type == Screen::TYPE_SCREEN)
    {
        activeScreenId = screenId;
        UIControlSystem::Instance()->SetScreen((UIScreen*)screen.value);
    }
    else
    {
        Logger::Error("[UIScreenManager::SetFirst] wrong type of screen");
    }
    Logger::Debug("[UIScreenManager::SetFirst] done");
}

void UIScreenManager::SetScreen(int screenId, UIScreenTransition* transition)
{
    Logger::Debug("[ScreenManager::SetScreen] screenID = %d", screenId);

    Screen& screen = screens[screenId];
    if (screen.type == Screen::TYPE_SCREEN)
    {
        activeScreenId = screenId;
        UIControlSystem::Instance()->SetScreen((UIScreen*)screen.value, transition);
    }

    Logger::Debug("[ScreenManager::SetScreen] done");
}

void UIScreenManager::RegisterScreen(int screenId, UIScreen* screen)
{
    screens[screenId] = Screen(Screen::TYPE_SCREEN, SafeRetain(screen));
}

void UIScreenManager::ResetScreen()
{
    activeScreenId = -1;
    UIControlSystem::Instance()->Reset();
}

UIScreen* UIScreenManager::GetScreen(int screenId)
{
    Screen& screen = screens[screenId];
    if (screen.type == Screen::TYPE_SCREEN)
    {
        return (UIScreen*)screen.value;
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

/*void ScreenManager::StopGLAnimation()
{
	Screen & glController = screens[glControllerId];
	EAGLViewController * controller = (EAGLViewController *)glController.value;
	EAGLView * view = (EAGLView *)controller.view;
	[view stopAnimation];
}

void ScreenManager::StartGLAnimation()
{
	Screen & glController = screens[glControllerId];
	EAGLViewController * controller = (EAGLViewController *)glController.value;
	EAGLView * view = (EAGLView *)controller.view;
	[view startAnimation];
}*/
};

#endif // __DAVAENGINE_ANDROID__
