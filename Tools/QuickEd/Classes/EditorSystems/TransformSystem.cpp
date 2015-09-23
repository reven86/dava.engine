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

#include "EditorSystems/TransformSystem.h"
#include "EditorSystems/EditorSystemsManager.h"
#include "UI/UIEvent.h"
#include "UI/UIControl.h"
#include "Input/KeyboardDevice.h"
#include "Model/PackageHierarchy/ControlNode.h"
#include "Model/ControlProperties/RootProperty.h"
#include "Model/ControlProperties/ValueProperty.h"
#include "UI/QtModelPackageCommandExecutor.h"
#include <chrono>

using namespace DAVA;
using namespace std::chrono;

namespace
{
const Array<Array<int, Vector2::AXIS_COUNT>, HUDAreaInfo::CORNERS_COUNT> cornersDirection =
{{
{{-1, -1}}, // TOP_LEFT_AREA
{{0, -1}}, // TOP_CENTER_AREA
{{1, -1}}, //TOP_RIGHT_AREA
{{-1, 0}}, //CENTER_LEFT_AREA
{{1, 0}}, //CENTER_RIGHT_AREA
{{-1, 1}}, //BOTTOM_LEFT_AREA
{{0, 1}}, //BOTTOM_CENTER_AREA
{{1, 1}} //BOTTOM_RIGHT_AREA
}};
}

TransformSystem::TransformSystem(EditorSystemsManager* parent)
    : BaseEditorSystem(parent)
    , steps({{15, 10, 10}}) //10 grad for rotate and 20 pix for move/resize
{
    systemManager->ActiveAreaChanged.Connect(this, &TransformSystem::OnActiveAreaChanged);
    systemManager->SelectionChanged.Connect(this, &TransformSystem::OnSelectionChanged);
}

void TransformSystem::OnActiveAreaChanged(const HUDAreaInfo& areaInfo)
{
    activeArea = areaInfo.area;
    activeControlNode = areaInfo.owner;
}

bool TransformSystem::OnInput(UIEvent* currentInput)
{
    switch (currentInput->phase)
    {
    case UIEvent::PHASE_KEYCHAR:
        return ProcessKey(currentInput->tid);

    case UIEvent::PHASE_BEGAN:
    {
        prevPos = currentInput->point;
        microseconds us = duration_cast<microseconds>(system_clock::now().time_since_epoch());
        currentHash = static_cast<size_t>(us.count());
        return activeArea != HUDAreaInfo::NO_AREA;
    }
    case UIEvent::PHASE_DRAG:
    {
        ProcessDrag(currentInput->point);
        prevPos = currentInput->point;
        return false;
    }
    case UIEvent::PHASE_ENDED:
        accumulates.fill({{0, 0}});
        extraDelta.SetZero();
        return false;
    default:
        return false;
    }
}

void TransformSystem::OnSelectionChanged(const SelectedNodes& selected, const SelectedNodes& deselected)
{
    SelectionContainer::MergeSelectionAndContainer(selected, deselected, selectedControlNodes);
    nodesToMove.resize(selectedControlNodes.size());
    std::copy(selectedControlNodes.begin(), selectedControlNodes.end(), nodesToMove.begin());
    auto iter = nodesToMove.begin();
    while (iter != nodesToMove.end())
    {
        bool isChild = false;
        auto iter2 = nodesToMove.begin();
        while (iter2 != nodesToMove.end() && !isChild)
        {
            PackageBaseNode* node1 = *iter;
            PackageBaseNode* node2 = *iter2;
            if (iter != iter2)
            {
                while (nullptr != node1->GetParent() && nullptr != node1->GetControl() && !isChild)
                {
                    if (node1 == node2)
                    {
                        isChild = true;
                    }
                    node1 = node1->GetParent();
                }
            }
            ++iter2;
        }
        if (isChild)
        {
            nodesToMove.erase(iter++);
        }
        else
        {
            ++iter;
        }
    }
}

bool TransformSystem::ProcessKey(const int32 key)
{
    if (!selectedControlNodes.empty())
    {
        float step = 1.0f;
        const KeyboardDevice& keyboard = InputSystem::Instance()->GetKeyboard();
        if (keyboard.IsKeyPressed(DVKEY_SHIFT))
            step = 10.0f;
        Vector2 deltaPos;
        switch (key)
        {
        case DVKEY_LEFT:
            deltaPos.dx -= step;
            break;
        case DVKEY_UP:
            deltaPos.dy -= step;
            break;
        case DVKEY_RIGHT:
            deltaPos.dx += step;
            break;
        case DVKEY_DOWN:
            deltaPos.dy += step;
            break;
        default:
            break;
        }
        if (!deltaPos.IsZero())
        {
            MoveAllSelectedControls(deltaPos);
            return true;
        }
    }
    return false;
}

