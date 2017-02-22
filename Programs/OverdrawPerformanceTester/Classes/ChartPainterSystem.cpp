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

    if (!shouldDrawGraph) return;

    VirtualCoordinatesSystem* vcs = DAVA::UIControlSystem::Instance()->vcs;

    int32 w = vcs->GetVirtualScreenSize().dx;
    int32 h = vcs->GetVirtualScreenSize().dy;

    Vector2 offset(0.1f, 0.1f);

    Vector2 origin(offset.x * w, 0.9f * h);
    Vector2 xAxis(0.9f * w, 0.9f * h);
    Vector2 yAxis(0.1f * w, offset.y * h);

    Polygon2 p;
    p.AddPoint(origin);
    p.AddPoint(xAxis);
    RenderSystem2D::Instance()->DrawPolygon(p, false, Color::White);
    p.Clear();
    p.AddPoint(origin);
    p.AddPoint(yAxis);
    RenderSystem2D::Instance()->DrawPolygon(p, false, Color::White);

    float32 maxOverdraw = 1000.0f;
    float32 maxFps = 70.0f;
    float32 overdrawLen = 0.8f;
    float32 fpsLen = 0.8f;

    Color col(0.4f, 0.4f, 0.4f, 0.4f);
    for (int32 i = 1; i < 8; i++)
    {
        p.Clear();
        float32 normalizedFps = (i * 10) / maxFps;
        normalizedFps *= fpsLen;
        float32 pointY = 1 - (normalizedFps + offset.y);
        pointY *= h;
        p.AddPoint({ offset.x * w, pointY });
        p.AddPoint({ (offset.x + overdrawLen) * w, pointY });
        RenderSystem2D::Instance()->DrawPolygon(p, false, col);
    }

    for (int32 i = 1; i < 11; i++)
    {
        p.Clear();
        float32 normalizedOverdraw = (i * 100) / maxOverdraw;
        normalizedOverdraw *= overdrawLen;
        float32 pointX = normalizedOverdraw + offset.x;
        pointX *= w;
        p.AddPoint({ pointX, offset.y * h });
        p.AddPoint({ pointX, (offset.y + overdrawLen) * h });
        RenderSystem2D::Instance()->DrawPolygon(p, false, col);
    }

    for (int i = 0; i < 6; i++)
    {
        Polygon2 p;
        for (int j = 0; j < (*performanceData)[i].size(); j++)
        {
            float32 overdraw = (*performanceData)[i][j].Overdraw;
            float32 fps = static_cast<DAVA::float32>((*performanceData)[i][j].FPS);

            float32 normalizedFps = fps / maxFps;
            float32 normalizedOverdraw = overdraw / maxOverdraw;

            normalizedFps *= fpsLen;
            normalizedOverdraw *= overdrawLen;

            float32 pointX = (normalizedOverdraw + offset.x);
            float32 pointY = 1 - (normalizedFps + offset.y);
            pointX *= w;
            pointY *= h;

            p.AddPoint( { pointX, pointY } );
        }

        RenderSystem2D::Instance()->DrawPolygon(p, false, chartColors[i]);
    }

//     Polygon2 p;
//     p.AddPoint(Vector2(0, 0));
//     p.AddPoint(Vector2(static_cast<float32>(w), static_cast<float32>(h)));
//     RenderSystem2D::Instance()->DrawPolygon(p, false, DAVA::Color::White);
}

}