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

#include "UIRenderSystem.h"
#include "Debug/Stats.h"
#include "Render/OcclusionQuery.h"
#include "Render/RenderHelper.h"
#include "Render/2D/Systems/RenderSystem2D.h"
#include "UI/UIControl.h"
#include "UI/UIControlSystem.h"
#include "UI/Components/UIRenderComponent.h"

namespace DAVA
{

UIRenderSystem::UIRenderSystem()
{
}

UIRenderSystem::~UIRenderSystem()
{
}

void UIRenderSystem::Draw()
{
    UIControl * screen = UIControlSystem::Instance()->GetScreen();
    UIControl * popup = UIControlSystem::Instance()->GetPopupContainer();

    TIME_PROFILE("UIRenderSystem::Process()");

    FrameOcclusionQueryManager::Instance()->BeginQuery(FRAME_QUERY_UI_DRAW);

    UIControlSystem::Instance()->drawCounter = 0;
    if (!UIControlSystem::Instance()->ui3DViewCount)
    {
        UniqueHandle prevState = RenderManager::Instance()->currentState.stateHandle;
        RenderManager::Instance()->SetRenderState(RenderState::RENDERSTATE_3D_BLEND);
        RenderManager::Instance()->FlushState();
        RenderManager::Instance()->Clear(Color(0, 0, 0, 0), 1.0f, 0);
        RenderManager::Instance()->SetRenderState(prevState);
    }

    if (screen)
    {
        SystemDraw(screen, UIControlSystem::Instance()->GetBaseGeometricData());
    }

    if (popup)
    {
        SystemDraw(popup, UIControlSystem::Instance()->GetBaseGeometricData());
    }

    if (UIControlSystem::Instance()->frameSkip > 0)
    {
        UIControlSystem::Instance()->frameSkip--;
    }
    
    FrameOcclusionQueryManager::Instance()->EndQuery(FRAME_QUERY_UI_DRAW);
}

void UIRenderSystem::SystemDraw(UIControl* control, const UIGeometricData& geometricData)
{
    if (control == NULL || !control->GetSystemVisible())
        return;

    UIRenderComponent* component = NULL;
    if ((control->GetAvailableComponentFlags() & GetRequiredComponents()) == GetRequiredComponents())
    {
        component = control->GetComponent<UIRenderComponent>();
        DVASSERT(component);
    }

    if (component && component->customBeforeSystemDraw != (int)NULL)
    {
        component->customBeforeSystemDraw(geometricData);
    }

    UIControlSystem::Instance()->drawCounter++;
    UIGeometricData drawData = control->GetLocalGeometricData();
    drawData.AddGeometricData(geometricData);

    const Color &parentColor = control->GetParent() ? control->GetParent()->GetBackground()->GetDrawColor() : Color::White;

    control->SetParentColor(parentColor);

    const Rect& unrotatedRect = drawData.GetUnrotatedRect();

    if (control->GetClipContents())
    {//WARNING: for now clip contents don't work for rotating controls if you have any ideas you are welcome
        RenderSystem2D::Instance()->PushClip();
        RenderSystem2D::Instance()->ClipRect(drawData.GetAABBox());
    }

    if (component)
    {
        if (component->customDraw != (int)NULL)
        {
            component->customDraw(drawData);
        }
        else
        {
            Draw(control, drawData);
        }
    }

    control->isIteratorCorrupted = false;
    List<UIControl*>::iterator it = control->childs.begin();
    List<UIControl*>::iterator itEnd = control->childs.end();
    for (; it != itEnd; ++it)
    {
        if (((*it)->GetAvailableComponentFlags() & GetRequiredComponents()) == GetRequiredComponents())
        {
            UIRenderComponent* itComponent = (*it)->GetComponent<UIRenderComponent>();
            DVASSERT(itComponent);
            if (itComponent->customSystemDraw != (int)NULL)
            {
                itComponent->customSystemDraw(drawData);
            }
            else
            {
                SystemDraw(*it, drawData);
            }
        }
        else
        {
            SystemDraw(*it, drawData);
        }
        DVASSERT(!control->isIteratorCorrupted);
    }

    if (component && component->customDrawAfterChilds != (int)NULL)
    {
        component->customDrawAfterChilds(drawData);
    }

    if (control->GetClipContents())
    {
        RenderSystem2D::Instance()->PopClip();
    }

    if (control->GetDebugDraw())
    {
        RenderSystem2D::Instance()->PushClip();
        RenderSystem2D::Instance()->RemoveClip();
        DrawDebugRect(control, drawData, false);
        DrawPivotPoint(control, unrotatedRect);
        RenderSystem2D::Instance()->PopClip();
    }

    if (component && component->customAfterSystemDraw != (int)NULL)
    {
        component->customAfterSystemDraw(geometricData);
    }

}

void UIRenderSystem::DrawDebugRect(UIControl* control, const UIGeometricData& geometricData, bool useAlpha)
{
    Color oldColor = RenderManager::Instance()->GetColor();
    RenderSystem2D::Instance()->PushClip();

    if (useAlpha)
    {
        Color drawColor = control->GetDebugDrawColor();
        drawColor.a = 0.4f;
        RenderManager::Instance()->SetColor(drawColor);
    }
    else
    {
        RenderManager::Instance()->SetColor(control->GetDebugDrawColor());
    }

    if (geometricData.angle != 0.0f)
    {
        Polygon2 poly;
        geometricData.GetPolygon(poly);

        RenderHelper::Instance()->DrawPolygon(poly, true, RenderState::RENDERSTATE_2D_BLEND);
    }
    else
    {
        RenderHelper::Instance()->DrawRect(geometricData.GetUnrotatedRect(), RenderState::RENDERSTATE_2D_BLEND);
    }

    RenderSystem2D::Instance()->PopClip();
    RenderManager::Instance()->SetColor(oldColor);
}

void UIRenderSystem::DrawPivotPoint(UIControl* control, const Rect& drawRect)
{
    if (control->GetDrawPivotPointMode() == UIControl::DRAW_NEVER)
    {
        return;
    }

    if (control->GetDrawPivotPointMode() == UIControl::DRAW_ONLY_IF_NONZERO && control->GetPivotPoint().IsZero())
    {
        return;
    }

    static const float32 PIVOT_POINT_MARK_RADIUS = 10.0f;
    static const float32 PIVOT_POINT_MARK_HALF_LINE_LENGTH = 13.0f;

    Color oldColor = RenderManager::Instance()->GetColor();
    RenderSystem2D::Instance()->PushClip();
    RenderManager::Instance()->SetColor(Color(1.0f, 0.0f, 0.0f, 1.0f));

    Vector2 pivotPointCenter = drawRect.GetPosition() + control->GetPivotPoint();
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

    RenderSystem2D::Instance()->PopClip();
    RenderManager::Instance()->SetColor(oldColor);
}

void UIRenderSystem::Draw(UIControl* control, const UIGeometricData& geometricData)
{
    if ((control->GetAvailableComponentFlags() & GetRequiredComponents()) != GetRequiredComponents())
    {
        return;
    }
    control->GetBackground()->Draw(geometricData);
}

}