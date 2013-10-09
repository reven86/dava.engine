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



#include "Scene/SceneEditor2.h"
#include "Scene/SceneSignals.h"
#include "CommandLine/SceneExporter/SceneExporter.h"
#include "SceneEditor/EditorSettings.h"

#include "Render/Highlevel/ShadowVolumeRenderPass.h"

#include "Classes/SceneEditor/SceneValidator.h"
#include "Classes/Qt/Scene/SceneHelper.h"

// framework
#include "Scene3D/SceneFileV2.h"

SceneEditor2::SceneEditor2()
	: Scene()
	, isLoaded(false)
{
	EditorCommandNotify *notify = new EditorCommandNotify(this);
	commandStack.SetNotify(notify);
	SafeRelease(notify);

	cameraSystem = new SceneCameraSystem(this);
	AddSystem(cameraSystem, 0);

	gridSystem = new SceneGridSystem(this);
	AddSystem(gridSystem, 0);
	
	collisionSystem = new SceneCollisionSystem(this);
	AddSystem(collisionSystem, 0);

	hoodSystem = new HoodSystem(this, cameraSystem);
	AddSystem(hoodSystem, 0);

	selectionSystem = new SceneSelectionSystem(this, collisionSystem, hoodSystem);
	AddSystem(selectionSystem, 0);
	
	particlesSystem = new EditorParticlesSystem(this);
	AddSystem(particlesSystem, (1 << DAVA::Component::PARTICLE_EFFECT_COMPONENT));

	modifSystem = new EntityModificationSystem(this, collisionSystem, cameraSystem, hoodSystem);
	AddSystem(modifSystem, 0);

	landscapeEditorDrawSystem = new LandscapeEditorDrawSystem(this);
	AddSystem(landscapeEditorDrawSystem, 0);

	heightmapEditorSystem = new HeightmapEditorSystem(this);
	AddSystem(heightmapEditorSystem, 0);

	tilemaskEditorSystem = new TilemaskEditorSystem(this);
	AddSystem(tilemaskEditorSystem, 0);

	customColorsSystem = new CustomColorsSystem(this);
	AddSystem(customColorsSystem, 0);

	visibilityToolSystem = new VisibilityToolSystem(this);
	AddSystem(visibilityToolSystem, 0);

	rulerToolSystem = new RulerToolSystem(this);
	AddSystem(rulerToolSystem, 0);

	structureSystem = new StructureSystem(this);
	AddSystem(structureSystem, 0);

	editorLightSystem = new EditorLightSystem(this);
	AddSystem(editorLightSystem, 1 << Component::LIGHT_COMPONENT);

	textDrawSystem = new TextDrawSystem(this, cameraSystem);
	AddSystem(textDrawSystem, 0);

	SetShadowBlendMode(ShadowVolumeRenderPass::MODE_BLEND_MULTIPLY);

	SceneSignals::Instance()->EmitOpened(this);
}

SceneEditor2::~SceneEditor2()
{
    RemoveSystem(editorLightSystem);
    SafeDelete(editorLightSystem);

    RemoveSystem(structureSystem);
    SafeDelete(structureSystem);
    
	SceneSignals::Instance()->EmitClosed(this);
}

bool SceneEditor2::Load(const DAVA::FilePath &path)
{
	bool ret = false;

	structureSystem->LockSignals();

	Entity * rootNode = GetRootNode(path);
	if(rootNode)
	{
		ret = true;

		DAVA::Vector<DAVA::Entity*> tmpEntities;
		int entitiesCount = rootNode->GetChildrenCount();

		// optimize scene
		SceneFileV2 *sceneFile = new SceneFileV2();
		sceneFile->OptimizeScene(rootNode);
		sceneFile->Release();

		// remember all child pointers, but don't add them to scene in this cycle
		// because when entity is adding it is automatically removing from its old hierarchy
		tmpEntities.reserve(entitiesCount);
		for (DAVA::int32 i = 0; i < entitiesCount; ++i)
		{
			tmpEntities.push_back(rootNode->GetChild(i));
		}

		// now we can safely add entities into our hierarchy
		for (DAVA::int32 i = 0; i < (DAVA::int32) tmpEntities.size(); ++i)
		{
			AddNode(tmpEntities[i]);
		}

		curScenePath = path;
		isLoaded = true;

		commandStack.SetClean(true);
	}

	structureSystem->Init();
	structureSystem->UnlockSignals();

	UpdateShadowColorFromLandscape();

    SceneValidator::Instance()->ValidateSceneAndShowErrors(this);
    
	SceneSignals::Instance()->EmitLoaded(this);
	return ret;
}

