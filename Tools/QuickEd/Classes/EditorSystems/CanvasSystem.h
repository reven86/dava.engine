#ifndef __QUICKED_CANVAS_SYSTEM_H__
#define __QUICKED_CANVAS_SYSTEM_H__

#include "EditorSystems/BaseEditorSystem.h"
#include "EditorSystems/EditorSystemsManager.h"
#include "SelectionContainer.h"
#include "Model/PackageHierarchy/PackageListener.h"

class EditorSystemsManager;
class PackageBaseNode;
class BackgroundController;

class CanvasSystem final : public BaseEditorSystem, PackageListener
{
public:
    CanvasSystem(EditorSystemsManager* parent);
    ~CanvasSystem() override;

    DAVA::uint32 GetIndexByPos(const DAVA::Vector2& pos) const;
    void LayoutCanvas();

private:
    void OnRootContolsChanged(const EditorSystemsManager::SortedPackageBaseNodeSet& rootControls_);
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
    DAVA::Set<ControlNode*> transformedNodes; //vector of weak pointers
};

#endif // __QUICKED_CANVAS_SYSTEM_H__
