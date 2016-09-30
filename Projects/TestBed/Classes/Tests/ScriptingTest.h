#pragma once

#include "Infrastructure/BaseScreen.h"

class ScriptingTest : public BaseScreen
{
public:
    ScriptingTest(GameCore* g);

protected:
    void LoadResources() override;
    void UnloadResources() override;
    void Update(DAVA::float32 timeElapsed) override;

private:
    DAVA::RefPtr<DAVA::UITextField> scriptText;
    DAVA::RefPtr<DAVA::UITextField> intArgText;
    DAVA::RefPtr<DAVA::UITextField> strArgText;
    DAVA::RefPtr<DAVA::UIStaticText> outputText;
    DAVA::RefPtr<DAVA::UIStaticText> timeText;
};
