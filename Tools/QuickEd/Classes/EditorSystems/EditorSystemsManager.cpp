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
#include "Model/ControlProperties/RootProperty.h"

#include "EditorSystems/SelectionSystem.h"
#include "EditorSystems/CanvasSystem.h"
#include "EditorSystems/CursorSystem.h"
#include "EditorSystems/HUDSystem.h"
#include "EditorSystems/TransformSystem.h"

using namespace DAVA;

namespace
{
class RootControl : public UIControl
{
public:
    RootControl(EditorSystemsManager* arg)
        : UIControl()
        , systemManager(arg)
    {
        DVASSERT(nullptr != systemManager);
    }
    bool SystemInput(UIEvent* currentInput) override
    {
        if (!emulationMode && nullptr != systemManager)
        {
            return systemManager->OnInput(currentInput);
        }
        return UIControl::SystemInput(currentInput);
    }
    void SetEmulationMode(bool arg)
    {
        emulationMode = arg;
    }

private:
    EditorSystemsManager* systemManager = nullptr;
    bool emulationMode = false;
};
};

bool CompareByLCA(PackageBaseNode* left, PackageBaseNode* right)
{
    DVASSERT(nullptr != left && nullptr != right);
    PackageBaseNode* leftParent = left;
    int depthLeft = 0;
    while (nullptr != leftParent->GetParent())
    {
        leftParent = leftParent->GetParent();
        ++depthLeft;
    }
    int depthRight = 0;
    PackageBaseNode* rightParent = right;
    while (nullptr != rightParent->GetParent())
    {
        rightParent = rightParent->GetParent();
        ++depthRight;
    }
    leftParent = left;
    rightParent = right;
    while (depthLeft != depthRight)
    {
        if (depthLeft > depthRight)
        {
            leftParent = leftParent->GetParent();
            --depthLeft;
        }
        else
        {
            rightParent = rightParent->GetParent();
            --depthRight;
        }
    }
    if (leftParent == right)
    {
        return false;
    }
    if (rightParent == left)
    {
        return true;
    }
    left = leftParent;
    right = rightParent;
    while (true)
    {
        leftParent = left->GetParent();
        rightParent = right->GetParent();
        DVASSERT(nullptr != leftParent && nullptr != rightParent)
        if (leftParent == rightParent)
        {
            return leftParent->GetIndex(left) < leftParent->GetIndex(right);
        }
        left = leftParent;
        right = rightParent;
    }
}

EditorSystemsManager::EditorSystemsManager(PackageNode* _package)
    : rootControl(new RootControl(this))
    , scalableControl(new UIControl())
    , package(SafeRetain(_package))
    , editingRootControls(CompareByLCA)
{
    rootControl->SetName("rootControl");
    rootControl->AddControl(scalableControl);
    scalableControl->SetName("scalableContent");

    systems.push_back(new CanvasSystem(this));
    systems.push_back(new SelectionSystem(this));
    systems.push_back(new HUDSystem(this));
    systems.push_back(new CursorSystem(this));
    systems.push_back(new ::TransformSystem(this));

    SelectionChanged.Connect(this, &EditorSystemsManager::OnSelectionChanged);

    package->AddListener(this);
}

EditorSystemsManager::~EditorSystemsManager()
{
    for (auto& system : systems)
    {
        delete system;
    }
    package->RemoveListener(this);
    SafeRelease(scalableControl);
    SafeRelease(rootControl);
    SafeRelease(package);
}

PackageNode* EditorSystemsManager::GetPackage()
{
    return package;
}

UIControl* EditorSystemsManager::GetRootControl()
{
    return rootControl;
}

UIControl* EditorSystemsManager::GetScalableControl()
{
    return scalableControl;
}

void EditorSystemsManager::Deactivate()
{
    for (auto& system : systems)
    {
        system->OnDeactivated();
    }
}

