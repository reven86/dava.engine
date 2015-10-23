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

	inline void EnableHangingObjectsMode(bool enabled);
	inline bool HangingObjectsModeEnabled() const;

    //need be moved to testing tool
    DAVA_DEPRECATED(inline void EnableSwithcesWithDifferentLODsMode(bool enabled));
    DAVA_DEPRECATED(inline bool SwithcesWithDifferentLODsModeEnabled() const);

protected:

	void Draw();
	void Draw(DAVA::Entity *entity);

	void DrawObjectBoxesByType(DAVA::Entity *entity);
	void DrawUserNode(DAVA::Entity *entity);
	void DrawLightNode(DAVA::Entity *entity);
    void DrawSoundNode(DAVA::Entity *entity);
    void DrawSelectedSoundNode(DAVA::Entity *entity);
	void DrawHangingObjects(DAVA::Entity *entity);
	void DrawEntityBox(DAVA::Entity *entity, const DAVA::Color &color);	
	void DrawSwitchesWithDifferentLods(DAVA::Entity *entity);
	void DrawWindNode(DAVA::Entity *entity);

	//hanging objects 
	bool IsObjectHanging(DAVA::Entity * entity);
    DAVA::Vector3 GetLandscapePointAtCoordinates(const DAVA::Vector2& centerXY);

	static void GetLowestVertexes(const DAVA::RenderObject *ro, DAVA::Vector<DAVA::Vector3> &vertexes, const DAVA::Vector3 & scale);
	static DAVA::float32 GetMinimalZ(const DAVA::RenderObject *ro);

private:
	SceneCollisionSystem *collSystem;
	SceneSelectionSystem *selSystem;

	ResourceEditor::eSceneObjectType objectType;
    DAVA::Color objectTypeColor;

	bool hangingObjectsModeEnabled;
    bool switchesWithDifferentLodsEnabled;
};

inline void DebugDrawSystem::EnableHangingObjectsMode( bool enabled )
{
    hangingObjectsModeEnabled = enabled;
}

inline bool DebugDrawSystem::HangingObjectsModeEnabled() const
{
    return hangingObjectsModeEnabled;
}


inline void DebugDrawSystem::EnableSwithcesWithDifferentLODsMode(bool enabled )
{
    switchesWithDifferentLodsEnabled = enabled;
}

inline bool DebugDrawSystem::SwithcesWithDifferentLODsModeEnabled() const
{
    return switchesWithDifferentLodsEnabled;
}


#endif // __DEBUG_DRAW_SYSTEM_H__
