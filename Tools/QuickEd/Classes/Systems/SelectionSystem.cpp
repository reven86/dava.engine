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
#include "Defines.h"
#include <QtWidgets/QMenu>

using namespace DAVA;

SelectionSystem::SelectionSystem(Document* doc)
    : BaseSystemClass(doc)
{
    
}

bool SelectionSystem::OnInput(UIEvent* currentInput, bool forUpdate)
{
    if (forUpdate)
    {
        return false;
    }
    switch(currentInput->phase)
    {
    case UIEvent::PHASE_BEGAN:
        return ProcessMousePress(currentInput->point);
    case UIEvent::PHASE_ENDED:
        return ProcessMousePress(currentInput->point);
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
    if (!nodesUnderPoint.empty())
    {
        auto node = nodesUnderPoint.back();
        if (InputSystem::Instance()->GetKeyboard().IsKeyPressed(DVKEY_SHIFT))
        {
            selected.insert(node);
        }
        else if (InputSystem::Instance()->GetKeyboard().IsKeyPressed(DVKEY_CTRL))
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
            document->SelectControlByPos(nodesUnderPoint, point);
        }
        else
        {
            deselected = selectedControls;
            deselected.erase(node);
            selected.insert(node);
        }
    }
    else if(!InputSystem::Instance()->GetKeyboard().IsKeyPressed(DVKEY_SHIFT))
    {
        deselected = selectedControls;
    }
    SetSelectedControls(selected, deselected);
    return !selected.empty() || !deselected.empty();
}

void SelectionSystem::SetSelectedControls(const SelectedControls &selected, const SelectedControls &deselected)
{
    SelectedControls reallySelected = selected;
    SelectedControls reallyDeselected = deselected;
    
    for (auto control : deselected)
    {
        if (selectedControls.find(control) == selectedControls.end())
        {
            reallyDeselected.erase(control);
        }
    }
    SubstractSets(reallyDeselected, selectedControls);

    for (auto control : selected)
    {
        if (selectedControls.find(control) != selectedControls.end())
        {
            reallySelected.erase(control);
        }
    }
    UniteSets(reallySelected, selectedControls);

    if (!reallySelected.empty() || !reallyDeselected.empty())
    {
        SelectionWasChanged.Emit(reallySelected, reallyDeselected);
    }
}
