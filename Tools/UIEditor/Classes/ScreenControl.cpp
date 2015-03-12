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



#include "ScreenControl.h"
#include <QString>
#include "HierarchyTreeNode.h"

#include "Render/RenderHelper.h"
#include "Render/RenderManager.h"
#include "Grid/GridVisualizer.h"

#include "DefaultScreen.h"
#include "ScreenWrapper.h"

ScreenControl::ScreenControl() :
    screenShotMode(false)
{
    chequeredBackground = new UIControlBackground();
    chequeredBackground->SetSprite("~res:/Gfx/chequered", 0);
    chequeredBackground->SetDrawType(UIControlBackground::DRAW_TILED);
}

ScreenControl::~ScreenControl()
{
    SafeRelease(chequeredBackground);
}

bool ScreenControl::IsPointInside(const Vector2& /*point*/, bool/* expandWithFocus*/)
{
	//YZ:
	//input must be handled by screen
	return false;
}

void ScreenControl::SetScale(const Vector2& value)
{
    this->scale = value;
}

void ScreenControl::SetPos(const Vector2& value)
{
    this->pos = value;
}

void ScreenControl::Draw(const UIGeometricData &geometricData)
{
    // Draw "transparent" (cheqered) backgound under the control.
    chequeredBackground->Draw(geometricData);
}

void ScreenControl::DrawAfterChilds(const UIGeometricData & geometricData)
{
    // Draw the grid over the control.
    GridVisualizer::Instance()->DrawGridIfNeeded( GetRect(), RenderState::RENDERSTATE_2D_BLEND);

    static const Color RED_COLOR = Color(1.0f, 0.0f, 0.0f, 1.0f);
    static const Color GREY_COLOR = Color(0.55f, 0.55f, 0.55f, 1.0f);

    const HierarchyTreeController::SELECTEDCONTROLNODES &selectedNodes = HierarchyTreeController::Instance()->GetActiveControlNodes();
    HierarchyTreeController::SELECTEDCONTROLNODES::const_iterator iter = selectedNodes.begin();
    HierarchyTreeController::SELECTEDCONTROLNODES::const_iterator end = selectedNodes.end();

    for (; iter != end; ++iter)
    {
        HierarchyTreeControlNode * controlNode = (*iter);
        if (!controlNode)
            continue;

        UIControl * control = controlNode->GetUIObject();
        if (!control)
            continue;

        if( control->GetParent() )
        {
            UIGeometricData parentGd = control->GetParent()->GetGeometricData();
            parentGd.AddToGeometricData(geometricData);
            DrawSelectionFrame(parentGd, GREY_COLOR);
        }

        UIGeometricData controlGd = control->GetGeometricData();
        controlGd.AddToGeometricData(geometricData);
        DrawSelectionFrame(controlGd, RED_COLOR);

        if (!control->pivotPoint.IsZero())
        {
            UIGeometricData pivotGd;
            pivotGd.position = control->pivotPoint;
            pivotGd.AddToGeometricData(controlGd);
            DrawPivotPoint(pivotGd, RED_COLOR);
        }
    }
}

void ScreenControl::DrawSelectionFrame(const UIGeometricData &gd, const Color &color)
{
    Color oldColor = RenderManager::Instance()->GetColor();
    RenderManager::Instance()->SetColor(color);
    if( gd.angle != 0.0f )
    {
        Polygon2 poly;
        gd.GetPolygon( poly );

        RenderHelper::Instance()->DrawPolygon( poly, true, RenderState::RENDERSTATE_2D_BLEND );
    }
    else
    {
        RenderHelper::Instance()->DrawRect( gd.GetUnrotatedRect(), RenderState::RENDERSTATE_2D_BLEND );
    }
    RenderManager::Instance()->SetColor(oldColor);
}

void ScreenControl::DrawPivotPoint(const UIGeometricData &gd, const Color &color)
{
    Vector2 pivotPointCenter = gd.GetUnrotatedRect().GetPosition() - Vector2(0.5f, -0.5f);

    static const float32 PIVOT_POINT_MARK_RADIUS = 4.5f;
    static const float32 PIVOT_POINT_MARK_HALF_LINE_LENGTH = 10.0f;

    Color oldColor = RenderManager::Instance()->GetColor();
    RenderManager::Instance()->SetColor(color);

    RenderHelper::Instance()->DrawCircle(pivotPointCenter, PIVOT_POINT_MARK_RADIUS, RenderState::RENDERSTATE_2D_BLEND);

    // Draw the cross mark.
    Vector2 lineStartPoint = pivotPointCenter;
    Vector2 lineEndPoint = pivotPointCenter;
    lineStartPoint.y -= PIVOT_POINT_MARK_HALF_LINE_LENGTH;
    lineEndPoint.y += PIVOT_POINT_MARK_HALF_LINE_LENGTH;
    RenderHelper::Instance()->DrawLine(lineStartPoint, lineEndPoint, RenderState::RENDERSTATE_2D_BLEND);

    lineStartPoint = pivotPointCenter;
    lineEndPoint = pivotPointCenter;
    lineStartPoint.x -= PIVOT_POINT_MARK_HALF_LINE_LENGTH;
    lineEndPoint.x += PIVOT_POINT_MARK_HALF_LINE_LENGTH;
    RenderHelper::Instance()->DrawLine(lineStartPoint, lineEndPoint, RenderState::RENDERSTATE_2D_BLEND);

    RenderManager::Instance()->SetColor(oldColor);
}

void ScreenControl::SetScreenshotMode(bool value)
{
    screenShotMode = value;

}

YamlNode* ScreenControl::SaveToYamlNode( UIYamlLoader * loader )
{
    YamlNode * node = UIControl::SaveToYamlNode(loader);

    node->RemoveNodeFromMap("drawType");
    node->RemoveNodeFromMap("type");
    node->RemoveNodeFromMap("rect");
    node->RemoveNodeFromMap("position");
    node->RemoveNodeFromMap("size");
    return node;
}

