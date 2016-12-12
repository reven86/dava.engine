#pragma once

#include "Infrastructure/BaseScreen.h"

class TestBed;

class SamplePluginTest final : public BaseScreen
{
public:
    SamplePluginTest(TestBed& app);

protected:
    void LoadResources() override;
    void UnloadResources() override;

private:
    DAVA::Engine& engine;
};
