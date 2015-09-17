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

EditorSystemsManager::EditorSystemsManager(PackageNode* _package)
    : rootControl(new RootControl(this))
    , scalableControl(new UIControl())
    , package(SafeRetain(_package))
{
    rootControl->SetName("rootControl");
    rootControl->AddControl(scalableControl);
    scalableControl->SetName("scalableContent");

    systems.push_back(new SelectionSystem(this));
    systems.push_back(new CanvasSystem(this));
    systems.push_back(new HUDSystem(this));
    systems.push_back(new CursorSystem(this));
    systems.push_back(new ::TransformSystem(this));
}

EditorSystemsManager::~EditorSystemsManager()
{
    for (auto& system : systems)
    {
        delete system;
    }
}

PackageNode* EditorSystemsManager::GetPackage()
{
    return package;
}

bool EditorSystemsManager::IsInEmulationMode() const
{
    return emulationMode;
}

UIControl* EditorSystemsManager::GetRootControl()
{
    return rootControl;
}

DAVA::UIControl* EditorSystemsManager::GetScalableControl()
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

void EditorSystemsManager::GetControlNodesByPos(DAVA::Vector<ControlNode*>& controlNodes, const DAVA::Vector2& pos) const
{
    auto controlsNode = package->GetPackageControlsNode();
    for (int index = 0; index < controlsNode->GetCount(); ++index)
    {
        auto tmpNode = controlsNode->Get(index);
        DVASSERT(nullptr != tmpNode);
        auto control = tmpNode->GetControl();
        if (control->GetParent() != nullptr)
        {
            GetControlNodesByPosImpl(controlNodes, pos, tmpNode);
        }
    }
}

void EditorSystemsManager::GetControlNodesByRect(SelectedControls& controlNodes, const Rect& rect) const
{
    auto controlsNode = package->GetPackageControlsNode();
    for (int index = 0; index < controlsNode->GetCount(); ++index)
    {
        auto tmpNode = controlsNode->Get(index);
        DVASSERT(nullptr != tmpNode);
        auto control = tmpNode->GetControl();
        if (control->GetParent() != nullptr)
        {
            GetControlNodesByRectImpl(controlNodes, rect, tmpNode);
        }
    }
}

void EditorSystemsManager::GetControlNodesByPosImpl(DAVA::Vector<ControlNode*>& controlNodes, const DAVA::Vector2& pos, ControlNode* node) const
{
    int count = node->GetCount();
    auto control = node->GetControl();
    if (control->IsPointInside(pos) && control->GetVisible() && control->GetVisibleForUIEditor())
    {
        controlNodes.push_back(node);
    }
    for (int i = 0; i < count; ++i)
    {
        GetControlNodesByPosImpl(controlNodes, pos, node->Get(i));
    }
}

void EditorSystemsManager::GetControlNodesByRectImpl(SelectedControls& controlNodes, const Rect& rect, ControlNode* node) const
{
    int count = node->GetCount();
    auto control = node->GetControl();
    if (control->GetVisible() && control->GetVisibleForUIEditor() && rect.RectContains(control->GetGeometricData().GetAABBox()))
    {
        controlNodes.insert(node);
    }
    for (int i = 0; i < count; ++i)
    {
        GetControlNodesByRectImpl(controlNodes, rect, node->Get(i));
    }
}

void EditorSystemsManager::SetEmulationMode(bool arg)
{
    if (emulationMode != arg)
    {
        emulationMode = arg;

        auto root = static_cast<RootControl*>(rootControl);
        root->SetEmulationMode(emulationMode);
        EmulationModeChangedSignal.Emit(std::move(emulationMode));
    }
}
