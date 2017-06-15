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
#include <TArc/Core/FieldBinder.h>
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

EditorSystemsManager::EditorSystemsManager(DAVA::TArc::ContextAccessor* accessor_)
    : rootControl(new UIControl())
    , inputLayerControl(new EditorSystemsManagerDetails::InputLayerControl(this))
    , scalableControl(new UIControl())
    , accessor(accessor_)
{
    using namespace DAVA;
    using namespace TArc;

    dragStateChanged.Connect(this, &EditorSystemsManager::OnDragStateChanged);
    displayStateChanged.Connect(this, &EditorSystemsManager::OnDisplayStateChanged);
    activeAreaChanged.Connect(this, &EditorSystemsManager::OnActiveHUDAreaChanged);

    rootControl->SetName(FastName("rootControl"));
    rootControl->AddControl(scalableControl.Get());
    inputLayerControl->SetName("inputLayerControl");
    rootControl->AddControl(inputLayerControl.Get());
    scalableControl->SetName(FastName("scalableContent"));

    InitDAVAScreen();

    controlViewPtr = new EditorControlsView(scalableControl.Get(), this, accessor);
    systems.emplace_back(controlViewPtr);

    selectionSystemPtr = new SelectionSystem(this, accessor);
    systems.emplace_back(selectionSystemPtr);
    systems.emplace_back(new HUDSystem(this, accessor));
    systems.emplace_back(new EditorTransformSystem(this, accessor));

    for (auto it = systems.begin(); it != systems.end(); ++it)
    {
        const std::unique_ptr<BaseEditorSystem>& editorSystem = *it;
        BaseEditorSystem* editorSystemPtr = editorSystem.get();
        dragStateChanged.Connect(editorSystemPtr, &BaseEditorSystem::OnDragStateChanged);
        displayStateChanged.Connect(editorSystemPtr, &BaseEditorSystem::OnDisplayStateChanged);
    }

    InitFieldBinder();
}

EditorSystemsManager::~EditorSystemsManager()
{
    const EngineContext* engineContext = GetEngineContext();
    engineContext->uiScreenManager->ResetScreen();
}

void EditorSystemsManager::InitFieldBinder()
{
    using namespace DAVA;
    using namespace DAVA::TArc;

    fieldBinder.reset(new FieldBinder(accessor));
    {
        FieldDescriptor fieldDescr;
        fieldDescr.type = ReflectedTypeDB::Get<DocumentData>();
        fieldDescr.fieldName = FastName(DocumentData::displayedRootControlsPropertyName);
        fieldBinder->BindField(fieldDescr, MakeFunction(this, &EditorSystemsManager::OnRootContolsChanged));
    }
    {
        FieldDescriptor fieldDescr;
        fieldDescr.type = ReflectedTypeDB::Get<DocumentData>();
        fieldDescr.fieldName = FastName(DocumentData::packagePropertyName);
        fieldBinder->BindField(fieldDescr, MakeFunction(this, &EditorSystemsManager::OnPackageChanged));
    }
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

ControlNode* EditorSystemsManager::GetControlNodeAtPoint(const DAVA::Vector2& point, bool canGoDeeper) const
{
    if (accessor->GetActiveContext() == nullptr)
    {
        return nullptr;
    }

    if (!KeyboardProxy::IsKeyPressed(KeyboardProxy::KEY_ALT))
    {
        return selectionSystemPtr->GetCommonNodeUnderPoint(point, canGoDeeper);
    }
    return selectionSystemPtr->GetNearestNodeUnderPoint(point);
}

uint32 EditorSystemsManager::GetIndexOfNearestRootControl(const DAVA::Vector2& point) const
{
    using namespace DAVA::TArc;

    SortedControlNodeSet displayedRootControls = GetDisplayedRootControls();
    if (displayedRootControls.empty())
    {
        return 0;
    }
    uint32 index = controlViewPtr->GetIndexByPos(point);
    bool insertToEnd = (index == displayedRootControls.size());

    auto iter = displayedRootControls.begin();
    std::advance(iter, insertToEnd ? index - 1 : index);
    PackageBaseNode* target = *iter;

    DataContext* activeContext = accessor->GetActiveContext();
    DVASSERT(activeContext != nullptr);
    DocumentData* documentData = activeContext->GetData<DocumentData>();
    PackageNode* package = documentData->GetPackageNode();

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

void EditorSystemsManager::OnRootContolsChanged(const DAVA::Any& rootControlsValue)
{
    const EngineContext* engineContext = GetEngineContext();
    // reset current screen
    engineContext->uiControlSystem->GetInputSystem()->SetCurrentScreen(engineContext->uiControlSystem->GetScreen());

    SortedControlNodeSet rootControls = rootControlsValue.Cast<SortedControlNodeSet>(SortedControlNodeSet());
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
    using namespace DAVA::TArc;

    if (currentState == Transform || previousState == Transform)
    {
        DataContext* activeContext = accessor->GetActiveContext();
        DVASSERT(activeContext != nullptr);
        DocumentData* documentData = activeContext->GetData<DocumentData>();
        PackageNode* package = documentData->GetPackageNode();
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

void EditorSystemsManager::OnPackageChanged(const DAVA::Any& /*packageValue*/)
{
    magnetLinesChanged.Emit({});
    SetDragState(NoDrag);
    ClearHighlight();
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

const SortedControlNodeSet& EditorSystemsManager::GetDisplayedRootControls() const
{
    using namespace DAVA::TArc;
    DataContext* activeContext = accessor->GetActiveContext();
    if (activeContext == nullptr)
    {
        static SortedControlNodeSet empty;
        return empty;
    }

    DocumentData* documentData = activeContext->GetData<DocumentData>();
    return documentData->GetDisplayedRootControls();
}
