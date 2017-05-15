#pragma once

#include "EditorSystems/BaseEditorSystem.h"
#include "EditorSystems/EditorSystemsManager.h"
#include "SelectionContainer.h"
#include "Utils/PackageListenerProxy.h"

#include <TArc/DataProcessing/DataWrapper.h>
#include <UI/Layouts/UILayoutSystemListener.h>
#include <Base/BaseTypes.h>

class EditorSystemsManager;
class PackageBaseNode;
class BackgroundController;

namespace DAVA
{
class Any;
namespace TArc
{
class FieldBinder;
}
}

class EditorControlsView final : public BaseEditorSystem, PackageListener, DAVA::UILayoutSystemListener
{
public:
    EditorControlsView(DAVA::UIControl* canvasParent, EditorSystemsManager* parent, DAVA::TArc::ContextAccessor* accessor);
    ~EditorControlsView() override;

    DAVA::uint32 GetIndexByPos(const DAVA::Vector2& pos) const;

private:
    void OnDragStateChanged(EditorSystemsManager::eDragState currentState, EditorSystemsManager::eDragState previousState) override;

    void InitFieldBinder();
    void Layout();
    void OnRootContolsChanged(const DAVA::Any& rootControls);
    void ControlWasRemoved(ControlNode* node, ControlsContainerNode* from) override;
    void ControlWasAdded(ControlNode* node, ControlsContainerNode* destination, int index) override;
    void ControlPropertyWasChanged(ControlNode* node, AbstractProperty* property) override;

    // UILayoutSystemListener
    void OnControlLayouted(DAVA::UIControl* control) override;

    BackgroundController* CreateControlBackground(PackageBaseNode* node);
    void AddBackgroundControllerToCanvas(BackgroundController* backgroundController, size_t pos);

    DAVA::RefPtr<DAVA::UIControl> controlsCanvas; //to attach or detach from document
    DAVA::List<std::unique_ptr<BackgroundController>> gridControls;

    DAVA::Set<ControlNode*> rootControls;
    DAVA::UIControl* canvasParent = nullptr;

    std::unique_ptr<DAVA::TArc::FieldBinder> fieldBinder;

    PackageListenerProxy packageListenerProxy;
};
