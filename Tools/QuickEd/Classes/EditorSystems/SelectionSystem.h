#ifndef __QUICKED_SELECTION_SYSTEM_H__
#define __QUICKED_SELECTION_SYSTEM_H__

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
    void GetNodesForSelection(DAVA::Vector<ControlNode*>& nodesUnderPoint, const DAVA::Vector2& point) const;
    bool OnInput(DAVA::UIEvent* currentInput) override;
    void OnPackageNodeChanged(PackageNode* packageNode);
    void ControlWasRemoved(ControlNode* node, ControlsContainerNode* from) override;
    void OnSelectByRect(const DAVA::Rect& rect);

    void FocusToChild(bool next);
    void OnSelectionChanged(const SelectedNodes& selected, const SelectedNodes& deselected);
    void SelectNode(const SelectedNodes& selected, const SelectedNodes& deselected);
    void ProcessMousePress(const DAVA::Vector2& point, DAVA::UIEvent::MouseButton buttonID);

    ControlNode* FindSmallNodeUnderNode(const DAVA::Vector<ControlNode*>& nodesUnderPoint) const;

    bool mousePressed = false;
    SelectionContainer selectionContainer;
    PackageNode* packageNode = nullptr;
    bool canFindCommonForSelection = true;

public:
    INTROSPECTION(SelectionSystem,
                  MEMBER(canFindCommonForSelection, "Control Selection/Can search most common node", DAVA::I_SAVE | DAVA::I_VIEW | DAVA::I_EDIT | DAVA::I_PREFERENCE)
                  )
};

#endif // __QUICKED_SELECTION_SYSTEM_H__
