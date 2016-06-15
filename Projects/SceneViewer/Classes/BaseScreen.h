#ifndef __BASE_SCREEN_H__
#define __BASE_SCREEN_H__

#include "DAVAEngine.h"

using namespace DAVA;

class BaseScreen : public UIScreen
{
    static int32 screensCount;

protected:
    virtual ~BaseScreen()
    {
    }

public:
    BaseScreen();

    virtual void LoadResources();
    virtual void UnloadResources();
    bool SystemInput(UIEvent* currentInput) override;
    void SystemScreenSizeChanged(const Rect& newFullScreenSize) override;

    int32 GetScreenID() const
    {
        return screenID;
    };

protected:
    UIButton* CreateButton(const Rect& rect, const WideString& text);

    void SetPreviousScreen() const;
    void SetNextScreen() const;

    Font* font;

    int32 screenID;
    bool loaded;
};

#endif //__BASE_SCREEN_H__
