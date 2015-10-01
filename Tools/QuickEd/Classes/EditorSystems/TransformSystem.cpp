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
#include "Model/PackageHierarchy/ControlNode.h"
#include "Model/ControlProperties/RootProperty.h"
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

Vector2 RotateVectorInv(Vector2 in, const UIGeometricData& gd)
{
    return Vector2(in.x * gd.cosA + in.y * -gd.sinA,
                   in.x * gd.sinA + in.y * gd.cosA);
}

Vector2 RotateVectorInv(Vector2 in, const float32& angle)
{
    return Vector2(in.x * cosf(angle) + in.y * -sinf(angle),
                   in.x * sinf(angle) + in.y * cosf(angle));
}
}

TransformSystem::TransformSystem(EditorSystemsManager* parent)
    : BaseEditorSystem(parent)
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
        RootProperty* rootProperty = activeControlNode->GetRootProperty();
        sizeProperty = rootProperty->FindPropertyByName("Size");
        positionProperty = rootProperty->FindPropertyByName("Position");
        angleProperty = rootProperty->FindPropertyByName("Angle");
        pivotProperty = rootProperty->FindPropertyByName("Pivot");
    }
    else
    {
        sizeProperty = nullptr;
        positionProperty = nullptr;
        angleProperty = nullptr;
        pivotProperty = nullptr;
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
        extraDelta.SetZero();
        prevPos = currentInput->point;
        microseconds us = duration_cast<microseconds>(system_clock::now().time_since_epoch());
        currentHash = static_cast<size_t>(us.count());
        return activeArea != HUDAreaInfo::NO_AREA;
    }
    case UIEvent::PHASE_DRAG:
    {
        if (currentInput->point != prevPos)
        {
            ProcessDrag(currentInput->point);
            prevPos = currentInput->point;
        }
        return false;
    }
    case UIEvent::PHASE_ENDED:
        return false;
    default:
        return false;
    }
}

void TransformSystem::OnSelectionChanged(const SelectedNodes& selected, const SelectedNodes& deselected)
{
    SelectionContainer::MergeSelectionAndContainer(selected, deselected, selectedControlNodes);
    nodesToMove.clear();
    for (ControlNode* selectedControl : selectedControlNodes)
    {
        nodesToMove.emplace_back(selectedControl, nullptr, nullptr);
    }
    CorrectNodesToMove();
}

