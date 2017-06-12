#include "Input/InputSystem.h"
#include "Input/KeyboardDevice.h"

#include "EditorSystems/EditorTransformSystem.h"
#include "EditorSystems/EditorSystemsManager.h"
#include "EditorSystems/KeyboardProxy.h"

#include "Model/PackageHierarchy/PackageNode.h"
#include "Model/PackageHierarchy/ControlNode.h"
#include "Model/ControlProperties/RootProperty.h"

#include "Modules/DocumentsModule/DocumentData.h"
#include "Modules/PreferencesModule/PreferencesData.h"

#include "QECommands/ChangePropertyValueCommand.h"
#include "QECommands/ResizeCommand.h"
#include "QECommands/ChangePivotCommand.h"

#include <TArc/Core/ContextAccessor.h>

#include <UI/UIEvent.h>
#include <UI/UIControl.h>

using namespace DAVA;

Vector2 EditorTransformSystem::GetMinimumSize()
{
    return Vector2(16.0f, 16.0f);
}

REGISTER_PREFERENCES_ON_START(EditorTransformSystem,
                              PREF_ARG("moveMagnetRange", Vector2(7.0f, 7.0f)),
                              PREF_ARG("resizeMagnetRange", Vector2(7.0f, 7.0f)),
                              PREF_ARG("pivotMagnetRange", Vector2(7.0f, 7.0f)),
                              PREF_ARG("moveStepByKeyboard2", Vector2(10.0f, 10.0f)),
                              PREF_ARG("expandedmoveStepByKeyboard2", Vector2(1.0f, 1.0f)),
                              PREF_ARG("shareOfSizeToMagnetPivot", Vector2(0.25f, 0.25f)),
                              PREF_ARG("angleSegment", static_cast<float32>(15.0f)),
                              PREF_ARG("shiftInverted", false),
                              PREF_ARG("canMagnet", true)
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
    MagnetLine(float32 controlSharePos_, const Rect& controlBox_, float32 targetSharePos, const Rect& targetBox_, Vector2::eAxis axis_)
        : controlSharePos(controlSharePos_)
        , controlBox(controlBox_)
        , targetBox(targetBox_)
        , axis(axis_)
    {
        controlPosition = controlBox.GetPosition()[axis] + controlBox.GetSize()[axis] * controlSharePos;
        targetPosition = targetBox.GetPosition()[axis] + targetBox.GetSize()[axis] * targetSharePos;
        interval = controlPosition - targetPosition;
    }

    MagnetLine(float32 controlSharePos_, const Rect& controlBox_, float32 targetPosition_, Vector2::eAxis axis_)
        : controlSharePos(controlSharePos_)
        , controlBox(controlBox_)
        , targetPosition(targetPosition_)
        , axis(axis_)
    {
        controlPosition = controlBox.GetPosition()[axis] + controlBox.GetSize()[axis] * controlSharePos;
        interval = controlPosition - targetPosition;
    }

    float32 controlSharePos;
    float32 controlPosition;
    Rect controlBox;

    float32 targetPosition;
    Rect targetBox;

    float32 interval = std::numeric_limits<float32>::max();
    Vector2::eAxis axis;
};

