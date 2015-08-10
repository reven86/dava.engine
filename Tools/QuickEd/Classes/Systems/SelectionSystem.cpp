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


#include "Systems/SelectionSystem.h"
#include "Model/PackageHierarchy/ControlNode.h"
#include "UI/UIEvent.h"

using namespace DAVA;

bool SelectionSystem::OnInput(UIEvent* currentInput)
{    
    if (currentInput->phase == UIEvent::PHASE_BEGAN || currentInput->phase == UIEvent::PHASE_DRAG)
    {
        /*UIControl *control = controlsCanvas->GetControlByPos(this, currentInput->point);
        if (nullptr != control)
        {
            UIControl *rootControl = control;
            while (rootControl->GetParent() != nullptr && rootControl->GetParent() != this)
            {
                rootControl = rootControl->GetParent();
            }
            if (rootControl->GetParent() == this)
            {
                selectedControls.push_back(std::make_pair(rootControl, control));
            }
        }
        for (auto listener : selectionListeners)
        {
            listener->OnControlSelected(selectedControls);
        }*/
        return true;
    }
    return false;
}

void SelectionSystem::ControlWasRemoved(ControlNode *node, ControlsContainerNode *from)
{
    auto iter = std::find(selectedControls.begin(), selectedControls.end(), node);
    if (iter != selectedControls.end())
    {
        selectedControls.erase(iter);
        SelectedControls deselected;
        deselected.insert(*iter);
        for (auto listener : listeners)
        {
            listener->SelectionWasChanged(SelectedControls(), deselected);
        }
    }
}

void SelectionSystem::SelectionWasChanged(const SelectedControls &selected, const SelectedControls &deselected)
{
    selectedControls.insert(selected.begin(), selected.end());
    selectedControls.erase(deselected.begin(), deselected.end());
    for (auto listener : listeners)
    {
        listener->SelectionWasChanged(SelectedControls(), deselected);
    }
}

void SelectionSystem::AddListener(SelectionInterface *listener)
{
    listeners.push_back(listener);
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
        DVASSERT(false);
    }
}