#ifndef __BASESCREEN_H__
#define __BASESCREEN_H__

#include "DAVAEngine.h"
#include "Infrastructure/GameCore.h"

class BaseScreen : public DAVA::UIScreen
{
protected:
    virtual ~BaseScreen()
    {
    }

public:
    BaseScreen();
    BaseScreen(const DAVA::String& screenName, DAVA::int32 skipBeforeTests = 10);

    inline DAVA::int32 GetScreenId();

protected:
    void LoadResources() override;
    void UnloadResources() override;
    bool SystemInput(DAVA::UIEvent* currentInput) override;

    virtual void OnExitButton(DAVA::BaseObject* obj, void* data, void* callerData);

private:
    static DAVA::int32 globalScreenId; // 1, on create of screen increment
    DAVA::int32 currentScreenId;
    DAVA::UIButton* exitButton;
};

DAVA::int32 BaseScreen::GetScreenId()
{
    return currentScreenId;
}


#endif // __BASESCREEN_H__
