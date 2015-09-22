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

#ifndef __QUICKED_CANVAS_SYSTEM_H__
#define __QUICKED_CANVAS_SYSTEM_H__

#include "EditorSystems/BaseEditorSystem.h"
#include "EditorSystems/EditorSystemsManager.h"
#include "Model/PackageHierarchy/PackageListener.h"
#include "Base/ScopedPtr.h"
#include "UI/UIControl.h"
#include "SelectionContainer.h"

class EditorSystemsManager;
class PackageBaseNode;
class GridControl;

class CanvasSystem final : public BaseEditorSystem, private PackageListener
{
public:
    CanvasSystem(EditorSystemsManager* parent);
    ~CanvasSystem() override;

    void OnActivated() override;
    void OnDeactivated() override;

    void LayoutCanvas();

private:
    void OnRootContolsChanged(const EditorSystemsManager::SortedPackageBaseNodeSet& rootControls);
    void ControlWasRemoved(ControlNode* node, ControlsContainerNode* from) override;
    void ControlWasAdded(ControlNode* node, ControlsContainerNode* /*destination*/, int /*index*/) override;
    void ControlPropertyWasChanged(ControlNode* node, AbstractProperty* property) override;
    void CreateAndInsertGrid(PackageBaseNode* node, size_t pos);

    DAVA::ScopedPtr<DAVA::UIControl> controlsCanvas; //to attach or detach from document
    DAVA::List<GridControl*> gridControls;
};

#endif // __QUICKED_CANVAS_SYSTEM_H__
