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

using namespace DAVA;

SelectionSystem::SelectionSystem(Document* doc)
    : document(doc)
{
    
}

bool SelectionSystem::OnInput(UIEvent* currentInput)
{
    switch(currentInput->phase)
    {
    case UIEvent::PHASE_BEGAN:
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

void SelectionSystem::SelectionWasChanged(const SelectedControls &selected, const SelectedControls &deselected)
{
    SetSelectedControls(selected, deselected);
}

void SelectionSystem::AddListener(SelectionInterface *listener)
{
    auto it = std::find(listeners.begin(), listeners.end(), listener);
    if (it == listeners.end())
    {
        listeners.push_back(listener);
    }
    else
    {
        DVASSERT_MSG(false, "listener has already attached");
    }
}

void SelectionSystem::RemoveListener(SelectionInterface *listener)
{
    auto it = std::find(listeners.begin(), listeners.end(), listener);
    if (it != listeners.end())
    {
        listeners.erase(it);
    }
    else
    {
        DVASSERT_MSG(false, "listener was not attached");
    }
}

bool SelectionSystem::ProcessMousePress(const DAVA::Vector2 &point)
{
    SelectedControls selected;
    SelectedControls deselected;
    auto node = document->GetControlNodeByPos(point);
    if (nullptr != node)
    {
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
        else
        {
            deselected = selectedControls;
            deselected.erase(node);
            selected.insert(node);
        }
        SetSelectedControls(selected, deselected);
    }
    return !selected.empty() && !deselected.empty();
}

void SelectionSystem::SetSelectedControls(const SelectedControls &selected, const SelectedControls &deselected)
{
    SelectedControls tmpSelected = selectedControls;
    UniteNodes(selected, tmpSelected);
    SubstractNodes(deselected, tmpSelected);
    if (selectedControls != tmpSelected)
    {
        selectedControls = tmpSelected;
        for (auto listener : listeners)
        {
            listener->SelectionWasChanged(selected, deselected);
        }
    }
}
