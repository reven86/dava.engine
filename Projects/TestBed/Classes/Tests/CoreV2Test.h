#if defined(__DAVAENGINE_COREV2__)

#pragma once

#include "Infrastructure/BaseScreen.h"

namespace DAVA
{
class UIButton;
class Font;
struct Rect;
class BaseObject;
};

class CoreV2Test : public BaseScreen
{
public:
    CoreV2Test(DAVA::Engine* e);
    ~CoreV2Test();

protected:
    void LoadResources() override;
    void UnloadResources() override;

private:
    void OnQuit(DAVA::BaseObject* obj, void* data, void* callerData);
    void OnResize(DAVA::BaseObject* obj, void* data, void* callerData);
    void OnRun(DAVA::BaseObject* obj, void* data, void* callerData);

    void OnWindowCreated(DAVA::Window* w);
    void OnWindowDestroyed(DAVA::Window* w);

    DAVA::UIButton* CreateUIButton(DAVA::Font* font, const DAVA::Rect& rect, const DAVA::String& text,
                                   void (CoreV2Test::*onClick)(DAVA::BaseObject*, void*, void*));

private:
    DAVA::UIButton* buttonQuit = nullptr;

    DAVA::UIButton* buttonResize640x480 = nullptr;
    DAVA::UIButton* buttonResize1024x768 = nullptr;

    DAVA::UIButton* buttonRunOnMain = nullptr;
    DAVA::UIButton* buttonRunOnUI = nullptr;

    DAVA::Engine* engine = nullptr;

    size_t tokenOnWindowCreated = 0;
    size_t tokenOnWindowDestroyed = 0;
};

#endif // __DAVAENGINE_COREV2__