bool TransformSystem::ProcessKey(const int32 key)
{
    if (!selectedControlNodes.empty())
    {
        float step = 1.0f;
        const KeyboardDevice& keyboard = InputSystem::Instance()->GetKeyboard();
        if (keyboard.IsKeyPressed(DVKEY_SHIFT))
        {
            step = 10.0f;
        }
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
        MoveAllSelectedControls(delta);
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
    Vector<std::tuple<ControlNode*, AbstractProperty*, VariantType>> propertiesToChange;
    if (nodesToMove.size() == 1)
    {
        const auto& tuple = nodesToMove.front();
        Vector2 finalPos = AdjustMove(delta);
        propertiesToChange.emplace_back(std::get<0>(tuple), std::get<1>(tuple), VariantType(finalPos));
        systemManager->PropertiesChanged.Emit(std::move(propertiesToChange), std::move(currentHash));
    }
    return;
    for (auto& tuple : nodesToMove)
    {
        ControlNode* node = std::get<0>(tuple);
        Vector2 scale = std::get<2>(tuple)->scale;
        if (node->IsEditingSupported() && scale.x != 0.0f && scale.y != 0.0f)
        {
            Vector2 scaledDelta = delta / scale;
            Vector2 rotatedDelta(RotateVector(scaledDelta, *std::get<2>(tuple)));

            AbstractProperty* property = std::get<1>(tuple);
            Vector2 originalPosition = property->GetValue().AsVector2();
            Vector2 finalPosition(originalPosition + rotatedDelta);
            propertiesToChange.emplace_back(node, property, VariantType(finalPosition));
        }
    }
    if (!propertiesToChange.empty())
    {
        systemManager->PropertiesChanged.Emit(std::move(propertiesToChange), std::move(currentHash));
    }
}

DAVA::Vector2 TransformSystem::AdjustMove(DAVA::Vector2& delta)
{
    const auto& tuple = nodesToMove.front();
    const UIGeometricData* gd = std::get<2>(tuple);
    const UIControl* control = std::get<0>(tuple)->GetControl();
    Rect box = control->GetLocalGeometricData().GetAABBox();
    Vector2 scaledDelta = delta / gd->scale;
    Vector2 deltaPosition(RotateVector(scaledDelta, *gd));

    const Vector2 scaledRange(Vector2(20.0f, 20.0f) / gd->scale);
    const Vector2 range(RotateVector(scaledRange, *gd));

    AbstractProperty* property = std::get<1>(tuple);
    Vector2 originalPosition = property->GetValue().AsVector2();
    Vector2 finalPosition(originalPosition + deltaPosition + extraDelta);
    const float32 delimiter = 0.5f;
    const float32 maxProp = 1.0f;

    Vector2 target;
    Vector2 distanceToTarget;
    bool magnetX = false;
    bool magnetY = false;

    const KeyboardDevice& keyboard = InputSystem::Instance()->GetKeyboard();
    if (keyboard.IsKeyPressed(DVKEY_SHIFT))
    {
        for (float32 part = 0.0f; part <= maxProp; part += delimiter)
        {
            float32 controlLeft = box.x;
            float32 controlRight = box.x + box.GetSize().dx;

            float32 targetPos = part * gd->size.dx;
            float32 left = targetPos - range.dx;
            float32 right = targetPos + range.dx;
            if (finalPosition.dx >= left && finalPosition.dx <= right)
            {
                float32 distance = fabs(finalPosition.dx - targetPos);
                if (distance == 0.0f || !magnetX || distance < distanceToTarget.dx)
                {
                    distanceToTarget.dx = distance;
                    target.dx = targetPos;
                }
                magnetX = true;
            }
        }

        for (float32 part = 0.0f; part <= maxProp; part += delimiter)
        {
            float32 targetPos = part * gd->size.dy;
            float32 top = targetPos - range.dy;
            float32 bottom = targetPos + range.dy;

            if (finalPosition.dy >= top && finalPosition.dy <= bottom)
            {
                float32 distance = fabs(finalPosition.dy - targetPos);
                if (distance == 0.0f || !magnetY || distance < distanceToTarget.dy)
                {
                    distanceToTarget.dy = distance;
                    target.dy = targetPos;
                }
                magnetY = true;
            }
        }
    }
    if (magnetX)
    {
        extraDelta.dx = finalPosition.dx - target.dx;
        finalPosition.dx = target.dx;
        delta.dx = target.dx - originalPosition.dx;
    }
    else if (extraDelta.dx != 0.0f)
    {
        deltaPosition.dx += extraDelta.dx;
        extraDelta.dx = 0.0f;
        delta.dx = deltaPosition.dx;
    }
    if (magnetY)
    {
        extraDelta.dy = finalPosition.dy - target.dy;
        finalPosition.dy = target.dy;
        delta.dy = target.dy - originalPosition.dy;
    }
    else if (extraDelta.dy != 0.0f)
    {
        deltaPosition.dy += extraDelta.dy;
        extraDelta.dy = 0.0f;
        delta.dy = deltaPosition.dy;
    }
    delta = RotateVectorInv(delta, *gd);
    delta *= gd->scale;
    return finalPosition;
}

void TransformSystem::ResizeControl(Vector2 delta, bool withPivot, bool rateably)
{
    DVASSERT(activeArea != HUDAreaInfo::NO_AREA);

    const auto directionX = cornersDirection.at(activeArea - HUDAreaInfo::TOP_LEFT_AREA)[Vector2::AXIS_X];
    const auto directionY = cornersDirection.at(activeArea - HUDAreaInfo::TOP_LEFT_AREA)[Vector2::AXIS_Y];

    Vector2 pivot(controlGeometricData.size.dx != 0.0f ? controlGeometricData.pivotPoint.dx / controlGeometricData.size.dx : 0.0f,
                  controlGeometricData.size.dy != 0.0f ? controlGeometricData.pivotPoint.dy / controlGeometricData.size.dy : 0.0f);

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
    propertiesDelta.emplace_back(positionProperty, VariantType(deltaPosition));
    propertiesDelta.emplace_back(sizeProperty, VariantType(deltaSize));
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
        float32 availableDelta = origSize.dx - scaledMinimum.dx;
        if (availableDelta <= 0.0f && deltaSize.dx <= 0.0f)
        {
            extraDelta.dx += delta.dx;
            delta.dx = 0.0f;
        }
        else if (origSize.dx > scaledMinimum.dx)
        {
            availableDelta *= directionX * -1;
            extraDelta.dx += delta.dx - availableDelta;
            delta.dx = availableDelta;
        }
    }
    if (finalSize.dy < scaledMinimum.dy)
    {
        float32 availableDelta = origSize.dy - scaledMinimum.dy;
        if (availableDelta <= 0.0f && deltaSize.dy <= 0.0f)
        {
            extraDelta.dy += delta.dy;
            delta.dy = 0.0f;
        }
        else if (origSize.dy > scaledMinimum.dy)
        {
            availableDelta *= directionY * -1;
            extraDelta.dy += delta.dy - availableDelta;
            delta.dy = availableDelta;
        }
    }
}

