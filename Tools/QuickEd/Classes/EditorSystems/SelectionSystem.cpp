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
        break;
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

void SelectionSystem::ProcessMousePress(const DAVA::Vector2& point, UIEvent::MouseButton buttonID)
{
    ControlNode* selectedNode = nullptr;
    if (buttonID == UIEvent::MouseButton::LEFT)
    {
        selectedNode = ControlNodeUnderPoint(point, false);
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
    for (PackageBaseNode* controlNode : selected)
    {
        deselected.erase(controlNode);
    }
    SelectNode(selected, deselected);
}

namespace SelectionSystemDetails
{
ControlNode* GetCommonNodeUnderPoint(const Vector<ControlNode*>& nodesUnderPoint)
{
    //if control much smaller than parent - we can want to select it
    Vector<std::pair<ControlNode*, Vector2>> sizes;
    sizes.reserve(nodesUnderPoint.size());
    for (auto iter = nodesUnderPoint.rbegin(); iter != nodesUnderPoint.rend(); ++iter)
    {
        ControlNode* node = *iter;
        Vector2 size = node->GetControl()->GetAbsoluteRect().GetSize();
        if (!sizes.empty())
        {
            const std::pair<ControlNode*, Vector2> lastNodeSize = sizes.back();
            Vector2 sizeDiff = size - lastNodeSize.second;
            if (sizeDiff.dx > 300.0f || sizeDiff.dx > 300.0f
                || sizeDiff.dx > 100.0f && sizeDiff.dy > 100.0f)
            {
                Logger::Debug("found common of selected %s with size %f %f - %s with size %f %f", node->GetName().c_str(), size.x, size.y, lastNodeSize.first->GetName().c_str(), lastNodeSize.second.x, lastNodeSize.second.y);
                return lastNodeSize.first;
            }
        }
        sizes.push_back(std::make_pair(node, size));
    }
    return nullptr;
}
}

ControlNode* SelectionSystem::ControlNodeUnderPoint(const DAVA::Vector2& point, bool nearest) const
{
    if (KeyboardProxy::IsKeyPressed(KeyboardProxy::KEY_ALT))
    {
        nearest = !nearest;
    }
    auto findPredicate = [point](const ControlNode* node) -> bool {
        const UIControl* control = node->GetControl();
        DVASSERT(nullptr != control);
        return control->GetVisibilityFlag() && control->IsPointInside(point);
    };
    Vector<ControlNode*> nodesUnderPoint;

    auto stopPredicate = [](const ControlNode* node) -> bool {
        const UIControl* control = node->GetControl();
        DVASSERT(nullptr != control);
        return !control->GetVisibilityFlag();
    };
    systemsManager->CollectControlNodes(std::back_inserter(nodesUnderPoint), findPredicate, stopPredicate);

    const SelectedNodes& selected = selectionContainer.selectedNodes;
    //no selection. Search for the child of root under cursor
    if (nodesUnderPoint.empty())
    {
        return nullptr;
    }

    //found only one item
    else if (nodesUnderPoint.size() == 1)
    {
        Logger::Debug("found only one %s", nodesUnderPoint.front()->GetName().c_str());
        return nodesUnderPoint.front();
    }

    else if (!nearest)
    {
        if (!selected.empty())
        {
            SelectedNodes parentsOfSelectedNodes;
            auto outIter = std::inserter(parentsOfSelectedNodes, parentsOfSelectedNodes.end());
            std::transform(selected.begin(), selected.end(), outIter, [](PackageBaseNode* node) { return node->GetParent(); });
            //move from deep nodes to root node
            for (auto iter = nodesUnderPoint.rbegin(); iter != nodesUnderPoint.rend(); ++iter)
            {
                ControlNode* node = *iter;
                PackageBaseNode* nodeParent = node->GetParent();

                //search child of selected to move down by hierarchy
                if (selected.find(nodeParent) != selected.end())
                {
                    Logger::Debug("found child of selected %s", node->GetName().c_str());
                    return node;
                }

                //search neighbor to move left-right
                else if (parentsOfSelectedNodes.find(nodeParent) != parentsOfSelectedNodes.end())
                {
                    Logger::Debug("found neighbor of selected %s", node->GetName().c_str());
                    return node;
                }

                //search parent of selected to move up
                else if (parentsOfSelectedNodes.find(node) != parentsOfSelectedNodes.end())
                {
                    Logger::Debug("found parent of selected %s", node->GetName().c_str());
                    return node;
                }

                //if already selected.
                else if (selected.find(node) != selected.end())
                {
                    Logger::Debug("found already of selected %s", node->GetName().c_str());
                    return node;
                }
            }
            //may be there some small node inside big one
            ControlNode* node = SelectionSystemDetails::GetCommonNodeUnderPoint(nodesUnderPoint);
            if (node != nullptr)
            {
                return node;
            }
        }
        //return child of root control
        else
        {
            ControlNode* node = SelectionSystemDetails::GetCommonNodeUnderPoint(nodesUnderPoint);
            if (node != nullptr)
            {
                return node;
            }
            else
            {
                ControlNode* firstNode = nodesUnderPoint.front();
                if (firstNode->GetParent()->GetControl() == nullptr)
                {
                    Logger::Debug("no selected but found root child %s %s", nodesUnderPoint.at(1)->GetName().c_str(), nodesUnderPoint.at(0)->GetName().c_str());
                    return nodesUnderPoint.at(1);
                }
                else
                {
                    Logger::Debug("no selected but found something %s %s", nodesUnderPoint.at(1)->GetName().c_str(), nodesUnderPoint.at(0)->GetName().c_str());
                    return firstNode;
                }
            }
        }
    }
    DAVA::Logger::Debug("search nearest node %s", nodesUnderPoint.empty() ? "" : nodesUnderPoint.back()->GetName().c_str());
    return nodesUnderPoint.empty() ? nullptr : nodesUnderPoint.back();
}