namespace EditorTransformSystemDetail
{
const float32 TRANSFORM_EPSILON = 0.0005f;

struct ChangePropertyAction
{
    ChangePropertyAction(ControlNode* node_, AbstractProperty* property_, const Any& value_)
        : node(node_)
        , property(property_)
        , value(value_)
    {
    }
    ControlNode* node = nullptr;
    AbstractProperty* property = nullptr;
    Any value;
};

//when we get request to add a value (x; y) to position it transforms to:
//for angle -45 : 45 returns (x; y)
//for angle 45 : (90 + 45) returns (y; -x)
//for angle (90 + 45) : (180 + 45) returns (-x; -y)
//for angle (180 + 45) : (360 - 45) returns (-y; x);

Vector2 RotateVectorForMove(const Vector2& delta, float32 angle)
{
    static const float32 positiveCos = std::cos(PI_025);
    static const float32 negativeCos = std::cos(PI + PI_025);

    float32 cos = std::cos(angle);
    Vector2 deltaPosition;
    if (cos > positiveCos)
    {
        return delta;
    }
    else if (cos < negativeCos)
    {
        return Vector2(-delta.dx, -delta.dy);
    }
    else
    {
        float32 sin = std::sin(angle);
        if (sin > 0)
        {
            return Vector2(delta.dy, -delta.dx);
        }
        else
        {
            return Vector2(-delta.dy, delta.dx);
        }
    }
}

void ClampProperty(Vector2& propertyValue, Vector2& extraDelta)
{
    Vector2 clampedValue(std::floor(propertyValue.x + TRANSFORM_EPSILON), std::floor(propertyValue.y + TRANSFORM_EPSILON));
    extraDelta += (propertyValue - clampedValue);
    propertyValue = clampedValue;
}

void ClampProperty(float32& propertyValue, float32& extraDelta)
{
    float32 clampedValue(std::floor(propertyValue));
    extraDelta += (propertyValue - clampedValue);
    propertyValue = clampedValue;
}

using DeltaPositionBehavior = Function<void(Vector2& deltaPosition)>;

Vector2 CreateFinalPosition(const UIGeometricData* parentGd, const Vector2& originalPosition, Vector2& mouseDelta, Vector2& extraDelta, DeltaPositionBehavior behavior = DeltaPositionBehavior())
{
    //transform mouse delta to control coordinates
    Vector2 scaledDelta = mouseDelta / parentGd->scale;
    Vector2 deltaPosition(::Rotate(scaledDelta, -parentGd->angle));

    //add delta from previous move event
    deltaPosition += extraDelta;
    extraDelta.SetZero();

    //call behavior if control must magnet somewhere
    if (behavior)
    {
        behavior(deltaPosition);
    }

    Vector2 finalPosition(originalPosition + deltaPosition);
    EditorTransformSystemDetail::ClampProperty(finalPosition, extraDelta);
    return finalPosition;
};
}

EditorTransformSystem::EditorTransformSystem(EditorSystemsManager* parent, TArc::ContextAccessor* accessor)
    : BaseEditorSystem(parent, accessor)
{
    systemsManager->activeAreaChanged.Connect(this, &EditorTransformSystem::OnActiveAreaChanged);
    PreferencesStorage::Instance()->RegisterPreferences(this);
}

EditorTransformSystem::~EditorTransformSystem()
{
    PreferencesStorage::Instance()->UnregisterPreferences(this);
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
        sizeProperty = rootProperty->FindPropertyByName("size");
        positionProperty = rootProperty->FindPropertyByName("position");
        angleProperty = rootProperty->FindPropertyByName("angle");
        pivotProperty = rootProperty->FindPropertyByName("pivot");
    }
    else
    {
        sizeProperty = nullptr;
        positionProperty = nullptr;
        angleProperty = nullptr;
        pivotProperty = nullptr;
    }
}

EditorSystemsManager::eDragState EditorTransformSystem::RequireNewState(UIEvent* currentInput)
{
    EditorSystemsManager::eDragState dragState = systemsManager->GetDragState();
    if (dragState == EditorSystemsManager::Transform)
    {
        if (currentInput->device == eInputDevices::MOUSE
            && currentInput->phase == UIEvent::Phase::ENDED
            && currentInput->mouseButton == eMouseButtons::LEFT)
        {
            return EditorSystemsManager::NoDrag;
        }
        else
        {
            return EditorSystemsManager::Transform;
        }
    }

    HUDAreaInfo areaInfo = systemsManager->GetCurrentHUDArea();
    if (areaInfo.area != HUDAreaInfo::NO_AREA
        && currentInput->phase == UIEvent::Phase::DRAG
        && currentInput->mouseButton == eMouseButtons::LEFT
        && dragState != EditorSystemsManager::SelectByRect)
    {
        //initialize start mouse position for correct rotation
        previousMousePos = currentInput->point;
        return EditorSystemsManager::Transform;
    }
    return EditorSystemsManager::NoDrag;
}

