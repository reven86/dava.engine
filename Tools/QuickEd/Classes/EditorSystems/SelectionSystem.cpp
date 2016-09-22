#include "Input/InputSystem.h"
#include "Input/KeyboardDevice.h"
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

SelectionSystem::SelectionSystem(EditorSystemsManager* parent)
    : BaseEditorSystem(parent)
{
    systemsManager->SelectionChanged.Connect(this, &SelectionSystem::OnSelectionChanged);
    systemsManager->PackageNodeChanged.Connect(this, &SelectionSystem::OnPackageNodeChanged);
    systemsManager->SelectionRectChanged.Connect(this, &SelectionSystem::OnSelectByRect);
}

SelectionSystem::~SelectionSystem() = default;

bool SelectionSystem::OnInput(UIEvent* currentInput)
{
    switch (currentInput->phase)
    {
    case UIEvent::Phase::BEGAN:
        mousePressed = true;
        ProcessMousePress(currentInput->point, currentInput->mouseButton);
    case UIEvent::Phase::ENDED:
        if (!mousePressed)
        {
            ProcessMousePress(currentInput->point, currentInput->mouseButton);
        }
        mousePressed = false;
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
        const auto control = node->GetControl();
        DVASSERT(nullptr != control);
        return control->GetVisibilityFlag() && rect.RectContains(control->GetGeometricData().GetAABBox());
    };
    auto stopPredicate = [](const ControlNode* node) -> bool {
        const auto control = node->GetControl();
        DVASSERT(nullptr != control);
        return !control->GetVisibilityFlag();
    };
    systemsManager->CollectControlNodes(std::inserter(areaNodes, areaNodes.end()), predicate, stopPredicate);
    if (!areaNodes.empty())
    {
        for (auto node : areaNodes)
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

void SelectionSystem::ProcessMousePress(const DAVA::Vector2& point, UIEvent::MouseButton buttonID)
{
    ControlNode* selectedNode = nullptr;
    if (buttonID == UIEvent::MouseButton::LEFT)
    {
        Vector<ControlNode*> nodesUnderPoint;
        auto predicate = [point](const ControlNode* node) -> bool {
            const auto control = node->GetControl();
            DVASSERT(nullptr != control);
            return control->GetVisibilityFlag() && control->IsPointInside(point);
        };
        auto stopPredicate = [](const ControlNode* node) -> bool {
            const auto control = node->GetControl();
            DVASSERT(nullptr != control);
            return !control->GetVisibilityFlag();
        };
        systemsManager->CollectControlNodes(std::back_inserter(nodesUnderPoint), predicate, stopPredicate);
        if (!nodesUnderPoint.empty())
        {
            selectedNode = nodesUnderPoint.back();
        }
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
        systemsManager->SelectionChanged.Emit(reallySelected, reallyDeselected);
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
    for (auto controlNode : selected)
    {
        deselected.erase(controlNode);
    }
    SelectNode(selected, deselected);
}
