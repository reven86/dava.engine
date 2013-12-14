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



#ifndef __DEBUG_DRAW_SYSTEM_H__
#define __DEBUG_DRAW_SYSTEM_H__

#include "DAVAEngine.h"
#include "Classes/Constants.h"

#include "Scene/System/CollisionSystem.h"
#include "Scene/System/SelectionSystem.h"

#include "Render/UniqueStateSet.h"

class Command2;
class DebugDrawSystem : public DAVA::SceneSystem
{
	friend class SceneEditor2;
	friend class EditorScene;

public:

	static DAVA::float32 HANGING_OBJECTS_HEIGHT;

public:
	DebugDrawSystem(DAVA::Scene * scene);
	virtual ~DebugDrawSystem();

	void SetRequestedObjectType(ResourceEditor::eSceneObjectType objectType);
	ResourceEditor::eSceneObjectType GetRequestedObjectType() const;

	void EnableHangingObjectsMode(bool enabled);
	bool HangingObjectsModeEnabled() const;

protected:

	void Draw();
	void Draw(DAVA::Entity *entity);

	void DrawObjectBoxesByType(DAVA::Entity *entity);
	void DrawUserNode(DAVA::Entity *entity);
	void DrawLightNode(DAVA::Entity *entity);
	void DrawSoundNode(DAVA::Entity *entity);
	void DrawHangingObjects(DAVA::Entity *entity);
	void DrawEntityBox(DAVA::Entity *entity, const DAVA::Color &color);
	void DrawStaticOcclusionComponent(DAVA::Entity *entity);

	//hanging objects 
	bool IsObjectHanging(DAVA::Entity * entity);
	DAVA::Vector3 GetLandscapePointAtCoordinates(const DAVA::Vector2& centerXY);

private:
	SceneCollisionSystem *collSystem;
	SceneSelectionSystem *selSystem;

	ResourceEditor::eSceneObjectType objectType;
    DAVA::Color objectTypeColor;

	bool hangingObjectsModeEnabled;
	
	DAVA::UniqueHandle debugDrawState;
};



#endif // __DEBUG_DRAW_SYSTEM_H__
