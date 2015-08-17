/*==================================================================================
    Copyright (c) 2008, binaryzebra
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    * Neither the name of the binaryzebra nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE binaryzebra AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL binaryzebra BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/


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
    if (emulationMode)
        return UIControl::SystemInput(currentInput);
    
    DAVA::List<std::pair<UIControl*, UIControl*> > selectedControls;
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
                selectedControls.push_back(std::make_pair(rootControl, control));
            }
        }
        for (auto listener : selectionListeners)
        {
            listener->OnControlSelected(selectedControls);
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

void CheckeredCanvas::SetEmulationMode(bool newMode)
{
    emulationMode = newMode;
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
