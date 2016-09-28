#ifndef __QUICKED_SELECTION_SYSTEM_H__
#define __QUICKED_SELECTION_SYSTEM_H__

#include "EditorSystems/SelectionContainer.h"
#include "EditorSystems/BaseEditorSystem.h"
#include "Math/Rect.h"
#include "UI/UIEvent.h"
#include <Functional/SignalBase.h>
#include "Model/PackageHierarchy/PackageListener.h"

class EditorSystemsManager;
class ControlNode;
class ControlsContainerNode;

namespace DAVA
{
class Vector2;
}

class SelectionSystem final : public BaseEditorSystem, PackageListener
{
public:
    SelectionSystem(EditorSystemsManager* doc);
    ~SelectionSystem() override;

    void ClearSelection();
    void SelectAllControls();
    void FocusNextChild();
    void FocusPreviousChild();

    void SelectNode(ControlNode* node);

    ControlNode* ControlNodeUnderPoint(const DAVA::Vector2& point, bool nearest) const;

private:
    bool OnInput(DAVA::UIEvent* currentInput) override;
    void OnPackageNodeChanged(PackageNode* packageNode);
    void ControlWasRemoved(ControlNode* node, ControlsContainerNode* from) override;
    void OnSelectByRect(const DAVA::Rect& rect);

    void FocusToChild(bool next);
    void OnSelectionChanged(const SelectedNodes& selected, const SelectedNodes& deselected);
    void SelectNode(const SelectedNodes& selected, const SelectedNodes& deselected);
    void ProcessMousePress(const DAVA::Vector2& point, DAVA::UIEvent::MouseButton buttonID);

    bool mousePressed = false;
    SelectionContainer selectionContainer;
    PackageNode* packageNode = nullptr;
};

#endif // __QUICKED_SELECTION_SYSTEM_H__
