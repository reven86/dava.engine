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

// framework
#include "Entity/SceneSystem.h"
#include "Scene3D/Entity.h"
#include "UI/UIEvent.h"

#include "Render/UniqueStateSet.h"

class SceneCollisionSystem;
class HoodSystem;

class SceneSelectionSystem : public DAVA::SceneSystem
{
	friend class SceneEditor2;
	friend class EntityModificationSystem;

public:
	SceneSelectionSystem(DAVA::Scene * scene, SceneCollisionSystem *collSys, HoodSystem *hoodSys);
	~SceneSelectionSystem();

	void SetSelection(DAVA::Entity *entity);
	void AddSelection(DAVA::Entity *entity);
	void RemSelection(DAVA::Entity *entity);
	void Clear();

	EntityGroup GetSelection() const;

	size_t GetSelectionCount() const;
	DAVA::Entity* GetSelectionEntity(int index) const;

	void SetDrawMode(int mode);
	int GetDrawMode() const;

	void SetPivotPoint(ST_PivotPoint pp);
	ST_PivotPoint GetPivotPoint() const;

	void SetSelectionAllowed(bool allowed);
	bool IsSelectionAllowed() const;

	virtual void SetLocked(bool lock);

	DAVA::AABBox3 GetSelectionAABox(int index) const;
	DAVA::AABBox3 GetSelectionAABox(DAVA::Entity *entity) const;
	DAVA::AABBox3 GetSelectionAABox(DAVA::Entity *entity, const DAVA::Matrix4 &transform) const;

	void ForceEmitSignals();

    DAVA::Entity* GetSelectableEntity(DAVA::Entity* entity);

protected:
	void Update(DAVA::float32 timeElapsed);
	void Draw();

	void ProcessUIEvent(DAVA::UIEvent *event);
	void ProcessCommand(const Command2 *command, bool redo);

	void UpdateHoodPos() const;
	void SelectedItemsWereModified();

	EntityGroup GetSelecetableFromCollision(const EntityGroup *collisionEntities);
    

private:
	int drawMode;
	bool selectionAllowed;
	bool applyOnPhaseEnd;

	SceneCollisionSystem *collisionSystem;
	HoodSystem* hoodSystem;

	bool selectionHasChanges;
	EntityGroup curSelections;
	EntityGroup curDeselections;

	DAVA::Entity *lastSelection;

	ST_PivotPoint curPivotPoint;
	
	DAVA::UniqueHandle selectionNormalDrawState;
	DAVA::UniqueHandle selectionDepthDrawState;
};

#endif //__SCENE_SELECTION_SYSTEM_H__
