#pragma once

#include "ViewSceneScreen.h"

namespace OverdrawPerformanceTester
{
class ChartPainterSystem : public DAVA::SceneSystem
{
public:
    ChartPainterSystem(DAVA::Scene* scene, DAVA::Array<DAVA::Vector<ViewSceneScreen::FrameData>, 6>* preformanceData_);
    ~ChartPainterSystem();

    void Process(float32 timeElapsed) override;
    inline void SetShouldDrawGraph(bool val);
    inline bool GetShouldDrawGraph() const;

private:
    DAVA::Array<DAVA::Vector<ViewSceneScreen::FrameData>, 6>* performanceData;
    bool shouldDrawGraph = false;

    static const DAVA::Array<DAVA::Color, 6> chartColors;
};

void ChartPainterSystem::SetShouldDrawGraph(bool val)
{
    shouldDrawGraph = val;
}

bool ChartPainterSystem::GetShouldDrawGraph() const
{
    return shouldDrawGraph;
}
}