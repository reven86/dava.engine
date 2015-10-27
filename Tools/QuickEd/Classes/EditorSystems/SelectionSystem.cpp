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
#include "EditorSystems/SelectionSystem.h"
#include "Model/PackageHierarchy/ControlNode.h"
#include "UI/UIEvent.h"
#include "EditorSystems/EditorSystemsManager.h"
#include "EditorSystems/KeyboardProxy.h"
#include "Model/PackageHierarchy/PackageNode.h"
#include "Model/PackageHierarchy/PackageControlsNode.h"

using namespace DAVA;

namespace
{
PackageBaseNode* FindFirstChildWithControl(PackageBaseNode* node, bool forward)
{
    DVASSERT(node != nullptr);
    if (node->GetControl() != nullptr)
    {
        return node;
    }
    const uint32 count = node->GetCount();
    const uint32 begin = 0;
    const uint32 end = count;
    const int sign = forward ? 1 : -1;
    const uint32 beginPos = forward ? begin : (end - 1);
    for (int i = begin; i < end; ++i)
    {
        PackageBaseNode* child = FindFirstChildWithControl(node->Get(beginPos + sign * i), forward);
        if (nullptr != child)
        {
            return child;
        }
    }
    return nullptr;
}

PackageBaseNode* FindNeighbour(PackageBaseNode* node, bool forward)
{
    PackageBaseNode* parent = node->GetParent();
    if (parent != nullptr)
    {
        const uint32 count = parent->GetCount();
        const uint32 begin = 0;
        const uint32 end = count;
        const int sign = forward ? 1 : -1;
        const uint32 beginPos = forward ? begin : (end - 1);
        for (int i = 0; i < count - 1; ++i)
        {
            if (node == parent->Get(beginPos + sign * i))
            {
                return parent->Get(i + sign);
            }
        }
        if (parent->GetControl() != nullptr)
        {
            return FindNeighbour(parent, forward);
        }
    }
    return nullptr;
}

PackageBaseNode* GetNextControl(PackageBaseNode* node, bool forward)
{
    DVASSERT(node != nullptr);
    if (node->GetCount() > 0)
    {
        PackageBaseNode* child = FindFirstChildWithControl(node->Get(0), forward);
        if (nullptr != child)
        {
            return child;
        }
    }

    //no child with controls
    PackageBaseNode* neighbour = FindNeighbour(node, forward);
    if (nullptr != neighbour)
    {
        if (neighbour->GetControl() != nullptr)
        {
            return neighbour;
        }
        PackageBaseNode* child = FindFirstChildWithControl(neighbour, forward);
        if (nullptr != child)
        {
            return child;
        }
    }
    while (node->GetParent() != nullptr && node->GetParent()->GetControl())
    {
        node = node->GetParent();
    }
    DVASSERT(node->GetParent() != nullptr)
    return node->GetParent()->Get(0);
}
} //unnamed namespace

SelectionSystem::SelectionSystem(EditorSystemsManager* parent)
    : BaseEditorSystem(parent)
{
    systemManager->GetPackage()->AddListener(this);
    systemManager->SelectionRectChanged.Connect(this, &SelectionSystem::OnSelectByRect);
}

SelectionSystem::~SelectionSystem()
{
    PackageNode* package = systemManager->GetPackage();
    if (nullptr != package)
    {
        systemManager->GetPackage()->RemoveListener(this);
    }
}

void SelectionSystem::OnActivated()
{
    systemManager->SelectionChanged.Emit(selectionContainer.selectedNodes, SelectedNodes());
    connectionID = systemManager->SelectionChanged.Connect(this, &SelectionSystem::SetSelection);
}

void SelectionSystem::OnDeactivated()
{
    systemManager->SelectionChanged.Disconnect(connectionID);
    systemManager->SelectionChanged.Emit(SelectedNodes(), selectionContainer.selectedNodes);
}

bool SelectionSystem::OnInput(UIEvent* currentInput)
{
    switch (currentInput->phase)
    {
    case UIEvent::PHASE_BEGAN:
        mousePressed = true;
        return ProcessMousePress(currentInput->point);

    case UIEvent::PHASE_ENDED:
        if (!mousePressed)
        {
            return ProcessMousePress(currentInput->point);
        }
        mousePressed = false;
        return false;

    case UIEvent::PHASE_KEYCHAR:
    {
        if (currentInput->tid == DVKEY_TAB)
        {
            PackageBaseNode* startNode = nullptr;
            if (selectionContainer.selectedNodes.empty())
            {
                PackageControlsNode* controlsNode = systemManager->GetPackage()->GetPackageControlsNode();
                DVASSERT(dynamic_cast<PackageBaseNode*>(controlsNode) != nullptr);
                startNode = static_cast<PackageBaseNode*>(controlsNode);
            }
            else
            {
                startNode = *selectionContainer.selectedNodes.rbegin();
            }
            PackageBaseNode* nextNode = GetNextControl(startNode, true);
            SelectedNodes newSelectedNodes;
            newSelectedNodes.insert(nextNode);
            SetSelection(newSelectedNodes, selectionContainer.selectedNodes);
            return true;
        }
    }
    default:
        return false;
    }
    return false;
}

void SelectionSystem::ControlWasRemoved(ControlNode* node, ControlsContainerNode* from)
{
    SelectedNodes deselected;
    deselected.insert(node);
    SetSelection(SelectedNodes(), deselected);
}

void SelectionSystem::OnSelectByRect(const Rect& rect)
{
    SelectedNodes deselected;
    SelectedNodes selected;
    Set<ControlNode*> areaNodes;
    systemManager->CollectControlNodesByRect(areaNodes, rect);
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
    SetSelection(selected, deselected);
}

bool SelectionSystem::ProcessMousePress(const DAVA::Vector2& point)
{
    SelectedNodes selected;
    SelectedNodes deselected;
    DAVA::Vector<ControlNode*> nodesUnderPoint;
    systemManager->CollectControlNodesByPos(nodesUnderPoint, point);
    if (!IsKeyPressed(KeyboardProxy::KEY_SHIFT) && !IsKeyPressed(KeyboardProxy::KEY_CTRL))
    {
        deselected = selectionContainer.selectedNodes;
    }
    if (!nodesUnderPoint.empty())
    {
        auto node = nodesUnderPoint.back();
        if (IsKeyPressed(KeyboardProxy::KEY_ALT))
        {
            ControlNode* selectedNode = nullptr;
            systemManager->SelectionByMenuRequested.Emit(nodesUnderPoint, point, selectedNode);
            if (nullptr != selectedNode)
            {
                node = selectedNode;
            }
            else
            {
                return true; //selection was required but cancelled
            }
        }
        if (IsKeyPressed(KeyboardProxy::KEY_CTRL) && selectionContainer.IsSelected(node))
        {
            deselected.insert(node);
        }
        else
        {
            selected.insert(node);
        }
    }
    for (auto controlNode : selected)
    {
        deselected.erase(controlNode);
    }
    SetSelection(selected, deselected);
    return !selected.empty() || !deselected.empty();
}

void SelectionSystem::SetSelection(const SelectedNodes& selected, const SelectedNodes& deselected)
{
    SelectedNodes reallySelected;
    SelectedNodes reallyDeselected;
    selectionContainer.GetOnlyExistedItems(deselected, reallyDeselected);
    selectionContainer.GetNotExistedItems(selected, reallySelected);
    selectionContainer.MergeSelection(reallySelected, reallyDeselected);

    if (!reallySelected.empty() || !reallyDeselected.empty())
    {
        systemManager->SelectionChanged.Emit(reallySelected, reallyDeselected);
    }
}
