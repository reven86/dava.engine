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
    document->SelectionChanged.Connect([this](const SelectedControls &selected, const SelectedControls &deselected)
        {
            this->MergeSelection(selected, deselected);
        });
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
            SetSelection(SelectedControls(), selectedItems);
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
    SelectedControls deselected;
    deselected.insert(node);
    SetSelection(SelectedControls(), deselected);
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
        //deselect all not selected by rect 
        std::set_difference(selectedItems.begin(), selectedItems.end(), areaNodes.begin(), areaNodes.end(), std::inserter(deselected, deselected.end()));
    }
    SetSelection(selected, deselected);
}

bool SelectionSystem::ProcessMousePress(const DAVA::Vector2 &point)
{
    SelectedControls selected;
    SelectedControls deselected;
    nodesUnderPoint.clear();
    document->GetControlNodesByPos(nodesUnderPoint, point);
    if(!InputSystem::Instance()->GetKeyboard().IsKeyPressed(DVKEY_SHIFT))
    {
        deselected = selectedItems;
    }
    if (!nodesUnderPoint.empty())
    {
        auto node = nodesUnderPoint.back();
        if (InputSystem::Instance()->GetKeyboard().IsKeyPressed(DVKEY_CTRL))
        {
            if (selectedItems.find(node) != selectedItems.end())
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
    SetSelection(selected, deselected);
    return !selected.empty() || !deselected.empty();
}

void SelectionSystem::SetSelection(const SelectedControls &selected, const SelectedControls &deselected)
{
    SelectedControls reallySelected;
    SelectedControls reallyDeselected;
    GetOnlyExistedItems(deselected, reallyDeselected);
    GetNotExistedItems(selected, reallySelected);
    MergeSelection(reallySelected, reallyDeselected);

    if (!reallySelected.empty() || !reallyDeselected.empty())
    {
        document->SelectionChanged.Emit(reallySelected, reallyDeselected);
    }
}
