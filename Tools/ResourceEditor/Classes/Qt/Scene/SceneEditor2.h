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



#ifndef __SCENE_EDITOR_PROXY_H__
#define __SCENE_EDITOR_PROXY_H__

#include <QObject>
#include "UI/UIEvent.h"
#include "Scene3D/Scene.h"
#include "Base/StaticSingleton.h"

#include "Commands2/CommandStack.h"

#include "Scene/System/CameraSystem.h"
#include "Scene/System/CollisionSystem.h"
#include "Scene/System/GridSystem.h"
#include "Scene/System/HoodSystem.h"
#include "Scene/System/SelectionSystem.h"
#include "Scene/System/ModifSystem.h"
#include "Scene/System/LandscapeEditorDrawSystem.h"
#include "Scene/System/HeightmapEditorSystem.h"
#include "Scene/System/TilemaskEditorSystem.h"
#include "Scene/System/CustomColorsSystem.h"
#include "Scene/System/VisibilityToolSystem.h"
#include "Scene/System/RulerToolSystem.h"
#include "Scene/System/StructureSystem.h"
#include "Scene/System/EditorParticlesSystem.h"
#include "Scene/System/EditorLightSystem.h"
#include "Scene/System/TextDrawSystem.h"

class SceneEditor2 : public DAVA::Scene
{
public:
	SceneEditor2();
	~SceneEditor2();

	// editor systems
	SceneCameraSystem *cameraSystem;
	SceneCollisionSystem *collisionSystem;
	SceneGridSystem *gridSystem;
	HoodSystem *hoodSystem;
	SceneSelectionSystem *selectionSystem;
	EntityModificationSystem *modifSystem;
	LandscapeEditorDrawSystem* landscapeEditorDrawSystem;
	HeightmapEditorSystem* heightmapEditorSystem;
	TilemaskEditorSystem* tilemaskEditorSystem;
	CustomColorsSystem* customColorsSystem;
	VisibilityToolSystem* visibilityToolSystem;
	RulerToolSystem* rulerToolSystem;
	StructureSystem *structureSystem;
	EditorParticlesSystem *particlesSystem;
	EditorLightSystem *editorLightSystem;
	TextDrawSystem *textDrawSystem;


	// save/load
	bool Load(const DAVA::FilePath &path);
	bool Save(const DAVA::FilePath &path);
	bool Save();
	bool Export(const DAVA::eGPUFamily newGPU);

	DAVA::FilePath GetScenePath();
	void SetScenePath(const DAVA::FilePath &newScenePath);

	// commands
	bool CanUndo() const;
	bool CanRedo() const;

	void Undo();
	void Redo();

	void BeginBatch(const DAVA::String &text);
	void EndBatch();

	void Exec(Command2 *command);

	// checks whether the scene changed since the last save
	bool IsLoaded() const;
	bool IsChanged() const;
	void SetChanged(bool changed);

	// DAVA events
	void PostUIEvent(DAVA::UIEvent *event);

	// this function should be called each time UI3Dview changes its position
	// viewport rect is used to calc. ray from camera to any 2d point on this viewport
	void SetViewportRect(const DAVA::Rect &newViewportRect);

	//Insert entity to begin of scene hierarchy to display editor entities at one place on top og scene tree
	void AddEditorEntity(Entity *editorEntity);

	void SetShadowColor(const Color &color);
	const Color GetShadowColor() const;
	void UpdateShadowColorFromLandscape();

	void SetShadowBlendMode(ShadowVolumeRenderPass::eBlend blend);
	ShadowVolumeRenderPass::eBlend GetShadowBlendMode() const;

    const RenderManager::Stats & GetRenderStats() const;

protected:
	bool isLoaded;

	DAVA::FilePath curScenePath;
	CommandStack commandStack;
    
    RenderManager::Stats renderStats;


	virtual void EditorCommandProcess(const Command2 *command, bool redo);
	virtual void Update(float timeElapsed);
	virtual void Draw();

private:
	friend struct EditorCommandNotify;

	struct EditorCommandNotify : public CommandNotify
	{
		SceneEditor2* editor;

		EditorCommandNotify(SceneEditor2 *_editor);
		virtual void Notify(const Command2 *command, bool redo);
		virtual void CleanChanged(bool clean);
	};
};

#endif // __SCENE_EDITOR_PROXY_H__
