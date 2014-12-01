#include "EditScreen.h"
#include "EditorSettings.h"

using namespace DAVA;


CheckeredCanvas::CheckeredCanvas()
: UIControl()
{
    GetBackground()->SetSprite("~res:/Gfx/CheckeredBg", 0);
    GetBackground()->SetDrawType(UIControlBackground::DRAW_TILED);
    GetBackground()->SetShader(SafeRetain(RenderManager::TEXTURE_MUL_FLAT_COLOR));
}

CheckeredCanvas::~CheckeredCanvas()
{}

void CheckeredCanvas::Draw( const UIGeometricData &geometricData )
{
    float32 invScale = 1.0f / geometricData.scale.x;
    UIGeometricData unscaledGd;
    unscaledGd.scale = Vector2(invScale, invScale);
    unscaledGd.size = geometricData.size * geometricData.scale.x;
    unscaledGd.AddGeometricData(geometricData);
    GetBackground()->Draw(unscaledGd);
}

void CheckeredCanvas::DrawAfterChilds( const UIGeometricData &geometricData )
{
    // Grid constants.
    static const float32 scaleTresholdToDrawGrid = 8.0f; // 800%+ as requested.

    if (GetScale().x < scaleTresholdToDrawGrid)
    {
        return;
    }

    Color gridColor = EditorSettings::Instance()->GetGrigColor();
    RenderHelper::Instance()->DrawGrid(geometricData.GetUnrotatedRect(), Vector2(GetScale().x, GetScale().x), gridColor, RenderState::RENDERSTATE_2D_BLEND);
}

PackageCanvas::PackageCanvas()
    : UIControl()
{

}

PackageCanvas::~PackageCanvas()
{

}

void PackageCanvas::LayoutCanvas()
{
    float32 maxWidth = 0.0f;
    float32 totalHeight = 0.0f;
    for (List< UIControl* >::const_iterator iter = childs.begin(); iter != childs.end(); ++iter)
    {
        maxWidth = Max(maxWidth, (*iter)->GetSize().x);
        totalHeight += (*iter)->GetSize().y;
    }

    SetSize(Vector2(maxWidth, totalHeight));

    float32 curY = 0.0f;
    for (List< UIControl* >::const_iterator iter = childs.begin(); iter != childs.end(); ++iter)
    {
        UIControl* control = *iter;

        Rect rect = control->GetRect();
        rect.y = curY;
        rect.x = (maxWidth - rect.dx)/2.0f;
        control->SetRect(rect);

        curY += rect.dy;
    }
}
