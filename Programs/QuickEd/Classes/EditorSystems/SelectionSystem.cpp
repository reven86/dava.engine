#include "EditorSystems/SelectionSystem.h"
#include "EditorSystems/EditorSystemsManager.h"
#include "EditorSystems/KeyboardProxy.h"
#include "Model/PackageHierarchy/PackageControlsNode.h"
#include "Model/ControlProperties/RootProperty.h"
#include "Model/ControlProperties/VisibleValueProperty.h"
#include "Model/PackageHierarchy/ControlNode.h"

#include "Modules/DocumentsModule/DocumentData.h"

#include <TArc/Core/ContextAccessor.h>

#include <Reflection/ReflectedTypeDB.h>
#include <UI/UIEvent.h>
#include <UI/UIControl.h>
#include <Input/InputSystem.h>

using namespace DAVA;

REGISTER_PREFERENCES_ON_START(SelectionSystem,
                              PREF_ARG("CanFindCommonForSelection", true),
                              )

SelectionSystem::SelectionSystem(EditorSystemsManager* parent, DAVA::TArc::ContextAccessor* accessor)
    : BaseEditorSystem(parent, accessor)
{
    documentDataWrapper = accessor->CreateWrapper(DAVA::ReflectedTypeDB::Get<DocumentData>());
    systemsManager->selectionRectChanged.Connect(this, &SelectionSystem::OnSelectByRect);

    PreferencesStorage::Instance()->RegisterPreferences(this);
}

SelectionSystem::~SelectionSystem()
{
    PreferencesStorage::Instance()->UnregisterPreferences(this);
}

void SelectionSystem::ProcessInput(UIEvent* currentInput)
{
    if (currentInput->phase == UIEvent::Phase::BEGAN)
    {
        ControlNode* selectedNode = systemsManager->GetControlNodeAtPoint(currentInput->point, currentInput->tapCount > 1);
        if (nullptr != selectedNode)
        {
            SelectNode(selectedNode);
        }
    }
}

void SelectionSystem::OnSelectByRect(const Rect& rect)
{
    using namespace DAVA::TArc;

    SelectedNodes newSelection;
    if (IsKeyPressed(KeyboardProxy::KEY_SHIFT))
    {
        newSelection = documentDataWrapper.GetFieldValue(DocumentData::selectionPropertyName).Cast<SelectedNodes>(SelectedNodes());
    }

    DataContext* activeContext = accessor->GetActiveContext();
    DVASSERT(activeContext != nullptr);
    DocumentData* documentData = activeContext->GetData<DocumentData>();

    for (const PackageBaseNode* node : documentData->GetDisplayedRootControls())
    {
        for (int i = 0, count = node->GetCount(); i < count; ++i)
        {
            PackageBaseNode* child = node->Get(i);
            UIControl* control = child->GetControl();
            DVASSERT(nullptr != control);
            if (control->IsVisible() && rect.RectContains(control->GetGeometricData().GetAABBox()))
            {
                newSelection.insert(node->Get(i));
            }
        }
    }

    SelectNodes(newSelection);
}

void SelectionSystem::ClearSelection()
{
    SelectNodes(SelectedNodes());
}

