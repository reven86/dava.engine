#include "ChartPainterSystem.h"

#include "FrameData.h"
#include "OverdrawTesterComponent.h"

#include "Render/2D/Systems/RenderSystem2D.h"
#include "Render/rhi/dbg_Draw.h"

namespace OverdrawPerformanceTester
{
using DAVA::int32;
using DAVA::float32;
using DAVA::Vector2;
using DAVA::Color;
using DAVA::Array;
using DAVA::String;
using DAVA::Scene;
using DAVA::SceneSystem;
using DAVA::Polygon2;
using DAVA::UIControlSystem;
using DAVA::DbgDraw;
using DAVA::RenderSystem2D;
using DAVA::Array;
using DAVA::Vector;

const Vector2 ChartPainterSystem::chartOffset(0.1f, 0.1f);
const Color ChartPainterSystem::gridColor(0.4f, 0.4f, 0.4f, 0.4f);
const float32 ChartPainterSystem::chartLen = 0.8f;
const float32 ChartPainterSystem::minFrametime = 1.0f / 60.0f;
const float32 ChartPainterSystem::overdrawStep = 100.0f;
const float32 ChartPainterSystem::frametimeStep = 0.016f;

#define MODS_COUNT 6

const Array<String, MODS_COUNT> ChartPainterSystem::legend =
{ {
"0 tex",
"1 tex",
"2 tex",
"3 tex",
"4 tex",
"dep r"
} };

const Array<Color, MODS_COUNT> ChartPainterSystem::chartColors =
{ {
{ 0.0f, 1.0f, 0.0f, 1.0f },
{ 1.0f, 1.0f, 0.0f, 1.0f },
{ 0.0f, 1.0f, 1.0f, 1.0f },
{ 0.0f, 0.0f, 1.0f, 1.0f },
{ 1.0f, 0.0f, 1.0f, 1.0f },
{ 1.0f, 0.0f, 0.0f, 1.0f }
} };

ChartPainterSystem::ChartPainterSystem(Scene* scene, float32 maxFrametime_)
    : SceneSystem(scene)
    , maxFrametime(maxFrametime_)
    , textColor(rhi::NativeColorRGBA(1.0f, 1.0f, 1.0f, 1.0f))
{
    passConfig.colorBuffer[0].loadAction = rhi::LOADACTION_LOAD;
    passConfig.colorBuffer[0].storeAction = rhi::STOREACTION_STORE;
    passConfig.depthStencilBuffer.loadAction = rhi::LOADACTION_NONE;
    passConfig.depthStencilBuffer.storeAction = rhi::STOREACTION_NONE;
    passConfig.priority = DAVA::PRIORITY_MAIN_2D - 10;
    passConfig.viewport.x = 0;
    passConfig.viewport.y = 0;

    DAVA::DbgDraw::EnsureInited();
}

void ChartPainterSystem::AddEntity(DAVA::Entity* entity)
{
    OverdrawTesterComponent* comp = static_cast<OverdrawTesterComponent*>(entity->GetComponent(OverdrawTesterComponent::OVERDRAW_TESTER_COMPONENT));
    if (comp != nullptr)
    {
        maxOverdraw = comp->GetStepOverdraw() * comp->GetStepsCount();
        overdrawStepCount = maxOverdraw / overdrawStep;
    }
}

void ChartPainterSystem::Process(float32 timeElapsed)
{
    if (performanceData == nullptr)
        return;

    int32 w = UIControlSystem::Instance()->vcs->GetVirtualScreenSize().dx;
    int32 h = UIControlSystem::Instance()->vcs->GetVirtualScreenSize().dy;

    DrawLegend(w, h);

    DrawGrid(w, h);
    DrawCharts(w, h);

    FlushDbgText();
}

void ChartPainterSystem::DrawGrid(int32 w, int32 h) const
{
    Vector2 origin(chartOffset.x * w, (chartOffset.y + chartLen) * h);
    Vector2 xAxis((chartOffset.x + chartLen) * w, (chartOffset.y + chartLen) * h);
    Vector2 yAxis(chartOffset.x * w, chartOffset.y * h);

    Polygon2 gridPoly;
    gridPoly.AddPoint(origin);
    gridPoly.AddPoint(xAxis);
    RenderSystem2D::Instance()->DrawPolygon(gridPoly, false, Color::White);

    gridPoly.Clear();
    gridPoly.AddPoint(origin);
    gridPoly.AddPoint(yAxis);
    RenderSystem2D::Instance()->DrawPolygon(gridPoly, false, Color::White);

    int32 stepCount = static_cast<int>(frametimeStepCount + 1);
    for (int32 i = 1; i < stepCount + 1; i++)
    {
        gridPoly.Clear();
        float32 normalizedFps = (i * frametimeStep) / frametimeAxisLen;
        normalizedFps *= chartLen;
        float32 pointY = 1 - (normalizedFps + chartOffset.y);
        int32 pointYInt = static_cast<int32>(pointY * DAVA::Renderer::GetFramebufferHeight());
        pointY *= h;

        gridPoly.AddPoint({ chartOffset.x * w, pointY });
        gridPoly.AddPoint({ (chartOffset.x + chartLen) * w, pointY });

        DbgDraw::Text2D(static_cast<int32>(0.05f * DAVA::Renderer::GetFramebufferWidth()), pointYInt, textColor, "%.4f", i * frametimeStep + minFrametime);
        RenderSystem2D::Instance()->DrawPolygon(gridPoly, false, gridColor);
    }

    for (int32 i = 1; i < overdrawStepCount + 1; i++)
    {
        gridPoly.Clear();
        float32 normalizedOverdraw = (i * overdrawStep) / maxOverdraw;
        normalizedOverdraw *= chartLen;
        float32 pointX = normalizedOverdraw + chartOffset.x;
        int32 pointXInt = static_cast<int32>(pointX * DAVA::Renderer::GetFramebufferWidth());
        pointX *= w;

        gridPoly.AddPoint({ pointX, chartOffset.y * h });
        gridPoly.AddPoint({ pointX, (chartOffset.y + chartLen) * h });

        DbgDraw::Text2D(pointXInt, static_cast<int32>((chartOffset.y + chartLen) * DAVA::Renderer::GetFramebufferHeight()), textColor, "%.2f", i * overdrawStep);
        RenderSystem2D::Instance()->DrawPolygon(gridPoly, false, gridColor);
    }
}

void ChartPainterSystem::DrawCharts(int32 w, int32 h) const
{
    Polygon2 chartsPoly;
    for (int i = 0; i < MODS_COUNT; i++)
    {
        chartsPoly.Clear();
        for (size_t j = 0; j < (*performanceData)[i].size(); j++)
        {
            if (j == 0) // Skip noise in the first frame.
                continue;

            float32 overdraw = (*performanceData)[i][j].Overdraw;
            float32 fps = static_cast<float32>((*performanceData)[i][j].FrameTime) - minFrametime;

            float32 normalizedFps = fps / frametimeAxisLen;
            float32 normalizedOverdraw = overdraw / maxOverdraw;

            normalizedFps *= chartLen;
            normalizedOverdraw *= chartLen;

            float32 pointX = (normalizedOverdraw + chartOffset.x);
            float32 pointY = 1 - (normalizedFps + chartOffset.y);
            pointX *= w;
            pointY *= h;

            chartsPoly.AddPoint({ pointX, pointY });
        }
        RenderSystem2D::Instance()->DrawPolygon(chartsPoly, false, chartColors[i]);
    }
}

void ChartPainterSystem::FlushDbgText()
{
    passConfig.viewport.width = DAVA::Renderer::GetFramebufferWidth();
    passConfig.viewport.height = DAVA::Renderer::GetFramebufferHeight();

    DbgDraw::SetScreenSize(DAVA::Renderer::GetFramebufferWidth(), DAVA::Renderer::GetFramebufferHeight());

    rhi::HPacketList packetList;
    rhi::HRenderPass pass = rhi::AllocateRenderPass(passConfig, 1, &packetList);
    rhi::BeginRenderPass(pass);
    rhi::BeginPacketList(packetList);

    DbgDraw::FlushBatched(packetList);

    rhi::EndPacketList(packetList);
    rhi::EndRenderPass(pass);
}

void ChartPainterSystem::ProcessPerformanceData(Array<Vector<FrameData>, 6>* performanceData_)
{
    performanceData = performanceData_;
    // use maxFrametime = GetMaxFrametime(); to get adaptive y axis
    UpdateChartParameters();
}

void ChartPainterSystem::UpdateChartParameters()
{
    frametimeAxisLen = maxFrametime - minFrametime;
    frametimeStepCount = frametimeAxisLen / frametimeStep;
}

void ChartPainterSystem::DrawLegend(int32 w, int32 h) const
{
    static const float offsetDivider = 14.0f;
    static const float stepDivider = 7.0f;
    static const float lineOffsetDivider = 9.0f;
    static const float yOffsetMultiplier = 0.05f;

    float32 initialOffset = static_cast<float32>(w) / offsetDivider;
    float32 textInitialOffset = static_cast<float32>(DAVA::Renderer::GetFramebufferWidth()) / offsetDivider;
    float32 step = static_cast<float32>(w) / stepDivider;
    float32 textStep = static_cast<float32>(DAVA::Renderer::GetFramebufferWidth()) / stepDivider;
    int32 lineOffset = static_cast<int32>(static_cast<float32>(w) / lineOffsetDivider);
    int32 yPos = static_cast<int32>(yOffsetMultiplier * DAVA::Renderer::GetFramebufferHeight());
    float32 yPosFloat = yOffsetMultiplier * h;
    Polygon2 p;
    for (int i = 0; i < MODS_COUNT; i++)
    {
        p.Clear();

        int32 startX = static_cast<int32>(textStep * i + textInitialOffset);
        float32 startXFloat = step * i + initialOffset;
        p.AddPoint({ startXFloat, yPosFloat });
        p.AddPoint({ startXFloat + lineOffset, yPosFloat });
        DbgDraw::Text2D(startX, yPos, textColor, "%s", legend[i].c_str());
        RenderSystem2D::Instance()->DrawPolygon(p, false, chartColors[i]);
    }
}

float32 ChartPainterSystem::GetMaxFrametimeFromData() const
{
    // Looking for the max frametime element in whole data
    Array<float32, MODS_COUNT> frametimes;
    for (int i = 0; i < MODS_COUNT; i++)
    {
        Vector<FrameData>& currVector = (*performanceData)[i];
        auto begin = currVector.begin();
        if (i == 0) // Skip noise in first frame, when all textures are generated.
            begin++;

        frametimes[i] = (*std::max_element(currVector.begin(), currVector.end(),
                                           [](const FrameData& f1, const FrameData& f2)
                                           {
                                               return f1.FrameTime < f2.FrameTime;
                                           }
                                           ))
                        .FrameTime;
    }
    return *std::max_element(frametimes.begin(), frametimes.end());
}
#undef MODS_COUNT
}
