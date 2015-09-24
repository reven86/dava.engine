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

Vector2 RotateVector(Vector2 in, const UIGeometricData& gd)
{
    return Vector2(in.x * gd.cosA + in.y * gd.sinA,
                   in.x * -gd.sinA + in.y * gd.cosA);
}

Vector2 RotateVectorInv(Vector2 in, const float32& angle)
{
    return Vector2(in.x * cosf(angle) + in.y * -sinf(angle),
                   in.x * sinf(angle) + in.y * cosf(angle));
}
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
    if (nullptr != activeControlNode)
    {
        UIControl* control = activeControlNode->GetControl();
        UIControl* parent = control->GetParent();
        parentGeometricData = parent->GetGeometricData();
        controlGeometricData = control->GetGeometricData();
        sizeProperty = activeControlNode->GetRootProperty()->FindPropertyByName("Size");
    }
    else
    {
        sizeProperty = nullptr;
    }
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
        bool toRemove = false;
        PackageBaseNode* parent = (*iter)->GetParent();
        if (nullptr == parent || nullptr == parent->GetControl())
        {
            toRemove = true;
        }
        else
        {
            auto iter2 = nodesToMove.begin();
            while (iter2 != nodesToMove.end() && !toRemove)
            {
                PackageBaseNode* node1 = *iter;
                PackageBaseNode* node2 = *iter2;
                if (iter != iter2)
                {
                    while (nullptr != node1->GetParent() && nullptr != node1->GetControl() && !toRemove)
                    {
                        if (node1 == node2)
                        {
                            toRemove = true;
                        }
                        node1 = node1->GetParent();
                    }
                }
                ++iter2;
            }
        }
        if (toRemove)
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

