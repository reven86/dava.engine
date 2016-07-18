#ifndef __SCENE_SELECTION_SYSTEM_H__
#define __SCENE_SELECTION_SYSTEM_H__

#include "Scene/SceneTypes.h"
#include "Commands2/Base/Command2.h"
#include "Scene/SelectableGroup.h"

#include "SystemDelegates.h"

// framework
#include "Entity/SceneSystem.h"
#include "Scene3D/Entity.h"
#include "UI/UIEvent.h"

#include "Render/UniqueStateSet.h"
#include "Render/RenderHelper.h"

class SceneCollisionSystem;
class HoodSystem;
class EntityModificationSystem;
class Command2;
class SceneEditor2;

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

class SceneSelectionSystem : public DAVA::SceneSystem
{
    static const DAVA::uint64 ALL_COMPONENTS_MASK = 0xFFFFFFFFFFFFFFFF;

public:
    SceneSelectionSystem(SceneEditor2* editor);
    ~SceneSelectionSystem();

    void AddObjectToSelection(Selectable::Object* entity);
    void AddGroupToSelection(const SelectableGroup& entities);

    void ExcludeEntityFromSelection(Selectable::Object* entity);
    void ExcludeSelection(const SelectableGroup& entities);

    void Clear();

    bool IsEntitySelectable(DAVA::Entity* entity) const;

    /*
	 * SetSelection could remove not selectable items from provided group
	 */
    void SetSelection(SelectableGroup& newSelection);
    const SelectableGroup& GetSelection() const;

    size_t GetSelectionCount() const;
    DAVA::Entity* GetFirstSelectionEntity() const;

    void SetPivotPoint(Selectable::TransformPivot pp);
    Selectable::TransformPivot GetPivotPoint() const;

    void ResetSelectionComponentMask();
    void SetSelectionComponentMask(DAVA::uint64 mask);
    DAVA::uint64 GetSelectionComponentMask() const;

    void SetSelectionAllowed(bool allowed);
    bool IsSelectionAllowed() const;

    void SetLocked(bool lock) override;

    DAVA::AABBox3 GetUntransformedBoundingBox(Selectable::Object* entity) const;
    DAVA::AABBox3 GetTransformedBoundingBox(const SelectableGroup& group) const;

    void ForceEmitSignals();

    DAVA::Entity* GetSelectableEntity(DAVA::Entity* entity);

    void Process(DAVA::float32 timeElapsed) override;
    void ProcessCommand(const Command2* command, bool redo);

    void Input(DAVA::UIEvent* event) override;

    void AddEntity(DAVA::Entity* entity) override;
    void RemoveEntity(DAVA::Entity* entity) override;

    void Activate() override;
    void Deactivate() override;

    void EnableSystem(bool enabled);
    bool IsSystemEnabled() const;

    void Draw();
    void CancelSelection();

    void AddDelegate(SceneSelectionSystemDelegate* delegate_);
    void RemoveDelegate(SceneSelectionSystemDelegate* delegate_);

private:
    void ImmediateEvent(DAVA::Component* component, DAVA::uint32 event) override;
    DAVA::AABBox3 GetTransformedBoundingBox(const Selectable& object, const DAVA::Matrix4& transform) const;

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

private:
    SceneCollisionSystem* collisionSystem = nullptr;
    HoodSystem* hoodSystem = nullptr;
    EntityModificationSystem* modificationSystem = nullptr;
    SelectableGroup currentSelection;
    SelectableGroup recentlySelectedEntities;
    SelectableGroup lastGroupSelection;
    SelectableGroup objectsToSelect;
    DAVA::List<DAVA::Entity*> entitiesForSelection;
    DAVA::Vector2 selectionStartPoint;
    DAVA::Vector2 selectionEndPoint;
    DAVA::uint64 componentMaskForSelection = ALL_COMPONENTS_MASK;
    DAVA::Vector<SceneSelectionSystemDelegate*> selectionDelegates;
    Selectable::TransformPivot curPivotPoint = Selectable::TransformPivot::CommonCenter;
    GroupSelectionMode groupSelectionMode = GroupSelectionMode::Replace;
    bool selectionAllowed = true;
    bool applyOnPhaseEnd = false;
    bool invalidSelectionBoxes = false;
    bool selectionHasChanges = false;
    bool selecting = false;
    bool systemIsEnabled = false;
};

inline void SceneSelectionSystem::ResetSelectionComponentMask()
{
    SetSelectionComponentMask(ALL_COMPONENTS_MASK);
}

inline DAVA::uint64 SceneSelectionSystem::GetSelectionComponentMask() const
{
    return componentMaskForSelection;
}

inline void SceneSelectionSystem::SetSelectionAllowed(bool allowed)
{
    selectionAllowed = allowed;
}

inline bool SceneSelectionSystem::IsSelectionAllowed() const
{
    return selectionAllowed;
}

#endif //__SCENE_SELECTION_SYSTEM_H__
