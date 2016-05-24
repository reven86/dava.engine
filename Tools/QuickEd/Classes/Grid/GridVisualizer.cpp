#include "GridVisualizer.h"
#include "Render/2D/Systems/RenderSystem2D.h"
using namespace DAVA;
// Construction/destruction.
GridVisualizer::GridVisualizer()
    :
    curScale(0.0f)
{
}

GridVisualizer::~GridVisualizer()
{
}

void GridVisualizer::SetScale(float32 scale)
{
    curScale = scale;
}

void GridVisualizer::DrawGridIfNeeded(const Rect& rect, UniqueHandle renderState)
{
    // Grid constants.
    static const Color gridColor = Color(0.5f, 0.5f, 0.5f, 0.5f); // TODO: customize with designers.
    static const Vector2 gridSize = Vector2(1.0f, 1.0f);
    static const float32 scaleTresholdToDrawGrid = 8.0f; // 800%+ as requested.

    if (curScale < scaleTresholdToDrawGrid)
    {
        return;
    }

    RenderSystem2D::Instance()->DrawGrid(rect, gridSize, gridColor);
}