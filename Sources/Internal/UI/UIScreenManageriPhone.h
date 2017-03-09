#ifndef __DAVAENGINE_IPHONEAPP_SCREENMANAGER_C_H__
#define __DAVAENGINE_IPHONEAPP_SCREENMANAGER_C_H__

#include "DAVAEngine.h"

namespace DAVA
{
class UIScreenManager : public Singleton<UIScreenManager>
{
public:
    UIScreenManager();
    virtual ~UIScreenManager();

    void RegisterController(int controllerId, void* controller);
    void RegisterScreen(int screenId, UIScreen* screen);

    void SetGLControllerId(int glController);

    void SetFirst(int screenId);
    void SetScreen(int screenId, UIScreenTransition* transition = 0);
    void ResetScreen();

    UIScreen* GetScreen(int screenId);
    UIScreen* GetScreen();
    int GetScreenId();

    void* GetController(int controllerId);
    void* GetController();
    int GetControllerId();

    // GetScreen, GetController
    // Stack of the screens
    // void SetScreen(int screen);
    void StopGLAnimation();
    void StartGLAnimation();

#if !defined(__DAVAENGINE_COREV2__)
    // Yuri Coder, 2013/02/06. Temporary method exist for iOS implementation only.
    // It blocks drawing of the RenderView, introduced for displaying assert messages.
    void BlockDrawing();
    void UnblockDrawing();
#endif // !__DAVAENGINE_COREV2__

private:
    void ActivateGLController();

    struct Screen
    {
        enum eType
        {
            TYPE_NULL = 0,
            TYPE_CONTROLLER,
            TYPE_SCREEN,
        };
        Screen(eType _type = TYPE_NULL, void* _value = 0)
        {
            type = _type;
            value = _value;
        }
        eType type;
        void* value;
    };

    Map<int, Screen> screens;
    int glControllerId;
    int activeControllerId;
    int activeScreenId;
#if defined(__DAVAENGINE_COREV2__)
    bool drawBlocked;
#endif // __DAVAENGINE_COREV2__
};
};

#endif // __DAVAENGINE_IPHONEAPP_SCREENMANAGER_C_H__
