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

#ifndef __QUICKED_SELECTION_SYSTEM_H__
#define __QUICKED_SELECTION_SYSTEM_H__

#include "EditorSystems/SelectionContainer.h"
#include "EditorSystems/BaseEditorSystem.h"
#include "Math/Rect.h"
#include "UI/UIEvent.h"
#include <Functional/SignalBase.h>
#include "Model/PackageHierarchy/PackageListener.h"

class EditorSystemsManager;
class ControlNode;
class ControlsContainerNode;

namespace DAVA
{
class Vector2;
}

class SelectionSystem final : public BaseEditorSystem, PackageListener
{
public:
    SelectionSystem(EditorSystemsManager* doc);
    ~SelectionSystem() override;

    void ClearSelection();
    void SelectAllControls();
    void FocusNextChild();
    void FocusPreviousChild();

private:
    bool OnInput(DAVA::UIEvent* currentInput) override;
    void OnPackageNodeChanged(PackageNode* packageNode);
    void ControlWasRemoved(ControlNode* node, ControlsContainerNode* from) override;
    void OnSelectByRect(const DAVA::Rect& rect);

    void FocusToChild(bool next);
    void OnSelectionChanged(const SelectedNodes& selected, const SelectedNodes& deselected);
    void SetSelection(const SelectedNodes& selected, const SelectedNodes& deselected);
    bool ProcessMousePress(const DAVA::Vector2& point, DAVA::UIEvent::MouseButton buttonID);

    bool mousePressed = false;
    SelectionContainer selectionContainer;
    PackageNode* packageNode = nullptr;
};

#endif // __QUICKED_SELECTION_SYSTEM_H__
