#include "EditorSystems/EditorSystemsManager.h"
#include "Modules/DocumentsModule/DocumentData.h"

#include "Model/PackageHierarchy/PackageNode.h"
#include "Model/PackageHierarchy/PackageControlsNode.h"
#include "Model/PackageHierarchy/ControlNode.h"

#include "EditorSystems/SelectionSystem.h"
#include "EditorSystems/EditorControlsView.h"
#include "EditorSystems/HUDSystem.h"
#include "EditorSystems/EditorTransformSystem.h"
#include "EditorSystems/KeyboardProxy.h"
#include "EditorSystems/EditorControlsView.h"

#include <TArc/Core/ContextAccessor.h>
#include <TArc/DataProcessing/DataContext.h>

#include <UI/UIControl.h>
#include <UI/Input/UIModalInputComponent.h>
#include <UI/Input/UIInputSystem.h>
#include <UI/UIControlSystem.h>
#include <UI/UIScreen.h>
#include <UI/UIScreenManager.h>
#include <Engine/Engine.h>

using namespace DAVA;

EditorSystemsManager::StopPredicate EditorSystemsManager::defaultStopPredicate = [](const ControlNode*) { return false; };

namespace EditorSystemsManagerDetails
{
class InputLayerControl : public UIControl
{
public:
    InputLayerControl(EditorSystemsManager* systemManager_)
        : UIControl()
        , systemManager(systemManager_)
    {
        GetOrCreateComponent<UIModalInputComponent>();
    }

    bool SystemProcessInput(UIEvent* currentInput) override
    {
        //redirect input from the framework to the editor
        systemManager->OnInput(currentInput);
        return true;
    }

private:
    EditorSystemsManager* systemManager = nullptr;
};
}

EditorSystemsManager::EditorSystemsManager(DAVA::TArc::ContextAccessor* accessor)
    : rootControl(new UIControl())
    , inputLayerControl(new EditorSystemsManagerDetails::InputLayerControl(this))
    , scalableControl(new UIControl())
    , editingRootControls(CompareByLCA)
{
    using namespace DAVA;
    using namespace TArc;

    dragStateChanged.Connect(this, &EditorSystemsManager::OnDragStateChanged);
    displayStateChanged.Connect(this, &EditorSystemsManager::OnDisplayStateChanged);
    activeAreaChanged.Connect(this, &EditorSystemsManager::OnActiveHUDAreaChanged);
    editingRootControlsChanged.Connect(this, &EditorSystemsManager::OnEditingRootControlsChanged);

    rootControl->SetName(FastName("rootControl"));
    rootControl->AddControl(scalableControl.Get());
    inputLayerControl->SetName("inputLayerControl");
    rootControl->AddControl(inputLayerControl.Get());
    scalableControl->SetName(FastName("scalableContent"));

    documentDataWrapper = accessor->CreateWrapper(ReflectedTypeDB::Get<DocumentData>());
    documentDataWrapper.SetListener(this);

    InitDAVAScreen();

    packageChanged.Connect(this, &EditorSystemsManager::OnPackageChanged);

    controlViewPtr = new EditorControlsView(scalableControl.Get(), this);
    systems.emplace_back(controlViewPtr);

    selectionSystemPtr = new SelectionSystem(this, accessor);
    systems.emplace_back(selectionSystemPtr);
    systems.emplace_back(new HUDSystem(this));
    systems.emplace_back(new ::EditorTransformSystem(this, accessor));

    for (auto it = systems.begin(); it != systems.end(); ++it)
    {
        const std::unique_ptr<BaseEditorSystem>& editorSystem = *it;
        BaseEditorSystem* editorSystemPtr = editorSystem.get();
        dragStateChanged.Connect(editorSystemPtr, &BaseEditorSystem::OnDragStateChanged);
        displayStateChanged.Connect(editorSystemPtr, &BaseEditorSystem::OnDisplayStateChanged);
    }
}

EditorSystemsManager::~EditorSystemsManager()
{
    const EngineContext* engineContext = GetEngineContext();
    engineContext->uiScreenManager->ResetScreen();
}

