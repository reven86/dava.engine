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


#ifndef __ENTITY_MODIFICATION_SYSTEM_HOOD_H__
#define __ENTITY_MODIFICATION_SYSTEM_HOOD_H__

#include "Scene/SceneTypes.h"
#include "Scene/System/HoodSystem/NormalHood.h"
#include "Scene/System/HoodSystem/MoveHood.h"
#include "Scene/System/HoodSystem/ScaleHood.h"
#include "Scene/System/HoodSystem/RotateHood.h"
#include "Commands2/Command2.h"

// bullet
#include "bullet/btBulletCollisionCommon.h"

// framework
#include "Entity/SceneSystem.h"
#include "UI/UIEvent.h"

class SceneCameraSystem;

class HoodSystem : public DAVA::SceneSystem
{
	friend class SceneEditor2;

public:
	HoodSystem(DAVA::Scene * scene, SceneCameraSystem *camSys);
	~HoodSystem();

	void SetModifMode(ST_ModifMode mode);
	ST_ModifMode GetModifMode() const;

	DAVA::Vector3 GetPosition() const;
	void SetPosition(const DAVA::Vector3 &pos);
	
	void SetModifOffset(const DAVA::Vector3 &offset);
	void SetModifRotate(const DAVA::float32 &angle);
	void SetModifScale(const DAVA::float32 &scale);

	void SetModifAxis(ST_Axis axis);
	ST_Axis GetModifAxis() const;
	ST_Axis GetPassingAxis() const;

	void SetScale(DAVA::float32 scale);
	DAVA::float32 GetScale() const;

	void LockScale(bool lock);
	void LockModif(bool lock);
	void LockAxis(bool lock);

	void SetVisible(bool visible);
	bool IsVisible() const;

    virtual void Process(DAVA::float32 timeElapsed);
    virtual void Input(DAVA::UIEvent *event);

protected:
	bool lockedScale;
	bool lockedModif;
	bool lockedAxis;
	bool isVisible;

	ST_ModifMode curMode;
	ST_Axis curAxis;
	ST_Axis moseOverAxis;
	DAVA::Vector3 curPos;
	DAVA::float32 curScale;
	DAVA::Vector3 modifOffset;

	SceneCameraSystem *cameraSystem;

	void Draw();

	void ProcessCommand(const Command2 *command, bool redo);

	void AddCollObjects(const DAVA::Vector<HoodCollObject*>* objects);
	void RemCollObjects(const DAVA::Vector<HoodCollObject*>* objects);

	void ResetModifValues();

private:
	btCollisionWorld* collWorld;
	btAxisSweep3* collBroadphase;
	btDefaultCollisionConfiguration* collConfiguration;
	btCollisionDispatcher* collDispatcher;
	btIDebugDraw* collDebugDraw;

	HoodObject *curHood;

	NormalHood normalHood;
	MoveHood moveHood;
	RotateHood rotateHood;
	ScaleHood scaleHood;
};

#endif // __ENTITY_MODIFICATION_SYSTEM_HOOD_H__
