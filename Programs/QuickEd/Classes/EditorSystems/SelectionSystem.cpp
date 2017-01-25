#include "Input/InputSystem.h"
#include "EditorSystems/SelectionSystem.h"
#include "Model/PackageHierarchy/ControlNode.h"
#include "UI/UIEvent.h"
#include "UI/UIControl.h"
#include "EditorSystems/EditorSystemsManager.h"
#include "EditorSystems/KeyboardProxy.h"
#include "Model/PackageHierarchy/PackageNode.h"
#include "Model/PackageHierarchy/PackageControlsNode.h"
#include "Model/ControlProperties/RootProperty.h"
#include "Model/ControlProperties/VisibleValueProperty.h"

using namespace DAVA;

REGISTER_PREFERENCES_ON_START(SelectionSystem,
                              PREF_ARG("CanFindCommonForSelection", true),
                              )

SelectionSystem::SelectionSystem(EditorSystemsManager* parent)
    : BaseEditorSystem(parent)
{
    systemsManager->selectionChanged.Connect(this, &SelectionSystem::OnSelectionChanged);
    systemsManager->packageNodeChanged.Connect(this, &SelectionSystem::OnPackageNodeChanged);
    systemsManager->selectionRectChanged.Connect(this, &SelectionSystem::OnSelectByRect);
    PreferencesStorage::Instance()->RegisterPreferences(this);
}

SelectionSystem::~SelectionSystem()
{
    PreferencesStorage::Instance()->UnregisterPreferences(this);
}

bool SelectionSystem::OnInput(UIEvent* currentInput)
{
    switch (currentInput->phase)
    {
    case UIEvent::Phase::BEGAN:
        mousePressed = true;
        ProcessMousePress(currentInput->point, currentInput->mouseButton);
        break;
    case UIEvent::Phase::ENDED:
        if (!mousePressed)
        {
            ProcessMousePress(currentInput->point, currentInput->mouseButton);
        }
        mousePressed = false;
    default:
        break;
    }
    return false;
}

void SelectionSystem::OnPackageNodeChanged(PackageNode* packageNode_)
{
    if (nullptr != packageNode)
    {
        packageNode->RemoveListener(this);
    }
    packageNode = packageNode_;
    if (nullptr != packageNode)
    {
        packageNode->AddListener(this);
    }
}

void SelectionSystem::ControlWasRemoved(ControlNode* node, ControlsContainerNode*)
{
    SelectedNodes deselected;
    deselected.insert(node);
    SelectNode(SelectedNodes(), deselected);
}

void SelectionSystem::OnSelectByRect(const Rect& rect)
{
    SelectedNodes deselected;
    SelectedNodes selected;
    Set<ControlNode*> areaNodes;
    auto predicate = [rect](const ControlNode* node) -> bool {
        const UIControl* control = node->GetControl();
        DVASSERT(nullptr != control);
        return control->GetVisibilityFlag() && rect.RectContains(control->GetGeometricData().GetAABBox());
    };
    auto stopPredicate = [](const ControlNode* node) -> bool {
        const UIControl* control = node->GetControl();
        DVASSERT(nullptr != control);
        return !control->GetVisibilityFlag();
    };
    systemsManager->CollectControlNodes(std::inserter(areaNodes, areaNodes.end()), predicate, stopPredicate);
    if (!areaNodes.empty())
    {
        for (ControlNode* node : areaNodes)
        {
            selected.insert(node);
        }
    }
    if (!IsKeyPressed(KeyboardProxy::KEY_SHIFT))
    {
        //deselect all not selected by rect
        std::set_difference(selectionContainer.selectedNodes.begin(), selectionContainer.selectedNodes.end(), areaNodes.begin(), areaNodes.end(), std::inserter(deselected, deselected.end()));
    }
    SelectNode(selected, deselected);
}

void SelectionSystem::ClearSelection()
{
    SelectNode(SelectedNodes(), selectionContainer.selectedNodes);
}

void SelectionSystem::SelectAllControls()
{
    SelectedNodes selected;
    systemsManager->CollectControlNodes(std::inserter(selected, selected.end()), [](const ControlNode*) { return true; });
    SelectNode(selected, SelectedNodes());
}

void SelectionSystem::FocusNextChild()
{
    FocusToChild(true);
}

void SelectionSystem::FocusPreviousChild()
{
    FocusToChild(false);
}

