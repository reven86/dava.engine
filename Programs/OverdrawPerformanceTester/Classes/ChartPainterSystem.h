#pragma once

#include "ViewSceneScreen.h"

namespace OverdrawPerformanceTester
{
struct FrameData;

class ChartPainterSystem : public DAVA::SceneSystem
{
public:
    ChartPainterSystem(DAVA::Scene* scene);
    ~ChartPainterSystem();

    void Process(float32 timeElapsed) override;

    void FlushDbgText();

    void DrawCharts(int32 w, int32 h);

    void DrawGrid(int32 w, int32 h);

    inline void SetShouldDrawGraph(bool val);
    inline bool GetShouldDrawGraph() const;

    inline void SetPerformanceData(DAVA::Array<DAVA::Vector<FrameData>, 6>* performanceData_);

private:
    DAVA::Array<DAVA::Vector<FrameData>, 6>* performanceData;
    void DrawLegend(int32 w, int32 h);
    bool shouldDrawGraph = false;

    static const DAVA::Array<DAVA::Color, 6> chartColors;
    static const DAVA::Array<DAVA::String, 6> legend;
    static const DAVA::Vector2 chartOffset;
    static const DAVA::Color gridColor;
    static const DAVA::float32 chartLen;
    static const DAVA::float32 maxFrametime;
    static const DAVA::float32 maxOverdraw;
    static const DAVA::float32 overdrawStep;
    static const DAVA::float32 frametimeStep;
    static const DAVA::float32 minFrametime;
    static const DAVA::float32 frametimeAxisLen;
    static const DAVA::float32 overdrawStepCount;
    static const DAVA::float32 frametimeStepCount;
    const DAVA::uint32 textColor;
};

void ChartPainterSystem::SetPerformanceData(DAVA::Array<DAVA::Vector<FrameData>, 6>* performanceData_)
{
    performanceData = performanceData_;
}

void ChartPainterSystem::SetShouldDrawGraph(bool val)
{
    shouldDrawGraph = val;
}

bool ChartPainterSystem::GetShouldDrawGraph() const
{
    return shouldDrawGraph;
}
}