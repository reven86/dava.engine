#pragma once

#include "Scene/SceneTypes.h"
#include "Classes/Selection/SelectableGroup.h"

#include "Scene/System/SystemDelegates.h"
#include "Classes/Qt/Scene/System/EditorSceneSystem.h"

// framework
#include "Entity/SceneSystem.h"
#include "Scene3D/Entity.h"
#include "UI/UIEvent.h"

#include "Render/UniqueStateSet.h"
#include "Render/RenderHelper.h"

class RECommandNotificationObject;
class SceneCollisionSystem;
class HoodSystem;
class EntityModificationSystem;

enum SelectionSystemDrawMode
{
    SS_DRAW_NOTHING = 0x0,

    SS_DRAW_SHAPE = 0x1,
    SS_DRAW_CORNERS = 0x2,
    SS_DRAW_BOX = 0x4,
    SS_DRAW_NO_DEEP_TEST = 0x10,

    SS_DRAW_DEFAULT = SS_DRAW_CORNERS | SS_DRAW_BOX,
    SS_DRAW_ALL = 0xFFFFFFFF
};

class SelectionSystem : public DAVA::SceneSystem, public EditorSceneSystem
{
    static const DAVA::uint64 ALL_COMPONENTS_MASK = 0xFFFFFFFFFFFFFFFF;

public:
    SelectionSystem(DAVA::Scene* scene);
    ~SelectionSystem();

    void AddObjectToSelection(Selectable::Object* entity);
    void AddGroupToSelection(const SelectableGroup& entities);

    void ExcludeEntityFromSelection(Selectable::Object* entity, bool forceRemove);
    void ExcludeSelection(const SelectableGroup& entities);

    void Clear();

    bool IsEntitySelectable(DAVA::Entity* entity) const;

    /*
	 * SetSelection could remove not selectable items from provided group
	 */
    void SetSelection(SelectableGroup& newSelection);
    const SelectableGroup& GetSelection() const;
    const DAVA::AABBox3& GetSelectionBox() const;

    void ResetSelectionComponentMask();
    void SetSelectionComponentMask(DAVA::uint64 mask);
    DAVA::uint64 GetSelectionComponentMask() const;

    void SetSelectionAllowed(bool allowed);
    bool IsSelectionAllowed() const;

    void SetLocked(bool lock) override;

    void Process(DAVA::float32 timeElapsed) override;
    bool Input(DAVA::UIEvent* event) override;

    void AddEntity(DAVA::Entity* entity) override;
    void RemoveEntity(DAVA::Entity* entity) override;

    void Activate() override;
    void Deactivate() override;

    void CancelSelection();

    void AddDelegate(SelectionSystemDelegate* delegate);
    void RemoveDelegate(SelectionSystemDelegate* delegate);

protected:
    void Draw() override;
    void ProcessCommand(const RECommandNotificationObject& commandNotification) override;

private:
    void ImmediateEvent(DAVA::Component* component, DAVA::uint32 event) override;

    void UpdateHoodPos() const;

    void PerformSelectionAtPoint(const DAVA::Vector2&);

    void PerformSelectionInCurrentBox();

    void ProcessSelectedGroup(const SelectableGroup::CollectionType&);

    void UpdateGroupSelectionMode();

    void UpdateSelectionGroup(const SelectableGroup& newSelection);
    void FinishSelection();

    void ExcludeSingleItem(Selectable::Object* object);

    void DrawItem(const DAVA::AABBox3& bbox, const DAVA::Matrix4& transform, DAVA::int32 drawMode,
                  DAVA::RenderHelper::eDrawType wireDrawType, DAVA::RenderHelper::eDrawType solidDrawType,
                  const DAVA::Color& color);

    enum class GroupSelectionMode
    {
        Replace,
        Add,
        Remove
    };

    SceneCollisionSystem* collisionSystem = nullptr;
    HoodSystem* hoodSystem = nullptr;
    EntityModificationSystem* modificationSystem = nullptr;
    SelectableGroup currentSelection;
    SelectableGroup lastGroupSelection;
    SelectableGroup objectsToSelect;
    DAVA::List<DAVA::Entity*> entitiesForSelection;
    DAVA::Vector2 selectionStartPoint;
    DAVA::Vector2 selectionEndPoint;
    DAVA::uint64 componentMaskForSelection = ALL_COMPONENTS_MASK;
    DAVA::Vector<SelectionSystemDelegate*> selectionDelegates;
    GroupSelectionMode groupSelectionMode = GroupSelectionMode::Replace;
    bool selectionAllowed = true;
    bool applyOnPhaseEnd = false;

    DAVA::AABBox3 selectionBox;
    bool invalidSelectionBoxes = false;

    bool selecting = false;

    bool wasLockedInActiveMode = false;
};

inline void SelectionSystem::ResetSelectionComponentMask()
{
    SetSelectionComponentMask(ALL_COMPONENTS_MASK);
}

inline DAVA::uint64 SelectionSystem::GetSelectionComponentMask() const
{
    return componentMaskForSelection;
}

inline void SelectionSystem::SetSelectionAllowed(bool allowed)
{
    selectionAllowed = allowed;
}

inline bool SelectionSystem::IsSelectionAllowed() const
{
    return selectionAllowed;
}