void SelectionSystem::FocusToChild(bool next)
{
    PackageBaseNode* startNode = nullptr;
    if (!selectionContainer.selectedNodes.empty())
    {
        startNode = *selectionContainer.selectedNodes.rbegin();
    }
    PackageBaseNode* nextNode = nullptr;
    Vector<PackageBaseNode*> allNodes;
    systemsManager->CollectControlNodes(std::back_inserter(allNodes), [](const ControlNode*) { return true; });
    if (allNodes.empty())
    {
        return;
    }
    auto findIt = std::find(allNodes.begin(), allNodes.end(), startNode);
    if (findIt == allNodes.end())
    {
        nextNode = next ? allNodes.front() : allNodes.back();
    }
    else if (next)
    {
        ++findIt;
        nextNode = findIt == allNodes.end() ? allNodes.front() : *findIt;
    }
    else
    {
        nextNode = findIt == allNodes.begin() ? allNodes.back() : *(--findIt);
    }

    SelectedNodes newSelectedNodes;
    newSelectedNodes.insert(nextNode);
    SelectNode(newSelectedNodes, selectionContainer.selectedNodes);
}

void SelectionSystem::ProcessMousePress(const DAVA::Vector2& point, DAVA::eMouseButtons buttonID)
{
    ControlNode* selectedNode = nullptr;
    if (buttonID == DAVA::eMouseButtons::LEFT)
    {
        selectedNode = systemsManager->GetControlNodeAtPoint(point);
    }
    if (nullptr != selectedNode)
    {
        SelectNode(selectedNode);
    }
}

void SelectionSystem::OnSelectionChanged(const SelectedNodes& selected, const SelectedNodes& deselected)
{
    selectionContainer.MergeSelection(selected, deselected);
}

void SelectionSystem::SelectNode(const SelectedNodes& selected, const SelectedNodes& deselected)
{
    SelectedNodes reallySelected;
    SelectedNodes reallyDeselected;
    selectionContainer.GetOnlyExistedItems(deselected, reallyDeselected);
    selectionContainer.GetNotExistedItems(selected, reallySelected);
    selectionContainer.MergeSelection(reallySelected, reallyDeselected);

    if (!reallySelected.empty() || !reallyDeselected.empty())
    {
        systemsManager->selectionChanged.Emit(reallySelected, reallyDeselected);
    }
}

void SelectionSystem::SelectNode(ControlNode* selectedNode)
{
    SelectedNodes selected;
    SelectedNodes deselected;
    if (!IsKeyPressed(KeyboardProxy::KEY_SHIFT) && !IsKeyPressed(KeyboardProxy::KEY_CTRL))
    {
        deselected = selectionContainer.selectedNodes;
    }

    if (selectedNode != nullptr)
    {
        if (IsKeyPressed(KeyboardProxy::KEY_CTRL) && selectionContainer.IsSelected(selectedNode))
        {
            deselected.insert(selectedNode);
        }
        else
        {
            selected.insert(selectedNode);
        }
    }
    for (PackageBaseNode* controlNode : selected)
    {
        deselected.erase(controlNode);
    }
    SelectNode(selected, deselected);
}

ControlNode* SelectionSystem::FindSmallNodeUnderNode(const Vector<ControlNode*>& nodesUnderPoint) const
{
    if (!canFindCommonForSelection || nodesUnderPoint.empty())
    {
        return nullptr;
    }
    //if control much smaller than parent - we can want to select it
    Vector<std::pair<ControlNode*, Vector2>> sizes;
    sizes.reserve(nodesUnderPoint.size());
    PackageBaseNode* node = nodesUnderPoint.back();
    Set<PackageBaseNode*> topLevelItemHierarchy;
    ControlNode* parentNode = dynamic_cast<ControlNode*>(node->GetParent());
    if (parentNode == nullptr)
    {
        return nullptr;
    }
    //we can place controls under each other
    for (ControlNode* child : *parentNode)
    {
        topLevelItemHierarchy.insert(child);
    }

    //get hierarchy to ensure that node under cursor is a top visible node
    do
    {
        topLevelItemHierarchy.insert(node);
        node = node->GetParent();
    } while (node != nullptr && node->GetControl() != nullptr);

    for (auto iter = nodesUnderPoint.rbegin(); iter != nodesUnderPoint.rend(); ++iter)
    {
        ControlNode* node = *iter;
        Vector2 size = node->GetControl()->GetAbsoluteRect().GetSize();
        if (!sizes.empty())
        {
            const std::pair<ControlNode*, Vector2> lastNodeSize = sizes.back();
            Vector2 previousSize = lastNodeSize.second;

            ControlNode* previousNode = lastNodeSize.first;
            //not toplevel node or it hierarchy. We don't search in background nodes
            if (topLevelItemHierarchy.find(previousNode) == topLevelItemHierarchy.end())
            {
                break;
            }

            //some size issues. They can migrate to the preferences system later
            const Vector2 acceptableSizeProportion(3.0f, 3.0f);
            const Vector2 acceptableRelativeSizeDifference(30.0f, 30.0f);
            const Vector2 acceptableAbsoluteSizeDifference(200.0f, 200.0f);
            for (int32 axisInt = Vector2::AXIS_X; axisInt < Vector2::AXIS_COUNT; ++axisInt)
            {
                Vector2::eAxis axis = static_cast<Vector2::eAxis>(axisInt);
                if ((size[axis] / previousSize[axis] > acceptableSizeProportion[axis]
                     && size[axis] - previousSize[axis] > acceptableRelativeSizeDifference[axis])
                    || size[axis] - previousSize[axis] > acceptableAbsoluteSizeDifference[axis])
                {
                    return previousNode;
                }
            }
        }
        //someone still can create control with a negative size
        if (size.dx > 0.0f && size.dy > 0.0f)
        {
            sizes.push_back(std::make_pair(node, size));
        }
    }
    return nullptr;
}

