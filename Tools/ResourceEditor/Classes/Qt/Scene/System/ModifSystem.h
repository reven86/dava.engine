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

#include "Commands2/Command2.h"

#include "Entity/SceneSystem.h"
#include "Scene3D/Entity.h"
#include "UI/UIEvent.h"

#include "Scene/SceneTypes.h"
#include "Render/Highlevel/RenderObject.h"

class SceneCollisionSystem;
class SceneCameraSystem;
class EntityGroup;
class HoodSystem;

class EntityModificationSystemDelegate
{
public:
    virtual ~EntityModificationSystemDelegate() {}

    virtual void WillClone(DAVA::Entity *origEntity) = 0;
    virtual void DidCloned(DAVA::Entity *origEntity, DAVA::Entity *newEntity) = 0;
};

class EntityModificationSystem : public DAVA::SceneSystem
{
	friend class SceneEditor2;

public:
	EntityModificationSystem(DAVA::Scene * scene, SceneCollisionSystem *colSys, SceneCameraSystem *camSys, HoodSystem *hoodSys);
	~EntityModificationSystem();

	ST_Axis GetModifAxis() const;
	void SetModifAxis(ST_Axis axis);

	ST_ModifMode GetModifMode() const;
	void SetModifMode(ST_ModifMode mode);

	bool GetLandscapeSnap() const;
	void SetLandscapeSnap(bool snap);

	void PlaceOnLandscape(const EntityGroup &entities);
	void ResetTransform(const EntityGroup &entities);

    void MovePivotZero(const EntityGroup &entities);
    void MovePivotCenter(const EntityGroup &entities);

    void LockTransform(const EntityGroup &entities, bool lock);

	bool InModifState() const;
	bool InCloneState() const;
    bool InCloneDoneState() const;
	bool ModifCanStart(const EntityGroup &selectedEntities) const;

	virtual void RemoveEntity(DAVA::Entity * entity);
	virtual void Process(DAVA::float32 timeElapsed);
    virtual void Input(DAVA::UIEvent *event);

    void SetCopyDelegate(EntityModificationSystemDelegate *delegate);

protected:
	SceneCollisionSystem *collisionSystem;
	SceneCameraSystem *cameraSystem;
	HoodSystem* hoodSystem;

	void Draw();

	void ProcessCommand(const Command2 *command, bool redo);

protected:
	struct EntityToModify
	{
		DAVA::Entity* entity;
		DAVA::Matrix4 inversedParentWorldTransform;
		DAVA::Matrix4 originalParentWorldTransform;
		DAVA::Matrix4 originalTransform;
		DAVA::Vector3 originalCenter;
		DAVA::Matrix4 moveToZeroPos;
		DAVA::Matrix4 moveFromZeroPos;
	};

	enum CloneState
	{
		CLONE_DONT,
		CLONE_NEED,
		CLONE_DONE
	};

    enum BakeMode
    {
        BAKE_ZERO_PIVOT,
        BAKE_CENTER_PIVOT
    };

	CloneState cloneState;

	bool inModifState;
    bool isOrthoModif;
	bool modified;

	ST_ModifMode curMode;
	ST_Axis curAxis;

	bool snapToLandscape;

	// starting modification pos
	DAVA::Vector3 modifStartPos3d;
	DAVA::Vector2 modifStartPos2d;

	// entities to modify
	DAVA::Vector<EntityToModify> modifEntities;
	DAVA::Vector<DAVA::Entity *> clonedEntities;

	// values calculated, when starting modification
	ST_PivotPoint modifPivotPoint;
	DAVA::Vector3 modifEntitiesCenter;
	DAVA::Matrix4 moveToZeroPosRelativeCenter;
	DAVA::Matrix4 moveFromZeroPosRelativeCenter;
	DAVA::Vector2 rotateNormal;
	DAVA::Vector3 rotateAround;
	DAVA::float32 crossXY;
	DAVA::float32 crossXZ;
	DAVA::float32 crossYZ;

	void BeginModification(const EntityGroup &entities);
	void EndModification();

	void CloneBegin();
	void CloneEnd();

	void ApplyModification();
	bool ModifCanStartByMouse(const EntityGroup &selectedEntities) const;

	DAVA::Vector3 CamCursorPosToModifPos(DAVA::Camera *camera, DAVA::Vector2 pos);
	DAVA::Vector2 Cam2dProjection(const DAVA::Vector3 &from, const DAVA::Vector3 &to);

	DAVA::Vector3 Move(const DAVA::Vector3 &newPos3d);
	DAVA::float32 Rotate(const DAVA::Vector2 &newPos2d);
	DAVA::float32 Scale(const DAVA::Vector2 &newPos2d);
    void BakeGeometry(const EntityGroup &entities, BakeMode mode);
    void SearchEntitiesWithRenderObject(DAVA::RenderObject *ro, DAVA::Entity *root, DAVA::Set<DAVA::Entity *> &result);

	DAVA::Matrix4 SnapToLandscape(const DAVA::Vector3 &point, const DAVA::Matrix4 &originalParentTransform) const;
	bool IsEntityContainRecursive(const DAVA::Entity *entity, const DAVA::Entity *child) const;

private:
    DAVA::List<EntityModificationSystemDelegate *> delegates;
};

#endif //__ENTITY_MODIFICATION_SYSTEM_H__