bool EditorTransformSystem::CanProcessInput(UIEvent* currentInput) const
{
    EditorSystemsManager::eDragState dragState = systemsManager->GetDragState();
    if (dragState == EditorSystemsManager::Transform || currentInput->device == eInputDevices::KEYBOARD)
    {
        return true;
    }
    return false;
}

void EditorTransformSystem::ProcessInput(UIEvent* currentInput)
{
    switch (currentInput->phase)
    {
    case UIEvent::Phase::KEY_DOWN:
        ProcessKey(currentInput->key);
        break;

    case UIEvent::Phase::DRAG:
        if (currentInput->mouseButton == eMouseButtons::LEFT)
        {
            ProcessDrag(currentInput->point);
        }
        break;

    case UIEvent::Phase::ENDED:
        if (activeArea == HUDAreaInfo::ROTATE_AREA)
        {
            ClampAngle();
        }
        break;
    default:
        break;
    }
}

void EditorTransformSystem::OnDragStateChanged(EditorSystemsManager::eDragState dragState, EditorSystemsManager::eDragState previousState)
{
    if (dragState == EditorSystemsManager::Transform)
    {
        extraDelta.SetZero();
        extraDeltaToMoveControls.clear();
        PrepareDrag();
    }
}

void EditorTransformSystem::ProcessKey(Key key)
{
    PrepareDrag();
    if (!selectedControlNodes.empty())
    {
        Vector2 step(expandedmoveStepByKeyboard2);
        if (IsShiftPressed())
        {
            step = moveStepByKeyboard2;
        }
        Vector2 deltaPos;
        switch (key)
        {
        case Key::LEFT:
            deltaPos.dx -= step.dx;
            break;
        case Key::UP:
            deltaPos.dy -= step.dy;
            break;
        case Key::RIGHT:
            deltaPos.dx += step.dx;
            break;
        case Key::DOWN:
            deltaPos.dy += step.dy;
            break;
        default:
            break;
        }
        if (!deltaPos.IsZero())
        {
            MoveAllSelectedControlsByKeyboard(deltaPos);
        }
    }
}

void EditorTransformSystem::PrepareDrag()
{
    using namespace DAVA::TArc;

    DataContext* activeContext = accessor->GetActiveContext();
    DVASSERT(activeContext != nullptr);
    DocumentData* data = activeContext->GetData<DocumentData>();
    SelectedNodes selection = data->GetSelectedNodes();

    selectedControlNodes.clear();
    for (PackageBaseNode* node : selection)
    {
        ControlNode* controlNode = dynamic_cast<ControlNode*>(node);
        if (controlNode != nullptr)
        {
            selectedControlNodes.insert(controlNode);
        }
    }
    nodesToMoveInfos.clear();
    for (ControlNode* selectedControl : selectedControlNodes)
    {
        nodesToMoveInfos.emplace_back(new MoveInfo(selectedControl, nullptr, nullptr));
    }
    CorrectNodesToMove();
    UpdateNeighboursToMove();
}

void EditorTransformSystem::ProcessDrag(const Vector2& pos)
{
    Vector2 delta = systemsManager->GetMouseDelta();
    switch (activeArea)
    {
    case HUDAreaInfo::FRAME_AREA:
        MoveAllSelectedControlsByMouse(delta, canMagnet);
        break;
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
        break;
    }
    case HUDAreaInfo::PIVOT_POINT_AREA:
    {
        MovePivot(delta);
        break;
    }
    case HUDAreaInfo::ROTATE_AREA:
    {
        RotateControl(pos);
        break;
    }
    default:
        break;
    }
}

