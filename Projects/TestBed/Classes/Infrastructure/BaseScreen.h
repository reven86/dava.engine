#pragma once

#include "DAVAEngine.h"

class GameCore;
class BaseScreen : public DAVA::UIScreen
{
protected:
    virtual ~BaseScreen()
    {
    }

public:
    BaseScreen(GameCore& gameCore, const DAVA::String& screenName, DAVA::int32 skipBeforeTests = 10);

    inline DAVA::int32 GetScreenId();

protected:
    void LoadResources() override;
    void UnloadResources() override;
    bool SystemInput(DAVA::UIEvent* currentInput) override;

    virtual void OnExitButton(DAVA::BaseObject* obj, void* data, void* callerData);

    GameCore& gameCore;

private:
    static DAVA::int32 globalScreenId; // 1, on create of screen increment

    DAVA::int32 currentScreenId;
    DAVA::UIButton* exitButton = nullptr;
};

DAVA::int32 BaseScreen::GetScreenId()
{
    return currentScreenId;
}