void EditorSystemsManager::OnInput(UIEvent* currentInput)
{
    if (currentInput->device == eInputDevices::MOUSE)
    {
        mouseDelta = currentInput->point - lastMousePos;
        lastMousePos = currentInput->point;
    }

    eDragState newState = NoDrag;
    for (auto it = systems.rbegin(); it != systems.rend(); ++it)
    {
        const std::unique_ptr<BaseEditorSystem>& editorSystem = *it;
        newState = Max(newState, editorSystem->RequireNewState(currentInput));
    }
    SetDragState(newState);

    for (auto it = systems.rbegin(); it != systems.rend(); ++it)
    {
        const std::unique_ptr<BaseEditorSystem>& editorSystem = *it;
        if (editorSystem->CanProcessInput(currentInput))
        {
            editorSystem->ProcessInput(currentInput);
        }
    }
}

void EditorSystemsManager::HighlightNode(ControlNode* node)
{
    highlightNode.Emit(node);
}

void EditorSystemsManager::ClearHighlight()
{
    highlightNode.Emit(nullptr);
}

void EditorSystemsManager::SetEmulationMode(bool emulationMode)
{
    SetDisplayState(emulationMode ? Emulation : previousDisplayState);
}

ControlNode* EditorSystemsManager::GetControlNodeAtPoint(const DAVA::Vector2& point) const
{
    if (!KeyboardProxy::IsKeyPressed(KeyboardProxy::KEY_ALT))
    {
        return selectionSystemPtr->GetCommonNodeUnderPoint(point);
    }
    return selectionSystemPtr->GetNearestNodeUnderPoint(point);
}

