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

#include "EditorSystems/EditorTransformSystem.h"
#include "EditorSystems/EditorSystemsManager.h"
#include "EditorSystems/KeyboardProxy.h"
#include "UI/UIEvent.h"
#include "UI/UIControl.h"
#include "Model/PackageHierarchy/ControlNode.h"
#include "Model/ControlProperties/RootProperty.h"
#include <chrono>

using namespace DAVA;
using namespace std::chrono;

REGISTER_PREFERENCES_ON_START(EditorTransformSystem,
                              PREF_ARG("magnetRange", DAVA::Vector2(7.0f, 7.0f)),
                              PREF_ARG("moveStepByKeyboard", static_cast<DAVA::float32>(10.0f)),
                              PREF_ARG("expandedMoveStepByKeyboard", static_cast<DAVA::float32>(1.0f)),
                              PREF_ARG("borderInParentToMagnet", DAVA::Vector2(20.0f, 20.0f)),
                              PREF_ARG("indentOfControlToManget", DAVA::Vector2(5.0f, 5.0f)),
                              PREF_ARG("shareOfSizeToMagnetPivot", DAVA::Vector2(0.25f, 0.25f)),
                              PREF_ARG("angleSegment", static_cast<DAVA::float32>(15.0f))
                              )

const EditorTransformSystem::CornersDirections EditorTransformSystem::cornersDirections =
{ {
{ { NEGATIVE_DIRECTION, NEGATIVE_DIRECTION } }, // TOP_LEFT_AREA
{ { NO_DIRECTION, NEGATIVE_DIRECTION } }, // TOP_CENTER_AREA
{ { POSITIVE_DIRECTION, NEGATIVE_DIRECTION } }, //TOP_RIGHT_AREA
{ { NEGATIVE_DIRECTION, NO_DIRECTION } }, //CENTER_LEFT_AREA
{ { POSITIVE_DIRECTION, NO_DIRECTION } }, //CENTER_RIGHT_AREA
{ { NEGATIVE_DIRECTION, POSITIVE_DIRECTION } }, //BOTTOM_LEFT_AREA
{ { NO_DIRECTION, POSITIVE_DIRECTION } }, //BOTTOM_CENTER_AREA
{ { POSITIVE_DIRECTION, POSITIVE_DIRECTION } } //BOTTOM_RIGHT_AREA
} };

struct EditorTransformSystem::MoveInfo
{
    MoveInfo(ControlNode* node_, AbstractProperty* positionProperty_, const UIGeometricData* parentGD_)
        : node(node_)
        , positionProperty(positionProperty_)
        , parentGD(parentGD_)
    {
    }
    ControlNode* node = nullptr;
    AbstractProperty* positionProperty = nullptr;
    const UIGeometricData* parentGD = nullptr;
};

struct EditorTransformSystem::MagnetLine
{
    //controlBox and targetBox in parent coordinates. controlSharePos ans targetSharePos is a share of corresponding size
    MagnetLine(float32 controlSharePos_, const Rect& controlBox_, float32 targetSharePos_, const Rect& targetBox_, Vector2::eAxis axis_)
        : controlSharePos(controlSharePos_)
        , controlBox(controlBox_)
        , targetSharePos(targetSharePos_)
        , targetBox(targetBox_)
        , axis(axis_)
    {
        controlPosition = controlBox.GetPosition()[axis] + controlBox.GetSize()[axis] * controlSharePos;
        targetPosition = targetBox.GetPosition()[axis] + targetBox.GetSize()[axis] * targetSharePos;
        interval = controlPosition - targetPosition;
    }

    float32 controlSharePos;
    float32 controlPosition;
    Rect controlBox;

    float32 targetSharePos;
    float32 targetPosition;
    Rect targetBox;

    float32 interval = std::numeric_limits<float32>::max();
    Vector2::eAxis axis;
};

namespace EditorTransformSystem_local
{
const float32 TRANSFORM_EPSILON = 0.0005f;
}

EditorTransformSystem::EditorTransformSystem(EditorSystemsManager* parent)
    : BaseEditorSystem(parent)
{
    systemsManager->ActiveAreaChanged.Connect(this, &EditorTransformSystem::OnActiveAreaChanged);
    systemsManager->SelectionChanged.Connect(this, &EditorTransformSystem::OnSelectionChanged);
}

