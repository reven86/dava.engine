#pragma once

#include "Infrastructure/BaseScreen.h"
#include "Functional/Signal.h"

class TestBed;
class AssertTest : public BaseScreen, public DAVA::TrackedObject
{
public:
    AssertTest(TestBed& app);

protected:
    void LoadResources() override;
    void UnloadResources() override;

private:
    void Update(DAVA::float32 timeElapsed);

    DAVA::float32 timeOut = 0.f;
    DAVA::RefPtr<DAVA::UIStaticText> countdownText;
};
