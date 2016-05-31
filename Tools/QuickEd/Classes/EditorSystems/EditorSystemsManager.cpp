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
#include "UI/Focus/UIFocusComponent.h"

using namespace DAVA;

EditorSystemsManager::StopPredicate EditorSystemsManager::defaultStopPredicate = [](const ControlNode*) { return false; };

class EditorSystemsManager::RootControl : public UIControl
{
public:
    RootControl(EditorSystemsManager* arg);
    void SetEmulationMode(bool arg);

private:
    bool SystemInput(UIEvent* currentInput) override;
    bool SystemProcessInput(UIEvent* currentInput) override;

    EditorSystemsManager* systemManager = nullptr;
    bool emulationMode = false;
    Vector2 prevPosition;
};

EditorSystemsManager::RootControl::RootControl(EditorSystemsManager* arg)
    : UIControl()
    , systemManager(arg)
{
    GetOrCreateComponent<UIFocusComponent>();
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
        return SystemProcessInput(currentInput);
    }
    return UIControl::SystemInput(currentInput);
}

bool EditorSystemsManager::RootControl::SystemProcessInput(UIEvent* currentInput)
{
    if (!emulationMode && nullptr != systemManager)
    {
        return systemManager->OnInput(currentInput);
    }

    return UIControl::SystemProcessInput(currentInput);
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

    selectionSystemPtr = new SelectionSystem(this);
    systems.emplace_back(selectionSystemPtr);
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

uint32 EditorSystemsManager::GetIndexOfNearestControl(const DAVA::Vector2& point) const
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
    for (uint32 i = 0, count = controlsNode->GetCount(); i < count; ++i)
    {
        if (controlsNode->Get(i) == target)
        {
            return insertToEnd ? i + 1 : i;
        }
    }
    DVASSERT(false && "editingRootControls contains nodes not from GetPackageControlsNode");

    return 0;
}

void EditorSystemsManager::SelectAll()
{
    selectionSystemPtr->SelectAllControls();
}

void EditorSystemsManager::FocusNextChild()
{
    selectionSystemPtr->FocusNextChild();
}

void EditorSystemsManager::FocusPreviousChild()
{
    selectionSystemPtr->FocusPreviousChild();
}

void EditorSystemsManager::ClearSelection()
{
    selectionSystemPtr->ClearSelection();
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
