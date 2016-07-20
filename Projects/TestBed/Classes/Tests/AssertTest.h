#pragma once

#include "Infrastructure/BaseScreen.h"

class AssertTest : public BaseScreen
{
public:
    AssertTest();

protected:
    void LoadResources() override;
    void UnloadResources() override;
    void Update(DAVA::float32 timeElapsed) override;

private:
    DAVA::float32 timeOut = 0.f;
};
