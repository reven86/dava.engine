#pragma once

#include "UI/UIScreen.h"
#include "Base/BaseTypes.h"

class TestBed;

namespace DAVA
{
class Scene;
class UIButton;
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
    void OnExitButton(DAVA::BaseObject* obj, void* data, void* callerData);

private:
    DAVA::UIButton* exitButton = nullptr;
    DAVA::Size2i initialVcsSize = {};
    DAVA::Scene* scene = nullptr;
    TestBed& app;

    OverdrawPerformanceTester::OverdrawTesterSystem* testerSystem;
    OverdrawPerformanceTester::ChartPainterSystem* chartPainterSystem;
};
