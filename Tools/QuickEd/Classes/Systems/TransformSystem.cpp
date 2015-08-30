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

#include "Input/InputSystem.h"
#include "Input/KeyboardDevice.h"

#include "Systems/TransformSystem.h"
#include "Document.h"
#include "Defines.h"
#include "UI/UIEvent.h"
#include "Input/KeyboardDevice.h"
#include "Model/PackageHierarchy/ControlNode.h"
#include "Model/ControlProperties/RootProperty.h"
#include "Model/ControlProperties/ValueProperty.h"
#include "UI/QtModelPackageCommandExecutor.h"

using namespace DAVA;

TransformSystem::TransformSystem(Document *parent)
    : document(parent)
{
    InitCornersDirection();
}

void TransformSystem::MouseEnterArea(ControlNode *targetNode, const eArea area)
{
    activeControl = targetNode;
    activeArea = area;
}

void TransformSystem::MouseLeaveArea()
{
    activeControl = nullptr;
    activeArea = NO_AREA;
}

bool TransformSystem::OnInput(UIEvent* currentInput)
{
    switch(currentInput->phase)
    {
        case UIEvent::PHASE_KEYCHAR:
            return ProcessKey(currentInput->tid);
        case UIEvent::PHASE_BEGAN:
            if(activeArea != NO_AREA)
            {
                beginPos = currentInput->point;
                prevPos = currentInput->point;
            }
            return activeArea != NO_AREA;
        case UIEvent::PHASE_DRAG:
            return ProcessDrag(currentInput->point);
        case UIEvent::PHASE_ENDED:
        {
            bool retVal = activeArea == FRAME && beginPos == currentInput->point;
            beginPos = Vector2(-1, -1);
            return !retVal;
        }
        default:
            return false;
            
    }
}

void TransformSystem::SelectionWasChanged(const SelectedControls &selected, const SelectedControls &deselected)
{
    UniteNodes(selected, selectedControls);
    SubstractNodes(deselected, selectedControls);
}

bool TransformSystem::ProcessKey(const int32 key)
{
    if(!selectedControls.empty())
    {
        switch(key)
        {
            case DVKEY_LEFT:
                MoveAllSelectedControls(Vector2(-1, 0));
                return true;
            case DVKEY_UP:
                MoveAllSelectedControls(Vector2(0, -1));
                return true;
            case DVKEY_RIGHT:
                MoveAllSelectedControls(Vector2(1, 0));
                return true;
            case DVKEY_DOWN:
                MoveAllSelectedControls(Vector2(0, 1));
                return true;
        }
    }
    return false;
}

bool TransformSystem::ProcessDrag(const Vector2 &pos)
{
    if(activeArea == NO_AREA)
    {
        return false;
    }
    const auto &keyBoard = InputSystem::Instance()->GetKeyboard();
    bool retval = true;
    auto gd =  activeControl->GetControl()->GetGeometricData();

    switch(activeArea)
    {
        case FRAME:
            MoveConrol(pos);
            break;
        case TOP_LEFT:
        case TOP_CENTER:
        case TOP_RIGHT:
        case CENTER_LEFT:
        case CENTER_RIGHT:
        case BOTTOM_LEFT:
        case BOTTOM_CENTER:
        case BOTTOM_RIGHT:
        {
            bool withPivot = keyBoard.IsKeyPressed(DVKEY_ALT);
            bool rateably = keyBoard.IsKeyPressed(DVKEY_SHIFT);
            ResizeControl(pos, withPivot, rateably);
        }
        break;
        case PIVOT_POINT:
        {
            auto control = activeControl->GetControl();
            Vector2 delta = pos - prevPos;
            AdjustProperty(activeControl, "Position", delta / gd.scale);
            Vector2 angeledDelta(delta.x * gd.cosA + delta.y * gd.sinA,
                        delta.x * -gd.sinA + delta.y * gd.cosA);
            Vector2 pivot((angeledDelta / gd.scale) / control->GetSize()); //pivot is half of pos / size
            AdjustProperty(activeControl, "Pivot", pivot);
        }
            break;
        case ROTATE:
        {
            auto control = activeControl->GetControl();
            const Rect &ur = gd.GetUnrotatedRect();
            Vector2 pivotPoint = ur.GetPosition() + ur.GetSize() * control->GetPivot();
            Vector2 rotatePoint = pivotPoint;
            Vector2 l1(prevPos.x - rotatePoint.x, prevPos.y - rotatePoint.y);
            Vector2 l2(pos.x - rotatePoint.x, pos.y - rotatePoint.y);
            float angleRad = atan2(l2.y, l2.x) - atan2(l1.y, l1.x);
            float angle = angleRad * 180.0f / PI;
            AdjustProperty(activeControl, "Angle", static_cast<float32>(round(angle)));
        }
            break;
        default:
            retval = false;
    }
    prevPos = pos;
    return retval;
}