bool SceneEditor2::Save(const DAVA::FilePath &path)
{
	bool ret = false;

	DAVA::Vector<DAVA::Entity *> allEntities;
	DAVA::Vector<DAVA::Entity *> editorEntities;

	// remember and remove all nodes, with name editor.*
	{
		GetChildNodes(allEntities);

		DAVA::uint32 count = allEntities.size();
		for(DAVA::uint32 i = 0; i < count; ++i)
		{
			if(allEntities[i]->GetName().find("editor.") != String::npos)
			{
				allEntities[i]->Retain();
				editorEntities.push_back(allEntities[i]);

				allEntities[i]->GetParent()->RemoveNode(allEntities[i]);
			}
		}
	}

	DAVA::SceneFileV2::eError err = SceneHelper::SaveScene(this, path);
	ret = (DAVA::SceneFileV2::ERROR_NO_ERROR == err);

	if(ret)
	{
		curScenePath = path;
		isLoaded = true;

		// mark current position in command stack as clean
		commandStack.SetClean(true);
	}

	// restore editor nodes
	{
		for(DAVA::uint32 i = 0; i < editorEntities.size(); ++i)
		{
			AddEditorEntity(editorEntities[i]);
			editorEntities[i]->Release();
		}
	}

	SceneSignals::Instance()->EmitSaved(this);
	return ret;
}

bool SceneEditor2::Save()
{
	return Save(curScenePath);
}

bool SceneEditor2::Export(const DAVA::eGPUFamily newGPU)
{
	SceneExporter exporter;
	
	KeyedArchive *keyedArchieve = EditorSettings::Instance()->GetSettings();
    FilePath projectPath(keyedArchieve->GetString(String("ProjectPath")));
	
	exporter.SetInFolder(projectPath + String("DataSource/3d/"));
    exporter.SetOutFolder(projectPath + String("Data/3d/"));
	exporter.SetGPUForExporting(newGPU);
	Set<String> errorLog;
	exporter.ExportScene(this, GetScenePath(), errorLog);
	for (Set<String>::iterator iter = errorLog.begin(); iter != errorLog.end(); ++iter)
	{
		Logger::Error("Export error: %s", iter->c_str());
	}
	return (errorLog.size() == 0);
}

DAVA::FilePath SceneEditor2::GetScenePath()
{
	return curScenePath;
}

void SceneEditor2::SetScenePath(const DAVA::FilePath &newScenePath)
{
	curScenePath = newScenePath;
}

bool SceneEditor2::CanUndo() const
{
	return commandStack.CanUndo();
}

bool SceneEditor2::CanRedo() const
{
	return commandStack.CanRedo();
}

void SceneEditor2::Undo()
{
	commandStack.Undo();
}

void SceneEditor2::Redo()
{
	commandStack.Redo();
}

void SceneEditor2::BeginBatch(const DAVA::String &text)
{
	commandStack.BeginBatch(text);
}

void SceneEditor2::EndBatch()
{
	commandStack.EndBatch();
}

void SceneEditor2::Exec(Command2 *command)
{
	commandStack.Exec(command);
}

bool SceneEditor2::IsLoaded() const
{
	return isLoaded;
}

bool SceneEditor2::IsChanged() const
{
	return (!commandStack.IsClean());
}

void SceneEditor2::SetChanged(bool changed)
{
	commandStack.SetClean(!changed);
}

void SceneEditor2::Update(float timeElapsed)
{
	Scene::Update(timeElapsed);
	gridSystem->Update(timeElapsed);
	cameraSystem->Update(timeElapsed);
	collisionSystem->Update(timeElapsed);
	hoodSystem->Update(timeElapsed);
	selectionSystem->Update(timeElapsed);
	modifSystem->Update(timeElapsed);
	landscapeEditorDrawSystem->Update(timeElapsed);
	heightmapEditorSystem->Update(timeElapsed);
	tilemaskEditorSystem->Update(timeElapsed);
	customColorsSystem->Update(timeElapsed);
	visibilityToolSystem->Update(timeElapsed);
	rulerToolSystem->Update(timeElapsed);
	structureSystem->Update(timeElapsed);
	particlesSystem->Update(timeElapsed);
	textDrawSystem->Update(timeElapsed);
	editorLightSystem->Process();
}

