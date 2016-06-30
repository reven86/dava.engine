#ifndef __BASESCREEN_H__
#define __BASESCREEN_H__

#include "DAVAEngine.h"

class GameCore;
class BaseScreen : public DAVA::UIScreen
{
protected:
    virtual ~BaseScreen()
    {
    }

public:
    BaseScreen(GameCore* g);
    BaseScreen(GameCore* g, const DAVA::String& screenName, DAVA::int32 skipBeforeTests = 10);

    inline DAVA::int32 GetScreenId();

protected:
    void LoadResources() override;
    void UnloadResources() override;
    bool SystemInput(DAVA::UIEvent* currentInput) override;

    virtual void OnExitButton(DAVA::BaseObject* obj, void* data, void* callerData);

private:
    static DAVA::int32 globalScreenId; // 1, on create of screen increment

    GameCore* gameCore = nullptr;
    DAVA::int32 currentScreenId;
    DAVA::UIButton* exitButton;
};

DAVA::int32 BaseScreen::GetScreenId()
{
    return currentScreenId;
}


#endif // __BASESCREEN_H__