bool TransformSystem::ProcessDrag(Vector2 pos)
{
    if (activeArea == HUDAreaInfo::NO_AREA)
    {
        return false;
    }
    Vector2 delta(pos - prevPos);
    switch (activeArea)
    {
    case HUDAreaInfo::FRAME_AREA:
        MoveControl(delta);
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
        ResizeControl(delta, withPivot, rateably);
        return true;
    }
    case HUDAreaInfo::PIVOT_POINT_AREA:
    {
        MovePivot(delta);
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

void TransformSystem::MoveAllSelectedControls(Vector2 delta)
{
    for (auto& controlNode : nodesToMove)
    {
        if (controlNode->IsEditingSupported())
        {
            AdjustProperty(controlNode, "Position", VariantType(delta));
        }
    }
}

void TransformSystem::MoveControl(Vector2 delta)
{
    if (parentGeometricData.scale.x != 0.0f && parentGeometricData.scale.y != 0.0f)
    {
        Vector2 scaledDelta = delta / parentGeometricData.scale;
        Vector2 angeledDelta(RotateVector(scaledDelta, parentGeometricData));
        MoveAllSelectedControls(angeledDelta);
    }
}

void TransformSystem::ResizeControl(Vector2 delta, bool withPivot, bool rateably)
{
    DVASSERT(activeArea != HUDAreaInfo::NO_AREA);
    if (parentGeometricData.scale.x == 0.0f || parentGeometricData.scale.y == 0.0f)
    {
        return;
    }

    const auto directionX = cornersDirection.at(activeArea - HUDAreaInfo::TOP_LEFT_AREA)[Vector2::AXIS_X];
    const auto directionY = cornersDirection.at(activeArea - HUDAreaInfo::TOP_LEFT_AREA)[Vector2::AXIS_Y];

    Vector2 pivot(controlGeometricData.size.x != 0.0f ? controlGeometricData.pivotPoint.x / controlGeometricData.size.x : 0.0f,
                  controlGeometricData.size.y != 0.0f ? controlGeometricData.pivotPoint.y / controlGeometricData.size.y : 0.0f);

    Vector2 deltaMappedToControl(delta / controlGeometricData.scale);
    deltaMappedToControl = RotateVector(deltaMappedToControl, controlGeometricData);

    AdjustResize(directionX, directionY, deltaMappedToControl);

    Vector2 deltaSize(deltaMappedToControl);
    Vector2 deltaPosition(deltaMappedToControl);

    deltaSize.x *= directionX;
    deltaSize.y *= directionY;

    deltaPosition.x *= directionX == -1 ? 1.0f - pivot.x : pivot.x;
    deltaPosition.y *= directionY == -1 ? 1.0f - pivot.y : pivot.y;

    if (directionX == 0)
    {
        deltaPosition.x = 0.0f;
    }
    if (directionY == 0)
    {
        deltaPosition.y = 0.0f;
    }

    //modify if pivot modificator selected
    if (withPivot)
    {
        deltaPosition.SetZero();

        auto pivotDeltaX = directionX == -1 ? pivot.x : 1.0f - pivot.x;
        if (pivotDeltaX != 0.0f)
        {
            deltaSize.x /= pivotDeltaX;
        }
        auto pivotDeltaY = directionY == -1 ? pivot.y : 1.0f - pivot.y;
        if (pivotDeltaY != 0.0f)
        {
            deltaSize.y /= pivotDeltaY;
        }
    }
    //modify rateably
    if (rateably)
    {
        //calculate proportion of control
        const Vector2& size = activeControlNode->GetControl()->GetSize();
        float proportion = size.y != 0.0f ? size.x / size.y : 0.0f;
        if (proportion != 0.0f)
        {
            //get current drag direction
            if (fabs(deltaSize.y) > fabs(deltaSize.x))
            {
                deltaSize.x = deltaSize.y * proportion;
                if (!withPivot)
                {
                    deltaPosition.x = deltaSize.y * proportion;
                    if (directionX == 0)
                    {
                        deltaPosition.x *= (0.5f - pivot.x) * -1.0f; //rainbow unicorn was here and add -1 to the right.
                    }
                    else
                    {
                        deltaPosition.x *= (directionX == -1 ? 1.0f - pivot.x : pivot.x) * directionX;
                    }
                }
            }
            else
            {
                deltaSize.y = deltaSize.x / proportion;
                if (!withPivot)
                {
                    deltaPosition.y = deltaSize.x / proportion;
                    if (directionY == 0)
                    {
                        deltaPosition.y *= (0.5f - pivot.y) * -1.0f; // another rainbow unicorn adds -1 here.
                    }
                    else
                    {
                        deltaPosition.y *= (directionY == -1 ? 1.0f - pivot.y : pivot.y) * directionY;
                    }
                }
            }
        }
    }

    UIControl* control = activeControlNode->GetControl();
    deltaPosition *= control->GetScale();
    deltaPosition = RotateVectorInv(deltaPosition, control->GetAngle());
    Vector<PropertyDelta> propertiesDelta;
    propertiesDelta.emplace_back("Position", VariantType(deltaPosition));
    propertiesDelta.emplace_back("Size", VariantType(deltaSize));
    AdjustProperty(activeControlNode, propertiesDelta);
}

void TransformSystem::AdjustResize(int directionX, int directionY, Vector2& delta)
{
    if (extraDelta.dx != 0.0f)
    {
        float overload = extraDelta.dx + delta.dx;
        if ((overload > 0.0f) ^ (extraDelta.dx > 0.0f)) //overload more than extraDelta
        {
            extraDelta.dx = 0.0f;
            delta.dx = overload;
        }
        else //delta less than we need to resize
        {
            extraDelta.dx += delta.dx;
            delta.dx = 0.0f;
        }
    }
    if (extraDelta.dy != 0.0f)
    {
        float overload = extraDelta.dy + delta.dy;
        if ((overload > 0.0f) ^ (extraDelta.dy > 0.0f)) //overload more than extraDelta
        {
            extraDelta.dy = 0.0f;
            delta.dy = overload;
        }
        else
        {
            extraDelta.dy += delta.dy;
            delta.dy = 0;
        }
    }

    const Vector2 scaledMinimum(minimumSize / controlGeometricData.scale);
    Vector2 origSize = sizeProperty->GetValue().AsVector2();

    Vector2 deltaSize(delta.x * directionX, delta.y * directionY);
    Vector2 finalSize(origSize + deltaSize);

    if (finalSize.dx < scaledMinimum.dx)
    {
        if (deltaSize.dx >= 0.0f)
        {
            return;
        }
        float32 availableDelta = origSize.dx - scaledMinimum.dx;
        availableDelta *= directionX * -1;
        extraDelta.dx += delta.dx - availableDelta;
        delta.dx = availableDelta;
    }
    if (finalSize.dy < scaledMinimum.dy)
    {
        if (deltaSize.dy >= 0.0f)
        {
            return;
        }
        float32 availableDelta = origSize.dy - scaledMinimum.dy;
        availableDelta *= directionY * -1;
        extraDelta.dy += delta.dy - availableDelta;
        delta.dy = availableDelta;
    }

}

void TransformSystem::MovePivot(Vector2 delta)
{
    const Rect ur(controlGeometricData.GetUnrotatedRect());
    const Vector2 controlSize(ur.GetSize());
    if (parentGeometricData.scale.x == 0.0f || parentGeometricData.scale.y == 0.0f || controlSize.dx == 0.0f || controlSize.dy == 0.0f)
    {
        return;
    }
    Vector<PropertyDelta> propertiesDelta;

    Vector2 scaledDelta(delta / parentGeometricData.scale);
    Vector2 angeledDeltaPosition(RotateVector(scaledDelta, parentGeometricData));
    propertiesDelta.emplace_back("Position", VariantType(angeledDeltaPosition));

    const Vector2 angeledDeltaPivot(RotateVector(delta, controlGeometricData));
    const Vector2 pivot(angeledDeltaPivot / controlSize);
    propertiesDelta.emplace_back("Pivot", VariantType(pivot));
    AdjustProperty(activeControlNode, propertiesDelta);
}

void TransformSystem::Rotate(Vector2 pos)
{
    Vector2 rotatePoint(controlGeometricData.GetUnrotatedRect().GetPosition());
    rotatePoint += controlGeometricData.pivotPoint * controlGeometricData.scale;
    Vector2 l1(prevPos - rotatePoint);
    Vector2 l2(pos - rotatePoint);
    float32 angleRad = atan2(l2.y, l2.x) - atan2(l1.y, l1.x);
    float32 angle = RadToDeg(angleRad);

    angle = round(angle);
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