uint32 EditorSystemsManager::GetIndexOfNearestRootControl(const DAVA::Vector2& point) const
{
    if (editingRootControls.empty())
    {
        return 0;
    }
    uint32 index = controlViewPtr->GetIndexByPos(point);
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

    return controlsNode->GetCount();
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

void EditorSystemsManager::SelectNode(ControlNode* node)
{
    selectionSystemPtr->SelectNode(node);
}

const SortedControlNodeSet& EditorSystemsManager::GetEditingRootControls() const
{
    return editingRootControls;
}

void EditorSystemsManager::SetDisplayState(eDisplayState newDisplayState)
{
    if (displayState == newDisplayState)
    {
        return;
    }

    previousDisplayState = displayState;
    displayState = newDisplayState;
    displayStateChanged.Emit(displayState, previousDisplayState);
}

void EditorSystemsManager::OnEditingRootControlsChanged(const SortedControlNodeSet& rootControls)
{
    const EngineContext* engineContext = GetEngineContext();
    engineContext->uiControlSystem->GetInputSystem()->SetCurrentScreen(engineContext->uiControlSystem->GetScreen()); // reset current screen

    editingRootControls = rootControls;
    eDisplayState state = rootControls.size() == 1 ? Edit : Preview;
    if (displayState == Emulation)
    {
        previousDisplayState = state;
    }
    else
    {
        SetDisplayState(state);
    }
}

void EditorSystemsManager::OnActiveHUDAreaChanged(const HUDAreaInfo& areaInfo)
{
    currentHUDArea = areaInfo;
}

void EditorSystemsManager::OnPackageChanged(PackageNode* package_)
{
    if (nullptr != package)
    {
        package->RemoveListener(this);
    }
    magnetLinesChanged.Emit({});
    SetDragState(NoDrag);
    ClearHighlight();

    package = package_;
    if (nullptr != package)
    {
        package->AddListener(this);
    }
}

void EditorSystemsManager::OnDataChanged(const DAVA::TArc::DataWrapper& wrapper, const DAVA::Vector<DAVA::Any>& fields)
{
    using namespace DAVA;
    using namespace TArc;

    if (wrapper.HasData() == false)
    {
        OnSelectionDataChanged(Any());
        editingRootControlsChanged.Emit({});
        OnPackageDataChanged(Any());
        return;
    }

    bool selectionChanged = std::find(fields.begin(), fields.end(), String(DocumentData::selectionPropertyName)) != fields.end();
    bool packageChanged = std::find(fields.begin(), fields.end(), String(DocumentData::packagePropertyName)) != fields.end();

    Function<SortedControlNodeSet(const SelectedNodes&, const PackageNode*)> CreateRootControls = [](const SelectedNodes& selection, const PackageNode* package)
    {
        SortedControlNodeSet newRootControls(CompareByLCA);
        if (selection.empty())
        {
            PackageControlsNode* controlsNode = package->GetPackageControlsNode();
            for (int index = 0; index < controlsNode->GetCount(); ++index)
            {
                newRootControls.insert(controlsNode->Get(index));
            }
            return newRootControls;
        }
        else
        {
            for (PackageBaseNode* selectedNode : selection)
            {
                if (dynamic_cast<ControlNode*>(selectedNode) == nullptr)
                {
                    continue;
                }
                PackageBaseNode* root = selectedNode;
                while (nullptr != root->GetParent() && nullptr != root->GetParent()->GetControl())
                {
                    root = root->GetParent();
                }
                if (nullptr != root)
                {
                    ControlNode* rootControl = dynamic_cast<ControlNode*>(root);
                    DVASSERT(rootControl != nullptr);
                    newRootControls.insert(rootControl);
                }
            }
            return newRootControls;
        }
    };

    Any selectionValue = wrapper.GetFieldValue(DocumentData::selectionPropertyName);
    SelectedNodes selection = selectionValue.Get(SelectedNodes());

    Any packageValue = wrapper.GetFieldValue(DocumentData::packagePropertyName);
    PackageNode* package = packageValue.Cast<PackageNode*>();

    if (fields.empty() || packageChanged)
    {
        if (selectionChanged == false || selection.empty())
        {
            SortedControlNodeSet newRootControls = CreateRootControls(selection, package);
            //TODO: remove this when systems will be separate TArc modules
            if (newRootControls.empty() && fields.empty())
            {
                newRootControls = CreateRootControls(SelectedNodes(), package);
            }
            editingRootControlsChanged.Emit(newRootControls);
        }
        OnPackageDataChanged(packageValue);
        //when document is closed and active document is changed we receive empty fields
        OnSelectionDataChanged(selection);
    }

    //update selection
    if (selectionChanged)
    {
        //if package was not changed and selection is empty -> do nothing
        if (selection.empty() == false)
        {
            SortedControlNodeSet newRootControls = CreateRootControls(selection, package);
            //TODO: remove this when systems will be separate TArc modules
            if (newRootControls.empty() && packageChanged)
            {
                newRootControls = CreateRootControls(SelectedNodes(), package);
            }
            //no controls selected, so don't refresh visible content
            if (newRootControls.empty() == false)
            {
                editingRootControlsChanged.Emit(newRootControls);
            }
        }
        OnSelectionDataChanged(selectionValue);
    }
}

void EditorSystemsManager::ControlWillBeRemoved(ControlNode* nodeToRemove, ControlsContainerNode* /*from*/)
{
    //if selected control and its children we will receive Removed signal only for top-level control
    DVASSERT(documentDataWrapper.HasData());
    Any selectionValue = documentDataWrapper.GetFieldValue(DocumentData::selectionPropertyName);
    SelectedNodes nodes = selectionValue.Cast<SelectedNodes>(SelectedNodes());
    for (auto iter = nodes.begin(); iter != nodes.end();)
    {
        PackageBaseNode* node = *iter;
        bool found = false;
        while (node != nullptr && found == false)
        {
            if (node == nodeToRemove)
            {
                iter = nodes.erase(iter);
                found = true;
            }
            node = node->GetParent();
        }
        if (found == false)
        {
            ++iter;
        }
    }
    //we need to synchronize Data in active context and systems state
    //TODO fix it when all editor systems will be separate TArc modules
    documentDataWrapper.SetFieldValue(DocumentData::selectionPropertyName, nodes);
    OnSelectionDataChanged(nodes);

    //when we removing items node is still highlighted
    //because selectionChanged sending before editingRootControlsChanged
    //and HUDSystem think that we have node under point to highlight
    HighlightNode(nullptr);

    if (std::find(editingRootControls.begin(), editingRootControls.end(), nodeToRemove) != editingRootControls.end())
    {
        editingRootControls.erase(nodeToRemove);
        editingRootControlsChanged.Emit(editingRootControls);
    }
}

void EditorSystemsManager::ControlWasAdded(ControlNode* node, ControlsContainerNode* destination, int)
{
    if (displayState == Preview || displayState == Emulation)
    {
        DVASSERT(nullptr != package);
        if (nullptr != package)
        {
            PackageControlsNode* packageControlsNode = package->GetPackageControlsNode();
            if (destination == packageControlsNode)
            {
                editingRootControls.insert(node);
                editingRootControlsChanged.Emit(editingRootControls);
            }
        }
    }
}

void EditorSystemsManager::InitDAVAScreen()
{
    RefPtr<UIControl> backgroundControl(new UIControl());

    backgroundControl->SetName(FastName("Background control of scroll area controller"));
    ScopedPtr<UIScreen> davaUIScreen(new UIScreen());
    UIControlBackground* screenBackground = davaUIScreen->GetOrCreateComponent<UIControlBackground>();
    screenBackground->SetDrawType(UIControlBackground::DRAW_FILL);
    screenBackground->SetColor(Color(0.3f, 0.3f, 0.3f, 1.0f));
    const EngineContext* engineContext = GetEngineContext();
    engineContext->uiScreenManager->RegisterScreen(0, davaUIScreen);
    engineContext->uiScreenManager->SetFirst(0);

    engineContext->uiScreenManager->GetScreen()->AddControl(backgroundControl.Get());
    backgroundControl->AddControl(rootControl.Get());
}

void EditorSystemsManager::OnDragStateChanged(eDragState currentState, eDragState previousState)
{
    if (currentState == Transform || previousState == Transform)
    {
        DVASSERT(package != nullptr);
        //calling this function can refresh all properties and styles in this node
        package->SetCanUpdateAll(previousState == Transform);
    }
}

void EditorSystemsManager::OnDisplayStateChanged(eDisplayState currentState, eDisplayState previousState)
{
    DVASSERT(currentState != previousState);
    if (currentState == Emulation)
    {
        rootControl->RemoveControl(inputLayerControl.Get());
    }
    if (previousState == Emulation)
    {
        rootControl->AddControl(inputLayerControl.Get());
    }
}

UIControl* EditorSystemsManager::GetRootControl() const
{
    return rootControl.Get();
}

DAVA::UIControl* EditorSystemsManager::GetScalableControl() const
{
    return scalableControl.Get();
}

Vector2 EditorSystemsManager::GetMouseDelta() const
{
    return mouseDelta;
}

DAVA::Vector2 EditorSystemsManager::GetLastMousePos() const
{
    return lastMousePos;
}

void EditorSystemsManager::OnPackageDataChanged(const DAVA::Any& packageValue)
{
    PackageNode* package = nullptr;
    if (packageValue.CanCast<PackageNode*>())
    {
        package = packageValue.Cast<PackageNode*>();
    }
    packageChanged.Emit(package);
}

void EditorSystemsManager::OnSelectionDataChanged(const DAVA::Any& newSelectionValue)
{
    SelectedNodes selection = newSelectionValue.Cast<SelectedNodes>(SelectedNodes());
    selectionChanged.Emit(selection);
}

EditorSystemsManager::eDragState EditorSystemsManager::GetDragState() const
{
    return dragState;
}

EditorSystemsManager::eDisplayState EditorSystemsManager::GetDisplayState() const
{
    return displayState;
}

HUDAreaInfo EditorSystemsManager::GetCurrentHUDArea() const
{
    return currentHUDArea;
}

void EditorSystemsManager::AddEditorSystem(BaseEditorSystem* system)
{
    dragStateChanged.Connect(system, &BaseEditorSystem::OnDragStateChanged);
    displayStateChanged.Connect(system, &BaseEditorSystem::OnDisplayStateChanged);
    systems.emplace_back(system);
}

void EditorSystemsManager::SetDragState(eDragState newDragState)
{
    if (dragState == newDragState)
    {
        return;
    }
    previousDragState = dragState;
    dragState = newDragState;
    dragStateChanged.Emit(dragState, previousDragState);
}
