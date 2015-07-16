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


#ifndef __SCENE_CAMERA_SYSTEM_H__
#define __SCENE_CAMERA_SYSTEM_H__

#include "Entity/SceneSystem.h"
#include "Render/Highlevel/Camera.h"
#include "UI/UIEvent.h"

#include "Commands2/Command2.h"
#include "Base/Introspection.h"

class SceneCameraSystem : public DAVA::SceneSystem
{
	friend class SceneEditor2;
	friend class EditorLightSystem;

public:
	SceneCameraSystem(DAVA::Scene * scene);
	~SceneCameraSystem();

    DAVA::Camera* GetCurCamera() const;

    DAVA::Vector3 GetPointDirection(const DAVA::Vector2 &point) const;
	DAVA::Vector3 GetCameraPosition() const;
	DAVA::Vector3 GetCameraDirection() const;
    void GetRayTo2dPoint(const DAVA::Vector2 &point, DAVA::float32 maxRayLen, DAVA::Vector3 &outPointFrom, DAVA::Vector3 &outPointTo) const;

	DAVA::float32 GetMoveSpeed();

	DAVA::uint32 GetActiveSpeedIndex();
	void SetMoveSpeedArrayIndex(DAVA::uint32 index);

	void SetViewportRect(const DAVA::Rect &rect);
	const DAVA::Rect& GetViewportRect() const;

	void LookAt(const DAVA::AABBox3 &box);
	void MoveTo(const DAVA::Vector3 &pos);
	void MoveTo(const DAVA::Vector3 &pos, const DAVA::Vector3 &target);

	DAVA::Vector2 GetScreenPos(const DAVA::Vector3 &pos3) const;
	DAVA::Vector3 GetScreenPosAndDepth(const DAVA::Vector3 &pos3) const;
	DAVA::Vector3 GetScenePos(const DAVA::float32 x, const DAVA::float32 y, const DAVA::float32 z) const;

    DAVA::float32 GetDistanceToCamera() const;
    void UpdateDistanceToCamera();

    void Process(DAVA::float32 timeElapsed) override;
    void Input(DAVA::UIEvent *event) override;
    
    bool SnapEditorCameraToLandscape(bool snap);
    bool IsEditorCameraSnappedToLandscape() const;

    void MoveToSelection();
    void MoveToStep( int ofs );
    
protected:
    void OnKeyboardInput( DAVA::UIEvent *event );

	void Draw();
	void ProcessCommand(const Command2 *command, bool redo);

	void AddEntity(DAVA::Entity * entity) override;
    void RemoveEntity(DAVA::Entity * entity) override;

	void CreateDebugCameras();
	void RecalcCameraAspect();
    
	void MoveAnimate(DAVA::float32 timeElapsed);
	DAVA::Entity* GetEntityFromCamera(DAVA::Camera *camera) const;
    DAVA::Entity* GetEntityWithEditorCamera() const;

	DAVA::Rect viewportRect;
	bool debugCamerasCreated;

	DAVA::Camera* curSceneCamera;

	bool animateToNewPos;
	DAVA::float32 animateToNewPosTime;
	DAVA::Vector3 newPos;
	DAVA::Vector3 newTar;

	DAVA::Set<DAVA::Entity *> sceneCameras;

    DAVA::float32 distanceToCamera;
    
	DAVA::uint32 activeSpeedIndex;
};

#endif