void SelectionSystem::SelectAllControls()
{
    using namespace DAVA::TArc;

    DataContext* activeContext = accessor->GetActiveContext();
    DVASSERT(activeContext != nullptr);
    DocumentData* documentData = activeContext->GetData<DocumentData>();

    SelectedNodes selected;
    //find only children of root controls
    for (const PackageBaseNode* node : documentData->GetDisplayedRootControls())
    {
        for (int i = 0, count = node->GetCount(); i < count; ++i)
        {
            selected.insert(node->Get(i));
        }
    }
    if (selected.empty() == false)
    {
        SelectNodes(selected);
    }
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
    SelectedNodes selection = documentDataWrapper.GetFieldValue(DocumentData::selectionPropertyName).Cast<SelectedNodes>(SelectedNodes());

    if (!selection.empty())
    {
        startNode = *selection.rbegin();
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

    SelectNodes({ nextNode });
}

void SelectionSystem::SelectNodes(const SelectedNodes& selection)
{
    //TODO: remove this "if" when other systems will not emit signal selectionChanged
    if (documentDataWrapper.HasData())
    {
        documentDataWrapper.SetFieldValue(DocumentData::selectionPropertyName, selection);
    }
}

void SelectionSystem::SelectNode(ControlNode* selectedNode)
{
    SelectedNodes newSelection;
    SelectedNodes currentSelection = documentDataWrapper.GetFieldValue(DocumentData::selectionPropertyName).Cast<SelectedNodes>(SelectedNodes());
    if (IsKeyPressed(KeyboardProxy::KEY_SHIFT) || IsKeyPressed(KeyboardProxy::KEY_CTRL))
    {
        newSelection = currentSelection;
    }

    if (selectedNode != nullptr)
    {
        if (IsKeyPressed(KeyboardProxy::KEY_CTRL) && currentSelection.find(selectedNode) != currentSelection.end())
        {
            newSelection.erase(selectedNode);
        }
        else
        {
            newSelection.insert(selectedNode);
        }
    }

    SelectNodes(newSelection);
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
            //not top level node or it hierarchy. We don't search in background nodes
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
        return control->GetVisibilityFlag() && !control->IsHiddenForDebug() && control->IsPointInside(point);
    };

    auto stopPredicate = [](const ControlNode* node) -> bool {
        const UIControl* control = node->GetControl();
        DVASSERT(nullptr != control);
        return !control->GetVisibilityFlag() || control->IsHiddenForDebug();
    };
    systemsManager->CollectControlNodes(std::back_inserter(nodesUnderPoint), findPredicate, stopPredicate);
}

bool SelectionSystem::CanProcessInput(DAVA::UIEvent* currentInput) const
{
    EditorSystemsManager::eDisplayState displayState = systemsManager->GetDisplayState();
    EditorSystemsManager::eDragState dragState = systemsManager->GetDragState();
    return (displayState == EditorSystemsManager::Edit
            || displayState == EditorSystemsManager::Preview)
    && dragState == EditorSystemsManager::NoDrag
    && currentInput->device == eInputDevices::MOUSE
    && currentInput->mouseButton == DAVA::eMouseButtons::LEFT
    && currentInput->phase == UIEvent::Phase::BEGAN;
}

ControlNode* SelectionSystem::GetCommonNodeUnderPoint(const DAVA::Vector2& point, bool canGoDeeper) const
{
    Vector<ControlNode*> nodesUnderPoint;
    GetNodesForSelection(nodesUnderPoint, point);
    SelectedNodes selection = documentDataWrapper.GetFieldValue(DocumentData::selectionPropertyName).Cast<SelectedNodes>(SelectedNodes());
    //no selection. Search for the child of root under cursor
    if (nodesUnderPoint.empty())
    {
        return nullptr;
    }
    else if (nodesUnderPoint.size() == 1)
    {
        return nodesUnderPoint.front();
    }

    if (!selection.empty())
    {
        //collect all selected hierarchy
        SelectedNodes parentsOfSelectedNodes;
        for (PackageBaseNode* node : selection)
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

            if (selection.find(node) != selection.end())
            {
                return node;
            }

            //search child of selected to move down by hierarchy
            // or search neighbor to move left-right
            if (selection.find(nodeParent) != selection.end())
            {
                if (canGoDeeper)
                {
                    return node;
                }
                else
                {
                    DVASSERT(dynamic_cast<ControlNode*>(nodeParent) != nullptr);
                    return dynamic_cast<ControlNode*>(nodeParent);
                }
            }
            else if (selection.find(node) == selection.end()
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
    return nullptr;
}

ControlNode* SelectionSystem::GetNearestNodeUnderPoint(const DAVA::Vector2& point) const
{
    Vector<ControlNode*> nodesUnderPoint;
    GetNodesForSelection(nodesUnderPoint, point);
    return nodesUnderPoint.empty() ? nullptr : nodesUnderPoint.back();
}
