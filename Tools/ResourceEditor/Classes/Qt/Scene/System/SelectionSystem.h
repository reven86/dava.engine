/*==================================================================================
    Copyright (c) 2008, binaryzebra
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    * Neither the name of the binaryzebra nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE binaryzebra AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL binaryzebra BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/


#ifndef __SCENE_SELECTION_SYSTEM_H__
#define __SCENE_SELECTION_SYSTEM_H__

#include "Scene/EntityGroup.h"
#include "Scene/SceneTypes.h"
#include "Commands2/Command2.h"
#include "SystemDelegates.h"

// framework
#include "Entity/SceneSystem.h"
#include "Scene3D/Entity.h"
#include "UI/UIEvent.h"

#include "Render/UniqueStateSet.h"
#include "Render/RenderHelper.h"

class SceneCollisionSystem;
class HoodSystem;

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
    SceneSelectionSystem(DAVA::Scene* scene, SceneCollisionSystem* collSys, HoodSystem* hoodSys);
    ~SceneSelectionSystem();

    void AddEntityToSelection(DAVA::Entity* entity);
    void AddSelection(const EntityGroup& entities);

    void ExcludeEntityFromSelection(DAVA::Entity* entity);
    void ExcludeSelection(const EntityGroup& entities);

    void Clear();

    bool IsEntitySelectable(DAVA::Entity* entity) const;

    /*
	 * SetSelection could remove not selectable items from provided EntityGroup
	 */
    void SetSelection(EntityGroup& newSelection);
    const EntityGroup& GetSelection() const;

    size_t GetSelectionCount() const;
    DAVA::Entity* GetFirstSelectionEntity() const;

    void SetPivotPoint(ST_PivotPoint pp);
    ST_PivotPoint GetPivotPoint() const;

    void ResetSelectionComponentMask();
    void SetSelectionComponentMask(DAVA::uint64 mask);
    DAVA::uint64 GetSelectionComponentMask() const;

    void SetSelectionAllowed(bool allowed);
    bool IsSelectionAllowed() const;

    void SetLocked(bool lock) override;

    DAVA::AABBox3 GetUntransformedBoundingBox(DAVA::Entity* entity) const;
    DAVA::AABBox3 GetTransformedBoundingBox(const EntityGroup& group) const;

    void ForceEmitSignals();

    DAVA::Entity* GetSelectableEntity(DAVA::Entity* entity);

    void Process(DAVA::float32 timeElapsed) override;
    void ProcessCommand(const Command2* command, bool redo);

    void Input(DAVA::UIEvent* event) override;

    void Activate() override;
    void Deactivate() override;

    bool IsEntitySelected(DAVA::Entity* entity);
    bool IsEntitySelectedHierarchically(DAVA::Entity* entity);

    void Draw();
    void CancelSelection();

    void AddSelectionDelegate(SceneSelectionSystemDelegate* delegate_);
    void RemoveSelectionDelegate(SceneSelectionSystemDelegate* delegate_);

private:
    void ImmediateEvent(DAVA::Component* component, DAVA::uint32 event) override;
    DAVA::AABBox3 GetTransformedBoundingBox(DAVA::Entity* entity, const DAVA::Matrix4& transform) const;

    void UpdateHoodPos() const;

    void PerformSelectionAtPoint(const DAVA::Vector2&);

    void PerformSelectionInCurrentBox();

    void ProcessSelectedGroup(const EntityGroup::EntityVector&);

    void UpdateGroupSelectionMode();

    void UpdateSelectionGroup(const EntityGroup& newSelection);
    void FinishSelection();

    void ExcludeSingleItem(DAVA::Entity*);

    void DrawItem(DAVA::Entity* item, const DAVA::AABBox3& bbox, DAVA::int32 drawMode,
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
    EntityGroup currentSelection;
    EntityGroup recentlySelectedEntities;
    EntityGroup lastGroupSelection;
    EntityGroup objectsToSelect;
    DAVA::Vector2 selectionStartPoint;
    DAVA::Vector2 selectionEndPoint;
    DAVA::uint64 componentMaskForSelection = ALL_COMPONENTS_MASK;
    DAVA::Vector<SceneSelectionSystemDelegate*> selectionDelegates;
    ST_PivotPoint curPivotPoint = ST_PIVOT_COMMON_CENTER;
    GroupSelectionMode groupSelectionMode = GroupSelectionMode::Replace;
    bool selectionAllowed = true;
    bool applyOnPhaseEnd = false;
    bool invalidSelectionBoxes = false;
    bool selectionHasChanges = false;
    bool selecting = false;
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
