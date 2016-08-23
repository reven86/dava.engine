#pragma once

#include "Infrastructure/BaseScreen.h"
#include "Functional/Signal.h"

class GameCore;
class AssertTest : public BaseScreen, public DAVA::TrackedObject
{
public:
    AssertTest(GameCore* g);

protected:
    void LoadResources() override;
    void UnloadResources() override;
    void Update(DAVA::float32 timeElapsed);

private:
    DAVA::float32 timeOut = 0.f;
    DAVA::RefPtr<DAVA::UIStaticText> countdownText;
};