void SelectionSystem::GetNodesForSelection(Vector<ControlNode*>& nodesUnderPoint, const Vector2& point) const
{
    auto findPredicate = [point](const ControlNode* node) -> bool {
        const UIControl* control = node->GetControl();
        DVASSERT(nullptr != control);
        return control->GetVisibilityFlag() && control->IsPointInside(point);
    };

    auto stopPredicate = [](const ControlNode* node) -> bool {
        const UIControl* control = node->GetControl();
        DVASSERT(nullptr != control);
        return !control->GetVisibilityFlag();
    };
    systemsManager->CollectControlNodes(std::back_inserter(nodesUnderPoint), findPredicate, stopPredicate);
}

ControlNode* SelectionSystem::GetCommonNodeUnderPoint(const DAVA::Vector2& point) const
{
    Vector<ControlNode*> nodesUnderPoint;
    GetNodesForSelection(nodesUnderPoint, point);
    const SelectedNodes& selected = selectionContainer.selectedNodes;
    //no selection. Search for the child of root under cursor
    if (nodesUnderPoint.empty())
    {
        return nullptr;
    }
    else if (nodesUnderPoint.size() == 1)
    {
        return nodesUnderPoint.front();
    }

    if (!selected.empty())
    {
        //collect all selected hierarchy
        SelectedNodes parentsOfSelectedNodes;
        for (PackageBaseNode* node : selected)
        {
            node = node->GetParent();
            while (node != nullptr && node->GetControl() != nullptr)
            {
                parentsOfSelectedNodes.insert(node);
                node = node->GetParent();
            }
        }

        //move from deep nodes to root node
        for (auto iter = nodesUnderPoint.rbegin(); iter != nodesUnderPoint.rend(); ++iter)
        {
            ControlNode* node = *iter;
            PackageBaseNode* nodeParent = node->GetParent();

            if (selected.find(node) != selected.end())
            {
                return node;
            }

            //search child of selected to move down by hierarchy
            // or search neighbor to move left-right
            if (selected.find(nodeParent) != selected.end())
            {
                return node;
            }
            else if (selected.find(node) == selected.end()
                     && parentsOfSelectedNodes.find(nodeParent) != parentsOfSelectedNodes.end())
            {
                return node;
            }
        }
        //may be there some small node inside big one
        ControlNode* node = FindSmallNodeUnderNode(nodesUnderPoint);
        if (node != nullptr)
        {
            return node;
        }
    }
    //try to find child of root control
    else
    {
        ControlNode* node = FindSmallNodeUnderNode(nodesUnderPoint);
        if (node != nullptr)
        {
            return node;
        }
        else
        {
            for (auto iter = nodesUnderPoint.rbegin(); iter != nodesUnderPoint.rend(); ++iter)
            {
                ControlNode* node = *iter;
                PackageBaseNode* parent = node->GetParent();
                if (parent != nullptr)
                {
                    parent = parent->GetParent();
                }
                if (parent != nullptr && parent->GetControl() == nullptr)
                {
                    return node;
                }
            }
        }
    }
    //did not found nothing, get nearest
    return GetNearestNodeUnderPoint(point);
}

ControlNode* SelectionSystem::GetNearestNodeUnderPoint(const DAVA::Vector2& point) const
{
    Vector<ControlNode*> nodesUnderPoint;
    GetNodesForSelection(nodesUnderPoint, point);
    return nodesUnderPoint.empty() ? nullptr : nodesUnderPoint.back();
}
