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
#include "Systems/SelectionSystem.h"
#include "Model/PackageHierarchy/ControlNode.h"
#include "UI/UIEvent.h"
#include "Systems/SystemsManager.h"

using namespace DAVA;

SelectionSystem::SelectionSystem(SystemsManager* systemManager)
    : BaseSystem(systemManager)
{
    systemManager->SelectionChanged.Connect(MakeFunction(&selectionTracker, &SelectionContainer::MergeSelection));
    systemManager->SelectionRectChanged.Connect(this, &SelectionSystem::OnSelectByRect);
}

bool SelectionSystem::OnInput(UIEvent* currentInput)
{
    switch(currentInput->phase)
    {
    case UIEvent::PHASE_BEGAN:
        mousePressed = true;
        return ProcessMousePress(currentInput->point);
    case UIEvent::PHASE_ENDED:
        if(!mousePressed)
        {
            return ProcessMousePress(currentInput->point);
        }
        mousePressed = false;
        return false;
    case UIEvent::PHASE_KEYCHAR:
    {
        if (currentInput->tid == DVKEY_TAB)
        {
            SetSelection(SelectedNodes(), selectionTracker.selectedNodes);
            //TODO: select next control
            return true;
        }
    }
    default: return false;
    }
    return false;
}

void SelectionSystem::ControlWasRemoved(ControlNode *node, ControlsContainerNode *from)
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
    systemManager->GetControlNodesByRect(areaNodes, rect);
    if (!areaNodes.empty())
    {
        for (auto node : areaNodes)
        {
            selected.insert(node);
        }
    }
    if (!InputSystem::Instance()->GetKeyboard().IsKeyPressed(DVKEY_SHIFT))
    {
        //deselect all not selected by rect
        std::set_difference(selectionTracker.selectedNodes.begin(), selectionTracker.selectedNodes.end(), areaNodes.begin(), areaNodes.end(), std::inserter(deselected, deselected.end()));
    }
    SetSelection(selected, deselected);
}

bool SelectionSystem::ProcessMousePress(const DAVA::Vector2 &point)
{
    SelectedNodes selected;
    SelectedNodes deselected;
    DAVA::Vector<ControlNode*> nodesUnderPoint;
    systemManager->GetControlNodesByPos(nodesUnderPoint, point);
    if(!InputSystem::Instance()->GetKeyboard().IsKeyPressed(DVKEY_SHIFT))
    {
        deselected = selectionTracker.selectedNodes;
    }
    if (!nodesUnderPoint.empty())
    {
        auto node = nodesUnderPoint.back();
        if (InputSystem::Instance()->GetKeyboard().IsKeyPressed(DVKEY_CTRL))
        {
            if (selectionTracker.selectedNodes.find(node) != selectionTracker.selectedNodes.end())
            {
                deselected.insert(node);
            }
            else
            {
                selected.insert(node);
            }
        }
        else if (InputSystem::Instance()->GetKeyboard().IsKeyPressed(DVKEY_ALT))
        {
            ControlNode *selectedNode = nullptr;
            systemManager->SelectionByMenuRequested.Emit(nodesUnderPoint, point, selectedNode);
            if(nullptr != selectedNode)
            {
                selected.insert(selectedNode);
            }
        }
        else
        {
            selected.insert(node);
        }
    }
    for(auto controlNode : selected)
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
    selectionTracker.GetOnlyExistedItems(deselected, reallyDeselected);
    selectionTracker.GetNotExistedItems(selected, reallySelected);
    selectionTracker.MergeSelection(reallySelected, reallyDeselected);

    if (!reallySelected.empty() || !reallyDeselected.empty())
    {
        systemManager->SelectionChanged.Emit(reallySelected, reallyDeselected);
    }
}