void TransformSystem::MoveAllSelectedControls(const Vector2 &delta)
{
    for( auto &controlNode : selectedControls)
    {
        if(controlNode->IsEditingSupported())
        {
            AdjustProperty(controlNode, "Position", delta);
        }
    }
}

void TransformSystem::MoveConrol(const Vector2& pos)
{
    Vector2 delta = pos - prevPos;
    auto gd =  activeControl->GetControl()->GetGeometricData();
    Vector2 realDelta = delta / gd.scale;
    if (InputSystem::Instance()->GetKeyboard().IsKeyPressed(DVKEY_SHIFT))
    {
        const int step = 20;
        static int x, y;
        if (abs(x) < step)
        {
            x += realDelta.x;
        }
        if (abs(y) < step)
        {
            y += realDelta.y;
        }
        realDelta = Vector2(x - x % step, y - y % step);
        x -= realDelta.x;
        y -= realDelta.y;
    }
    AdjustProperty(activeControl, "Position", realDelta);
}

void TransformSystem::ResizeWithPivot(const Vector2 &pos, bool rateably)
{
    Vector2 delta = pos - prevPos;
    if(delta.x == 0.0f && delta.y == 0.0f)
    {
        return;
    }
    auto gd = activeControl->GetControl()->GetGeometricData();
    DVASSERT(gd.scale.x != 0 && gd.scale.y != 0);
    Vector2 angeledDelta(delta.x * cosf(gd.angle) + delta.y * sinf(gd.angle),
                         delta.x * -sinf(gd.angle) + delta.y * cosf(gd.angle));
    Vector2 sizeDelta = angeledDelta / gd.scale;
    const auto invertX = cornersDirection[activeArea][X_AXIS];
    const auto invertY = cornersDirection[activeArea][Y_AXIS];
    Vector2 deltaSize(sizeDelta.x * invertX, sizeDelta.y * invertY);
    
    auto pivotProp = document->GetPropertyByName(activeControl, "Pivot");
    DVASSERT(nullptr != pivotProp);
    Vector2 pivot = pivotProp->GetValue().AsVector2();
    
    auto pivotDeltaX = invertX == -1 ? pivot.x : 1 - pivot.x;
    if (pivotDeltaX != 0)
    {
        deltaSize.x /= pivotDeltaX;
    }
    auto pivotDeltaY = invertY == -1 ? pivot.y : 1 - pivot.y;
    if (pivotDeltaY != 0)
    {
        deltaSize.y /= pivotDeltaY;
    }
    if (rateably)
    {
        if(invertX != 0 && invertY != 0)
        {
            if(fabs(angeledDelta.x) > 0 && fabs(angeledDelta.y) > 0)
            {
                bool canNotResize = (angeledDelta.x * invertX > 0) ^ (angeledDelta.y * invertY > 0);
                if(canNotResize)
                {
                    return;
                }
            }
        }
        const Vector2 &size = activeControl->GetControl()->GetSize();
        float proportion = size.y != 0  ? size.x / size.y : 0;
        if (proportion != 0)
        {
            if (fabs(angeledDelta.y) > fabs(angeledDelta.x))
            {
                deltaSize.x = deltaSize.y * proportion;
            }
            else
            {
                deltaSize.y = deltaSize.x / proportion;
            }
        }
    }
    AdjustProperty(activeControl, "Size", deltaSize);
}