void TransformSystem::MovePivot(Vector2 delta)
{
    DAVA::Vector<std::tuple<ControlNode*, AbstractProperty*, VariantType>> propertiesToChange;
    Vector2 pivot = AdjustPivot(delta);
    propertiesToChange.emplace_back(activeControlNode, pivotProperty, VariantType(pivot));

    Vector2 scaledDelta(delta / parentGeometricData.scale);
    Vector2 rotatedDeltaPosition(RotateVector(scaledDelta, parentGeometricData));
    Vector2 originalPos(positionProperty->GetValue().AsVector2());
    Vector2 finalPos(originalPos + rotatedDeltaPosition);
    propertiesToChange.emplace_back(activeControlNode, positionProperty, VariantType(finalPos));

    systemManager->PropertiesChanged.Emit(std::move(propertiesToChange), std::move(currentHash));
}

Vector2 TransformSystem::AdjustPivot(Vector2& delta)
{
    const Rect ur(controlGeometricData.GetUnrotatedRect());
    const Vector2 controlSize(ur.GetSize());

    const Vector2 rotatedDeltaPivot(RotateVector(delta, controlGeometricData));
    Vector2 deltaPivot(rotatedDeltaPivot / controlSize);

    const Vector2 scaledRange(Vector2(10.0f, 10.0f));
    const Vector2 range(scaledRange / controlSize); //range in pivot coordinates

    Vector2 origPivot = pivotProperty->GetValue().AsVector2();
    Vector2 finalPivot(origPivot + deltaPivot + extraDelta);

    bool found = false;
    const KeyboardDevice& keyboard = InputSystem::Instance()->GetKeyboard();
    if (keyboard.IsKeyPressed(DVKEY_SHIFT))
    {
        const float32 delimiter = 0.25f;
        const float32 maxPivot = 1.0f;

        Vector2 target;
        Vector2 distanceToTarget;
        for (float32 targetX = 0.0f; targetX <= maxPivot; targetX += delimiter)
        {
            for (float32 targetY = 0.0f; targetY <= maxPivot; targetY += delimiter)
            {
                float32 left = targetX - range.dx;
                float32 right = targetX + range.dx;
                float32 top = targetY - range.dy;
                float32 bottom = targetY + range.dy;
                if (finalPivot.dx >= left && finalPivot.dx <= right && finalPivot.dy >= top && finalPivot.dy <= bottom)
                {
                    Vector2 currentDistance(fabs(finalPivot.dx - targetX), fabs(finalPivot.dy - targetY));
                    if (currentDistance.IsZero() || !found || currentDistance.x < distanceToTarget.x || currentDistance.y < distanceToTarget.y)
                    {
                        distanceToTarget = currentDistance;
                        target = Vector2(targetX, targetY);
                    }
                    found = true;
                }
            }
        }
        if (found)
        {
            extraDelta.dx = finalPivot.dx - target.x;
            extraDelta.dy = finalPivot.dy - target.y;
            delta = RotateVectorInv((target - origPivot) * controlSize, controlGeometricData);

            finalPivot = target;
        }
    }

    if (!found && !extraDelta.IsZero())
    {
        deltaPivot.dx += extraDelta.dx;
        extraDelta.dx = 0.0f;
        deltaPivot.dy += extraDelta.dy;
        extraDelta.dy = 0.0f;
        delta = RotateVectorInv(deltaPivot * controlSize, controlGeometricData);
    }
    return finalPivot;
}