void EditorSystemsManager::Activate()
{
    for (auto system : systems)
    {
        system->OnActivated();
    }
    if (editingRootControls.empty())
    {
        PackageControlsNode* controlsNode = package->GetPackageControlsNode();
        for (int index = 0; index < controlsNode->GetCount(); ++index)
        {
            editingRootControls.insert(controlsNode->Get(index));
        }
        EditingRootControlsChanged.Emit(editingRootControls);
    }
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

void EditorSystemsManager::CollectControlNodesByPos(DAVA::Vector<ControlNode*>& controlNodes, const DAVA::Vector2& pos) const
{
    for (PackageBaseNode* rootControl : editingRootControls)
    {
        ControlNode* controlNode = dynamic_cast<ControlNode*>(rootControl);
        DVASSERT(nullptr != controlNode);
        CollectControlNodesByPosImpl(controlNodes, pos, controlNode);
    }
}

void EditorSystemsManager::CollectControlNodesByRect(SelectedControls& controlNodes, const Rect& rect) const
{
    for (PackageBaseNode* rootControl : editingRootControls)
    {
        ControlNode* controlNode = dynamic_cast<ControlNode*>(rootControl);
        DVASSERT(nullptr != controlNode);
        CollectControlNodesByRectImpl(controlNodes, rect, controlNode);
    }
}

void EditorSystemsManager::OnSelectionChanged(const SelectedNodes& selected, const SelectedNodes& deselected)
{
    SelectionContainer::MergeSelectionAndContainer(selected, deselected, selectedControlNodes);
    if (selectedControlNodes.empty())
    {
        return;
    }
    else
    {
        editingRootControls.clear();
        for (ControlNode* selectedControlNode : selectedControlNodes)
        {
            PackageBaseNode* root = static_cast<PackageBaseNode*>(selectedControlNode);
            while (nullptr != root->GetParent() && nullptr != root->GetParent()->GetControl())
            {
                root = root->GetParent();
            }
            if (nullptr != root)
            {
                editingRootControls.insert(root);
            }
        }
    }
    EditingRootControlsChanged.Emit(std::move(editingRootControls));
}

void EditorSystemsManager::CollectControlNodesByPosImpl(DAVA::Vector<ControlNode*>& controlNodes, const DAVA::Vector2& pos, ControlNode* node) const
{
    int count = node->GetCount();
    auto control = node->GetControl();
    if (control->IsPointInside(pos) && control->GetVisible() && control->GetVisibleForUIEditor())
    {
        controlNodes.push_back(node);
    }
    for (int i = 0; i < count; ++i)
    {
        CollectControlNodesByPosImpl(controlNodes, pos, node->Get(i));
    }
}

void EditorSystemsManager::CollectControlNodesByRectImpl(SelectedControls& controlNodes, const Rect& rect, ControlNode* node) const
{
    int count = node->GetCount();
    auto control = node->GetControl();
    if (control->GetVisible() && control->GetVisibleForUIEditor() && rect.RectContains(control->GetGeometricData().GetAABBox()))
    {
        controlNodes.insert(node);
    }
    for (int i = 0; i < count; ++i)
    {
        CollectControlNodesByRectImpl(controlNodes, rect, node->Get(i));
    }
}

void EditorSystemsManager::ControlWillBeRemoved(ControlNode* node, ControlsContainerNode* from)
{
    if (std::find(editingRootControls.begin(), editingRootControls.end(), node) != editingRootControls.end())
    {
        editingRootControls.erase(node);
        EditingRootControlsChanged.Emit(std::move(editingRootControls));
    }
}

void EditorSystemsManager::ControlWasAdded(ControlNode* node, ControlsContainerNode* destination, int)
{
    if (selectedControlNodes.empty())
    {
        PackageControlsNode* packageControlsNode = package->GetPackageControlsNode();
        if (destination == packageControlsNode)
        {
            editingRootControls.insert(node);
            EditingRootControlsChanged.Emit(std::move(editingRootControls));
        }
    }
}

void EditorSystemsManager::SetEmulationMode(bool emulationMode)
{
    auto root = static_cast<RootControl*>(rootControl);
    root->SetEmulationMode(emulationMode);
    EmulationModeChangedSignal.Emit(std::move(emulationMode));
}