void EditorTransformSystem::MoveAllSelectedControlsByKeyboard(Vector2 delta)
{
    using namespace TArc;
    if (nodesToMoveInfos.empty())
    {
        return;
    }
    DVASSERT(delta.dx == 0.0f || delta.dy == 0.0f);
    Vector<EditorTransformSystemDetail::ChangePropertyAction> propertiesToChange;
    for (auto& nodeToMove : nodesToMoveInfos)
    {
        ControlNode* node = nodeToMove->node;
        const UIGeometricData* gd = nodeToMove->parentGD;
        float32 angle = gd->angle;
        AbstractProperty* property = nodeToMove->positionProperty;
        Vector2 originalPosition = property->GetValue().Cast<Vector2>();

        Vector2 deltaPosition = EditorTransformSystemDetail::RotateVectorForMove(delta, angle);
        Vector2 finalPosition(originalPosition + deltaPosition);
        propertiesToChange.emplace_back(node, property, Any(finalPosition));
    }
    DataContext* activeContext = accessor->GetActiveContext();
    DVASSERT(activeContext != nullptr);
    DocumentData* data = activeContext->GetData<DocumentData>();
    std::unique_ptr<ChangePropertyValueCommand> command = data->CreateCommand<ChangePropertyValueCommand>();
    DVASSERT(propertiesToChange.empty() == false);
    for (const EditorTransformSystemDetail::ChangePropertyAction& changePropertyAction : propertiesToChange)
    {
        command->AddNodePropertyValue(changePropertyAction.node, changePropertyAction.property, changePropertyAction.value);
    }
    data->ExecCommand(std::move(command));
}

void EditorTransformSystem::MoveAllSelectedControlsByMouse(Vector2 mouseDelta, bool canAdjust)
{
    using namespace TArc;

    Vector<EditorTransformSystemDetail::ChangePropertyAction> propertiesToChange;

    if (canAdjust)
    {
        //find hovered node alias in nodesToMoveInfos
        Set<PackageBaseNode*> activeControlNodeHierarchy;
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

        const MoveInfo* nodeInfo = iter->get();

        auto deltaPositionBehavior = [this, nodeInfo](Vector2& deltaPosition) {
            ControlNode* node = nodeInfo->node;
            const UIGeometricData* gd = nodeInfo->parentGD;
            UIControl* control = node->GetControl();
            Vector<MagnetLineInfo> magnets;
            deltaPosition = AdjustMoveToNearestBorder(deltaPosition, magnets, gd, control);
            systemsManager->magnetLinesChanged.Emit(magnets);
        };
        const UIGeometricData* gd = nodeInfo->parentGD;

        AbstractProperty* positionProperty = nodeInfo->positionProperty;
        Vector2 originalPosition = positionProperty->GetValue().Cast<Vector2>();

        Vector2 finalPosition = EditorTransformSystemDetail::CreateFinalPosition(gd, originalPosition, mouseDelta, extraDelta, deltaPositionBehavior);

        ControlNode* node = nodeInfo->node;
        propertiesToChange.emplace_back(node, positionProperty, Any(finalPosition));

        //if control will clap it position or will magnet somewhere - it will jump from mouse cursor
        //and all other selected controls must jump too
        //in this case we get actual delta in control coordinates and transform it to global
        mouseDelta = ::Rotate((finalPosition - originalPosition), gd->angle);
        mouseDelta *= gd->scale;
    }
    for (auto& nodeToMove : nodesToMoveInfos)
    {
        ControlNode* node = nodeToMove->node;
        if (canAdjust && node == activeControlNode)
        {
            continue; //we already move it in this function
        }
        Vector2& activeExtraDelta = extraDeltaToMoveControls[node];
        AbstractProperty* positionProperty = nodeToMove->positionProperty;
        Vector2 originalPosition = positionProperty->GetValue().Cast<Vector2>();
        const UIGeometricData* gd = nodeToMove->parentGD;
        Vector2 finalPosition = EditorTransformSystemDetail::CreateFinalPosition(gd, originalPosition, mouseDelta, activeExtraDelta);

        propertiesToChange.emplace_back(node, positionProperty, Any(finalPosition));
    }

    DataContext* activeContext = accessor->GetActiveContext();
    DVASSERT(activeContext != nullptr);
    DocumentData* data = activeContext->GetData<DocumentData>();
    std::unique_ptr<ChangePropertyValueCommand> command = data->CreateCommand<ChangePropertyValueCommand>();
    DVASSERT(propertiesToChange.empty() == false);
    for (const EditorTransformSystemDetail::ChangePropertyAction& changePropertyAction : propertiesToChange)
    {
        command->AddNodePropertyValue(changePropertyAction.node, changePropertyAction.property, changePropertyAction.value);
    }
    data->ExecCommand(std::move(command));
}

