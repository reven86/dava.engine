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

#include <QApplication>

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

struct MagnetLine
{
    //controlBox and targetBox in parent coordinates. controlSharePos ans targetSharePos is a share of corresponding size
    MagnetLine(float32 controlSharePos_, const Rect& controlBox_, float32 targetSharePos_, const Rect& targetBox_, Vector2::eAxis axis_)
        : controlSharePos(controlSharePos_)
        , controlBox(controlBox_)
        , targetSharePos(targetSharePos_)
        , targetBox(targetBox_)
        , multiplyer(controlSharePos)
        , axis(axis_)
    {
        controlPosition = controlBox.GetPosition()[axis] + controlBox.GetSize()[axis] * controlSharePos;
        targetPosition = targetBox.GetPosition()[axis] + targetBox.GetSize()[axis] * targetSharePos;
        distance = controlPosition - targetPosition;
    }

    float32 controlSharePos;
    float32 controlPosition;
    Rect controlBox;

    float32 targetSharePos;
    float32 targetPosition;
    Rect targetBox;

    float32 multiplyer = 1.0f;
    float32 distance;

    Vector2::eAxis axis;
};

const float32 TRANSFORM_EPSILON = static_cast<float32>(0.0005);
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
    UpdateNeighboursToMove();
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
        systemManager->MagnetLinesChanged.Emit(Vector<MagnetLineInfo>());
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
    UpdateNeighboursToMove();
}

