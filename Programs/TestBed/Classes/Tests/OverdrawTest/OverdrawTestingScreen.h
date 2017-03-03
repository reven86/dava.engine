#pragma once

#include "UI/UIScreen.h"

class TestBed;

namespace DAVA
{
class Scene;
}

namespace OverdrawPerformanceTester
{
class OverdrawTesterSystem;
class ChartPainterSystem;
}

class OverdrawTestingScreen : public DAVA::UIScreen
{
protected:
    virtual ~OverdrawTestingScreen()
    {
    }

public:
    OverdrawTestingScreen(TestBed& app_);
    virtual void LoadResources();
    virtual void UnloadResources();

protected:
    DAVA::Scene* scene = nullptr;
    TestBed& app;

    OverdrawPerformanceTester::OverdrawTesterSystem* testerSystem;
    OverdrawPerformanceTester::ChartPainterSystem* chartPainterSystem;
};
