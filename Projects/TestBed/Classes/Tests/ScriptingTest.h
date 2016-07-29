#pragma once

#include "Infrastructure/BaseScreen.h"

class ScriptingTest : public BaseScreen
{
public:
    ScriptingTest();

protected:
    void LoadResources() override;
    void UnloadResources() override;
};