Vector<EditorTransformSystem::MagnetLine> EditorTransformSystem::CreateMagnetLines(const Rect& box, const UIGeometricData* parentGD, const Vector<UIControl*>& neighbours, Vector2::eAxis axis)
{
    using namespace DAVA;
    using namespace TArc;

    DVASSERT(nullptr != parentGD);
    Vector<MagnetLine> magnets;

    CreateMagnetLinesToParent(box, parentGD, axis, magnets);
    CreateMagnetLinesToNeghbours(box, neighbours, axis, magnets);
    CreateMagnetLinesToGuides(box, parentGD, axis, magnets);

    return magnets;
}

void EditorTransformSystem::CreateMagnetLinesToParent(const Rect& box, const UIGeometricData* parentGD, Vector2::eAxis axis, Vector<MagnetLine>& lines)
{
    Rect parentBox(Vector2(), parentGD->size);

    if (parentBox.GetSize()[axis] > 0.0f)
    {
        //0.0f is equal to control left and 1.0f is equal to control right
        //first value is share of selected control and second value is share of parent control
        Vector<std::pair<float32, float32>> bordersToMagnet = {
            { 0.0f, 0.0f }, { 0.0f, 0.5f }, { 0.5f, 0.5f }, { 1.0f, 0.5f }, { 1.0f, 1.0f }
        };

        lines.reserve(lines.capacity() + bordersToMagnet.size());

        for (const auto& bordersPair : bordersToMagnet)
        {
            lines.emplace_back(bordersPair.first, box, bordersPair.second, parentBox, axis);
        }
    }
}

void EditorTransformSystem::CreateMagnetLinesToNeghbours(const Rect& box, const Vector<UIControl*>& neighbours, Vector2::eAxis axis, Vector<MagnetLine>& lines)
{
    //0.0f is equal to control left and 1.0f is equal to control right
    //first value is share of selected control and second value is share of neighbour
    Vector<std::pair<float32, float32>> bordersToMagnet = {
        { 0.0f, 0.0f }, { 0.0f, 0.5f }, { 0.5f, 0.5f }, { 1.0f, 0.5f }, { 1.0f, 1.0f }, { 0.0f, 1.0f }, { 1.0f, 0.0f }
    };

    lines.reserve(lines.capacity() + neighbours.size() * bordersToMagnet.size());

    for (UIControl* neighbour : neighbours)
    {
        DVASSERT(nullptr != neighbour);
        Rect neighbourBox = neighbour->GetLocalGeometricData().GetAABBox();

        for (const auto& bordersPair : bordersToMagnet)
        {
            lines.emplace_back(bordersPair.first, box, bordersPair.second, neighbourBox, axis);
        }
    }
}