void TransformSystem::Rotate(Vector2 pos)
{
    Vector2 rotatePoint(controlGeometricData.GetUnrotatedRect().GetPosition());
    rotatePoint += controlGeometricData.pivotPoint * controlGeometricData.scale;
    Vector2 l1(prevPos - rotatePoint);
    Vector2 l2(pos - rotatePoint);
    float32 angleRad = atan2(l2.y, l2.x) - atan2(l1.y, l1.x);
    float32 deltaAngle = RadToDeg(angleRad);

    float32 originalAngle = angleProperty->GetValue().AsFloat();

    const KeyboardDevice& keyboard = InputSystem::Instance()->GetKeyboard();
    if (keyboard.IsKeyPressed(DVKEY_SHIFT))
    {
        static const int step = 15; //fixed angle step
        float32 finalAngle = originalAngle + deltaAngle + extraDelta.dx;
        int32 nearestTargetAngle = static_cast<int32>(finalAngle - static_cast<int32>(finalAngle) % step);
        if (finalAngle >= 0.0f && deltaAngle < 0.0f)
        {
            nearestTargetAngle += step;
        }
        else if (finalAngle < 0.0f && deltaAngle >= 0.0f)
        {
            nearestTargetAngle -= step;
        }
        if ((deltaAngle >= 0.0f && nearestTargetAngle <= originalAngle + 0.005f) || (deltaAngle < 0.0f && nearestTargetAngle >= originalAngle - 0.005))
        {
            extraDelta.dx += deltaAngle;
            return;
        }
        if ((deltaAngle != 0))
        {
            extraDelta.dx = finalAngle - nearestTargetAngle;
        }
        deltaAngle = nearestTargetAngle - originalAngle;
    }
    float32 finalAngle = originalAngle + deltaAngle;
    DAVA::Vector<std::tuple<ControlNode*, AbstractProperty*, VariantType>> propertiesToChange;
    propertiesToChange.emplace_back(activeControlNode, angleProperty, VariantType(finalAngle));
    systemManager->PropertiesChanged.Emit(std::move(propertiesToChange), std::move(currentHash));
}

void TransformSystem::AdjustProperty(ControlNode* node, const Vector<PropertyDelta>& propertiesDelta)
{
    DAVA::Vector<std::tuple<ControlNode*, AbstractProperty*, VariantType>> propertiesToChange;
    for (const PropertyDelta& pair : propertiesDelta)
    {
        AbstractProperty* property = pair.first;
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
        propertiesToChange.emplace_back(node, property, var);
    }
    if (!propertiesToChange.empty())
    {
        systemManager->PropertiesChanged.Emit(std::move(propertiesToChange), std::move(currentHash));
    }
}

void TransformSystem::CorrectNodesToMove()
{
    auto iter = nodesToMove.begin();
    while (iter != nodesToMove.end())
    {
        bool toRemove = false;
        PackageBaseNode* parent = std::get<0>(*iter)->GetParent();
        if (nullptr == parent || nullptr == parent->GetControl())
        {
            toRemove = true;
        }
        else
        {
            auto iter2 = nodesToMove.begin();
            while (iter2 != nodesToMove.end() && !toRemove)
            {
                PackageBaseNode* node1 = std::get<0>(*iter);
                PackageBaseNode* node2 = std::get<0>(*iter2);
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
    for (auto& tuple : nodesToMove)
    {
        ControlNode* node = std::get<0>(tuple);
        UIControl* control = node->GetControl();
        std::get<1>(tuple) = node->GetRootProperty()->FindPropertyByName("Position");
        std::get<2>(tuple) = &control->GetParent()->GetGeometricData();
    }
}
