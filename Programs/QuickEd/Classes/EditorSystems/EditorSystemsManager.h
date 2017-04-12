#pragma once

#include "Model/PackageHierarchy/PackageListener.h"
#include "EditorSystems/SelectionContainer.h"

#include <TArc/DataProcessing/DataWrapper.h>
#include <TArc/DataProcessing/DataListener.h>

#include <Base/BaseTypes.h>
#include <Base/RefPtr.h>
#include <Functional/Signal.h>

#include <Math/Rect.h>
#include <Math/Vector.h>

namespace DAVA
{
class UIControl;
class UIEvent;
class UIGeometricData;

namespace TArc
{
class ContextAccessor;
class DataContext;
}
}

struct HUDAreaInfo
{
    enum eArea
    {
        AREAS_BEGIN,
        ROTATE_AREA = AREAS_BEGIN,
        TOP_LEFT_AREA,
        TOP_CENTER_AREA,
        TOP_RIGHT_AREA,
        CENTER_LEFT_AREA,
        CENTER_RIGHT_AREA,
        BOTTOM_LEFT_AREA,
        BOTTOM_CENTER_AREA,
        BOTTOM_RIGHT_AREA,
        PIVOT_POINT_AREA,
        FRAME_AREA,
        NO_AREA,
        CORNERS_BEGIN = TOP_LEFT_AREA,
        CORNERS_COUNT = PIVOT_POINT_AREA - TOP_LEFT_AREA + CORNERS_BEGIN,
        AREAS_COUNT = NO_AREA - AREAS_BEGIN
    };
    HUDAreaInfo(ControlNode* owner_ = nullptr, eArea area_ = NO_AREA)
        : owner(owner_)
        , area(area_)
    {
        DVASSERT((owner != nullptr && area != HUDAreaInfo::NO_AREA) || (owner == nullptr && area == HUDAreaInfo::NO_AREA));
    }
    ControlNode* owner = nullptr;
    eArea area = NO_AREA;
};

struct MagnetLineInfo
{
    MagnetLineInfo(const DAVA::Rect& targetRect_, const DAVA::Rect& rect_, const DAVA::UIGeometricData* gd_, DAVA::Vector2::eAxis axis_)
        : targetRect(targetRect_)
        , rect(rect_)
        , gd(gd_)
        , axis(axis_)
    {
    }
    DAVA::Rect targetRect;
    DAVA::Rect rect;
    const DAVA::UIGeometricData* gd;
    const DAVA::Vector2::eAxis axis;
};

class BaseEditorSystem;
class AbstractProperty;
class PackageNode;
class EditorControlsView;
class SelectionSystem;
class HUDSystem;

class EditorSystemsManager : PackageListener, DAVA::TArc::DataListener
{
    using StopPredicate = std::function<bool(const ControlNode*)>;
    static StopPredicate defaultStopPredicate;

public:
    //we have situations, when one input can produce two different state. To resolve this conflict we declare that state priority is equal to it value
    //as an example dragging control with pressed spacebar button will perform drag screen and transform at the same time
    enum eDragState
    {
        //invalid state to request new state from baseEditorSystem
        NoDrag,
        //if cursor is under control and it selectable
        SelectByRect,
        //if cursor under selected control, pressed left mouse button and starts dragging
        Transform,
        //all user input used only to drag canvas inide rednder widget
        DragScreen
    };

    enum eDisplayState
    {
        //remove hud and throw all input to the DAVA frameworkx
        Emulation,
        //just display all root controls, no other iteraction enabled
        Preview,
        //display one root control
        Edit
    };

    explicit EditorSystemsManager(DAVA::TArc::ContextAccessor* accessor);
    ~EditorSystemsManager();

    eDragState GetDragState() const;
    eDisplayState GetDisplayState() const;
    HUDAreaInfo GetCurrentHUDArea() const;

    void AddEditorSystem(BaseEditorSystem* system);

    void OnInput(DAVA::UIEvent* currentInput);

    template <class OutIt, class Predicate>
    void CollectControlNodes(OutIt destination, Predicate predicate, StopPredicate stopPredicate = defaultStopPredicate) const;

    void HighlightNode(ControlNode* node);
    void ClearHighlight();

    void SetEmulationMode(bool emulationMode);

    ControlNode* GetControlNodeAtPoint(const DAVA::Vector2& point) const;
    DAVA::uint32 GetIndexOfNearestRootControl(const DAVA::Vector2& point) const;

    void SelectAll();
    void FocusNextChild();
    void FocusPreviousChild();
    void ClearSelection();
    void SelectNode(ControlNode* node);

    const SortedControlNodeSet& GetEditingRootControls() const;

