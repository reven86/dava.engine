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
#include "Scene/SceneHelper.h"

#include "SceneEditor/EditorSettings.h"
#include "SceneEditor/SceneValidator.h"
#include "Commands2/VisibilityToolActions.h"
#include "Commands2/CustomColorsCommands2.h"
#include "Commands2/HeightmapEditorCommands2.h"
#include "Commands2/TilemaskEditorCommands.h"
#include "Commands2/RulerToolActions.h"
#include "Commands2/LandscapeEditorDrawSystemActions.h"

#include "CommandLine/SceneExporter/SceneExporter.h"

// framework
#include "Scene3D/SceneFileV2.h"
#include "Render/Highlevel/ShadowVolumeRenderPass.h"

SceneEditor2::SceneEditor2()
	: Scene()
	, isLoaded(false)
	, isHUDVisible(true)
{
	renderStats.Clear();

	EditorCommandNotify *notify = new EditorCommandNotify(this);
	commandStack.SetNotify(notify);
	SafeRelease(notify);

	cameraSystem = new SceneCameraSystem(this);
	AddSystem(cameraSystem, (1 << DAVA::Component::CAMERA_COMPONENT));

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

	debugDrawSystem = new DebugDrawSystem(this);
	AddSystem(debugDrawSystem, 0);
	
	beastSystem = new BeastSystem(this);
	AddSystem(beastSystem, 0);
	
	ownersSignatureSystem = new OwnersSignatureSystem(this);
	AddSystem(ownersSignatureSystem, 0);

	SetShadowBlendMode(ShadowVolumeRenderPass::MODE_BLEND_MULTIPLY);

	SceneSignals::Instance()->EmitOpened(this);

	wasChanged = false;
}

SceneEditor2::~SceneEditor2()
{
	RemoveSystems();
    
	SceneSignals::Instance()->EmitClosed(this);
}

bool SceneEditor2::Load(const DAVA::FilePath &path)
{
	bool ret = structureSystem->Init(path);
    
    if(ret)
    {
        curScenePath = path;
		isLoaded = true;
        
		commandStack.SetClean(true);
    }

	UpdateShadowColorFromLandscape();

    SceneValidator::Instance()->ValidateSceneAndShowErrors(this, path);
    
	SceneSignals::Instance()->EmitLoaded(this);
	return ret;
}

SceneFileV2::eError SceneEditor2::Save(const DAVA::FilePath & path, bool saveForGame /*= false*/)
{
	ExtractEditorEntities();

	DAVA::SceneFileV2::eError err = Scene::Save(path, saveForGame);
	if(DAVA::SceneFileV2::ERROR_NO_ERROR == err)
	{
		curScenePath = path;
		isLoaded = true;

		// mark current position in command stack as clean
		wasChanged = false;
		commandStack.SetClean(true);
	}

	if(landscapeEditorDrawSystem)
		landscapeEditorDrawSystem->SaveTileMaskTexture();

	InjectEditorEntities();

	SceneSignals::Instance()->EmitSaved(this);

	return err;
}

