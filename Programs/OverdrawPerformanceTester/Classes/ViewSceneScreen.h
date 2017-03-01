#pragma once

#include "BaseScreen.h"

namespace OverdrawPerformanceTester
{
class OverdrawTesterSystem;
class ChartPainterSystem;
}

class ViewSceneScreen : public BaseScreen
{
protected:
    virtual ~ViewSceneScreen()
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
