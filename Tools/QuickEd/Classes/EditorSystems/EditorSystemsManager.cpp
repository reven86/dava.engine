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
#include "EditorSystems/KeyboardProxy.h"

#include "Model/PackageHierarchy/PackageNode.h"
#include "Model/PackageHierarchy/PackageControlsNode.h"
#include "Model/PackageHierarchy/ControlNode.h"
#include "Model/ControlProperties/RootProperty.h"

#include "EditorSystems/SelectionSystem.h"
#include "EditorSystems/CanvasSystem.h"
#include "EditorSystems/CursorSystem.h"
#include "EditorSystems/HUDSystem.h"
#include "EditorSystems/TransformSystem.h"

#include "UI/UIControl.h"

using namespace DAVA;

class EditorSystemsManager::RootControl : public UIControl
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
    , keyboardProxy(new KeyboardProxy)
    , package(SafeRetain(_package))
    , editingRootControls(CompareByLCA)
{
    rootControl->SetName("rootControl");
    rootControl->AddControl(scalableControl.Get());
    scalableControl->SetName("scalableContent");

    systems.emplace_back(new CanvasSystem(this));
    systems.emplace_back(new SelectionSystem(this));
    systems.emplace_back(new HUDSystem(this));
    systems.emplace_back(new CursorSystem(this));
    systems.emplace_back(new ::TransformSystem(this));

    SelectionChanged.Connect(this, &EditorSystemsManager::OnSelectionChanged);

    package->AddListener(this);
}

EditorSystemsManager::~EditorSystemsManager()
{
    package->RemoveListener(this);
    SafeRelease(package);
}

const KeyboardProxy* EditorSystemsManager::GetKeyboardProxy() const
{
    return keyboardProxy.get();
}

PackageNode* EditorSystemsManager::GetPackage()
{
    return package;
}

UIControl* EditorSystemsManager::GetRootControl()
{
    return rootControl.Get();
}

UIControl* EditorSystemsManager::GetScalableControl()
{
    return scalableControl.Get();
}

void EditorSystemsManager::Deactivate()
{
    for (auto& system : systems)
    {
        system->OnDeactivated();
    }
    rootControl->RemoveFromParent();
}

void EditorSystemsManager::Activate()
{
    for (auto& system : systems)
    {
        system->OnActivated();
    }
    if (editingRootControls.empty())
    {
        SetPreviewMode(true);
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

void EditorSystemsManager::SetEmulationMode(bool emulationMode)
{
    rootControl->SetEmulationMode(emulationMode);
    EmulationModeChangedSignal.Emit(std::move(emulationMode));
}

void EditorSystemsManager::OnSelectionChanged(const SelectedNodes& selected, const SelectedNodes& deselected)
{
    SelectionContainer::MergeSelectionAndContainer(selected, deselected, selectedControlNodes);
    if (!selectedControlNodes.empty())
    {
        SetPreviewMode(false);
    }
}

/*
void EditorSystemsManager::CollectControlNodesByPosImpl(Vector<ControlNode*>& controlNodes, const Vector2& pos, ControlNode* node) const
{
    int count = node->GetCount();
    auto control = node->GetControl();
    if (control->IsPointInside(pos) && control->GetSystemVisible())
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
}*/

void EditorSystemsManager::ControlWillBeRemoved(ControlNode* node, ControlsContainerNode* from)
{
    if (std::find(editingRootControls.begin(), editingRootControls.end(), node) != editingRootControls.end())
    {
        if (!previewMode)
        {
            recentlyRemovedControls.insert(node);
            if (editingRootControls.size() == 1)
            {
                SetPreviewMode(true);
            }
        }
        editingRootControls.erase(node);
        EditingRootControlsChanged.Emit(std::move(editingRootControls));
    }
}

void EditorSystemsManager::ControlWasAdded(ControlNode* node, ControlsContainerNode* destination, int)
{
    if (previewMode)
    {
        PackageControlsNode* packageControlsNode = package->GetPackageControlsNode();
        if (destination == packageControlsNode)
        {
            editingRootControls.insert(node);
            EditingRootControlsChanged.Emit(std::move(editingRootControls));
        }
    }
    else
    {
        if (recentlyRemovedControls.find(node) != recentlyRemovedControls.end())
        {
            PackageBaseNode* parent = node;
            while (parent->GetParent() != nullptr && parent->GetParent()->GetControl() != nullptr)
            {
                parent = parent->GetParent();
            }
            DVASSERT(nullptr != parent);
            editingRootControls.insert(parent);
            EditingRootControlsChanged.Emit(std::move(editingRootControls));
        }
    }
    recentlyRemovedControls.erase(node);
}

void EditorSystemsManager::SetPreviewMode(bool mode)
{
    previewMode = mode;
    editingRootControls.clear();
    if (previewMode)
    {
        PackageControlsNode* controlsNode = package->GetPackageControlsNode();
        for (int index = 0; index < controlsNode->GetCount(); ++index)
        {
            editingRootControls.insert(controlsNode->Get(index));
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
                editingRootControls.insert(root);
            }
        }
    }
    EditingRootControlsChanged.Emit(editingRootControls);
}
