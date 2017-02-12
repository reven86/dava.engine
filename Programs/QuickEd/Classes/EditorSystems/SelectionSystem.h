#pragma once

#include "EditorSystems/SelectionContainer.h"
#include "EditorSystems/BaseEditorSystem.h"
#include "Math/Rect.h"
#include "UI/UIEvent.h"
#include "Functional/SignalBase.h"
#include "Model/PackageHierarchy/PackageListener.h"
#include "Preferences/PreferencesRegistrator.h"

class EditorSystemsManager;
class ControlNode;
class ControlsContainerNode;

namespace DAVA
{
class Vector2;
}

class SelectionSystem : public BaseEditorSystem, PackageListener, public DAVA::InspBase
{
public:
    SelectionSystem(EditorSystemsManager* doc);
    ~SelectionSystem() override;

    void ClearSelection();
    void SelectAllControls();
    void FocusNextChild();
    void FocusPreviousChild();

    void SelectNode(ControlNode* node);

    ControlNode* GetNearestNodeUnderPoint(const DAVA::Vector2& point) const;
    ControlNode* GetCommonNodeUnderPoint(const DAVA::Vector2& point) const;

private:
    bool CanProcessInput(DAVA::UIEvent* currentInput) const override;

    void GetNodesForSelection(DAVA::Vector<ControlNode*>& nodesUnderPoint, const DAVA::Vector2& point) const;
    void ProcessInput(DAVA::UIEvent* currentInput) override;
    void OnPackageChanged(PackageNode* packageNode);
    void ControlWasRemoved(ControlNode* node, ControlsContainerNode* from) override;
    void OnSelectByRect(const DAVA::Rect& rect);

    void FocusToChild(bool next);
    void SelectNodes(const SelectedNodes& selection);
    void OnSelectionChanged(const SelectedNodes& selection);

    ControlNode* FindSmallNodeUnderNode(const DAVA::Vector<ControlNode*>& nodesUnderPoint) const;

    SelectionContainer selectionContainer;
    DAVA::RefPtr<PackageNode> packageNode;
    bool canFindCommonForSelection = true;

    bool selectOnRelease = false;
    DAVA::Vector2 pressedPoint = DAVA::Vector2(-1.0f, -1.0f);

public:
    INTROSPECTION(SelectionSystem,
                  MEMBER(canFindCommonForSelection, "Control Selection/Can search most common node", DAVA::I_SAVE | DAVA::I_VIEW | DAVA::I_EDIT | DAVA::I_PREFERENCE)
                  )
};
