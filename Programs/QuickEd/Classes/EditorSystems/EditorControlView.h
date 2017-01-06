#pragma once

#include "EditorSystems/BaseEditorSystem.h"
#include "Base/BaseTypes.h"
#include "EditorSystems/EditorSystemsManager.h"
#include "SelectionContainer.h"
#include "Model/PackageHierarchy/PackageListener.h"

class EditorSystemsManager;
class PackageBaseNode;
class BackgroundController;

class EditorControlView final : public BaseEditorSystem, PackageListener
{
public:
    EditorControlView(EditorSystemsManager* parent);
    ~EditorControlView() override;

    DAVA::uint32 GetIndexByPos(const DAVA::Vector2& pos) const;
    void Layout();

private:
    void OnRootContolsChanged(const SortedPackageBaseNodeSet& rootControls_);
    void OnPackageNodeChanged(PackageNode* node);
    void OnTransformStateChanged(bool inTransformState);
    void ControlWasRemoved(ControlNode* node, ControlsContainerNode* from) override;
    void ControlWasAdded(ControlNode* node, ControlsContainerNode* destination, int index) override;
    void ControlPropertyWasChanged(ControlNode* node, AbstractProperty* property) override;
    BackgroundController* CreateControlBackground(PackageBaseNode* node);
    void AddBackgroundControllerToCanvas(BackgroundController* backgroundController, size_t pos);

    DAVA::RefPtr<DAVA::UIControl> controlsCanvas; //to attach or detach from document
    DAVA::List<std::unique_ptr<BackgroundController>> gridControls;

    DAVA::Set<PackageBaseNode*> rootControls;
    PackageNode* package = nullptr;
    bool inTransformState = false;
    bool needRecalculate = false;
};