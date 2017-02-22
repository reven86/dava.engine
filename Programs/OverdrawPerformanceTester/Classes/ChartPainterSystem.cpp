#include "ChartPainterSystem.h"

#include "Render/2D/Systems/RenderSystem2D.h"

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
    if (!shouldDrawGraph) return;

    int32 h = Renderer::GetFramebufferHeight();
    int32 w = Renderer::GetFramebufferWidth();
    for (int i = 0; i < 6; i++)
    {
        Polygon2 p;
        for (int j = 0; j < performanceData[i].size(); j++)
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