bool TransformSystem::ProcessDrag(const Vector2& pos)
{
    if (activeArea == HUDAreaInfo::NO_AREA)
    {
        return false;
    }

    switch (activeArea)
    {
    case HUDAreaInfo::FRAME_AREA:
        MoveControl(pos);
        return true;
    case HUDAreaInfo::TOP_LEFT_AREA:
    case HUDAreaInfo::TOP_CENTER_AREA:
    case HUDAreaInfo::TOP_RIGHT_AREA:
    case HUDAreaInfo::CENTER_LEFT_AREA:
    case HUDAreaInfo::CENTER_RIGHT_AREA:
    case HUDAreaInfo::BOTTOM_LEFT_AREA:
    case HUDAreaInfo::BOTTOM_CENTER_AREA:
    case HUDAreaInfo::BOTTOM_RIGHT_AREA:
    {
        const auto& keyBoard = InputSystem::Instance()->GetKeyboard();
        bool withPivot = keyBoard.IsKeyPressed(DVKEY_ALT);
        bool rateably = keyBoard.IsKeyPressed(DVKEY_SHIFT);
        ResizeControl(pos, withPivot, rateably);
        return true;
    }
    case HUDAreaInfo::PIVOT_POINT_AREA:
    {
        MovePivot(pos);
        return true;
    }
    case HUDAreaInfo::ROTATE_AREA:
    {
        Rotate(pos);
        return true;
    }
    default:
        return false;
    }
}

void TransformSystem::MoveAllSelectedControls(const Vector2& delta)
{
    for (auto& controlNode : nodesToMove)
    {
        if (controlNode->IsEditingSupported())
        {
            AdjustProperty(controlNode, "Position", VariantType(delta));
        }
    }
}

void TransformSystem::MoveControl(const Vector2& pos)
{
    Vector2 delta = pos - prevPos;
    UIControl* control = activeControlNode->GetControl();
    auto gd = control->GetGeometricData();
    if (gd.scale.x != 0.0f && gd.scale.y != 0.0f)
    {
        Vector2 realDelta = delta / gd.scale;
        realDelta *= control->GetScale();
        if (InputSystem::Instance()->GetKeyboard().IsKeyPressed(DVKEY_SHIFT))
        {
            AbstractProperty* property = activeControlNode->GetRootProperty()->FindPropertyByName("Position");
            Vector2 position = property->GetValue().AsVector2();
            Vector2 newPosition(position + realDelta);
            AccumulateOperation(MOVE_OPERATION, newPosition);
            realDelta = newPosition - position;
        }
        MoveAllSelectedControls(realDelta);
    }
}