void EditorTransformSystem::CreateMagnetLinesToGuides(const Rect& box, const UIGeometricData* parentGD, Vector2::eAxis axis, Vector<MagnetLine>& lines)
{
    using namespace TArc;

    DataContext* globalContext = accessor->GetGlobalContext();
    PreferencesData* preferencesData = globalContext->GetData<PreferencesData>();

    if (preferencesData->IsGuidesEnabled() && parentGD->angle == 0.0f)
    {
        DataContext* activeContext = accessor->GetActiveContext();
        DVASSERT(activeContext != nullptr);
        const DocumentData* data = activeContext->GetData<DocumentData>();
        const SortedControlNodeSet& rootControls = data->GetDisplayedRootControls();
        DVASSERT(rootControls.size() == 1);
        PackageNode* package = data->GetPackageNode();
        PackageBaseNode* root = *rootControls.begin();
        PackageNode::AxisGuides values = package->GetAxisGuides(root->GetName(), axis);

        Vector<float32> bordersToMagnet = { 0.0f, 0.5f, 1.0f };

        lines.reserve(lines.capacity() + values.size() * bordersToMagnet.size());

        const UIGeometricData rootGD = root->GetControl()->GetGeometricData();
        for (float32 value : values)
        {
            //position in global coordinates, while pivotPoint and value in root control coordinates
            float32 valueInGlobalCoordinates = value * rootGD.scale[axis] + (rootGD.position[axis] - rootGD.pivotPoint[axis] * rootGD.scale[axis]);
            float32 valueInControlCoordinates = (valueInGlobalCoordinates - (parentGD->position[axis] - parentGD->pivotPoint[axis] * parentGD->scale[axis])) / parentGD->scale[axis];

            for (float32 borderToManget : bordersToMagnet)
            {
                lines.emplace_back(borderToManget, box, valueInControlCoordinates, axis);
            }
        }
    }
}

