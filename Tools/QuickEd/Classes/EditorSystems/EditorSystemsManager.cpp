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

#include "EditorSystems/EditorSystemsManager.h"

#include "Model/PackageHierarchy/PackageNode.h"
#include "Model/PackageHierarchy/PackageControlsNode.h"
#include "Model/PackageHierarchy/ControlNode.h"

#include "EditorSystems/SelectionSystem.h"
#include "EditorSystems/CanvasSystem.h"
#include "EditorSystems/CursorSystem.h"
#include "EditorSystems/HUDSystem.h"
#include "EditorSystems/EditorTransformSystem.h"

#include "UI/UIControl.h"

using namespace DAVA;

EditorSystemsManager::StopPredicate EditorSystemsManager::defaultStopPredicate = [](const ControlNode*) { return false; };

class EditorSystemsManager::RootControl : public UIControl
{
public:
    RootControl(EditorSystemsManager* arg);
    void SetEmulationMode(bool arg);

private:
    bool SystemInput(UIEvent* currentInput) override;

    EditorSystemsManager* systemManager = nullptr;
    bool emulationMode = false;
    Vector2 prevPosition;
};

EditorSystemsManager::RootControl::RootControl(EditorSystemsManager* arg)
    : UIControl()
    , systemManager(arg)
{
    DVASSERT(nullptr != systemManager);
}

void EditorSystemsManager::RootControl::SetEmulationMode(bool arg)
{
    emulationMode = arg;
}

bool EditorSystemsManager::RootControl::SystemInput(UIEvent* currentInput)
{
    if (!emulationMode && nullptr != systemManager)
    {
        return systemManager->OnInput(currentInput);
    }
    return UIControl::SystemInput(currentInput);
}

EditorSystemsManager::EditorSystemsManager()
    : rootControl(new RootControl(this))
    , scalableControl(new UIControl())
    , editingRootControls(CompareByLCA)
{
    rootControl->SetName(FastName("rootControl"));
    rootControl->AddControl(scalableControl.Get());
    scalableControl->SetName(FastName("scalableContent"));

    PackageNodeChanged.Connect(this, &EditorSystemsManager::OnPackageNodeChanged);
    SelectionChanged.Connect(this, &EditorSystemsManager::OnSelectionChanged);

    canvasSystemPtr = new CanvasSystem(this);
    systems.emplace_back(canvasSystemPtr);

    systems.emplace_back(new SelectionSystem(this));
    systems.emplace_back(new HUDSystem(this));
    systems.emplace_back(new CursorSystem(this));
    systems.emplace_back(new ::EditorTransformSystem(this));
}

EditorSystemsManager::~EditorSystemsManager() = default;

UIControl* EditorSystemsManager::GetRootControl() const
{
    return rootControl.Get();
}

UIControl* EditorSystemsManager::GetScalableControl() const
{
    return scalableControl.Get();
}

bool EditorSystemsManager::OnInput(UIEvent* currentInput)
{
    for (auto it = systems.rbegin(); it != systems.rend(); ++it)
    {
        if ((*it)->OnInput(currentInput))
        {
            return true;
        }
    }
    return false;
}

void EditorSystemsManager::SetEmulationMode(bool emulationMode)
{
    rootControl->SetEmulationMode(emulationMode);
    EmulationModeChangedSignal.Emit(emulationMode);
}

ControlNode* EditorSystemsManager::ControlNodeUnderPoint(const DAVA::Vector2& point) const
{
    Vector<ControlNode*> nodesUnderPoint;
    auto predicate = [point](const ControlNode* node) -> bool {
        auto control = node->GetControl();
        DVASSERT(control != nullptr);
        return control->IsVisible() && control->IsPointInside(point);
    };
    CollectControlNodes(std::back_inserter(nodesUnderPoint), predicate);
    return nodesUnderPoint.empty() ? nullptr : nodesUnderPoint.back();
}

int EditorSystemsManager::GetIndexOfNearestControl(const DAVA::Vector2& point) const
{
    if (editingRootControls.empty())
    {
        return 0;
    }
    uint32 index = canvasSystemPtr->GetIndexByPos(point);
    bool insertToEnd = (index == editingRootControls.size());

    auto iter = editingRootControls.begin();
    std::advance(iter, insertToEnd ? index - 1 : index);
    PackageBaseNode* target = *iter;
    PackageControlsNode* controlsNode = package->GetPackageControlsNode();
    for (int i = 0; i < controlsNode->GetCount(); ++i)
    {
        if (controlsNode->Get(i) == target)
        {
            return insertToEnd ? i + 1 : i;
        }
    }
    DVASSERT(false && "editingRootControls contains nodes not from GetPackageControlsNode");

    return 0;
}

void EditorSystemsManager::OnSelectionChanged(const SelectedNodes& selected, const SelectedNodes& deselected)
{
    SelectionContainer::MergeSelectionToContainer(selected, deselected, selectedControlNodes);
    if (!selectedControlNodes.empty())
    {
        SetPreviewMode(false);
    }
}

void EditorSystemsManager::OnPackageNodeChanged(PackageNode* package_)
{
    if (nullptr != package)
    {
        package->RemoveListener(this);
    }
    package = package_;
    SetPreviewMode(true);
    if (nullptr != package)
    {
        package->AddListener(this);
    }
}

void EditorSystemsManager::ControlWasRemoved(ControlNode* node, ControlsContainerNode* /*from*/)
{
    if (std::find(editingRootControls.begin(), editingRootControls.end(), node) != editingRootControls.end())
    {
        if (!previewMode && editingRootControls.size() == 1)
        {
            SetPreviewMode(true);
        }
        else
        {
            editingRootControls.erase(node);
            EditingRootControlsChanged.Emit(editingRootControls);
        }
    }
}

void EditorSystemsManager::ControlWasAdded(ControlNode* node, ControlsContainerNode* destination, int)
{
    if (previewMode)
    {
        DVASSERT(nullptr != package);
        if (nullptr != package)
        {
            PackageControlsNode* packageControlsNode = package->GetPackageControlsNode();
            if (destination == packageControlsNode)
            {
                editingRootControls.insert(node);
                EditingRootControlsChanged.Emit(editingRootControls);
            }
        }
    }
}

void EditorSystemsManager::SetPreviewMode(bool mode)
{
    previewMode = mode;
    RefreshRootControls();
}

void EditorSystemsManager::RefreshRootControls()
{
    SortedPackageBaseNodeSet newRootControls(CompareByLCA);

    if (previewMode)
    {
        if (nullptr != package)
        {
            PackageControlsNode* controlsNode = package->GetPackageControlsNode();
            for (int index = 0; index < controlsNode->GetCount(); ++index)
            {
                newRootControls.insert(controlsNode->Get(index));
            }
        }
    }
    else
    {
        for (ControlNode* selectedControlNode : selectedControlNodes)
        {
            PackageBaseNode* root = static_cast<PackageBaseNode*>(selectedControlNode);
            while (nullptr != root->GetParent() && nullptr != root->GetParent()->GetControl())
            {
                root = root->GetParent();
            }
            if (nullptr != root)
            {
                newRootControls.insert(root);
            }
        }
    }
    if (editingRootControls != newRootControls)
    {
        editingRootControls = newRootControls;
        EditingRootControlsChanged.Emit(editingRootControls);
    }
}
