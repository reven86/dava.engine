#pragma once

#include "BaseScreen.h"

namespace OverdrawPerformanceTester
{
class OverdrawTesterSystem;
class ChartPainterSystem;
}

class OverdrawTestingScreen : public BaseScreen
{
protected:
    OverdrawTestingScreen(TestBed& app_, const DAVA::String& screenName);
    virtual ~OverdrawTestingScreen()
    {
    }

public:
    virtual void LoadResources();
    virtual void UnloadResources();

protected:
    DAVA::Scene* scene = nullptr;
    TestBed& app;

    OverdrawPerformanceTester::OverdrawTesterSystem* testerSystem;
    OverdrawPerformanceTester::ChartPainterSystem* chartPainterSystem;
};