void SceneEditor2::PostUIEvent(DAVA::UIEvent *event)
{
	gridSystem->ProcessUIEvent(event);
	cameraSystem->ProcessUIEvent(event);
	collisionSystem->ProcessUIEvent(event);
	hoodSystem->ProcessUIEvent(event);
	selectionSystem->ProcessUIEvent(event);
	modifSystem->ProcessUIEvent(event);
	heightmapEditorSystem->ProcessUIEvent(event);
	tilemaskEditorSystem->ProcessUIEvent(event);
	customColorsSystem->ProcessUIEvent(event);
	visibilityToolSystem->ProcessUIEvent(event);
	rulerToolSystem->ProcessUIEvent(event);
	structureSystem->ProcessUIEvent(event);
	particlesSystem->ProcessUIEvent(event);
}

void SceneEditor2::SetViewportRect(const DAVA::Rect &newViewportRect)
{
	cameraSystem->SetViewportRect(newViewportRect);
}

void SceneEditor2::Draw()
{
    RenderManager::Instance()->ClearStats();
	Scene::Draw();
    renderStats = RenderManager::Instance()->GetStats();

	gridSystem->Draw();
	cameraSystem->Draw();
	collisionSystem->Draw();
	selectionSystem->Draw();
	modifSystem->Draw();
	structureSystem->Draw();
	tilemaskEditorSystem->Draw();
	particlesSystem->Draw();

	// should be last
	hoodSystem->Draw();
	textDrawSystem->Draw();
}

void SceneEditor2::EditorCommandProcess(const Command2 *command, bool redo)
{
	gridSystem->ProcessCommand(command, redo);
	cameraSystem->ProcessCommand(command, redo);
	collisionSystem->ProcessCommand(command, redo);
	selectionSystem->ProcessCommand(command, redo);
	hoodSystem->ProcessCommand(command, redo);
	modifSystem->ProcessCommand(command, redo);
	structureSystem->ProcessCommand(command, redo);
	particlesSystem->ProcessCommand(command, redo);
	editorLightSystem->ProcessCommand(command, redo);
}

void SceneEditor2::AddEditorEntity( Entity *editorEntity )
{
	if(GetChildrenCount())
	{
		InsertBeforeNode(editorEntity, GetChild(0));
	}
	else
	{
		AddNode(editorEntity);
	}
}

SceneEditor2::EditorCommandNotify::EditorCommandNotify(SceneEditor2 *_editor)
	: editor(_editor)
{ }

void SceneEditor2::EditorCommandNotify::Notify(const Command2 *command, bool redo)
{
	if(NULL != editor)
	{
		editor->EditorCommandProcess(command, redo);
		SceneSignals::Instance()->EmitCommandExecuted(editor, command, redo);
	}
}

void SceneEditor2::EditorCommandNotify::CleanChanged(bool clean)
{
	if(NULL != editor)
	{
		SceneSignals::Instance()->EmitModifyStatusChanged(editor, !clean);
	}
}

void SceneEditor2::UpdateShadowColorFromLandscape()
{
	// try to get shadow color for landscape
	Entity *land = structureSystem->FindLandscapeEntity();
	if(!land || !GetRenderSystem()) return;

	KeyedArchive * props = land->GetCustomProperties();
	if (props->IsKeyExists("ShadowColor"))
	{
		GetRenderSystem()->SetShadowRectColor(props->GetVariant("ShadowColor")->AsColor());
	}
}

void SceneEditor2::SetShadowColor( const Color &color )
{
	Entity *land = structureSystem->FindLandscapeEntity();
	if(!land) return;

	KeyedArchive * props = land->GetCustomProperties();
	if(!props) return;

	props->SetVariant("ShadowColor", VariantType(color));

	UpdateShadowColorFromLandscape();
}

const Color SceneEditor2::GetShadowColor() const
{
	if(GetRenderSystem())
		return GetRenderSystem()->GetShadowRectColor();

	return Color::White();
}

void SceneEditor2::SetShadowBlendMode( ShadowVolumeRenderPass::eBlend blend )
{
	if(GetRenderSystem())
	{
		ShadowVolumeRenderPass *shadowPass = DynamicTypeCheck<ShadowVolumeRenderPass*>( GetRenderSystem()->GetRenderPass(PASS_SHADOW_VOLUME) );
		shadowPass->SetBlendMode(blend);
	}
}

ShadowVolumeRenderPass::eBlend SceneEditor2::GetShadowBlendMode() const
{
	if(GetRenderSystem())
	{
		ShadowVolumeRenderPass *shadowPass = DynamicTypeCheck<ShadowVolumeRenderPass*>( GetRenderSystem()->GetRenderPass(PASS_SHADOW_VOLUME) );
		return shadowPass->GetBlendMode();
	}

	return ShadowVolumeRenderPass::MODE_BLEND_COUNT;
}

const RenderManager::Stats & SceneEditor2::GetRenderStats() const
{
    return renderStats;
}

