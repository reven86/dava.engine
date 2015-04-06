#include "EditScreen.h"
#include "EditorSettings.h"

#include "ControlSelectionListener.h"

using namespace DAVA;


CheckeredCanvas::CheckeredCanvas()
: UIControl()
{
    GetBackground()->SetSprite("~res:/Gfx/CheckeredBg", 0);
    GetBackground()->SetDrawType(UIControlBackground::DRAW_TILED);
    GetBackground()->SetShader(SafeRetain(RenderSystem2D::TEXTURE_MUL_FLAT_COLOR));
}

CheckeredCanvas::~CheckeredCanvas()
{
    ClearSelections();
}

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

    if (GetScale().x >= scaleTresholdToDrawGrid)
    {
        Color gridColor = EditorSettings::Instance()->GetGrigColor();
        RenderHelper::Instance()->DrawGrid(geometricData.GetUnrotatedRect(), Vector2(GetScale().x, GetScale().x), gridColor, RenderState::RENDERSTATE_2D_BLEND);
    }
    
    for (auto &control : selectionControls)
    {
        Color oldColor = RenderManager::Instance()->GetColor();

        UIControl *parent = control->GetParent();
        if (parent && parent != this)
        {
            RenderManager::Instance()->SetColor(0.5f, 0.5f, 0.5f, 1.f);
            RenderHelper::Instance()->DrawRect(parent->GetGeometricData().GetUnrotatedRect(), RenderState::RENDERSTATE_2D_BLEND);
        }
        
        RenderManager::Instance()->SetColor(1, 0, 0, 1);
        RenderHelper::Instance()->DrawRect(control->GetGeometricData().GetUnrotatedRect(), RenderState::RENDERSTATE_2D_BLEND);
        
        RenderManager::Instance()->SetColor(oldColor);
    }
}

bool CheckeredCanvas::SystemInput(UIEvent *currentInput)
{
    if (currentInput->phase == UIEvent::PHASE_BEGAN || currentInput->phase == UIEvent::PHASE_DRAG)
    {
        UIControl *control = GetControlByPos(this, currentInput->point);
        if (nullptr != control)
        {
            UIControl *rootControl = control;
            while (rootControl->GetParent() != nullptr && rootControl->GetParent() != this)
            {
                rootControl = rootControl->GetParent();
            }
            if (rootControl->GetParent() == this)
            {
                for (auto listener : selectionListeners)
                {
                    listener->OnControlSelected(rootControl, control);
                }
            }
            else
            {
                DVASSERT(false);
            }
        }
        else
        {
            for (auto listener : selectionListeners)
            {
                listener->OnControlSelected(nullptr, nullptr);
            }
        }
    }
    return true;
}

void CheckeredCanvas::SelectControl(UIControl *control)
{
    if (selectionControls.find(control) == selectionControls.end())
    {
        selectionControls.insert(SafeRetain(control));
    }
}

void CheckeredCanvas::RemoveSelection(UIControl *control)
{
    auto it = selectionControls.find(control);
    if (it != selectionControls.end())
    {
        (*it)->Release();
        selectionControls.erase(it);
    }
}

void CheckeredCanvas::ClearSelections()
{
    for (auto &control : selectionControls)
    {
        control->Release();
    }
    selectionControls.clear();
}

UIControl *CheckeredCanvas::GetControlByPos(UIControl *control, const DAVA::Vector2 &pos)
{
    const List<UIControl*> &children = control->GetChildren();
    for (auto it = children.rbegin(); it != children.rend(); ++it)
    {
        UIControl *c = GetControlByPos(*it, pos);
        if (nullptr != c)
        {
            return c;
        }
    }
    
    if (control->IsPointInside(pos) && control->GetVisible() && control->GetVisibleForUIEditor())
    {
        return control;
    }
    return nullptr;
}

void CheckeredCanvas::AddControlSelectionListener(ControlSelectionListener *listener)
{
    selectionListeners.push_back(listener);
}

void CheckeredCanvas::RemoveControlSelectionListener(ControlSelectionListener *listener)
{
    auto it = std::find(selectionListeners.begin(), selectionListeners.end(), listener);
    if (it != selectionListeners.end())
    {
        selectionListeners.erase(it);
    }
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

        curY += rect.dy + 5;
    }
}