void TransformSystem::ResizeRateably(const DAVA::Vector2 &pos)
{
    DVASSERT(activeArea != NO_AREA);
    Vector2 delta = pos - prevPos;
    if(delta.x == 0.0f && delta.y == 0.0f)
    {
        return;
    }
    const auto invertX = cornersDirection[activeArea][X_AXIS];
    const auto invertY = cornersDirection[activeArea][Y_AXIS];
    
    auto gd = activeControl->GetControl()->GetGeometricData();
    Vector2 angeledDelta(delta.x * cosf(gd.angle) + delta.y * sinf(gd.angle),
                         delta.x * -sinf(gd.angle) + delta.y * cosf(gd.angle));
    DVASSERT(gd.scale.x != 0 && gd.scale.y != 0);
    //scale rotated delta
    DVASSERT(gd.scale.x != 0 && gd.scale.y != 0);
    Vector2 realDelta(angeledDelta / gd.scale);
    Vector2 deltaPosition(realDelta);
    Vector2 deltaSize(realDelta);
    //make resize absolutely
    deltaSize.x *= invertX;
    deltaSize.y *= invertY;
    //disable move if not accepted
    if(invertX == 0)
    {
        deltaPosition.x = 0;
    }
    if(invertY == 0)
    {
        deltaPosition.y = 0;
    }

    auto pivotProp = document->GetPropertyByName(activeControl, "Pivot");
    DVASSERT(nullptr != pivotProp);
    Vector2 pivot = pivotProp->GetValue().AsVector2();
    
    deltaPosition.x *= invertX == -1 ? 1 - pivot.x : pivot.x;
    deltaPosition.y *= invertY == -1 ? 1 - pivot.y : pivot.y;
    DVASSERT(gd.scale.x != 0.0f && gd.scale.y != 0.0f);
    
    //check situation when we try to resize up and down simultaneously
    if(invertX != 0 && invertY != 0) //actual only for corners
    {
        if(fabs(angeledDelta.x) > 0 && fabs(angeledDelta.y) > 0) //only if up and down requested
        {
            bool canNotResize = (angeledDelta.x * invertX > 0) ^ (angeledDelta.y * invertY > 0);
            if(canNotResize) // and they have different sign for corner
            {
                float prop = fabs(angeledDelta.x) / fabs(angeledDelta.y);
                if(prop > 0.48 && prop < 0.52) // like "resize 10 to up and 10 to down rateably"
                {
                    return;
                }
            }
        }
    }
    
    const Vector2 &size = activeControl->GetControl()->GetSize();
    float proportion = size.y != 0  ? size.x / size.y : 0;
    if (proportion != 0)
    {
        if (fabs(angeledDelta.y) > fabs(angeledDelta.x))
        {
            deltaSize.x = deltaSize.y * proportion;
            deltaPosition.x = deltaSize.y * proportion;
            if(invertX == 0)
            {
                deltaPosition.x *= (0.5 - pivot.x) * -1; //rainbow unicorn was here and add -1 to the right.
            }
            else
            {
                deltaPosition.x *= (invertX == -1 ? 1 - pivot.x : pivot.x) * invertX;
            }
        }
        else
        {
            deltaSize.y = deltaSize.x / proportion;
            deltaPosition.y =  deltaSize.x / proportion;
            if(invertY == 0)
            {
                deltaPosition.y *= (0.5 - pivot.y) * -1; // another rainbow unicorn adds -1 here.
            }
            else
            {
                deltaPosition.y *= (invertY == -1 ? 1 - pivot.y : pivot.y) * invertY;
            }
        }
    }
    
    Vector2 rotatedPosition;
    rotatedPosition.x = deltaPosition.x * cosf(-gd.angle) + deltaPosition.y * sinf(-gd.angle);
    rotatedPosition.y = deltaPosition.x * -sinf(-gd.angle) + deltaPosition.y * cosf(-gd.angle);
    AdjustProperty(activeControl, "Position", rotatedPosition);
    
    AdjustProperty(activeControl, "Size", deltaSize);
}

