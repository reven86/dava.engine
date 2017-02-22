#include "ChartPainterSystem.h"

#include "Render/2D/Systems/RenderSystem2D.h"
#include "Render/rhi/dbg_Draw.h"

namespace OverdrawPerformanceTester
{

const DAVA::Array<DAVA::Color, 6> ChartPainterSystem::chartColors =
{ {
    { 0.0f, 1.0f, 0.0f, 1.0f},
    { 1.0f, 1.0f, 0.0f, 1.0f},
    { 0.0f, 1.0f, 1.0f, 1.0f},
    { 0.0f, 0.0f, 1.0f, 1.0f},
    { 1.0f, 0.0f, 1.0f, 1.0f},
    { 1.0f, 0.0f, 0.0f, 1.0f}
} };

ChartPainterSystem::ChartPainterSystem(DAVA::Scene* scene, DAVA::Array<DAVA::Vector<ViewSceneScreen::FrameData>, 6>* preformanceData_)
    : DAVA::SceneSystem(scene), performanceData(preformanceData_)
{

}

ChartPainterSystem::~ChartPainterSystem()
{

}

void ChartPainterSystem::Process(float32 timeElapsed)
{
    DAVA::DbgDraw::EnsureInited();
    uint32 TEXT_COLOR = rhi::NativeColorRGBA(1.f, 1.f, 1.f, 1.f);
    DAVA::DbgDraw::Text2D(100, 100, TEXT_COLOR, "some text %d", 5);
    DAVA::DbgDraw::FilledRect2D(0, 0, 200, 200, TEXT_COLOR);


    VirtualCoordinatesSystem* vcs = DAVA::UIControlSystem::Instance()->vcs;
//     vcs->SetVirtualScreenSize(10, 10);


    int32 w = vcs->GetVirtualScreenSize().dx;
    int32 h = vcs->GetVirtualScreenSize().dy;

    Vector2 origin(0.1f * w, 0.9f * h);
    Vector2 xAxis(0.9f * w, 0.9f * h);
    Vector2 yAxis(0.1f * w, 0.1f * h);

    Polygon2 p;
    p.AddPoint(origin);
    p.AddPoint(xAxis);
    RenderSystem2D::Instance()->DrawPolygon(p, false, Color::White);
    p.Clear();
    p.AddPoint(origin);
    p.AddPoint(yAxis);
    RenderSystem2D::Instance()->DrawPolygon(p, false, Color::White);

    if (!shouldDrawGraph) return;

    for (int i = 0; i < 6; i++)
    {
        Polygon2 p;
        for (int j = 0; j < (*performanceData)[i].size(); j++)
        {
            float32 overdraw = (*performanceData)[i][j].Overdraw;
            float32 fps = static_cast<DAVA::float32>((*performanceData)[i][j].FPS);
            fps *= 10;
            fps = h - fps;

            p.AddPoint( { overdraw, fps } );
        }

        RenderSystem2D::Instance()->DrawPolygon(p, false, chartColors[i]);
    }

//     Polygon2 p;
//     p.AddPoint(Vector2(0, 0));
//     p.AddPoint(Vector2(static_cast<float32>(w), static_cast<float32>(h)));
//     RenderSystem2D::Instance()->DrawPolygon(p, false, DAVA::Color::White);
}

}