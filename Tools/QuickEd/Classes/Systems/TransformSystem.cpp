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

bool TransformSystem::ProcessKey(const DAVA::int32 key)
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

bool TransformSystem::ProcessDrag(const DAVA::Vector2 &pos)
{
    if(activeArea == NO_AREA)
    {
        return false;
    }
    const auto &keyBoard = InputSystem::Instance()->GetKeyboard();
    Vector2 delta = pos - prevPos;
    bool retval = true;
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
            auto gd =  activeControl->GetControl()->GetGeometricData();
            Vector2 realDelta = delta / gd.scale;
            Vector2 pivot(realDelta / control->GetSize()); //pivot is half of pos / size
            AdjustProperty(activeControl, "Pivot", pivot);
            AdjustProperty(activeControl, "Position", realDelta);
        }
            break;
        case ROTATE:
        {
            auto control = activeControl->GetControl();
            Vector2 pivotPoint = control->GetGeometricData().GetUnrotatedRect().GetPosition() + control->GetPivotPoint();
            
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

void TransformSystem::MoveAllSelectedControls(const DAVA::Vector2 &delta)
{
    for( auto &controlNode : selectedControls)
    {
        if(controlNode->IsEditingSupported())
        {
            AdjustProperty(controlNode, "Position", delta);
        }
    }
}

void TransformSystem::MoveConrol(const DAVA::Vector2& pos)
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

void TransformSystem::ResizeControl(const DAVA::Vector2& pos, bool withPivot, bool rateably)
{
    DVASSERT(activeArea != NO_AREA);
    Vector2 delta = pos - prevPos;
    auto gd = activeControl->GetControl()->GetGeometricData();
    Vector2 realDelta = delta / gd.scale;
    if (rateably)
    {
        const Vector2 &size = activeControl->GetControl()->GetSize();
        float perc = size.x / size.y;
        if (fabs(realDelta.y) > fabs(realDelta.x))
            realDelta.x = realDelta.y * perc;
        else
            realDelta.y = realDelta.x / perc;
    }
    Vector2 deltaPosition = realDelta;
    Vector2 deltaSize = realDelta;
    auto pivotProp = document->GetPropertyByName(activeControl, "Pivot");
    DVASSERT(nullptr != pivotProp);
    Vector2 pivot = pivotProp->GetValue().AsVector2();

    switch (activeArea)
    {
    default:
        DVASSERT_MSG(false, "wrong parameter passed to function");
    case TOP_LEFT:
            if (withPivot)
            {
                deltaSize /= pivot;
                deltaPosition.SetZero();
            }
            else
            {
                deltaPosition *= Vector2(1, 1) - pivot;
            }
        AdjustProperty(activeControl, "Position", deltaPosition);
        AdjustProperty(activeControl, "Size", deltaSize * -1);
        break;
    case TOP_CENTER:
        if (withPivot)
        {
            deltaSize /= pivot;
            deltaPosition.SetZero();
        }
        else
        {
            deltaPosition *= Vector2(1, 1) - pivot;
        }
        if(!rateably)
        {
            deltaPosition.x = 0;
            deltaSize.x = 0;
        }
        AdjustProperty(activeControl, "Position", deltaPosition);
        AdjustProperty(activeControl, "Size", deltaSize * -1);
        break;
    case TOP_RIGHT:
            if (withPivot)
            {
                deltaSize /= pivot;
                deltaPosition.SetZero();
            }
            else
            {
                deltaPosition *= Vector2(0, 1) - pivot;
                deltaPosition.x *= -1;
            }
        deltaSize.y = deltaSize.y * -1;
        AdjustProperty(activeControl, "Position", deltaPosition);
        AdjustProperty(activeControl, "Size", deltaSize);
        break;
    case CENTER_LEFT:
        if (withPivot)
        {
            deltaSize /= pivot;
            deltaPosition.SetZero();
        }
        else
        {
            deltaPosition *= Vector2(1, 1) - pivot;
        }
        if(!rateably)
        {
            deltaPosition.y = 0;
            deltaSize.y = 0;
        }
        AdjustProperty(activeControl, "Position", deltaPosition);
        AdjustProperty(activeControl, "Size", deltaSize * -1);
        break;
    case CENTER_RIGHT:
        deltaSize.y = 0;
        AdjustProperty(activeControl, "Size", deltaSize);
        break;
    case BOTTOM_LEFT:
        deltaPosition.y = 0;
        deltaSize.x *= -1;
        AdjustProperty(activeControl, "Position", deltaPosition);
        AdjustProperty(activeControl, "Size", deltaSize);
        break;
    case BOTTOM_CENTER:
        deltaSize.x = 0;
        AdjustProperty(activeControl, "Size", deltaSize);
        break;
    case BOTTOM_RIGHT:
        AdjustProperty(activeControl, "Size", deltaSize);
        break;
    }
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