bool TransformSystem::ProcessKey(const int32 key)
{
    if (!selectedControlNodes.empty())
    {
        float step = 1.0f;
        if (!QApplication::keyboardModifiers().testFlag(Qt::ShiftModifier))
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
            MoveAllSelectedControls(deltaPos, false);
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
        MoveAllSelectedControls(delta, !QApplication::keyboardModifiers().testFlag(Qt::ShiftModifier));
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
        bool withPivot = QApplication::keyboardModifiers().testFlag(Qt::AltModifier);
        bool rateably = QApplication::keyboardModifiers().testFlag(Qt::ShiftModifier);
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

void TransformSystem::MoveAllSelectedControls(Vector2 delta, bool canAdjust)
{
    Vector<std::tuple<ControlNode*, AbstractProperty*, VariantType>> propertiesToChange;
    Vector<MagnetLineInfo> magnets;

    if (canAdjust)
    {
        Vector2 scaledDelta = delta / parentGeometricData.scale;
        Vector2 deltaPosition(RotateVector(scaledDelta, parentGeometricData));
        Vector2 movedDeltaPosition = deltaPosition + extraDelta;
        extraDelta.SetZero();
        Vector2 adjustedPositon = AdjustMove(movedDeltaPosition, magnets, &parentGeometricData, activeControlNode->GetControl());
        AbstractProperty* property = positionProperty;
        Vector2 originalPosition = property->GetValue().AsVector2();
        Vector2 finalPosition(originalPosition + adjustedPositon);
        propertiesToChange.emplace_back(activeControlNode, property, VariantType(finalPosition));
        for (int32 axisInt = Vector2::AXIS_X; axisInt < Vector2::AXIS_COUNT; ++axisInt)
        {
            Vector2::eAxis axis = static_cast<Vector2::eAxis>(axisInt);
            if (fabs(deltaPosition[axis]) > TRANSFORM_EPSILON)
            {
                delta[axis] *= adjustedPositon[axis] / deltaPosition[axis];
            }
            else
            {
                delta[axis] = 0.0f;
            }
        }
    }

    for (auto& tuple : nodesToMove)
    {
        ControlNode* node = std::get<NODE_INDEX>(tuple);
        if (node == activeControlNode)
        {
            continue;
        }
        const UIGeometricData* gd = std::get<GD_INDEX>(tuple);
        Vector2 scaledDelta = delta / gd->scale;
        Vector2 deltaPosition(RotateVector(scaledDelta, *gd));
        AbstractProperty* property = std::get<PROPERTY_INDEX>(tuple);
        Vector2 originalPosition = property->GetValue().AsVector2();
        Vector2 finalPosition(originalPosition + deltaPosition);
        propertiesToChange.emplace_back(node, property, VariantType(finalPosition));
    }
    if (!propertiesToChange.empty())
    {
        systemManager->PropertiesChanged.Emit(std::move(propertiesToChange), std::move(currentHash));
    }
    systemManager->MagnetLinesChanged.Emit(magnets);
}

List<MagnetLine> CreateMagnetPairsForMove(const Rect& box, const UIGeometricData* parentGD, const Vector<UIControl*>& neighbours, Vector2::eAxis axis)
{
    List<MagnetLine> magnets;

    Rect parentBox(Vector2(), parentGD->size);
    parentBox.SetPosition(Vector2(0.0f, 0.0f));
    DVASSERT(box.dx > 0.0f && box.dy > 0.0f);

    magnets.emplace_back(0.0f, box, 0.0f, parentBox, axis);
    magnets.emplace_back(0.5f, box, 0.5f, parentBox, axis);
    magnets.emplace_back(1.0f, box, 1.0f, parentBox, axis);

    const float32 border = 20.0f;
    if (parentGD->GetUnrotatedRect().GetSize()[axis] > 5 * border)
    {
        const float32 borderShare = border / parentBox.GetSize()[axis];
        magnets.emplace_back(0.0f, box, borderShare, parentBox, axis);
        magnets.emplace_back(1.0f, box, 1.0f - borderShare, parentBox, axis);
    }

    for (UIControl* neighbour : neighbours)
    {
        Rect neighbourBox = neighbour->GetLocalGeometricData().GetAABBox();

        magnets.emplace_back(0.0f, box, 0.0f, neighbourBox, axis);
        magnets.emplace_back(0.5f, box, 0.5f, neighbourBox, axis);
        magnets.emplace_back(1.0f, box, 1.0f, neighbourBox, axis);
        magnets.emplace_back(0.0f, box, 1.0f, neighbourBox, axis);
        magnets.emplace_back(1.0f, box, 0.0f, neighbourBox, axis);

        const float32 neighbourSpacing = 5.0f;
        const float32 neighbourSpacingShare = neighbourSpacing / neighbourBox.GetSize()[axis];
        magnets.emplace_back(1.0f, box, -neighbourSpacingShare, neighbourBox, axis);
        magnets.emplace_back(0.0f, box, 1.0f + neighbourSpacingShare, neighbourBox, axis);
    }
    DVASSERT(!magnets.empty());

    return magnets;
}

void ExtractMatchedLines(Vector<MagnetLineInfo>& magnets, const List<MagnetLine>& magnetLines, const UIControl* control, Vector2::eAxis axis, float32 extraDelta)
{
    UIControl* parent = control->GetParent();
    const UIGeometricData* parentGD = &parent->GetGeometricData();
    for (const MagnetLine& line : magnetLines)
    {
        if (fabs(line.distance - extraDelta) < TRANSFORM_EPSILON)
        {
            Vector2 position = parentGD->position - RotateVectorInv(parentGD->pivotPoint * parentGD->scale, *parentGD);

            const Vector2::eAxis axis2 = axis == Vector2::AXIS_X ? Vector2::AXIS_Y : Vector2::AXIS_X;

            float32 controlTop = line.controlBox.GetPosition()[axis2];
            float32 controlBottom = controlTop + line.controlBox.GetSize()[axis2];

            float32 targetTop = line.targetBox.GetPosition()[axis2];
            float32 targetBottom = targetTop + line.targetBox.GetSize()[axis2];

            Vector2 linePos;
            linePos[axis] = line.targetPosition;
            linePos[axis2] = Min(controlTop, targetTop);
            Vector2 lineSize;
            lineSize[axis] = 0.0f;
            lineSize[axis2] = Max(controlBottom, targetBottom) - linePos[axis2];

            linePos = RotateVectorInv(linePos, *parentGD);
            linePos *= parentGD->scale;
            lineSize *= parentGD->scale;
            Rect lineRect(linePos + position, lineSize);
            magnets.emplace_back(lineRect, parentGD, axis2);
        }
    }
}

Vector2 TransformSystem::AdjustMove(Vector2 delta, Vector<MagnetLineInfo>& magnets, const UIGeometricData* parentGD, const UIControl* control)
{
    const Vector2 scaledRange(Vector2(20.0f, 20.0f) / parentGD->scale);
    const Vector2 range(RotateVector(scaledRange, *parentGD));

    UIGeometricData controlGD = control->GetLocalGeometricData();
    Rect box = controlGD.GetAABBox();
    box.SetPosition(box.GetPosition() + delta);

    for (int32 axisInt = Vector2::AXIS_X; axisInt < Vector2::AXIS_COUNT; ++axisInt)
    {
        Vector2::eAxis axis = static_cast<Vector2::eAxis>(axisInt);

        List<MagnetLine> magnetLines = CreateMagnetPairsForMove(box, parentGD, neighbours, axis);
        std::function<bool(const MagnetLine&, const MagnetLine&)> predicate = [](const MagnetLine& left, const MagnetLine& right) -> bool
        {
            return fabs(left.distance) < fabs(right.distance);
        };
        MagnetLine nearestLine = *std::min_element(magnetLines.begin(), magnetLines.end(), predicate);
        float32 areaNearLineRight = nearestLine.targetPosition + range[axis];
        float32 areaNearLineLeft = nearestLine.targetPosition - range[axis];

        if (nearestLine.controlPosition >= areaNearLineLeft && nearestLine.controlPosition <= areaNearLineRight)
        {
            extraDelta[axis] = nearestLine.distance;
            delta[axis] -= nearestLine.distance;
        }

        ExtractMatchedLines(magnets, magnetLines, control, axis, extraDelta[axis]);
    }
    return delta;
}

void TransformSystem::ResizeControl(Vector2 delta, bool withPivot, bool rateably)
{
    DVASSERT(activeArea != HUDAreaInfo::NO_AREA);

    const auto directions = cornersDirection.at(activeArea - HUDAreaInfo::TOP_LEFT_AREA);

    Vector2 pivot(controlGeometricData.size.dx != 0.0f ? controlGeometricData.pivotPoint.dx / controlGeometricData.size.dx : 0.0f,
                  controlGeometricData.size.dy != 0.0f ? controlGeometricData.pivotPoint.dy / controlGeometricData.size.dy : 0.0f);

    Vector2 deltaMappedToControl(delta / controlGeometricData.scale);
    deltaMappedToControl = RotateVector(deltaMappedToControl, controlGeometricData);

    Vector2 adjustedDelta = AdjustResize(directions, deltaMappedToControl);

    Vector2 deltaSize(adjustedDelta);
    Vector2 deltaPosition(adjustedDelta);
    for (int32 axisInt = Vector2::AXIS_X; axisInt < Vector2::AXIS_COUNT; ++axisInt)
    {
        Vector2::eAxis axis = static_cast<Vector2::eAxis>(axisInt);
        const int direction = directions[axis];

        deltaSize[axis] *= direction;
        deltaPosition[axis] *= direction == -1 ? 1.0f - pivot[axis] : pivot[axis];

        if (direction == 0)
        {
            deltaPosition[axis] = 0.0f;
        }

        //modify if pivot modificator selected
        if (withPivot)
        {
            deltaPosition[axis] = 0.0f;

            auto pivotDelta = direction == -1 ? pivot[axis] : 1.0f - pivot[axis];
            if (pivotDelta != 0.0f)
            {
                deltaSize[axis] /= pivotDelta;
            }
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
            Vector2 propDeltaSize(deltaSize.y * proportion, deltaSize.x / proportion);
            //get current drag direction
            Vector2::eAxis axis = fabs(deltaSize.y) > fabs(deltaSize.x) ? Vector2::AXIS_X : Vector2::AXIS_Y;

            deltaSize[axis] = propDeltaSize[axis];
            if (!withPivot)
            {
                deltaPosition[axis] = propDeltaSize[axis];
                if (directions[axis] == 0)
                {
                    deltaPosition[axis] *= (0.5f - pivot[axis]) * -1.0f; //rainbow unicorn was here and add -1 to the right.
                }
                else
                {
                    deltaPosition[axis] *= (directions[axis] == -1 ? 1.0f - pivot[axis] : pivot[axis]) * directions[axis];
                }
            }
        }
    }

    UIControl* control = activeControlNode->GetControl();
    deltaPosition *= control->GetScale();
    deltaPosition = RotateVectorInv(deltaPosition, control->GetAngle());

    Vector<std::tuple<ControlNode*, AbstractProperty*, VariantType>> propertiesToChange;

    if (activeControlNode->GetParent() != nullptr && activeControlNode->GetParent()->GetControl() != nullptr)
    {
        Vector2 originalPosition = positionProperty->GetValue().AsVector2();
        propertiesToChange.emplace_back(activeControlNode, positionProperty, VariantType(originalPosition + deltaPosition));
    }
    Vector2 originalSize = sizeProperty->GetValue().AsVector2();
    propertiesToChange.emplace_back(activeControlNode, sizeProperty, VariantType(originalSize + deltaSize));

    systemManager->PropertiesChanged.Emit(std::move(propertiesToChange), std::move(currentHash));
}

Vector2 TransformSystem::AdjustResize(Array<int, Vector2::AXIS_COUNT> directions, Vector2 delta)
{
    const Vector2 scaledMinimum(minimumSize / controlGeometricData.scale);
    Vector2 origSize = sizeProperty->GetValue().AsVector2();
    delta += extraDelta; //transform to virtual coordinates
    extraDelta.SetZero();
    Vector2 deltaSize(delta.x * directions[Vector2::AXIS_X], delta.y * directions[Vector2::AXIS_Y]);
    Vector2 finalSize(origSize + deltaSize);

    Vector<MagnetLineInfo> magnets;

    /*for (int32 axisInt = Vector2::AXIS_X; axisInt < Vector2::AXIS_COUNT; ++axisInt)
    {
        Vector2::eAxis axis = static_cast<Vector2::eAxis>(axisInt);
        if (directions[axis] == 0)
        {
            continue;
        }
        if (finalSize[axis] < scaledMinimum[axis])
        {
            extraDelta[axis] = finalSize[axis] - scaledMinimum[axis];
            extraDelta[axis] /= directions[axis];
            int sign = delta[axis] > 0 ? 1 : -1;
            delta[axis] = Max(0.0f, origSize[axis] - scaledMinimum[axis]); //truncate delta
            delta[axis] *= sign; // restore delta sign;
        }
        else
        {
            if (!QApplication::keyboardModifiers().testFlag(Qt::ShiftModifier))
            {
                UIGeometricData* parentGD = &parentGeometricData;
                UIControl* control = activeControlNode->GetControl();
                const Vector2 scaledRange(Vector2(20.0f, 20.0f) / parentGD->scale);
                const Vector2 range(RotateVector(scaledRange, *parentGD));

                UIGeometricData controlGD = control->GetLocalGeometricData();
                Rect parentBox = parentGD->GetUnrotatedRect();
                parentBox.SetPosition(Vector2(0.0f, 0.0f));
                Rect box = controlGD.GetAABBox();

                List<MagnetLine> magnetPairs;
                float32 controlLeft = box.GetPosition()[axis];
                float32 controlRight = controlLeft + box.GetSize()[axis];

                float32 parentLeft = 0.0f;
                float32 parentRight = parentLeft + parentBox.GetSize()[axis];
                DVASSERT(controlLeft < controlRight);
                magnetPairs.emplace_back(parentLeft, controlLeft, parentBox, directions[axis] != 1);
                magnetPairs.emplace_back(parentLeft + (parentRight - parentLeft) / 2.0f, controlLeft + (controlRight - controlLeft) / 2.0f, parentBox, 0.5f);
                magnetPairs.emplace_back(parentLeft + parentRight, controlRight, parentBox, directions[axis] != -1);
                const float32 border = 20.0f;
                if (parentGD->GetUnrotatedRect().GetSize()[axis] > 100)
                {
                    magnetPairs.emplace_back(parentLeft + border, controlLeft, parentBox, directions[axis] != 1);
                    magnetPairs.emplace_back(parentRight - border, controlRight, parentBox, directions[axis] != -1);
                }

                for (UIControl* neighbour : control->GetParent()->GetChildren())
                {
                    if (std::find_if(selectedControlNodes.begin(), selectedControlNodes.end(), [neighbour](ControlNode* node)
                                     {
                        return neighbour == node->GetControl();
                                     }) == selectedControlNodes.end())
                    {
                        Rect neighbourBox = neighbour->GetLocalGeometricData().GetAABBox();

                        float32 neighbourLeft = neighbourBox.GetPosition()[axis];
                        float32 neighbourRight = neighbourLeft + neighbourBox.GetSize()[axis];
                        DVASSERT(neighbourLeft < neighbourRight);

                        magnetPairs.emplace_back(neighbourLeft, controlLeft, neighbourBox, directions[axis] != 1);
                        magnetPairs.emplace_back(neighbourLeft + (neighbourRight - neighbourLeft) / 2.0f, controlLeft + (controlRight - controlLeft) / 2.0f, neighbourBox, 0.5f);
                        magnetPairs.emplace_back(neighbourRight, controlRight, neighbourBox, directions[axis] != -1);
                    }
                }
                DVASSERT(!magnetPairs.empty());
                magnetPairs.sort([](const MagnetLine& left, const MagnetLine& right)
                                 {return left.difference < right.difference;
                                 });
                MagnetLine nearestLine = magnetPairs.front();
                float32 areaNearLineLeft = nearestLine.linePos - range[axis] * nearestLine.multiplyer;
                float32 areaNearLineRight = nearestLine.linePos + range[axis] * nearestLine.multiplyer;
                float32 cursorPos = nearestLine.controlSharePos + delta[axis] * nearestLine.multiplyer;
                if (cursorPos >= areaNearLineLeft && cursorPos <= areaNearLineRight)
                {
                    extraDelta[axis] = (cursorPos - nearestLine.linePos) / nearestLine.multiplyer;
                    delta[axis] = (nearestLine.linePos - nearestLine.controlSharePos) / nearestLine.multiplyer;
                }
                for (const MagnetLine& line : magnetPairs)
                {
                    if (line.linePos == line.controlSharePos)
                    {
                        Vector2 position = parentGD->position - RotateVectorInv(parentGD->pivotPoint * parentGD->scale, *parentGD);

                        const Vector2::eAxis axis2 = axis == Vector2::AXIS_X ? Vector2::AXIS_Y : Vector2::AXIS_X;

                        float32 controlTop = box.GetPosition()[axis2];
                        float32 controlBottom = controlTop + box.GetSize()[axis2];

                        float32 targetTop = line.targetBox.GetPosition()[axis2];
                        float32 targetBottom = targetTop + line.targetBox.GetSize()[axis2];

                        Vector2 linePos;
                        linePos[axis] = line.linePos;
                        linePos[axis2] = Min(controlTop, targetTop);
                        Vector2 lineSize;
                        lineSize[axis] = 0.0f;
                        lineSize[axis2] = Max(controlBottom, targetBottom) - linePos[axis2];
                        linePos = RotateVectorInv(linePos, parentGeometricData);
                        Rect lineRect(linePos + position, lineSize);
                        magnets.emplace_back(lineRect, parentGeometricData, axis2);
                    }
                }
            }
        }
    }
    systemManager->MagnetLinesChanged.Emit(magnets);
    */
    return delta;
}

void TransformSystem::MovePivot(Vector2 delta)
{
    Vector<std::tuple<ControlNode*, AbstractProperty*, VariantType>> propertiesToChange;
    Vector2 pivot = AdjustPivot(delta);
    propertiesToChange.emplace_back(activeControlNode, pivotProperty, VariantType(pivot));

    Vector2 scaledDelta(delta / parentGeometricData.scale);
    Vector2 rotatedDeltaPosition(RotateVector(scaledDelta, parentGeometricData));
    Vector2 originalPos(positionProperty->GetValue().AsVector2());
    Vector2 finalPosition(originalPos + rotatedDeltaPosition);
    propertiesToChange.emplace_back(activeControlNode, positionProperty, VariantType(finalPosition));

    systemManager->PropertiesChanged.Emit(std::move(propertiesToChange), std::move(currentHash));
}

void TransformSystem::EmitMagnetLinesForPivot(Vector2& target)
{
    const Rect ur(controlGeometricData.GetUnrotatedRect());
    Vector2 offset = ur.GetSize() * target;
    Vector2 position = controlGeometricData.position - RotateVectorInv(controlGeometricData.pivotPoint * controlGeometricData.scale, controlGeometricData);

    Vector2 horizontalLinePos(0.0f, offset.y);
    horizontalLinePos = RotateVectorInv(horizontalLinePos, controlGeometricData);
    horizontalLinePos += position;

    Vector2 verticalLinePos(offset.x, 0.0f);
    verticalLinePos = RotateVectorInv(verticalLinePos, controlGeometricData);
    verticalLinePos += position;

    Rect horizontalRect(horizontalLinePos, Vector2(ur.GetSize().x, 0.0f));
    Rect verticalRect(verticalLinePos, Vector2(0.0f, ur.GetSize().y));
    Vector<MagnetLineInfo> magnets;
    magnets.emplace_back(horizontalRect, &controlGeometricData, Vector2::AXIS_X);
    magnets.emplace_back(verticalRect, &controlGeometricData, Vector2::AXIS_Y);
    systemManager->MagnetLinesChanged.Emit(magnets);
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
    if (!QApplication::keyboardModifiers().testFlag(Qt::ShiftModifier))
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
            EmitMagnetLinesForPivot(target);
            extraDelta = finalPivot - target;
            delta = RotateVectorInv((target - origPivot) * controlSize, controlGeometricData);

            finalPivot = target;
        }
    }

    if (!found)
    {
        systemManager->MagnetLinesChanged.Emit(Vector<MagnetLineInfo>());
        if (!extraDelta.IsZero())
        {
            deltaPivot += extraDelta;
            extraDelta.SetZero();
            delta = RotateVectorInv(deltaPivot * controlSize, controlGeometricData);
        }
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
    deltaAngle += extraDelta.dx;
    extraDelta.SetZero();    
    float32 originalAngle = angleProperty->GetValue().AsFloat();
    float32 finalAngle = originalAngle + deltaAngle;

    if (!AdjustRotate(deltaAngle, originalAngle, finalAngle))
    {
        return;
    }
    Vector<std::tuple<ControlNode*, AbstractProperty*, VariantType>> propertiesToChange;
    propertiesToChange.emplace_back(activeControlNode, angleProperty, VariantType(finalAngle));
    systemManager->PropertiesChanged.Emit(std::move(propertiesToChange), std::move(currentHash));
}

bool TransformSystem::AdjustRotate(float32 deltaAngle, float32 originalAngle, float32& finalAngle)
{
    if (QApplication::keyboardModifiers().testFlag(Qt::ShiftModifier))
    {
        static const int step = 15; //fixed angle step
        int32 nearestTargetAngle = static_cast<int32>(finalAngle - static_cast<int32>(finalAngle) % step);
        if ((finalAngle >= 0) ^ (deltaAngle > 0))
        {
            nearestTargetAngle += step * (finalAngle >= 0.0f ? 1 : -1);
        }
        if ((deltaAngle >= 0.0f && nearestTargetAngle <= originalAngle + TRANSFORM_EPSILON) || (deltaAngle < 0.0f && nearestTargetAngle >= originalAngle - TRANSFORM_EPSILON))
        {
            extraDelta.dx = deltaAngle;
            return false;
            //disable rotate backwards if we move cursor forward
        }
        extraDelta.dx = finalAngle - nearestTargetAngle;
        finalAngle = nearestTargetAngle;
    }
    return true;
}

void TransformSystem::CorrectNodesToMove()
{
    auto iter = nodesToMove.begin();
    while (iter != nodesToMove.end())
    {
        bool toRemove = false;
        PackageBaseNode* parent = std::get<NODE_INDEX>(*iter)->GetParent();
        if (nullptr == parent || nullptr == parent->GetControl())
        {
            toRemove = true;
        }
        else
        {
            auto iter2 = nodesToMove.begin();
            while (iter2 != nodesToMove.end() && !toRemove)
            {
                PackageBaseNode* node1 = std::get<NODE_INDEX>(*iter);
                PackageBaseNode* node2 = std::get<NODE_INDEX>(*iter2);
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
        ControlNode* node = std::get<NODE_INDEX>(tuple);
        UIControl* control = node->GetControl();
        std::get<PROPERTY_INDEX>(tuple) = node->GetRootProperty()->FindPropertyByName("Position");
        UIControl* parent = control->GetParent();
        DVASSERT(nullptr != parent);
        std::get<GD_INDEX>(tuple) = &parent->GetGeometricData();
    }
}

void CollectNeighbours(Vector<UIControl*>& neighbours, const SelectedControls& selectedControlNodes, UIControl* controlParent)
{
    DVASSERT(nullptr != controlParent);
    Vector<UIControl*> ignoredNeighbours;
    for (ControlNode* node : selectedControlNodes)
    {
        if (node->GetControl()->GetParent() == controlParent)
        {
            ignoredNeighbours.push_back(node->GetControl());
        }
    }

    const List<UIControl*>& children = controlParent->GetChildren();
    std::copy_if(children.begin(), children.end(), std::back_inserter(neighbours), [&ignoredNeighbours](UIControl* left) -> bool
                                                                                   {
                     return std::find(ignoredNeighbours.begin(), ignoredNeighbours.end(), left) == ignoredNeighbours.end();
                                                                                   });
}

void TransformSystem::UpdateNeighboursToMove()
{
    neighbours.clear();
    if (nullptr != activeControlNode)
    {
        UIControl* parent = activeControlNode->GetControl()->GetParent();
        if (nullptr != parent)
        {
            CollectNeighbours(neighbours, selectedControlNodes, parent);
        }
    }
}
