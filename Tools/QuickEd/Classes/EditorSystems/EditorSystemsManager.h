#ifndef __QUICKED_SYSTEMS_MANAGER_H__
#define __QUICKED_SYSTEMS_MANAGER_H__

#include "Base/BaseTypes.h"
#include "Base/RefPtr.h"
#include "Functional/Signal.h"
#include "Model/PackageHierarchy/PackageListener.h"
#include "EditorSystems/SelectionContainer.h"
#include "Math/Rect.h"
#include "Math/Vector.h"

namespace DAVA
{
class UIControl;
class UIEvent;
class VariantType;
class UIGeometricData;
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

struct ChangePropertyAction
{
    ChangePropertyAction(ControlNode* node_, AbstractProperty* property_, const DAVA::VariantType& value_)
        : node(node_)
        , property(property_)
        , value(value_)
    {
    }
    ControlNode* node = nullptr;
    AbstractProperty* property = nullptr;
    DAVA::VariantType value;
};

class BaseEditorSystem;
class AbstractProperty;
class PackageNode;
class CanvasSystem;
class SelectionSystem;

class EditorSystemsManager : PackageListener
{
    using StopPredicate = std::function<bool(const ControlNode*)>;
    static StopPredicate defaultStopPredicate;

public:
    using SortedPackageBaseNodeSet = DAVA::Set<PackageBaseNode*, std::function<bool(PackageBaseNode*, PackageBaseNode*)>>;

    explicit EditorSystemsManager();
    ~EditorSystemsManager();

    DAVA::UIControl* GetRootControl() const;
    DAVA::UIControl* GetInputLayerControl() const;
    DAVA::UIControl* GetScalableControl() const;

    bool OnInput(DAVA::UIEvent* currentInput);

    void SetEmulationMode(bool emulationMode);

    template <class OutIt, class Predicate>
    void CollectControlNodes(OutIt destination, Predicate predicate, StopPredicate stopPredicate = defaultStopPredicate) const;

    ControlNode* ControlNodeUnderPoint(const DAVA::Vector2& point) const;
    DAVA::uint32 GetIndexOfNearestControl(const DAVA::Vector2& point) const;

    void SelectAll();
    void FocusNextChild();
    void FocusPreviousChild();
    void ClearSelection();

    DAVA::Signal<const SelectedNodes& /*selected*/, const SelectedNodes& /*deselected*/> SelectionChanged;
    DAVA::Signal<const HUDAreaInfo& /*areaInfo*/> ActiveAreaChanged;
    DAVA::Signal<const DAVA::Rect& /*selectionRectControl*/> SelectionRectChanged;
    DAVA::Signal<bool> EmulationModeChangedSignal;
    DAVA::Signal<> CanvasSizeChanged;
    DAVA::Signal<const DAVA::Vector<ChangePropertyAction>& /*propertyActions*/, size_t /*hash*/> PropertiesChanged;
    DAVA::Signal<const SortedPackageBaseNodeSet&> EditingRootControlsChanged;
    DAVA::Signal<const DAVA::Vector<MagnetLineInfo>& /*magnetLines*/> MagnetLinesChanged;
    DAVA::Signal<const DAVA::Vector2& /*new position*/> RootControlPositionChanged;
    DAVA::Signal<PackageNode* /*node*/> PackageNodeChanged;
    DAVA::Signal<const DAVA::Vector<ControlNode*>&> NodesHovered;
    DAVA::Signal<bool> TransformStateChanged; //indicates when user transform control

    std::function<ControlNode*(const DAVA::Vector<ControlNode*>& /*nodes*/, const DAVA::Vector2& /*pos*/)> GetControlByMenu;

private:
    class InputLayerControl;
    void OnSelectionChanged(const SelectedNodes& selected, const SelectedNodes& deselected);

    template <class OutIt, class Predicate>
    void CollectControlNodesImpl(OutIt destination, Predicate predicate, StopPredicate stopPredicate, ControlNode* node) const;

    void OnPackageNodeChanged(PackageNode* node);
    void ControlWasRemoved(ControlNode* node, ControlsContainerNode* from) override;
    void ControlWasAdded(ControlNode* node, ControlsContainerNode* destination, int index) override;
    void SetPreviewMode(bool mode);
    void RefreshRootControls();

    DAVA::RefPtr<DAVA::UIControl> rootControl;
    DAVA::RefPtr<InputLayerControl> inputLayerControl;
    DAVA::RefPtr<DAVA::UIControl> scalableControl;

    DAVA::List<std::unique_ptr<BaseEditorSystem>> systems;

    PackageNode* package = nullptr;
    SelectedControls selectedControlNodes;
    SortedPackageBaseNodeSet editingRootControls;
    bool previewMode = true;
    SelectionContainer selectionContainer;
    CanvasSystem* canvasSystemPtr = nullptr; //weak pointer to canvas system;
    SelectionSystem* selectionSystemPtr = nullptr; // weak pointer to selection system

public:
    DAVA::Vector2 minimumSize = DAVA::Vector2(16.0f, 16.0f);
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

#endif // __QUICKED_SYSTEMS_MANAGER_H__
