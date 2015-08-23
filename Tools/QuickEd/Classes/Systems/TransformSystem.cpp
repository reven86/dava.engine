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
                prevPos = currentInput->point;
            }
            return activeArea != NO_AREA;
        case UIEvent::PHASE_DRAG:
            return ProcessDrag(currentInput->point);
            
    }
    return false;
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
    Vector2 delta = pos - prevPos;
    bool retval = true;
    switch(activeArea)
    {
        case FRAME:
            AdjustProperty(activeControl, "Position", delta);
            break;
        case TOP_LEFT:
            AdjustProperty(activeControl, "Position", delta);
            AdjustProperty(activeControl, "Size", delta * -1);
            break;
        case TOP_CENTER:
            delta.x = 0;
            AdjustProperty(activeControl, "Position", delta);
            AdjustProperty(activeControl, "Size", delta * -1);
            break;
        case TOP_RIGHT:
        {
            Vector2 position = delta;
            position.x = 0;
            Vector2 size = delta;
            size.y = delta.y * -1;
            AdjustProperty(activeControl, "Position", position);
            AdjustProperty(activeControl, "Size", size);
        }
            break;
        case CENTER_LEFT:
            delta.y = 0;
            AdjustProperty(activeControl, "Position", delta);
            AdjustProperty(activeControl, "Size", delta * -1);
            break;
        case CENTER_RIGHT:
            delta.y = 0;
            AdjustProperty(activeControl, "Size", delta);
            break;
        case BOTTOM_LEFT:
        {
            Vector2 position = delta;
            position.y = 0;
            Vector2 size = delta;
            size.x = size.x * -1;
            AdjustProperty(activeControl, "Position", position);
            AdjustProperty(activeControl, "Size", size);
        }
            break;
        case BOTTOM_CENTER:
            delta.x = 0;
            AdjustProperty(activeControl, "Size", delta);
            break;
        case BOTTOM_RIGHT:
            AdjustProperty(activeControl, "Size", delta);
            break;
        case PIVOT_POINT:
        {
            auto control = activeControl->GetControl();
            Vector2 pivotPoint = control->GetPivotPoint();
            control->SetPivotPoint(pivotPoint - delta);
        }
            break;
        case ROTATE:
        {
            auto control = activeControl->GetControl();
            Vector2 pivotPoint = control->GetGeometricData().GetUnrotatedRect().GetPosition() + control->GetPivotPoint();
            
            Vector2 rotatePoint = pivotPoint;
            Vector2 l1(prevPos.x - rotatePoint.x, prevPos.y - rotatePoint.y);
            Vector2 l2(pos.x - rotatePoint.x, pos.y - rotatePoint.y);
            float dot = l1.x * l2.x + l1.y * l2.y;
            float cross = l1.x * l2.y - l1.y * l2.x;
            float angleRad = atan2(cross, dot);
            angleRad = atan2(l2.y, l2.x) - atan2(l1.y, l1.x);
            float angle = angleRad * 180.0f / PI;
            angle = static_cast<int>(Round(angle));
            AdjustProperty(activeControl, "Angle", angle);
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

template <typename T>
void TransformSystem::AdjustProperty(ControlNode *node, const DAVA::String &propertyName, const T &delta)
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