void SceneEditor2::ExtractEditorEntities()
{
	DVASSERT(editorEntities.size() == 0);

	DAVA::Vector<DAVA::Entity *> allEntities;
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

void SceneEditor2::InjectEditorEntities()
{
	for(DAVA::int32 i = editorEntities.size() - 1; i >= 0; i--)
	{
		AddEditorEntity(editorEntities[i]);
		editorEntities[i]->Release();
	}

	editorEntities.clear();
}


SceneFileV2::eError SceneEditor2::Save()
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

	SceneEditor2 *clonedScene = CreateCopyForExport();
	exporter.ExportScene(clonedScene, GetScenePath(), errorLog);
	for (Set<String>::iterator iter = errorLog.begin(); iter != errorLog.end(); ++iter)
	{
		Logger::Error("Export error: %s", iter->c_str());
	}

	clonedScene->Release();
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

void SceneEditor2::ClearCommands(int commandId)
{
	commandStack.Clear(commandId);
}

const CommandStack* SceneEditor2::GetCommandStack() const
{
	return (&commandStack);
}

bool SceneEditor2::IsLoaded() const
{
	return isLoaded;
}

void SceneEditor2::SetHUDVisible(bool visible)
{
	isHUDVisible = visible;
	hoodSystem->LockAxis(!visible);
}

bool SceneEditor2::IsHUDVisible() const
{
	return isHUDVisible;
}

bool SceneEditor2::IsChanged() const
{
	return ((!commandStack.IsClean()) || wasChanged);
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
	
	if(collisionSystem)
		collisionSystem->Update(timeElapsed);

	hoodSystem->Update(timeElapsed);
	selectionSystem->Update(timeElapsed);
	modifSystem->Update(timeElapsed);

	if(landscapeEditorDrawSystem)
		landscapeEditorDrawSystem->Update(timeElapsed);

	heightmapEditorSystem->Update(timeElapsed);
	tilemaskEditorSystem->Update(timeElapsed);
	customColorsSystem->Update(timeElapsed);
	visibilityToolSystem->Update(timeElapsed);
	rulerToolSystem->Update(timeElapsed);
	
	if(structureSystem)
		structureSystem->Update(timeElapsed);
	
	particlesSystem->Update(timeElapsed);
	textDrawSystem->Update(timeElapsed);
	
	if(editorLightSystem)
		editorLightSystem->Process();
}

void SceneEditor2::PostUIEvent(DAVA::UIEvent *event)
{
	gridSystem->ProcessUIEvent(event);
	cameraSystem->ProcessUIEvent(event);
	if(collisionSystem)
		collisionSystem->ProcessUIEvent(event);
	hoodSystem->ProcessUIEvent(event);
	selectionSystem->ProcessUIEvent(event);
	modifSystem->ProcessUIEvent(event);
	heightmapEditorSystem->ProcessUIEvent(event);
	tilemaskEditorSystem->ProcessUIEvent(event);
	customColorsSystem->ProcessUIEvent(event);
	visibilityToolSystem->ProcessUIEvent(event);
	rulerToolSystem->ProcessUIEvent(event);

	if(structureSystem)
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

	if(isHUDVisible)
	{
		gridSystem->Draw();
		cameraSystem->Draw();

		if(collisionSystem)
			collisionSystem->Draw();

		modifSystem->Draw();

		if(structureSystem)
			structureSystem->Draw();
	}

	tilemaskEditorSystem->Draw();

	if(isHUDVisible)
	{
		particlesSystem->Draw();
		debugDrawSystem->Draw();

		// should be last
		selectionSystem->Draw();
		hoodSystem->Draw();
		textDrawSystem->Draw();
	}
}

void SceneEditor2::EditorCommandProcess(const Command2 *command, bool redo)
{
	gridSystem->ProcessCommand(command, redo);
	cameraSystem->ProcessCommand(command, redo);

	if(collisionSystem)
		collisionSystem->ProcessCommand(command, redo);

	selectionSystem->ProcessCommand(command, redo);
	hoodSystem->ProcessCommand(command, redo);
	modifSystem->ProcessCommand(command, redo);
	
	if(structureSystem)
		structureSystem->ProcessCommand(command, redo);

	particlesSystem->ProcessCommand(command, redo);

	if(editorLightSystem)
		editorLightSystem->ProcessCommand(command, redo);
	
	if(ownersSignatureSystem)
		ownersSignatureSystem->ProcessCommand(command, redo);
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
	Entity *land = FindLandscapeEntity(this);
	if(!land || !GetRenderSystem()) return;

	KeyedArchive * props = land->GetCustomProperties();
	if (props->IsKeyExists("ShadowColor"))
	{
		GetRenderSystem()->SetShadowRectColor(props->GetVariant("ShadowColor")->AsColor());
	}
}

void SceneEditor2::SetShadowColor( const Color &color )
{
	Entity *land = FindLandscapeEntity(this);
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

void SceneEditor2::DisableTools(int32 toolFlags)
{
	if (toolFlags & LANDSCAPE_TOOL_CUSTOM_COLOR)
	{
		Exec(new ActionDisableCustomColors(this));
	}
	
	if (toolFlags & LANDSCAPE_TOOL_VISIBILITY)
	{
		Exec(new ActionDisableVisibilityTool(this));
	}
	
	if (toolFlags & LANDSCAPE_TOOL_HEIGHTMAP_EDITOR)
	{
		Exec(new ActionDisableHeightmapEditor(this));
	}
	
	if (toolFlags & LANDSCAPE_TOOL_TILEMAP_EDITOR)
	{
		Exec(new ActionDisableTilemaskEditor(this));
	}
	
	if (toolFlags & LANDSCAPE_TOOL_RULER)
	{
		Exec(new ActionDisableRulerTool(this));
	}
	
	if (toolFlags & LANDSCAPE_TOOL_NOT_PASSABLE_TERRAIN)
	{
		Exec(new ActionDisableNotPassable(this));
	}
}

bool SceneEditor2::IsToolsEnabled(int32 toolFlags)
{
	bool res = false;

	if (toolFlags & LANDSCAPE_TOOL_CUSTOM_COLOR)
	{
		res |= customColorsSystem->IsLandscapeEditingEnabled();
	}
	
	if (toolFlags & LANDSCAPE_TOOL_VISIBILITY)
	{
		res |= visibilityToolSystem->IsLandscapeEditingEnabled();
	}
	
	if (toolFlags & LANDSCAPE_TOOL_HEIGHTMAP_EDITOR)
	{
		res |= heightmapEditorSystem->IsLandscapeEditingEnabled();
	}
	
	if (toolFlags & LANDSCAPE_TOOL_TILEMAP_EDITOR)
	{
		res |= tilemaskEditorSystem->IsLandscapeEditingEnabled();
	}
	
	if (toolFlags & LANDSCAPE_TOOL_RULER)
	{
		res |= rulerToolSystem->IsLandscapeEditingEnabled();
	}
	
	if (toolFlags & LANDSCAPE_TOOL_NOT_PASSABLE_TERRAIN)
	{
		res |= landscapeEditorDrawSystem->IsNotPassableTerrainEnabled();
	}

	return res;
}

int32 SceneEditor2::GetEnabledTools()
{
	int32 toolFlags = 0;
	
	if (customColorsSystem->IsLandscapeEditingEnabled())
	{
		toolFlags |= LANDSCAPE_TOOL_CUSTOM_COLOR;
	}
	
	if (visibilityToolSystem->IsLandscapeEditingEnabled())
	{
		toolFlags |= LANDSCAPE_TOOL_VISIBILITY;
	}
	
	if (heightmapEditorSystem->IsLandscapeEditingEnabled())
	{
		toolFlags |= LANDSCAPE_TOOL_HEIGHTMAP_EDITOR;
	}
	
	if (tilemaskEditorSystem->IsLandscapeEditingEnabled())
	{
		toolFlags |= LANDSCAPE_TOOL_TILEMAP_EDITOR;
	}
	
	if (rulerToolSystem->IsLandscapeEditingEnabled())
	{
		toolFlags |= LANDSCAPE_TOOL_RULER;
	}
	
	if (landscapeEditorDrawSystem->IsNotPassableTerrainEnabled())
	{
		toolFlags |= LANDSCAPE_TOOL_NOT_PASSABLE_TERRAIN;
	}
	
	return toolFlags;
}

Entity* SceneEditor2::Clone( Entity *dstNode /*= NULL*/ )
{
	if(!dstNode)
	{
		DVASSERT_MSG(IsPointerToExactClass<SceneEditor2>(this), "Can clone only SceneEditor2");
		dstNode = new SceneEditor2();
	}
	
	return Scene::Clone(dstNode);
}

SceneEditor2 * SceneEditor2::CreateCopyForExport()
{
	SceneEditor2 *clonedScene = new SceneEditor2();
	clonedScene->RemoveSystems();

	return (SceneEditor2 *)Clone(clonedScene);
}

void SceneEditor2::RemoveSystems()
{
	if(editorLightSystem)
	{
		RemoveSystem(editorLightSystem);
		SafeDelete(editorLightSystem);
	}

	if(structureSystem)
	{
		RemoveSystem(structureSystem);
		SafeDelete(structureSystem);
	}

	if(landscapeEditorDrawSystem)
	{
		RemoveSystem(landscapeEditorDrawSystem);
		SafeDelete(landscapeEditorDrawSystem);
	}

	if(collisionSystem)
	{
		RemoveSystem(collisionSystem);
		SafeDelete(collisionSystem);
	}
	
}

void SceneEditor2::MarkAsChanged()
{
	if(!wasChanged)
	{
		wasChanged = true;
		SceneSignals::Instance()->EmitModifyStatusChanged(this, wasChanged);
	}
}