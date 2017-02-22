#include "ChartPainterSystem.h"

#include "Render/2D/Systems/RenderSystem2D.h"
#include "Render/rhi/dbg_Draw.h"

namespace OverdrawPerformanceTester
{

const DAVA::Vector2 ChartPainterSystem::chartOffset(0.1f, 0.1f);
const DAVA::Color ChartPainterSystem::gridColor(0.4f, 0.4f, 0.4f, 0.4f);
const DAVA::float32 ChartPainterSystem::chartLen = 0.8f;
const DAVA::float32 ChartPainterSystem::maxFps = 70.0f;
const DAVA::float32 ChartPainterSystem::maxOverdraw = 1000.0f;
const DAVA::float32 ChartPainterSystem::overdrawStep = 100.0f;
const DAVA::float32 ChartPainterSystem::fpsStep = 10.0f;
const DAVA::float32 ChartPainterSystem::overdrawStepCount = maxOverdraw / overdrawStep;
const DAVA::float32 ChartPainterSystem::fpsStepCount = maxFps / fpsStep;

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
    : DAVA::SceneSystem(scene), performanceData(preformanceData_), textColor(rhi::NativeColorRGBA(1.0f, 1.0f, 1.0f, 1.0f))
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
    
    DrawGrid(w, h);
    DrawCharts(w, h);

    FlushDbgText();
}

void ChartPainterSystem::DrawGrid(int32 w, int32 h)
{
    Vector2 origin(chartOffset.x * w, (chartOffset.y + chartLen) * h);
    Vector2 xAxis((chartOffset.x + chartLen) * w, (chartOffset.y + chartLen) * h);
    Vector2 yAxis(chartOffset.x * w, chartOffset.y * h);

    Polygon2 p;
    p.AddPoint(origin);
    p.AddPoint(xAxis);
    RenderSystem2D::Instance()->DrawPolygon(p, false, Color::White);
    p.Clear();
    p.AddPoint(origin);
    p.AddPoint(yAxis);
    RenderSystem2D::Instance()->DrawPolygon(p, false, Color::White);

    for (int32 i = 1; i < fpsStepCount + 1; i++)
    {
        p.Clear();
        float32 normalizedFps = (i * fpsStep) / maxFps;
        normalizedFps *= chartLen;
        float32 pointY = 1 - (normalizedFps + chartOffset.y);
        pointY *= h;
        p.AddPoint({ chartOffset.x * w, pointY });
        p.AddPoint({ (chartOffset.x + chartLen) * w, pointY });
        RenderSystem2D::Instance()->DrawPolygon(p, false, gridColor);

        DbgDraw::Text2D(static_cast<int32>(chartOffset.x * w), static_cast<int32>(pointY), textColor, "%f", i * fpsStep);
    }

    for (int32 i = 1; i < overdrawStepCount + 1; i++)
    {
        p.Clear();
        float32 normalizedOverdraw = (i * overdrawStep) / maxOverdraw;
        normalizedOverdraw *= chartLen;
        float32 pointX = normalizedOverdraw + chartOffset.x;
        pointX *= w;
        p.AddPoint({ pointX, chartOffset.y * h });
        p.AddPoint({ pointX, (chartOffset.y + chartLen) * h });
        RenderSystem2D::Instance()->DrawPolygon(p, false, gridColor);
        DbgDraw::Text2D(static_cast<int32>(pointX), static_cast<int32>((chartOffset.y + chartLen) * h), textColor, "%f", i * overdrawStep);
    }
}

void ChartPainterSystem::DrawCharts(int32 w, int32 h)
{
    for (int i = 0; i < 6; i++)
    {
        Polygon2 p;
        for (int j = 0; j < (*performanceData)[i].size(); j++)
        {
            float32 overdraw = (*performanceData)[i][j].Overdraw;
            float32 fps = static_cast<DAVA::float32>((*performanceData)[i][j].FPS);

            float32 normalizedFps = fps / maxFps;
            float32 normalizedOverdraw = overdraw / maxOverdraw;

            normalizedFps *= chartLen;
            normalizedOverdraw *= chartLen;

            float32 pointX = (normalizedOverdraw + chartOffset.x);
            float32 pointY = 1 - (normalizedFps + chartOffset.y);
            pointX *= w;
            pointY *= h;

            p.AddPoint({ pointX, pointY });
        }

        RenderSystem2D::Instance()->DrawPolygon(p, false, chartColors[i]);
    }
}

void ChartPainterSystem::FlushDbgText()
{
    rhi::RenderPassConfig passConfig;
    passConfig.colorBuffer[0].loadAction = rhi::LOADACTION_LOAD;
    passConfig.colorBuffer[0].storeAction = rhi::STOREACTION_STORE;
    passConfig.depthStencilBuffer.loadAction = rhi::LOADACTION_NONE;
    passConfig.depthStencilBuffer.storeAction = rhi::STOREACTION_NONE;
    passConfig.priority = PRIORITY_MAIN_2D - 10;
    passConfig.viewport.x = 0;
    passConfig.viewport.y = 0;
    passConfig.viewport.width = Renderer::GetFramebufferWidth();
    passConfig.viewport.height = Renderer::GetFramebufferHeight();

    DbgDraw::SetScreenSize(Renderer::GetFramebufferWidth(), Renderer::GetFramebufferHeight());

    rhi::HPacketList packetList;
    rhi::HRenderPass pass = rhi::AllocateRenderPass(passConfig, 1, &packetList);
    rhi::BeginRenderPass(pass);
    rhi::BeginPacketList(packetList);

    DbgDraw::FlushBatched(packetList);

    rhi::EndPacketList(packetList);
    rhi::EndRenderPass(pass);
}

}