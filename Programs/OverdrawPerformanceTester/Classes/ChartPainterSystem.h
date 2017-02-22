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

    void FlushDbgText();

    void DrawCharts(int32 w, int32 h);

    void DrawGrid(int32 w, int32 h);

    inline void SetShouldDrawGraph(bool val);
    inline bool GetShouldDrawGraph() const;

private:
    DAVA::Array<DAVA::Vector<ViewSceneScreen::FrameData>, 6>* performanceData;
    bool shouldDrawGraph = false;

    static const DAVA::Array<DAVA::Color, 6> chartColors;
    static const DAVA::Vector2 chartOffset;
    static const DAVA::Color gridColor;
    static const DAVA::float32 chartLen;
    static const DAVA::float32 maxFps;
    static const DAVA::float32 maxOverdraw;
    static const DAVA::float32 overdrawStep;
    static const DAVA::float32 fpsStep;
    static const DAVA::float32 overdrawStepCount;
    static const DAVA::float32 fpsStepCount;
    const DAVA::uint32 textColor;
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