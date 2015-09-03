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
#include "Document.h"

using namespace DAVA;

SelectionSystem::SelectionSystem(Document* doc)
    : BaseSystem(doc)
{
    
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
            SetSelectedControls(SelectedControls(), selectedControls);
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
    auto iter = std::find(selectedControls.begin(), selectedControls.end(), node);
    if (iter != selectedControls.end())
    {
        SelectedControls deselected;
        deselected.insert(*iter);
        SetSelectedControls(SelectedControls(), deselected);
    }
}

void SelectionSystem::OnSelectionWasChanged(const SelectedControls &selected, const SelectedControls &deselected)
{
    SetSelectedControls(selected, deselected);
}

void SelectionSystem::SelectByRect(const Rect& rect)
{
    SelectedControls deselected;
    SelectedControls selected;
    Set<ControlNode*> areaNodes;
    document->GetControlNodesByRect(areaNodes, rect);
    if (!areaNodes.empty())
    {
        for (auto node : areaNodes)
        {
            selected.insert(node);
        }
    }
    if (!InputSystem::Instance()->GetKeyboard().IsKeyPressed(DVKEY_SHIFT))
    {
        std::set_difference(selectedControls.begin(), selectedControls.end(), areaNodes.begin(), areaNodes.end(), std::inserter(deselected, deselected.end()));
    }
    SetSelectedControls(selected, deselected);
}

bool SelectionSystem::ProcessMousePress(const DAVA::Vector2 &point)
{
    SelectedControls selected;
    SelectedControls deselected;
    nodesUnderPoint.clear();
    document->GetControlNodesByPos(nodesUnderPoint, point);
    if(!InputSystem::Instance()->GetKeyboard().IsKeyPressed(DVKEY_SHIFT))
    {
        deselected = selectedControls;
    }
    if (!nodesUnderPoint.empty())
    {
        auto node = nodesUnderPoint.back();
        if (InputSystem::Instance()->GetKeyboard().IsKeyPressed(DVKEY_CTRL))
        {
            if (selectedControls.find(node) != selectedControls.end())
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
            auto controlNode = document->GetControlByMenu(nodesUnderPoint, point);
            if(nullptr != controlNode)
            {
                selected.insert(controlNode);
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
    SetSelectedControls(selected, deselected);
    return !selected.empty() || !deselected.empty();
}

void SelectionSystem::SetSelectedControls(const SelectedControls &selected, const SelectedControls &deselected)
{
    SelectedControls reallySelected;
    SelectedControls reallyDeselected;
    
    std::set_intersection(selectedControls.begin(), selectedControls.end(), deselected.begin(), deselected.end(), std::inserter(reallyDeselected, reallyDeselected.end()));

    std::set_difference(selected.begin(), selected.end(), selectedControls.begin(), selectedControls.end(), std::inserter(reallySelected, reallySelected.end()));
    
    for(const auto &controlNode : reallyDeselected)
    {
        selectedControls.erase(controlNode);
    }
    for(const auto &controlNode : reallySelected)
    {
        selectedControls.insert(controlNode);
    }

    if (!reallySelected.empty() || !reallyDeselected.empty())
    {
        SelectionWasChanged.Emit(reallySelected, reallyDeselected);
    }
}
