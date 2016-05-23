#ifndef __QUICKED_HUD_SYSTEM_H__
#define __QUICKED_HUD_SYSTEM_H__

#include "Math/Vector.h"
#include "EditorSystems/BaseEditorSystem.h"
#include "EditorSystems/EditorSystemsManager.h"

class HUDSystem final : public BaseEditorSystem
{
public:
    HUDSystem(EditorSystemsManager* parent);
    ~HUDSystem() override;

private:
    enum eSearchOrder
    {
        SEARCH_FORWARD,
        SEARCH_BACKWARD
    };
    struct HUD;

    bool OnInput(DAVA::UIEvent* currentInput) override;
    void OnRootContolsChanged(const EditorSystemsManager::SortedPackageBaseNodeSet& rootControls);
    void OnSelectionChanged(const SelectedNodes& selected, const SelectedNodes& deselected);
    void OnEmulationModeChanged(bool emulationMode);
    void OnNodesHovered(const DAVA::Vector<ControlNode*>& node);

    void OnMagnetLinesChanged(const DAVA::Vector<MagnetLineInfo>& magnetLines);

    void ProcessCursor(const DAVA::Vector2& pos, eSearchOrder searchOrder = SEARCH_FORWARD);
    HUDAreaInfo GetControlArea(const DAVA::Vector2& pos, eSearchOrder searchOrder) const;
    void SetNewArea(const HUDAreaInfo& HUDAreaInfo);

    void SetCanDrawRect(bool canDrawRect_);
    void UpdateAreasVisibility();
    void InvalidatePressedPoint();
    void UpdatePlacedOnScreenStatus();
    HUDAreaInfo activeAreaInfo;

    DAVA::RefPtr<DAVA::UIControl> hudControl;

    DAVA::Vector2 pressedPoint; //corner of selection rect
    DAVA::Vector2 hoveredPoint;
    bool canDrawRect = false; //selection rect state

    DAVA::Map<ControlNode*, std::unique_ptr<HUD>> hudMap;
    DAVA::UIControl* selectionRectControl = nullptr;
    DAVA::Vector<DAVA::RefPtr<DAVA::UIControl>> magnetControls;
    DAVA::Vector<DAVA::RefPtr<DAVA::UIControl>> magnetTargetControls;
    EditorSystemsManager::SortedPackageBaseNodeSet sortedControlList;
    bool dragRequested = false;
    SelectionContainer selectionContainer;
    DAVA::Map<ControlNode*, DAVA::RefPtr<DAVA::UIControl>> hoveredNodes;
    bool inEmulationMode = false;
    EditorSystemsManager::SortedPackageBaseNodeSet rootControls;
    bool isPlacedOnScreen = false;
};

#endif // __QUICKED_HUD_SYSTEM_H__