    DAVA::UIControl* GetRootControl() const;
    DAVA::UIControl* GetScalableControl() const;

    DAVA::Signal<const SelectedNodes& /*selection*/> selectionChanged;
    DAVA::Signal<const HUDAreaInfo& /*areaInfo*/> activeAreaChanged;
    DAVA::Signal<const DAVA::Vector<MagnetLineInfo>& /*magnetLines*/> magnetLinesChanged;
    DAVA::Signal<const ControlNode*> highlightNode;
    DAVA::Signal<const DAVA::Rect& /*selectionRectControl*/> selectionRectChanged;
    DAVA::Signal<const DAVA::Vector2&> contentSizeChanged;
    DAVA::Signal<ControlNode*, AbstractProperty*, const DAVA::Any&> propertyChanged;
    DAVA::Signal<const SortedControlNodeSet&> editingRootControlsChanged;
    DAVA::Signal<const DAVA::Vector2& /*new position*/> rootControlPositionChanged;
    DAVA::Signal<PackageNode* /*node*/> packageChanged;
    DAVA::Signal<bool> emulationModeChanged;
    DAVA::Signal<eDragState /*currentState*/, eDragState /*previousState*/> dragStateChanged;
    DAVA::Signal<eDisplayState /*currentState*/, eDisplayState /*previousState*/> displayStateChanged;

    //helpers
    DAVA::Vector2 GetMouseDelta() const;
    DAVA::Vector2 GetLastMousePos() const;

private:
    void SetDragState(eDragState dragState);
    void SetDisplayState(eDisplayState displayState);

    void OnEditingRootControlsChanged(const SortedControlNodeSet& rootControls);
    void OnActiveHUDAreaChanged(const HUDAreaInfo& areaInfo);

    template <class OutIt, class Predicate>
    void CollectControlNodesImpl(OutIt destination, Predicate predicate, StopPredicate stopPredicate, ControlNode* node) const;

    void OnPackageChanged(PackageNode* node);
    void ControlWillBeRemoved(ControlNode* node, ControlsContainerNode* from) override;
    void ControlWasAdded(ControlNode* node, ControlsContainerNode* destination, int index) override;
    void InitDAVAScreen();

    void OnDragStateChanged(eDragState currentState, eDragState previousState);
    void OnDisplayStateChanged(eDisplayState currentState, eDisplayState previousState);

    void OnSelectionDataChanged(const DAVA::Any& newSelection);
    void OnPackageDataChanged(const DAVA::Any& package);

    void OnDataChanged(const DAVA::TArc::DataWrapper& wrapper, const DAVA::Vector<DAVA::Any>& fields) override;

    DAVA::RefPtr<DAVA::UIControl> rootControl;
    DAVA::RefPtr<DAVA::UIControl> inputLayerControl;
    DAVA::RefPtr<DAVA::UIControl> scalableControl;

    DAVA::List<std::unique_ptr<BaseEditorSystem>> systems;

    DAVA::RefPtr<PackageNode> package;
    SortedControlNodeSet editingRootControls;
    EditorControlsView* controlViewPtr = nullptr; //weak pointer to canvas system;
    SelectionSystem* selectionSystemPtr = nullptr; // weak pointer to selection system
    HUDSystem* hudSystemPtr = nullptr;

    eDragState dragState = NoDrag;
    eDragState previousDragState = NoDrag;
    eDisplayState displayState = Preview;
    eDisplayState previousDisplayState = Preview;

    HUDAreaInfo currentHUDArea;
    //helpers
    DAVA::Vector2 lastMousePos;
    DAVA::Vector2 mouseDelta;

    DAVA::TArc::DataWrapper documentDataWrapper;
};

template <class OutIt, class Predicate>
void EditorSystemsManager::CollectControlNodes(OutIt destination, Predicate predicate, StopPredicate stopPredicate) const
{
    for (PackageBaseNode* rootControl : editingRootControls)
    {
        ControlNode* controlNode = dynamic_cast<ControlNode*>(rootControl);
        DVASSERT(nullptr != controlNode);
        CollectControlNodesImpl(destination, predicate, stopPredicate, controlNode);
    }
}

template <class OutIt, class Predicate>
void EditorSystemsManager::CollectControlNodesImpl(OutIt destination, Predicate predicate, StopPredicate stopPredicate, ControlNode* node) const
{
    if (predicate(node))
    {
        *destination++ = node;
    }
    if (!stopPredicate(node))
    {
        int count = node->GetCount();
        for (int i = 0; i < count; ++i)
        {
            CollectControlNodesImpl(destination, predicate, stopPredicate, node->Get(i));
        }
    }
}