void EditorTransformSystem::ExtractMatchedLines(Vector<MagnetLineInfo>& magnets, const Vector<MagnetLine>& magnetLines, const UIControl* control, Vector2::eAxis axis)
{
    UIControl* parent = control->GetParent();
    const UIGeometricData* parentGD = &parent->GetGeometricData();
    for (const MagnetLine& line : magnetLines)
    {
        if (fabs(line.interval) < EditorTransformSystemDetail::TRANSFORM_EPSILON)
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
        Vector<MagnetLine> magnetLines = CreateMagnetLines(box, parentGD, neighbours, axis);

        //get nearest magnet line
        std::function<bool(const MagnetLine&, const MagnetLine&)> predicate = [](const MagnetLine& left, const MagnetLine& right) -> bool {
            return fabs(left.interval) < fabs(right.interval);
        };
        if (magnetLines.empty())
        {
            continue;
        }

        MagnetLine nearestLine = *std::min_element(magnetLines.begin(), magnetLines.end(), predicate);
        float32 areaNearLineRight = nearestLine.targetPosition + moveMagnetRange[axis];
        float32 areaNearLineLeft = nearestLine.targetPosition - moveMagnetRange[axis];
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
    deltaMappedToControl = ::Rotate(deltaMappedToControl, -controlGeometricData.angle);

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

    EditorTransformSystemDetail::ClampProperty(adjustedSize, extraDelta);

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
    deltaPosition = ::Rotate(deltaPosition, control->GetAngle());

    Vector2 originalSize = sizeProperty->GetValue().Cast<Vector2>();
    Vector2 finalSize(originalSize + adjustedSize);
    Any sizeValue(finalSize);

    Vector2 originalPosition = positionProperty->GetValue().Cast<Vector2>();
    Vector2 finalPosition = originalPosition;
    if (activeControlNode->GetParent() != nullptr && activeControlNode->GetParent()->GetControl() != nullptr)
    {
        finalPosition += deltaPosition;
    }

    Any positionValue(finalPosition);

    TArc::DataContext* activeContext = accessor->GetActiveContext();
    DVASSERT(activeContext != nullptr);
    DocumentData* data = activeContext->GetData<DocumentData>();
    std::unique_ptr<ResizeCommand> command = data->CreateCommand<ResizeCommand>();

    command->AddNodePropertyValue(activeControlNode,
                                  sizeProperty,
                                  sizeValue,
                                  positionProperty,
                                  positionValue);

    data->ExecCommand(std::move(command));
}

Vector2 EditorTransformSystem::AdjustResizeToMinimumSize(Vector2 deltaSize)
{
    const Vector2 scaledMinimum(GetMinimumSize() / controlGeometricData.scale);
    Vector2 origSize = sizeProperty->GetValue().Cast<Vector2>();

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

    bool canAdjustResize = canMagnet && activeControlNode->GetControl()->GetAngle() == 0.0f && activeControlNode->GetParent()->GetControl() != nullptr;
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
    systemsManager->magnetLinesChanged.Emit(magnets);

    return adjustedSize;
}

Vector2 EditorTransformSystem::AdjustResizeToBorder(Vector2 deltaSize, Vector2 transformPoint, Directions directions, Vector<MagnetLineInfo>& magnets)
{
    UIControl* control = activeControlNode->GetControl();

    UIGeometricData controlGD = control->GetLocalGeometricData();
    //calculate control box in parent
    controlGD.size += deltaSize;
    Rect box = controlGD.GetAABBox();
    Vector2 sizeAffect = ::Rotate(deltaSize * transformPoint * controlGD.scale, controlGD.angle);
    box.SetPosition(box.GetPosition() - sizeAffect);

    Vector2 transformPosition = box.GetPosition() + box.GetSize() * transformPoint;

    for (int32 axisInt = Vector2::AXIS_X; axisInt < Vector2::AXIS_COUNT; ++axisInt)
    {
        Vector2::eAxis axis = static_cast<Vector2::eAxis>(axisInt);
        if (directions[axis] != NO_DIRECTION)
        {
            Vector<MagnetLine> magnetLines = CreateMagnetLines(box, &parentGeometricData, neighbours, axis);
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
            float32 rangeForPosition = resizeMagnetRange[axis] * share;
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
    Vector<EditorTransformSystemDetail::ChangePropertyAction> propertiesToChange;
    Vector2 pivot = AdjustPivotToNearestArea(delta);

    Vector2 scaledDelta(delta / parentGeometricData.scale);
    Vector2 rotatedDeltaPosition(::Rotate(scaledDelta, -parentGeometricData.angle));
    Vector2 originalPos(positionProperty->GetValue().Cast<Vector2>());
    Vector2 finalPosition(originalPos + rotatedDeltaPosition);

    TArc::DataContext* activeContext = accessor->GetActiveContext();
    DVASSERT(activeContext != nullptr);
    DocumentData* data = activeContext->GetData<DocumentData>();
    std::unique_ptr<ChangePivotCommand> command = data->CreateCommand<ChangePivotCommand>();
    command->AddNodePropertyValue(activeControlNode,
                                  pivotProperty,
                                  Any(pivot),
                                  positionProperty,
                                  Any(finalPosition));
    data->ExecCommand(std::move(command));
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

Vector2 EditorTransformSystem::AdjustPivotToNearestArea(Vector2& delta)
{
    Vector<MagnetLineInfo> magnetLines;

    const Rect ur(controlGeometricData.GetUnrotatedRect());
    const Vector2 controlSize(ur.GetSize());
    DVASSERT(controlSize.x > 0.0f && controlSize.y > 0.0f);
    const Vector2 rotatedDeltaPivot(::Rotate(delta, -controlGeometricData.angle));
    Vector2 deltaPivot(rotatedDeltaPivot / controlSize);

    const Vector2 range(pivotMagnetRange / controlSize); //range in pivot coordinates

    Vector2 origPivot = pivotProperty->GetValue().Cast<Vector2>();
    Vector2 finalPivot(origPivot + deltaPivot + extraDelta);

    bool found = false;
    if (IsShiftPressed() && shareOfSizeToMagnetPivot.x > 0.0f && shareOfSizeToMagnetPivot.y > 0.0f)
    {
        const float32 maxPivot = 1.0f;

        Vector2 target;
        Vector2 distanceToTarget;
        Vector2 shareOfSizeToMagnetPivot_;
        for (float32 targetX = 0.0f; targetX <= maxPivot; targetX += shareOfSizeToMagnetPivot.x)
        {
            for (float32 targetY = 0.0f; targetY <= maxPivot; targetY += shareOfSizeToMagnetPivot.y)
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
            delta = ::Rotate((target - origPivot) * controlSize, controlGeometricData.angle);

            finalPivot = target;
        }
    }

    if (!found)
    {
        if (!extraDelta.IsZero())
        {
            deltaPivot += extraDelta;
            extraDelta.SetZero();
            delta = ::Rotate(deltaPivot * controlSize, controlGeometricData.angle);
        }
    }
    systemsManager->magnetLinesChanged.Emit(magnetLines);
    return finalPivot;
}

bool EditorTransformSystem::RotateControl(const Vector2& pos)
{
    Vector2 rotatePoint(controlGeometricData.GetUnrotatedRect().GetPosition());
    rotatePoint += controlGeometricData.pivotPoint * controlGeometricData.scale;
    Vector2 l1(previousMousePos - rotatePoint);
    Vector2 l2(pos - rotatePoint);

    if (l2.Length() < 15.0f)
    {
        return false;
    }

    float32 angleRad = atan2(l1.x * l2.y - l2.x * l1.y, l1.x * l2.x + l1.y * l2.y);
    float32 deltaAngle = RadToDeg(angleRad);
    //after modification deltaAngle is less than mouse delta positions

    deltaAngle += extraDelta.dx;
    extraDelta.SetZero();
    float32 originalAngle = angleProperty->GetValue().Cast<float32>();

    float32 finalAngle = AdjustRotateToFixedAngle(deltaAngle, originalAngle);

    TArc::DataContext* activeContext = accessor->GetActiveContext();
    DVASSERT(activeContext != nullptr);
    DocumentData* data = activeContext->GetData<DocumentData>();
    std::unique_ptr<ChangePropertyValueCommand> command = data->CreateCommand<ChangePropertyValueCommand>();
    command->AddNodePropertyValue(activeControlNode, angleProperty, Any(finalAngle));
    data->ExecCommand(std::move(command));

    previousMousePos = pos;
    return true;
}

float32 EditorTransformSystem::AdjustRotateToFixedAngle(float32 deltaAngle, float32 originalAngle)
{
    float32 finalAngle = originalAngle + deltaAngle;
    if (IsShiftPressed())
    {
        const int step = angleSegment; //fixed angle step
        int32 nearestTargetAngle = static_cast<int32>(finalAngle - static_cast<int32>(finalAngle) % step);
        if ((finalAngle >= 0) ^ (deltaAngle > 0))
        {
            nearestTargetAngle += step * (finalAngle >= 0.0f ? 1 : -1);
        }
        //disable rotate backwards if we move cursor forward
        if ((deltaAngle >= 0.0f && nearestTargetAngle <= originalAngle + EditorTransformSystemDetail::TRANSFORM_EPSILON)
            || (deltaAngle < 0.0f && nearestTargetAngle >= originalAngle - EditorTransformSystemDetail::TRANSFORM_EPSILON))
        {
            extraDelta.dx = deltaAngle;
            return originalAngle;
        }
        extraDelta.dx = finalAngle - nearestTargetAngle;
        return nearestTargetAngle;
    }
    else
    {
        EditorTransformSystemDetail::ClampProperty(finalAngle, extraDelta.dx);
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
        moveInfo->positionProperty = node->GetRootProperty()->FindPropertyByName("position");
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
    float32 angle = angleProperty->GetValue().Cast<float32>();
    if (fabs(angle) > 360)
    {
        angle += angle > 0.0f ? EditorTransformSystemDetail::TRANSFORM_EPSILON : -EditorTransformSystemDetail::TRANSFORM_EPSILON;
        angle = static_cast<int32>(angle) % 360;
    }

    TArc::DataContext* activeContext = accessor->GetActiveContext();
    DVASSERT(activeContext != nullptr);
    DocumentData* data = activeContext->GetData<DocumentData>();
    std::unique_ptr<ChangePropertyValueCommand> command = data->CreateCommand<ChangePropertyValueCommand>();
    command->AddNodePropertyValue(activeControlNode, angleProperty, Any(angle));
    data->ExecCommand(std::move(command));
}

bool EditorTransformSystem::IsShiftPressed() const
{
    return IsKeyPressed(KeyboardProxy::KEY_SHIFT) ^ (shiftInverted);
}