void TransformSystem::ResizeControl(const Vector2& pos, bool withPivot, bool rateably)
{
    DVASSERT(activeArea != NO_AREA);
    if(withPivot)
    {
        return ResizeWithPivot(pos, rateably); //TODO: refactor it
    }
    else if(rateably)
    {
        return ResizeRateably(pos); //TODO:refactor it
    }
    Vector2 delta = pos - prevPos;
    if(delta.x == 0.0f && delta.y == 0.0f)
    {
        return;
    }
    const auto invertX = cornersDirection[activeArea][X_AXIS];
    const auto invertY = cornersDirection[activeArea][Y_AXIS];

    auto gd = activeControl->GetControl()->GetGeometricData();
    Vector2 angeledDelta(delta.x * cosf(gd.angle) + delta.y * sinf(gd.angle),
                         delta.x * -sinf(gd.angle) + delta.y * cosf(gd.angle)); //rotate delta
     //scale rotated delta
    DVASSERT(gd.scale.x != 0 && gd.scale.y != 0);
    Vector2 realDelta(angeledDelta / gd.scale);
    Vector2 deltaPosition(realDelta);
    Vector2 deltaSize(realDelta);
    //make resize absolutely
    deltaSize.x *= invertX;
    deltaSize.y *= invertY;
    //disable move if not accepted
    if(invertX == 0)
    {
        deltaPosition.x = 0;
    }
    if(invertY == 0)
    {
        deltaPosition.y = 0;
    }

    auto pivotProp = document->GetPropertyByName(activeControl, "Pivot");
    DVASSERT(nullptr != pivotProp);
    Vector2 pivot = pivotProp->GetValue().AsVector2();

    //calculate new positionp
    deltaPosition.x *= invertX == -1 ? 1 - pivot.x : pivot.x;
    deltaPosition.y *= invertY == -1 ? 1 - pivot.y : pivot.y;
    DVASSERT(gd.scale.x != 0.0f && gd.scale.y != 0.0f);
    //rotate delta position backwards, because SetPosition require absolute coordinates
    Vector2 rotatedPosition;
    rotatedPosition.x = deltaPosition.x * cosf(-gd.angle) + deltaPosition.y * sinf(-gd.angle);
    rotatedPosition.y = deltaPosition.x * -sinf(-gd.angle) + deltaPosition.y * cosf(-gd.angle);
    AdjustProperty(activeControl, "Position", rotatedPosition);

    AdjustProperty(activeControl, "Size", deltaSize);
}

template <typename T>
void TransformSystem::AdjustProperty(ControlNode *node, const String &propertyName, const T &delta)
{

    AbstractProperty *property = document->GetPropertyByName(node, propertyName);
    DVASSERT(nullptr != property);
    VariantType var(delta);
    
    switch(var.GetType())
    {
        case VariantType::TYPE_VECTOR2:
            var = VariantType(property->GetValue().AsVector2() + delta);
            break;
        case VariantType::TYPE_FLOAT:
            var = VariantType(property->GetValue().AsFloat() + delta);
            break;
        default:
            DVASSERT_MSG(false, "unexpected type");
            break;
    }
    
    document->GetCommandExecutor()->ChangeProperty(node, property, var);
}

void TransformSystem::InitCornersDirection()
{
    cornersDirection[TOP_LEFT] = {{ -1, -1 }};
    cornersDirection[TOP_CENTER] = {{ 0, -1 }};
    cornersDirection[TOP_RIGHT] = {{ 1, -1 }};
    cornersDirection[CENTER_LEFT] = {{ -1, 0 }};
    cornersDirection[CENTER_RIGHT] = {{ 1, 0 }};
    cornersDirection[BOTTOM_LEFT] = {{ -1, 1 }};
    cornersDirection[BOTTOM_CENTER] = {{ 0, 1 }};
    cornersDirection[BOTTOM_RIGHT] = {{ 1, 1 }};
}
