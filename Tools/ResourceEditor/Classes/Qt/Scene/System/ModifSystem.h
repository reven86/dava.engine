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


#ifndef __ENTITY_MODIFICATION_SYSTEM_H__
#define __ENTITY_MODIFICATION_SYSTEM_H__

#include "Commands2/Base/Command2.h"
#include "SystemDelegates.h"

#include "Entity/SceneSystem.h"
#include "Scene3D/Entity.h"
#include "UI/UIEvent.h"

#include "Scene/SceneTypes.h"
#include "Scene/SelectableGroup.h"
#include "Render/Highlevel/RenderObject.h"

class SceneCollisionSystem;
class SceneCameraSystem;
class HoodSystem;

class EntityModificationSystem : public DAVA::SceneSystem, public SceneSelectionSystemDelegate
{
    friend class SceneEditor2;

public:
    EntityModificationSystem(DAVA::Scene* scene, SceneCollisionSystem* colSys, SceneCameraSystem* camSys, HoodSystem* hoodSys);
    ~EntityModificationSystem();

    ST_Axis GetModifAxis() const;
    void SetModifAxis(ST_Axis axis);

    Selectable::TransformType GetTransformType() const;
    void SetTransformType(Selectable::TransformType mode);

    bool GetLandscapeSnap() const;
    void SetLandscapeSnap(bool snap);

    void PlaceOnLandscape(const SelectableGroup& entities);
    void ResetTransform(const SelectableGroup& entities);

    void MovePivotZero(const SelectableGroup& entities);
    void MovePivotCenter(const SelectableGroup& entities);

    void LockTransform(const SelectableGroup& entities, bool lock);

    bool InModifState() const;
    bool InCloneState() const;
    bool InCloneDoneState() const;

    bool ModifCanStart(const SelectableGroup& objects) const;
    bool ModifCanStartByMouse(const SelectableGroup& objects) const;

    void RemoveEntity(DAVA::Entity* entity) override;
    void Process(DAVA::float32 timeElapsed) override;
    void Input(DAVA::UIEvent* event) override;

    void AddDelegate(EntityModificationSystemDelegate* delegate);
    void RemoveDelegate(EntityModificationSystemDelegate* delegate);

    void ApplyMoveValues(ST_Axis axis, const SelectableGroup& entities, const DAVA::Vector3& values, bool absoluteTransform);
    void ApplyRotateValues(ST_Axis axis, const SelectableGroup& entities, const DAVA::Vector3& values, bool absoluteTransform);
    void ApplyScaleValues(ST_Axis axis, const SelectableGroup& entities, const DAVA::Vector3& values, bool absoluteTransform);

protected:
    struct EntityToModify
    {
        Selectable object;

        DAVA::Matrix4 inversedParentWorldTransform;
        DAVA::Matrix4 originalParentWorldTransform;
        DAVA::Matrix4 originalTransform;

        DAVA::Matrix4 toLocalZero;
        DAVA::Matrix4 fromLocalZero;
        DAVA::Matrix4 toWorldZero;
        DAVA::Matrix4 fromWorldZero;
    };

    enum CloneState : DAVA::uint32
    {
        CLONE_DONT,
        CLONE_NEED,
        CLONE_DONE
    };

    enum BakeMode : DAVA::uint32
    {
        BAKE_ZERO_PIVOT,
        BAKE_CENTER_PIVOT
    };

    SelectableGroup BeginModification(const SelectableGroup& entities);
    void EndModification();

    void CloneBegin();
    void CloneEnd();

    void ApplyModification();

    DAVA::Vector3 CamCursorPosToModifPos(DAVA::Camera* camera, DAVA::Vector2 pos);
    DAVA::Vector2 Cam2dProjection(const DAVA::Vector3& from, const DAVA::Vector3& to);

    DAVA::Vector3 Move(const DAVA::Vector3& newPos3d);
    DAVA::float32 Rotate(const DAVA::Vector2& newPos2d);
    DAVA::float32 Scale(const DAVA::Vector2& newPos2d);
    void BakeGeometry(const SelectableGroup& entities, BakeMode mode);
    void SearchEntitiesWithRenderObject(DAVA::RenderObject* ro, DAVA::Entity* root, DAVA::Set<DAVA::Entity*>& result);

    DAVA::Matrix4 SnapToLandscape(const DAVA::Vector3& point, const DAVA::Matrix4& originalParentTransform) const;
    bool IsEntityContainRecursive(const DAVA::Entity* entity, const DAVA::Entity* child) const;

    bool AllowPerformSelectionHavingCurrent(const SelectableGroup& currentSelection) override;
    bool AllowChangeSelectionReplacingCurrent(const SelectableGroup& currentSelection, const SelectableGroup& newSelection) override;

private:
    SceneCollisionSystem* collisionSystem = nullptr;
    SceneCameraSystem* cameraSystem = nullptr;
    HoodSystem* hoodSystem = nullptr;

    // entities to modify
    DAVA::Vector<EntityToModify> modifEntities;
    DAVA::Vector<DAVA::Entity*> clonedEntities;
    DAVA::List<EntityModificationSystemDelegate*> delegates;

    // values calculated, when starting modification
    DAVA::Vector3 modifEntitiesCenter;
    DAVA::Vector3 modifStartPos3d;
    DAVA::Vector2 modifStartPos2d;
    DAVA::Vector2 rotateNormal;
    DAVA::Vector3 rotateAround;
    DAVA::float32 crossXY = 0.0f;
    DAVA::float32 crossXZ = 0.0f;
    DAVA::float32 crossYZ = 0.0f;

    CloneState cloneState = CloneState::CLONE_DONT;
    Selectable::TransformType transformType = Selectable::TransformType::Disabled;
    ST_Axis curAxis = ST_Axis::ST_AXIS_NONE;

    bool inModifState = false;
    bool isOrthoModif = false;
    bool modified = false;
    bool snapToLandscape = false;
};

#endif //__ENTITY_MODIFICATION_SYSTEM_H__