void TransformSystem::ResizeControl(const Vector2& pos, bool withPivot, bool rateably)
{
    DVASSERT(activeArea != HUDAreaInfo::NO_AREA);
    Vector2 delta = pos - prevPos;
    if (delta.x == 0.0f && delta.y == 0.0f)
    {
        return;
    }
    const auto invertX = cornersDirection.at(activeArea - HUDAreaInfo::TOP_LEFT_AREA)[Vector2::AXIS_X];
    const auto invertY = cornersDirection.at(activeArea - HUDAreaInfo::TOP_LEFT_AREA)[Vector2::AXIS_Y];

    UIControl* control = activeControlNode->GetControl();
    auto gd = control->GetGeometricData();
    if (gd.scale.x == 0.0f || gd.scale.y == 0.0f)
    {
        return;
    }
    Vector2 angeledDelta(delta.x * gd.cosA + delta.y * gd.sinA,
                         delta.x * -gd.sinA + delta.y * gd.cosA); //rotate delta
    //scale rotated delta
    Vector2 realDelta(angeledDelta / gd.scale);
    Vector2 deltaPosition(realDelta);
    Vector2 deltaSize(realDelta);
    //make resize absolutely
    deltaSize.x *= invertX;
    deltaSize.y *= invertY;
    //disable move if not accepted
    if (invertX == 0)
    {
        deltaPosition.x = 0.0f;
    }
    if (invertY == 0)
    {
        deltaPosition.y = 0.0f;
    }

    auto pivotProp = activeControlNode->GetRootProperty()->FindPropertyByName("Pivot");
    DVASSERT(nullptr != pivotProp);
    Vector2 pivot = pivotProp->GetValue().AsVector2();

    //calculate new positionp
    deltaPosition.x *= invertX == -1 ? 1 - pivot.x : pivot.x;
    deltaPosition.y *= invertY == -1 ? 1 - pivot.y : pivot.y;

    //modify if pivot modificator selected
    if (withPivot)
    {
        deltaPosition.SetZero();
        auto pivotDeltaX = invertX == -1 ? pivot.x : 1.0f - pivot.x;
        if (pivotDeltaX != 0.0f)
        {
            deltaSize.x /= pivotDeltaX;
        }
        auto pivotDeltaY = invertY == -1 ? pivot.y : 1.0f - pivot.y;
        if (pivotDeltaY != 0.0f)
        {
            deltaSize.y /= pivotDeltaY;
        }
    }
    //modify rateably
    if (rateably)
    {
        //check situation when we try to resize up and down simultaneously
        if (invertX != 0 && invertY != 0) //actual only for corners
        {
            if (fabs(angeledDelta.x) > 0.0f && fabs(angeledDelta.y) > 0.0f) //only if up and down requested
            {
                bool canNotResize = ((angeledDelta.x * invertX) > 0.0f) ^ ((angeledDelta.y * invertY) > 0.0f);
                if (canNotResize) // and they have different sign for corner
                {
                    float prop = fabs(angeledDelta.x) / fabs(angeledDelta.y);
                    if (prop > 0.48f && prop < 0.52f) // like "resize 10 to up and 10 to down rateably"
                    {
                        return;
                    }
                }
            }
        }
        //calculate proportion of control
        const Vector2& size = activeControlNode->GetControl()->GetSize();
        float proportion = size.y != 0.0f ? size.x / size.y : 0.0f;
        if (proportion != 0.0f)
        {
            //get current drag direction
            if (fabs(angeledDelta.y) > fabs(angeledDelta.x))
            {
                deltaSize.x = deltaSize.y * proportion;
                if (!withPivot)
                {
                    deltaPosition.x = deltaSize.y * proportion;
                    if (invertX == 0)
                    {
                        deltaPosition.x *= (0.5f - pivot.x) * -1.0f; //rainbow unicorn was here and add -1 to the right.
                    }
                    else
                    {
                        deltaPosition.x *= (invertX == -1 ? 1.0f - pivot.x : pivot.x) * invertX;
                    }
                }
            }
            else
            {
                deltaSize.y = deltaSize.x / proportion;
                if (!withPivot)
                {
                    deltaPosition.y = deltaSize.x / proportion;
                    if (invertY == 0)
                    {
                        deltaPosition.y *= (0.5f - pivot.y) * -1.0f; // another rainbow unicorn adds -1 here.
                    }
                    else
                    {
                        deltaPosition.y *= (invertY == -1 ? 1.0f - pivot.y : pivot.y) * invertY;
                    }
                }
            }
        }
    }

    deltaPosition *= control->GetScale();

    AdjustResize(deltaSize, deltaPosition);

    //rotate delta position backwards, because SetPosition require absolute coordinates
    Vector2 rotatedPosition;
    rotatedPosition.x = deltaPosition.x * gd.cosA + deltaPosition.y * -gd.sinA;
    rotatedPosition.y = deltaPosition.x * gd.sinA + deltaPosition.y * gd.cosA;

    Vector<PropertyDelta> propertiesDelta;
    propertiesDelta.emplace_back("Position", VariantType(rotatedPosition));
    propertiesDelta.emplace_back("Size", VariantType(deltaSize));
    AdjustProperty(activeControlNode, propertiesDelta);
}

void TransformSystem::AdjustResize(DAVA::Vector2& deltaSize, DAVA::Vector2& deltaPosition)
{
    if (extraDelta.dx != 0.0f)
    {
        float overload = extraDelta.dx + deltaSize.dx;
        if ((overload > 0.0f) ^ (extraDelta.dx > 0.0f)) //overload more than extraDelta
        {
            extraDelta.dx = 0.0f;
            deltaPosition.dx *= overload / deltaSize.dx;
            deltaSize.dx = overload;
        }
        else //delta less than we need to resize
        {
            extraDelta.x += deltaSize.dx;
            deltaSize.dx = 0.0f;
            deltaPosition.dx = 0.0f;
        }
    }
    if (extraDelta.dy != 0.0f)
    {
        float overload = extraDelta.dy + deltaSize.dy;
        if ((overload > 0.0f) ^ (extraDelta.dy > 0.0f)) //overload more than extraDelta
        {
            extraDelta.dy = 0.0f;
            deltaPosition.dy *= overload / deltaSize.dy;
            deltaSize.dy = overload;
        }
        else
        {
            extraDelta.y += deltaSize.dy;
            deltaSize.dy = 0;
            deltaPosition.dy = 0;
        }
    }

    AbstractProperty* sizeProperty = activeControlNode->GetRootProperty()->FindPropertyByName("Size");
    Vector2 origSize = sizeProperty->GetValue().AsVector2();
    Vector2 finalSize(origSize + deltaSize);

    if (finalSize.dx < minimumSize.dx)
    {
        float32 availableDelta = minimumSize.dx - origSize.dx;
        extraDelta.dx += deltaSize.dx - availableDelta;
        deltaPosition.dx *= availableDelta / deltaSize.dx;
        deltaSize.dx = availableDelta;
    }
    if (finalSize.dy < minimumSize.dy)
    {
        float32 availableDelta = minimumSize.dy - origSize.dy;
        extraDelta.dy += deltaSize.dy - availableDelta;
        deltaPosition.dy *= availableDelta / deltaSize.dy;
        deltaSize.dy = availableDelta;
    }

}

