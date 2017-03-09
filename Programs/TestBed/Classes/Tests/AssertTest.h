#pragma once

#include "Infrastructure/BaseScreen.h"

class TestBed;
class AssertTest : public BaseScreen
{
public:
    AssertTest(TestBed& app);

protected:
    void LoadResources() override;
    void UnloadResources() override;
    void Update(DAVA::float32 timeElapsed) override;

private:
    void ShowMessageBoxFromMainThread();
    void ShowMessageBoxFromUIThread();
    void ShowMessageBoxFromOtherThreads();

    DAVA::float32 timeOut = 0.f;
    DAVA::RefPtr<DAVA::UIStaticText> countdownText;
};
