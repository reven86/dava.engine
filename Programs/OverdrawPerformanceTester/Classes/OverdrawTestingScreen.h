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
    virtual ~OverdrawTestingScreen()
    {
    }

public:
    virtual void LoadResources();
    virtual void UnloadResources();

protected:
    DAVA::Scene* scene = nullptr;

    OverdrawPerformanceTester::OverdrawTesterSystem* testerSystem;
    OverdrawPerformanceTester::ChartPainterSystem* chartPainterSystem;
};
