#pragma once

#include "EditorSystems/SelectionContainer.h"
#include "EditorSystems/BaseEditorSystem.h"

#include <TArc/DataProcessing/DataWrapper.h>

#include <Math/Rect.h>
#include <UI/UIEvent.h>
#include <Preferences/PreferencesRegistrator.h>

class EditorSystemsManager;
class ControlNode;
class ControlsContainerNode;

namespace DAVA
{
class Vector2;
}

class SelectionSystem : public BaseEditorSystem, public DAVA::InspBase
{
public:
    SelectionSystem(EditorSystemsManager* doc, DAVA::TArc::ContextAccessor* accessor);
    ~SelectionSystem() override;

    void ClearSelection();
    void SelectAllControls();
    void FocusNextChild();
    void FocusPreviousChild();

    void SelectNode(ControlNode* node);

    ControlNode* GetNearestNodeUnderPoint(const DAVA::Vector2& point) const;
    ControlNode* GetCommonNodeUnderPoint(const DAVA::Vector2& point, bool canGoDeeper) const;

private:
    bool CanProcessInput(DAVA::UIEvent* currentInput) const override;
    void ProcessInput(DAVA::UIEvent* currentInput) override;

    void GetNodesForSelection(DAVA::Vector<ControlNode*>& nodesUnderPoint, const DAVA::Vector2& point) const;
    void OnSelectByRect(const DAVA::Rect& rect);

    void FocusToChild(bool next);
    void SelectNodes(const SelectedNodes& selection);

    ControlNode* FindSmallNodeUnderNode(const DAVA::Vector<ControlNode*>& nodesUnderPoint) const;

    bool canFindCommonForSelection = true;

    DAVA::Vector2 pressedPoint = DAVA::Vector2(-1.0f, -1.0f);

    DAVA::TArc::DataWrapper documentDataWrapper;

public:
    INTROSPECTION(SelectionSystem,
                  MEMBER(canFindCommonForSelection, "Control Selection/Can search most common node", DAVA::I_SAVE | DAVA::I_VIEW | DAVA::I_EDIT | DAVA::I_PREFERENCE)
                  )
};