void TransformSystem::MovePivot(const Vector2& pos)
{
    const UIControl* control = activeControlNode->GetControl();
    const UIGeometricData& gd = control->GetGeometricData();
    const Vector2& size = control->GetSize();
    if (size.x != 0.0f && size.y != 0.0f && gd.scale.x != 0.0f && gd.scale.y != 0.0f)
    {
        Vector<PropertyDelta> propertiesDelta;

        const Vector2 delta = pos - prevPos;
        const Vector2 scaledDelta = delta / gd.scale;

        const Vector2 deltaPosition = scaledDelta * control->GetScale();
        //position calculates in absolute
        propertiesDelta.emplace_back("Position", VariantType(deltaPosition));
        //pivot point calculate in rotate coordinates
        const Vector2 angeledDelta(scaledDelta.x * gd.cosA + scaledDelta.y * gd.sinA,
                                   scaledDelta.x * -gd.sinA + scaledDelta.y * gd.cosA);
        const Vector2 pivot(angeledDelta / size);
        propertiesDelta.emplace_back("Pivot", VariantType(pivot));

        AdjustProperty(activeControlNode, propertiesDelta);
    }
}

void TransformSystem::Rotate(const Vector2& pos)
{
    const UIControl* control = activeControlNode->GetControl();
    const UIGeometricData& gd = control->GetGeometricData();
    const Rect& ur = gd.GetUnrotatedRect();
    Vector2 pivotPoint = ur.GetPosition() + ur.GetSize() * control->GetPivot();
    Vector2 rotatePoint = pivotPoint;
    Vector2 l1(prevPos - rotatePoint);
    Vector2 l2(pos - rotatePoint);
    float32 angleRad = atan2(l2.y, l2.x) - atan2(l1.y, l1.x);
    float32 angle = RadToDeg(angleRad);
    if (InputSystem::Instance()->GetKeyboard().IsKeyPressed(DVKEY_SHIFT))
    {
        AbstractProperty* property = activeControlNode->GetRootProperty()->FindPropertyByName("Angle");
        float32 currentAngle = property->GetValue().AsFloat();
        Vector2 newAngle(currentAngle + angle, 0.0f);
        AccumulateOperation(ROTATE_OPERATION, newAngle);
        angle = newAngle.dx - currentAngle;
    }
    else
    {
        angle = round(angle);
    }
    AdjustProperty(activeControlNode, "Angle", VariantType(angle));
}

void TransformSystem::AdjustProperty(ControlNode* node, const String& propertyName, const VariantType& delta)
{
    Vector<PropertyDelta> propertiesDelta;
    propertiesDelta.emplace_back(propertyName, delta);
    AdjustProperty(node, propertiesDelta);
}

void TransformSystem::AdjustProperty(ControlNode* node, const Vector<PropertyDelta>& propertiesDelta)
{
    Vector<std::pair<AbstractProperty*, VariantType>> propertiesToChange;
    for (const auto& pair : propertiesDelta)
    {
        AbstractProperty* property = node->GetRootProperty()->FindPropertyByName(pair.first);
        DVASSERT(nullptr != property);
        const VariantType& delta = pair.second;
        VariantType var(delta);

        switch (delta.GetType())
        {
        case VariantType::TYPE_VECTOR2:
            var = VariantType(property->GetValue().AsVector2() + delta.AsVector2());
            break;
        case VariantType::TYPE_FLOAT:
            var = VariantType(property->GetValue().AsFloat() + delta.AsFloat());
            break;
        default:
            DVASSERT_MSG(false, "unexpected type");
            break;
        }
        propertiesToChange.emplace_back(property, var);
    }
    systemManager->PropertiesChanged.Emit(std::move(node), std::move(propertiesToChange), std::move(currentHash));
}

void TransformSystem::AccumulateOperation(ACCUMULATE_OPERATIONS operation, DAVA::Vector2& delta)
{
    const int step = steps[operation];
    int x = accumulates[operation][Vector2::AXIS_X];
    int y = accumulates[operation][Vector2::AXIS_Y];

    if (abs(x) < step)
    {
        x += delta.x;
    }

    if (abs(y) < step)
    {
        y += delta.y;
    }

    delta = Vector2(x - x % step, y - y % step);
    x -= delta.x;
    y -= delta.y;
    accumulates[operation][Vector2::AXIS_X] = x;
    accumulates[operation][Vector2::AXIS_Y] = y;
}