void EditorTransformSystem::OnActiveAreaChanged(const HUDAreaInfo& areaInfo)
{
    activeArea = areaInfo.area;
    activeControlNode = areaInfo.owner;
    if (nullptr != activeControlNode)
    {
        UIControl* control = activeControlNode->GetControl();
        UIControl* parent = control->GetParent();
        parentGeometricData = parent->GetGeometricData();
        controlGeometricData = control->GetGeometricData();

        DVASSERT(parentGeometricData.scale.x > 0.0f && parentGeometricData.scale.y > 0.0f);
        DVASSERT(controlGeometricData.scale.x > 0.0f && controlGeometricData.scale.y > 0.0f);
        DVASSERT(controlGeometricData.size.x >= 0.0f && controlGeometricData.size.y >= 0.0f);

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

bool EditorTransformSystem::OnInput(UIEvent* currentInput)
{
    switch (currentInput->phase)
    {
    case UIEvent::Phase::KEY_DOWN:
        return ProcessKey(currentInput->key);

    case UIEvent::Phase::BEGAN:
    {
        microseconds us = duration_cast<microseconds>(system_clock::now().time_since_epoch());
        currentHash = static_cast<size_t>(us.count());
        extraDelta.SetZero();
        prevPos = currentInput->point;
        if (activeControlNode != nullptr && activeControlNode->GetParent()->GetControl() != nullptr)
        {
            systemsManager->TransformStateChanged.Emit(true);
        }
        return false;
    }
    case UIEvent::Phase::DRAG:
    {
        UIEvent::MouseButton button = currentInput->mouseButton;
        if (button == UIEvent::MouseButton::LEFT && currentInput->point != prevPos)
        {
            if (ProcessDrag(currentInput->point))
            {
                prevPos = currentInput->point;
            }
        }
        return false;
    }
    case UIEvent::Phase::ENDED:
        if (activeArea == HUDAreaInfo::ROTATE_AREA)
        {
            ClampAngle();
        }
        systemsManager->MagnetLinesChanged.Emit(Vector<MagnetLineInfo>());
        if (activeControlNode != nullptr && activeControlNode->GetParent()->GetControl() != nullptr)
        {
            systemsManager->TransformStateChanged.Emit(false);
        }
        return false;
    default:
        return false;
    }
}

void EditorTransformSystem::OnSelectionChanged(const SelectedNodes& selected, const SelectedNodes& deselected)
{
    SelectionContainer::MergeSelectionToContainer(selected, deselected, selectedControlNodes);
    nodesToMoveInfos.clear();
    for (ControlNode* selectedControl : selectedControlNodes)
    {
        nodesToMoveInfos.emplace_back(new MoveInfo(selectedControl, nullptr, nullptr));
    }
    CorrectNodesToMove();
    UpdateNeighboursToMove();
}

bool EditorTransformSystem::ProcessKey(Key key)
{
    if (!selectedControlNodes.empty())
    {
        float32 step = expandedMoveStepByKeyboard;
        if (!IsKeyPressed(KeyboardProxy::KEY_SHIFT))
        {
            step = moveStepByKeyboard;
        }
        Vector2 deltaPos;
        switch (key)
        {
        case Key::LEFT:
            deltaPos.dx -= step;
            break;
        case Key::UP:
            deltaPos.dy -= step;
            break;
        case Key::RIGHT:
            deltaPos.dx += step;
            break;
        case Key::DOWN:
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

bool EditorTransformSystem::ProcessDrag(Vector2 pos)
{
    if (activeArea == HUDAreaInfo::NO_AREA)
    {
        return false;
    }
    Vector2 delta(pos - prevPos);
    switch (activeArea)
    {
    case HUDAreaInfo::FRAME_AREA:
        MoveAllSelectedControls(delta, !IsKeyPressed(KeyboardProxy::KEY_SHIFT));
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
        bool withPivot = IsKeyPressed(KeyboardProxy::KEY_ALT);
        bool rateably = IsKeyPressed(KeyboardProxy::KEY_CTRL);
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
        return Rotate(pos);
    }
    default:
        return false;
    }
}

void EditorTransformSystem::MoveAllSelectedControls(Vector2 delta, bool canAdjust)
{
    Vector<ChangePropertyAction> propertiesToChange;
    Vector<MagnetLineInfo> magnets;
    //at furst we need to magnet control under cursor or unmagnet it
    ControlNode* activeNode = activeControlNode;
    if (canAdjust)
    {
        //find hovered node alias in nodesToMoveInfos
        DAVA::Set<PackageBaseNode*> activeControlNodeHierarchy;
        PackageBaseNode* parent = activeControlNode;
        while (parent != nullptr && parent->GetControl() != nullptr)
        {
            activeControlNodeHierarchy.insert(parent);
            parent = parent->GetParent();
        }
        auto iter = std::find_if(nodesToMoveInfos.begin(), nodesToMoveInfos.end(), [&activeControlNodeHierarchy](const std::unique_ptr<MoveInfo>& nodeInfoPtr)
                                 {
                                     PackageBaseNode* target = nodeInfoPtr->node;
                                     return activeControlNodeHierarchy.find(target) != activeControlNodeHierarchy.end();
                                 });
        DVASSERT(iter != nodesToMoveInfos.end());
        const auto& nodeToMoveInfo = iter->get();
        const auto& gd = nodeToMoveInfo->parentGD;
        const auto& node = nodeToMoveInfo->node;
        activeNode = node;
        const auto& positionProperty = nodeToMoveInfo->positionProperty;

        Vector2 scaledDelta = delta / gd->scale;
        Vector2 deltaPosition(DAVA::Rotate(scaledDelta, -gd->angle));
        Vector2 adjustedPosition(deltaPosition);
        adjustedPosition += extraDelta;
        extraDelta.SetZero();
        adjustedPosition = AdjustMoveToNearestBorder(adjustedPosition, magnets, gd, node->GetControl());
        AbstractProperty* property = positionProperty;
        Vector2 originalPosition = property->GetValue().AsVector2();
        Vector2 finalPosition(originalPosition + adjustedPosition);
        propertiesToChange.emplace_back(node, property, VariantType(finalPosition));
        delta = DAVA::Rotate(adjustedPosition, gd->angle);
        delta *= gd->scale;
    }
    for (auto& nodeToMove : nodesToMoveInfos)
    {
        ControlNode* node = nodeToMove->node;
        if (canAdjust && node == activeNode)
        {
            continue; //we already move it in this function
        }
        const UIGeometricData* gd = nodeToMove->parentGD;
        Vector2 scaledDelta = delta / gd->scale;
        Vector2 deltaPosition(DAVA::Rotate(scaledDelta, -gd->angle));
        AbstractProperty* property = nodeToMove->positionProperty;
        Vector2 originalPosition = property->GetValue().AsVector2();
        Vector2 finalPosition(originalPosition + deltaPosition);
        propertiesToChange.emplace_back(node, property, VariantType(finalPosition));
    }
    if (!propertiesToChange.empty())
    {
        systemsManager->PropertiesChanged.Emit(propertiesToChange, currentHash);
    }
    systemsManager->MagnetLinesChanged.Emit(magnets);
}

Vector<EditorTransformSystem::MagnetLine> EditorTransformSystem::CreateMagnetPairs(const Rect& box, const UIGeometricData* parentGD, const Vector<UIControl*>& neighbours, Vector2::eAxis axis)
{
    DVASSERT(nullptr != parentGD);
    Vector<MagnetLine> magnets;
    Rect parentBox(Vector2(), parentGD->size);
    if (parentBox.GetSize()[axis] <= 0.0f)
    {
        return magnets;
    }
    const size_t magnetsCountForParent = 7;
    const size_t magnetsCountForOneNeighbour = 9;
    magnets.reserve(magnetsCountForParent + magnetsCountForOneNeighbour * neighbours.size()); //TODO: replace digits with calculated values

    magnets.emplace_back(0.0f, box, 0.0f, parentBox, axis);
    magnets.emplace_back(0.0f, box, 0.5f, parentBox, axis);
    magnets.emplace_back(0.5f, box, 0.5f, parentBox, axis);
    magnets.emplace_back(1.0f, box, 0.5f, parentBox, axis);
    magnets.emplace_back(1.0f, box, 1.0f, parentBox, axis);

    const float32 border = borderInParentToMagnet[axis];
    float32 requiredSizeToMagnetToBorders = 5 * border;
    if (parentGD->GetUnrotatedRect().GetSize()[axis] > requiredSizeToMagnetToBorders)
    {
        const float32 borderShare = border / parentBox.GetSize()[axis];
        magnets.emplace_back(0.0f, box, borderShare, parentBox, axis);
        magnets.emplace_back(1.0f, box, 1.0f - borderShare, parentBox, axis);
    }

    for (UIControl* neighbour : neighbours)
    {
        DVASSERT(nullptr != neighbour);
        Rect neighbourBox = neighbour->GetLocalGeometricData().GetAABBox();

        magnets.emplace_back(0.0f, box, 0.0f, neighbourBox, axis);
        magnets.emplace_back(0.0f, box, 0.5f, neighbourBox, axis);
        magnets.emplace_back(0.5f, box, 0.5f, neighbourBox, axis);
        magnets.emplace_back(1.0f, box, 0.5f, neighbourBox, axis);
        magnets.emplace_back(1.0f, box, 1.0f, neighbourBox, axis);
        magnets.emplace_back(0.0f, box, 1.0f, neighbourBox, axis);
        magnets.emplace_back(1.0f, box, 0.0f, neighbourBox, axis);

        const float32 neighbourSpacing = indentOfControlToManget[axis];
        const float32 neighbourSpacingShare = neighbourSpacing / neighbourBox.GetSize()[axis];
        magnets.emplace_back(1.0f, box, -neighbourSpacingShare, neighbourBox, axis);
        magnets.emplace_back(0.0f, box, 1.0f + neighbourSpacingShare, neighbourBox, axis);
    }

    return magnets;
}

void EditorTransformSystem::ExtractMatchedLines(Vector<MagnetLineInfo>& magnets, const Vector<MagnetLine>& magnetLines, const UIControl* control, Vector2::eAxis axis)
{
    UIControl* parent = control->GetParent();
    const UIGeometricData* parentGD = &parent->GetGeometricData();
    for (const MagnetLine& line : magnetLines)
    {
        if (fabs(line.interval) < EditorTransformSystem_local::TRANSFORM_EPSILON)
        {
            const Vector2::eAxis oppositeAxis = axis == Vector2::AXIS_X ? Vector2::AXIS_Y : Vector2::AXIS_X;

            float32 controlTop = line.controlBox.GetPosition()[oppositeAxis];
            float32 controlBottom = controlTop + line.controlBox.GetSize()[oppositeAxis];

            float32 targetTop = line.targetBox.GetPosition()[oppositeAxis];
            float32 targetBottom = targetTop + line.targetBox.GetSize()[oppositeAxis];

            Vector2 linePos;
            linePos[axis] = line.targetPosition;
            linePos[oppositeAxis] = Min(controlTop, targetTop);
            Vector2 lineSize;
            lineSize[axis] = 1.0f;
            lineSize[oppositeAxis] = Max(controlBottom, targetBottom) - linePos[oppositeAxis];

            Rect lineRect(linePos, lineSize);
            magnets.emplace_back(line.targetBox, lineRect, parentGD, oppositeAxis);
        }
    }
}

Vector2 EditorTransformSystem::AdjustMoveToNearestBorder(Vector2 delta, Vector<MagnetLineInfo>& magnets, const UIGeometricData* parentGD, const UIControl* control)
{
    const UIGeometricData controlGD = control->GetLocalGeometricData();
    Rect box = controlGD.GetAABBox();
    box.SetPosition(box.GetPosition() + delta);

    for (int32 axisInt = Vector2::AXIS_X; axisInt < Vector2::AXIS_COUNT; ++axisInt)
    {
        Vector2::eAxis axis = static_cast<Vector2::eAxis>(axisInt);
        Vector<MagnetLine> magnetLines = CreateMagnetPairs(box, parentGD, neighbours, axis);

        //get nearest magnet line
        std::function<bool(const MagnetLine&, const MagnetLine&)> predicate = [](const MagnetLine& left, const MagnetLine& right) -> bool {
            return fabs(left.interval) < fabs(right.interval);
        };
        if (magnetLines.empty())
        {
            continue;
        }

        MagnetLine nearestLine = *std::min_element(magnetLines.begin(), magnetLines.end(), predicate);
        float32 areaNearLineRight = nearestLine.targetPosition + magnetRange[axis];
        float32 areaNearLineLeft = nearestLine.targetPosition - magnetRange[axis];
        if (nearestLine.controlPosition >= areaNearLineLeft && nearestLine.controlPosition <= areaNearLineRight)
        {
            Vector2 oldDelta(delta);
            delta[axis] -= nearestLine.interval;
            extraDelta[axis] = oldDelta[axis] - delta[axis];
        }
        //adjust all lines to transformed state to get matched lines
        for (MagnetLine& line : magnetLines)
        {
            line.interval -= extraDelta[line.axis];
        }
        ExtractMatchedLines(magnets, magnetLines, control, axis);
    }
    return delta;
}

void EditorTransformSystem::ResizeControl(Vector2 delta, bool withPivot, bool rateably)
{
    UIControl* control = activeControlNode->GetControl();

    DVASSERT(activeArea != HUDAreaInfo::NO_AREA);

    const Directions& directions = cornersDirections.at(activeArea - HUDAreaInfo::TOP_LEFT_AREA);

    Vector2 pivot(control->GetPivot());

    Vector2 deltaMappedToControl(delta / controlGeometricData.scale);
    deltaMappedToControl = DAVA::Rotate(deltaMappedToControl, -controlGeometricData.angle);

    Vector2 deltaSize(deltaMappedToControl);
    Vector2 deltaPosition(deltaMappedToControl);
    for (int32 axisInt = Vector2::AXIS_X; axisInt < Vector2::AXIS_COUNT; ++axisInt)
    {
        Vector2::eAxis axis = static_cast<Vector2::eAxis>(axisInt);
        const int direction = directions[axis];

        deltaSize[axis] *= direction;
        deltaPosition[axis] *= direction == NEGATIVE_DIRECTION ? 1.0f - pivot[axis] : pivot[axis];

        if (direction == NO_DIRECTION)
        {
            deltaPosition[axis] = 0.0f;
        }

        //modify if pivot modificator selected
        if (withPivot)
        {
            deltaPosition[axis] = 0.0f;

            auto pivotDelta = direction == NEGATIVE_DIRECTION ? pivot[axis] : 1.0f - pivot[axis];
            if (pivotDelta != 0.0f)
            {
                deltaSize[axis] /= pivotDelta;
            }
        }
    }
    //modify rateably
    const Vector2& size = control->GetSize();
    if (rateably && size.x > 0.0f && size.y > 0.0f)
    {
        //calculate proportion of control
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
                if (directions[axis] == NO_DIRECTION)
                {
                    deltaPosition[axis] *= (0.5f - pivot[axis]) * -1.0f; //rainbow unicorn was here and add -1 to the right.
                }
                else
                {
                    deltaPosition[axis] *= (directions[axis] == NEGATIVE_DIRECTION ? 1.0f - pivot[axis] : pivot[axis]) * directions[axis];
                }
            }
        }
    }

    Vector2 transformPoint = withPivot ? pivot : Vector2();
    for (int32 axisInt = Vector2::AXIS_X; axisInt < Vector2::AXIS_COUNT; ++axisInt)
    {
        Vector2::eAxis axis = static_cast<Vector2::eAxis>(axisInt);
        if (directions[axis] == NEGATIVE_DIRECTION)
        {
            transformPoint[axis] = 1.0f - transformPoint[axis];
        }
    }
    Vector2 origDeltaSize(deltaSize);
    deltaSize += extraDelta; //transform to virtual coordinates
    extraDelta.SetZero();

    Vector2 adjustedSize = AdjustResizeToBorderAndToMinimum(deltaSize, transformPoint, directions);
    //adjust delta position to new delta size
    for (int32 axisInt = Vector2::AXIS_X; axisInt < Vector2::AXIS_COUNT; ++axisInt)
    {
        Vector2::eAxis axis = static_cast<Vector2::eAxis>(axisInt);
        if (origDeltaSize[axis] != 0.0f)
        {
            deltaPosition[axis] *= origDeltaSize[axis] != 0.0f ? adjustedSize[axis] / origDeltaSize[axis] : 0.0f;
        }
    }

    deltaPosition *= control->GetScale();
    deltaPosition = DAVA::Rotate(deltaPosition, control->GetAngle());

    Vector<ChangePropertyAction> propertiesToChange;

    if (activeControlNode->GetParent() != nullptr && activeControlNode->GetParent()->GetControl() != nullptr)
    {
        Vector2 originalPosition = positionProperty->GetValue().AsVector2();
        propertiesToChange.emplace_back(activeControlNode, positionProperty, VariantType(originalPosition + deltaPosition));
    }
    Vector2 originalSize = sizeProperty->GetValue().AsVector2();
    Vector2 finalSize(originalSize + adjustedSize);
    propertiesToChange.emplace_back(activeControlNode, sizeProperty, VariantType(finalSize));

    systemsManager->PropertiesChanged.Emit(propertiesToChange, currentHash);
}

Vector2 EditorTransformSystem::AdjustResizeToMinimumSize(Vector2 deltaSize)
{
    const Vector2 scaledMinimum(systemsManager->minimumSize / controlGeometricData.scale);
    Vector2 origSize = sizeProperty->GetValue().AsVector2();

    Vector2 finalSize(origSize + deltaSize);
    Vector<MagnetLineInfo> magnets;

    for (int32 axisInt = Vector2::AXIS_X; axisInt < Vector2::AXIS_COUNT; ++axisInt)
    {
        Vector2::eAxis axis = static_cast<Vector2::eAxis>(axisInt);
        if (deltaSize[axis] > 0.0f)
        {
            continue;
        }
        if (origSize[axis] > scaledMinimum[axis])
        {
            if (finalSize[axis] > scaledMinimum[axis])
            {
                continue;
            }
            extraDelta[axis] += finalSize[axis] - scaledMinimum[axis];
            deltaSize[axis] = scaledMinimum[axis] - origSize[axis];
        }
        else
        {
            extraDelta[axis] += deltaSize[axis];
            deltaSize[axis] = 0.0f;
        }
    }

    return deltaSize;
}

Vector2 EditorTransformSystem::AdjustResizeToBorderAndToMinimum(Vector2 deltaSize, Vector2 transformPoint, Directions directions)
{
    Vector<MagnetLineInfo> magnets;

    bool canAdjustResize = !IsKeyPressed(KeyboardProxy::KEY_SHIFT) && activeControlNode->GetControl()->GetAngle() == 0.0f && activeControlNode->GetParent()->GetControl() != nullptr;
    Vector2 adjustedDeltaToBorder(deltaSize);
    if (canAdjustResize)
    {
        adjustedDeltaToBorder = AdjustResizeToBorder(deltaSize, transformPoint, directions, magnets);
    }
    Vector2 adjustedSize = AdjustResizeToMinimumSize(adjustedDeltaToBorder);
    if (adjustedDeltaToBorder != adjustedSize)
    {
        magnets.clear();
    }
    systemsManager->MagnetLinesChanged.Emit(magnets);

    return adjustedSize;
}

DAVA::Vector2 EditorTransformSystem::AdjustResizeToBorder(Vector2 deltaSize, Vector2 transformPoint, Directions directions, Vector<MagnetLineInfo>& magnets)
{
    UIControl* control = activeControlNode->GetControl();

    UIGeometricData controlGD = control->GetLocalGeometricData();
    //calculate control box in parent
    controlGD.size += deltaSize;
    Rect box = controlGD.GetAABBox();
    Vector2 sizeAffect = DAVA::Rotate(deltaSize * transformPoint * controlGD.scale, controlGD.angle);
    box.SetPosition(box.GetPosition() - sizeAffect);

    Vector2 transformPosition = box.GetPosition() + box.GetSize() * transformPoint;

    for (int32 axisInt = Vector2::AXIS_X; axisInt < Vector2::AXIS_COUNT; ++axisInt)
    {
        Vector2::eAxis axis = static_cast<Vector2::eAxis>(axisInt);
        if (directions[axis] != NO_DIRECTION)
        {
            Vector<MagnetLine> magnetLines = CreateMagnetPairs(box, &parentGeometricData, neighbours, axis);
            if (magnetLines.empty())
            {
                continue;
            }
            std::function<bool(const MagnetLine&)> removePredicate = [directions, transformPosition](const MagnetLine& line) -> bool {
                bool needRemove = true;
                if (directions[line.axis] == POSITIVE_DIRECTION)
                {
                    needRemove = line.targetPosition <= transformPosition[line.axis] || line.controlPosition <= transformPosition[line.axis];
                }
                else
                {
                    needRemove = line.targetPosition >= transformPosition[line.axis] || line.controlPosition >= transformPosition[line.axis];
                }
                return needRemove;
            };

            magnetLines.erase(std::remove_if(magnetLines.begin(), magnetLines.end(), removePredicate));
            if (magnetLines.empty())
            {
                continue;
            }

            std::function<bool(const MagnetLine&, const MagnetLine&)> predicate = [transformPoint, directions](const MagnetLine& left, const MagnetLine& right) -> bool {
                float32 shareLeft = left.controlSharePos - transformPoint[left.axis];
                float32 shareRight = right.controlSharePos - transformPoint[right.axis];
                float32 distanceLeft = shareLeft == 0.0f ? std::numeric_limits<float32>::max() : left.interval / shareLeft;
                float32 distanceRight = shareRight == 0.0f ? std::numeric_limits<float32>::max() : right.interval / shareRight;
                return fabs(distanceLeft) < fabs(distanceRight);
            };

            MagnetLine nearestLine = *std::min_element(magnetLines.begin(), magnetLines.end(), predicate);
            float32 share = fabs(nearestLine.controlSharePos - transformPoint[nearestLine.axis]);
            float32 rangeForPosition = magnetRange[axis] * share;
            float32 areaNearLineRight = nearestLine.targetPosition + rangeForPosition;
            float32 areaNearLineLeft = nearestLine.targetPosition - rangeForPosition;

            Vector2 oldDeltaSize(deltaSize);

            if (nearestLine.controlPosition >= areaNearLineLeft && nearestLine.controlPosition <= areaNearLineRight)
            {
                float32 interval = nearestLine.interval * directions[axis] * -1;
                DVASSERT(share > 0.0f);
                interval /= share;
                float32 scaledDistance = interval / controlGD.scale[axis];
                deltaSize[axis] += scaledDistance;
                extraDelta[axis] += oldDeltaSize[axis] - deltaSize[axis];
            }

            for (MagnetLine& line : magnetLines)
            {
                float32 lineShare = fabs(line.controlSharePos - transformPoint[line.axis]);
                line.interval -= extraDelta[line.axis] * controlGD.scale[line.axis] * lineShare * directions[line.axis];
            }
            ExtractMatchedLines(magnets, magnetLines, control, axis);
        }
    }
    return deltaSize;
}

void EditorTransformSystem::MovePivot(Vector2 delta)
{
    Vector<ChangePropertyAction> propertiesToChange;
    Vector2 pivot = AdjustPivotToNearestArea(delta);
    propertiesToChange.emplace_back(activeControlNode, pivotProperty, VariantType(pivot));

    Vector2 scaledDelta(delta / parentGeometricData.scale);
    Vector2 rotatedDeltaPosition(DAVA::Rotate(scaledDelta, -parentGeometricData.angle));
    Vector2 originalPos(positionProperty->GetValue().AsVector2());
    Vector2 finalPosition(originalPos + rotatedDeltaPosition);
    propertiesToChange.emplace_back(activeControlNode, positionProperty, VariantType(finalPosition));

    systemsManager->PropertiesChanged.Emit(propertiesToChange, currentHash);
}

namespace
{
void CreateMagnetLinesForPivot(Vector<MagnetLineInfo>& magnetLines, Vector2 target, const UIGeometricData& controlGeometricData)
{
    const Vector2 targetSize(controlGeometricData.size);
    Vector2 offset = targetSize * target;
    Vector2 horizontalLinePos(0.0f, offset.y);

    Vector2 verticalLinePos(offset.x, 0.0f);

    Rect horizontalRect(horizontalLinePos, Vector2(targetSize.x, 1.0f));
    Rect verticalRect(verticalLinePos, Vector2(1.0f, targetSize.y));
    Rect targetBox(Vector2(0.0f, 0.0f), targetSize);
    magnetLines.emplace_back(targetBox, horizontalRect, &controlGeometricData, Vector2::AXIS_X);
    magnetLines.emplace_back(targetBox, verticalRect, &controlGeometricData, Vector2::AXIS_Y);
}
}; //unnamed namespace

DAVA::Vector2 EditorTransformSystem::AdjustPivotToNearestArea(Vector2& delta)
{
    Vector<MagnetLineInfo> magnetLines;

    const Rect ur(controlGeometricData.GetUnrotatedRect());
    const Vector2 controlSize(ur.GetSize());
    DVASSERT(controlSize.x > 0.0f && controlSize.y > 0.0f);
    const Vector2 rotatedDeltaPivot(DAVA::Rotate(delta, -controlGeometricData.angle));
    Vector2 deltaPivot(rotatedDeltaPivot / controlSize);

    const Vector2 range(magnetRange / controlSize); //range in pivot coordinates

    Vector2 origPivot = pivotProperty->GetValue().AsVector2();
    Vector2 finalPivot(origPivot + deltaPivot + extraDelta);

    bool found = false;
    if (!IsKeyPressed(KeyboardProxy::KEY_SHIFT))
    {
        const float32 maxPivot = 1.0f;

        Vector2 target;
        Vector2 distanceToTarget;
        for (float32 targetX = 0.0f; targetX <= maxPivot; targetX += shareOfSizeToMagnetPivot[Vector2::AXIS_X])
        {
            for (float32 targetY = 0.0f; targetY <= maxPivot; targetY += shareOfSizeToMagnetPivot[Vector2::AXIS_Y])
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
            CreateMagnetLinesForPivot(magnetLines, target, controlGeometricData);
            extraDelta = finalPivot - target;
            delta = DAVA::Rotate((target - origPivot) * controlSize, controlGeometricData.angle);

            finalPivot = target;
        }
    }

    if (!found)
    {
        if (!extraDelta.IsZero())
        {
            deltaPivot += extraDelta;
            extraDelta.SetZero();
            delta = DAVA::Rotate(deltaPivot * controlSize, controlGeometricData.angle);
        }
    }
    systemsManager->MagnetLinesChanged.Emit(magnetLines);
    return finalPivot;
}

bool EditorTransformSystem::Rotate(Vector2 pos)
{
    Vector2 rotatePoint(controlGeometricData.GetUnrotatedRect().GetPosition());
    rotatePoint += controlGeometricData.pivotPoint * controlGeometricData.scale;
    Vector2 l1(prevPos - rotatePoint);
    Vector2 l2(pos - rotatePoint);

    if (l2.Length() < 15)
    {
        return false;
    }

    float32 angleRad = atan2(l1.x * l2.y - l2.x * l1.y, l1.x * l2.x + l1.y * l2.y);
    float32 deltaAngle = RadToDeg(angleRad);
    //after modification deltaAngle is less than mouse delta positions

    deltaAngle += extraDelta.dx;
    extraDelta.SetZero();
    float32 originalAngle = angleProperty->GetValue().AsFloat();

    float32 finalAngle = AdjustRotateToFixedAngle(deltaAngle, originalAngle);
    Vector<ChangePropertyAction> propertiesToChange;
    propertiesToChange.emplace_back(activeControlNode, angleProperty, VariantType(finalAngle));
    systemsManager->PropertiesChanged.Emit(propertiesToChange, currentHash);
    return true;
}

float32 EditorTransformSystem::AdjustRotateToFixedAngle(float32 deltaAngle, float32 originalAngle)
{
    float32 finalAngle = originalAngle + deltaAngle;
    if (!IsKeyPressed(KeyboardProxy::KEY_SHIFT))
    {
        static const int step = angleSegment; //fixed angle step
        int32 nearestTargetAngle = static_cast<int32>(finalAngle - static_cast<int32>(finalAngle) % step);
        if ((finalAngle >= 0) ^ (deltaAngle > 0))
        {
            nearestTargetAngle += step * (finalAngle >= 0.0f ? 1 : -1);
        }
        if ((deltaAngle >= 0.0f && nearestTargetAngle <= originalAngle + EditorTransformSystem_local::TRANSFORM_EPSILON)
            || (deltaAngle < 0.0f && nearestTargetAngle >= originalAngle - EditorTransformSystem_local::TRANSFORM_EPSILON))
        {
            extraDelta.dx = deltaAngle;
            return originalAngle;
            //disable rotate backwards if we move cursor forward
        }
        extraDelta.dx = finalAngle - nearestTargetAngle;
        return nearestTargetAngle;
    }
    return finalAngle;
}

void EditorTransformSystem::CorrectNodesToMove()
{
    nodesToMoveInfos.remove_if([](std::unique_ptr<MoveInfo>& item) {
        const PackageBaseNode* parent = item->node->GetParent();
        return nullptr == parent || nullptr == parent->GetControl();
    });

    auto iter = nodesToMoveInfos.begin();
    while (iter != nodesToMoveInfos.end())
    {
        bool toRemove = false;
        auto iter2 = nodesToMoveInfos.begin();
        while (iter2 != nodesToMoveInfos.end() && !toRemove)
        {
            PackageBaseNode* node = (*iter)->node;
            if (iter != iter2)
            {
                while (nullptr != node->GetParent() && nullptr != node->GetControl() && !toRemove)
                {
                    if (node == (*iter2)->node)
                    {
                        toRemove = true;
                    }
                    node = node->GetParent();
                }
            }
            ++iter2;
        }
        if (toRemove)
        {
            nodesToMoveInfos.erase(iter++);
        }
        else
        {
            ++iter;
        }
    }
    for (auto& moveInfo : nodesToMoveInfos)
    {
        ControlNode* node = moveInfo->node;
        UIControl* control = node->GetControl();
        moveInfo->positionProperty = node->GetRootProperty()->FindPropertyByName("Position");
        UIControl* parent = control->GetParent();
        DVASSERT(nullptr != parent);
        moveInfo->parentGD = &parent->GetGeometricData();
    }
}

void CollectNeighbours(Vector<UIControl*>& neighbours, const SelectedControls& selectedControlNodes, UIControl* controlParent)
{
    DVASSERT(nullptr != controlParent);
    Set<UIControl*> ignoredNeighbours;
    for (ControlNode* node : selectedControlNodes)
    {
        if (node->GetControl()->GetParent() == controlParent)
        {
            ignoredNeighbours.insert(node->GetControl());
        }
    }

    const List<UIControl*>& children = controlParent->GetChildren();
    Set<UIControl*> sortedChildren(children.begin(), children.end());
    std::set_difference(sortedChildren.begin(), sortedChildren.end(), ignoredNeighbours.begin(), ignoredNeighbours.end(), std::back_inserter(neighbours));
}

void EditorTransformSystem::UpdateNeighboursToMove()
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

void EditorTransformSystem::ClampAngle()
{
    float32 angle = angleProperty->GetValue().AsFloat();
    if (fabs(angle) > 360)
    {
        angle += angle > 0.0f ? EditorTransformSystem_local::TRANSFORM_EPSILON : -EditorTransformSystem_local::TRANSFORM_EPSILON;
        angle = static_cast<int32>(angle) % 360;
    }
    Vector<ChangePropertyAction> propertiesToChange;
    propertiesToChange.emplace_back(activeControlNode, angleProperty, VariantType(angle));
    systemsManager->PropertiesChanged.Emit(propertiesToChange, currentHash);
}
