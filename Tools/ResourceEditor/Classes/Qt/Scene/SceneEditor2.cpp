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

#include "Qt/Settings/SettingsManager.h"
#include "Deprecated/SceneValidator.h"
#include "Commands2/VisibilityToolActions.h"
#include "Commands2/CustomColorsCommands2.h"
#include "Commands2/HeightmapEditorCommands2.h"
#include "Commands2/TilemaskEditorCommands.h"
#include "Commands2/RulerToolActions.h"
#include "Commands2/LandscapeEditorDrawSystemActions.h"
#include "Project/ProjectManager.h"
#include "CommandLine/SceneExporter/SceneExporter.h"

// framework
#include "Scene3D/SceneFileV2.h"
#include "Render/Highlevel/ShadowVolumeRenderPass.h"
#include "Scene3D/Systems/RenderUpdateSystem.h"

const FastName MATERIAL_FOR_REBIND = FastName("Global");

SceneEditor2::SceneEditor2()
	: Scene()
	, isLoaded(false)
	, isHUDVisible(true)
{
	renderStats.Clear();

	EditorCommandNotify *notify = new EditorCommandNotify(this);
	commandStack.SetNotify(notify);
	SafeRelease(notify);

	gridSystem = new SceneGridSystem(this);
	AddSystem(gridSystem, 0, true, renderUpdateSystem);

    cameraSystem = new SceneCameraSystem(this);
    AddSystem(cameraSystem, (1 << DAVA::Component::CAMERA_COMPONENT), true, transformSystem);

	collisionSystem = new SceneCollisionSystem(this);
	AddSystem(collisionSystem, 0, true, renderUpdateSystem);

	hoodSystem = new HoodSystem(this, cameraSystem);
	AddSystem(hoodSystem, 0, true, renderUpdateSystem);

	selectionSystem = new SceneSelectionSystem(this, collisionSystem, hoodSystem);
	AddSystem(selectionSystem, 0, true, renderUpdateSystem);

	modifSystem = new EntityModificationSystem(this, collisionSystem, cameraSystem, hoodSystem);
	AddSystem(modifSystem, 0, true, renderUpdateSystem);

	landscapeEditorDrawSystem = new LandscapeEditorDrawSystem(this);
	AddSystem(landscapeEditorDrawSystem, 0, true, renderUpdateSystem);

	heightmapEditorSystem = new HeightmapEditorSystem(this);
	AddSystem(heightmapEditorSystem, 0, true, renderUpdateSystem);

	tilemaskEditorSystem = new TilemaskEditorSystem(this);
	AddSystem(tilemaskEditorSystem, 0, true, renderUpdateSystem);

	customColorsSystem = new CustomColorsSystem(this);
	AddSystem(customColorsSystem, 0, true, renderUpdateSystem);

	visibilityToolSystem = new VisibilityToolSystem(this);
	AddSystem(visibilityToolSystem, 0, true, renderUpdateSystem);

    grassEditorSystem = new GrassEditorSystem(this);
    AddSystem(grassEditorSystem, 0);

	rulerToolSystem = new RulerToolSystem(this);
	AddSystem(rulerToolSystem, 0, true, renderUpdateSystem);

	structureSystem = new StructureSystem(this);
	AddSystem(structureSystem, 0, true, renderUpdateSystem);

    particlesSystem = new EditorParticlesSystem(this);
    AddSystem(particlesSystem, (1 << DAVA::Component::PARTICLE_EFFECT_COMPONENT), true, renderUpdateSystem);

	textDrawSystem = new TextDrawSystem(this, cameraSystem);
    AddSystem(textDrawSystem, 0, true, renderUpdateSystem);

    editorLightSystem = new EditorLightSystem(this);
    AddSystem(editorLightSystem, 1 << Component::LIGHT_COMPONENT, true, renderUpdateSystem);

	debugDrawSystem = new DebugDrawSystem(this);
	AddSystem(debugDrawSystem, 0);
	
	beastSystem = new BeastSystem(this);
	AddSystem(beastSystem, 0);
	
	ownersSignatureSystem = new OwnersSignatureSystem(this);
	AddSystem(ownersSignatureSystem, 0);
    
    staticOcclusionBuildSystem = new StaticOcclusionBuildSystem(this);
    AddSystem(staticOcclusionBuildSystem, (1 << Component::STATIC_OCCLUSION_COMPONENT) | (1 << Component::TRANSFORM_COMPONENT), true, renderUpdateSystem);

	materialSystem = new EditorMaterialSystem(this);
	AddSystem(materialSystem, 1 << Component::RENDER_COMPONENT, true, renderUpdateSystem);

	SetShadowBlendMode(ShadowPassBlendMode::MODE_BLEND_MULTIPLY);

	SceneSignals::Instance()->EmitOpened(this);

	wasChanged = false;
    
    //RenderTechnique * technique1 = RenderTechniqueSingleton::Instance()->RetainRenderTechniqueByName(FastName("~res:/Materials/Legacy/PixelLit.Opaque.material"));
    //FastNameSet set;
    //technique1->GetPassByIndex(technique1->GetIndexByName(FastName("ForwardPass")))->RecompileShader(set);

    //RenderTechnique * technique2 = RenderTechniqueSingleton::Instance()->RetainRenderTechniqueByName(FastName("~res:/Materials/Legacy/PixelLit.Alphatest.material"));
    //technique2->GetPassByIndex(technique2->GetIndexByName(FastName("ForwardPass")))->RecompileShader(set);

    //RenderTechnique * technique3 = RenderTechniqueSingleton::Instance()->RetainRenderTechniqueByName(FastName("~res:/Materials/Legacy/Textured.Opaque.material"));
    //technique3->GetPassByIndex(technique3->GetIndexByName(FastName("ForwardPass")))->RecompileShader(set);
    
    //RenderTechnique * technique4 = RenderTechniqueSingleton::Instance()->RetainRenderTechniqueByName(FastName("~res:/Materials/Legacy/Textured.Alphatest.material"));
    //technique4->GetPassByIndex(technique4->GetIndexByName(FastName("ForwardPass")))->RecompileShader(set);

    //RenderTechnique * technique5 = RenderTechniqueSingleton::Instance()->RetainRenderTechniqueByName(FastName("~res:/Materials/Legacy/Textured.Alphablend.material"));
    //technique5->GetPassByIndex(technique5->GetIndexByName(FastName("ForwardPass")))->RecompileShader(set);
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
	
	FilePath projectPath(ProjectManager::Instance()->CurProjectPath().toStdString());
	
	exporter.SetInFolder(projectPath + String("DataSource/3d/"));
    exporter.SetOutFolder(projectPath + String("Data/3d/"));
    exporter.SetOutSoundsFolder(projectPath + String("Data/Sfx/"));
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

void SceneEditor2::ClearAllCommands()
{
    commandStack.Clear();
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
    grassEditorSystem->ProcessUIEvent(event);

	if(structureSystem)
		structureSystem->ProcessUIEvent(event);

	particlesSystem->ProcessUIEvent(event);
	materialSystem->ProcessUIEvent(event);
}

void SceneEditor2::SetViewportRect(const DAVA::Rect &newViewportRect)
{
	cameraSystem->SetViewportRect(newViewportRect);
}

void SceneEditor2::Draw()
{

    RenderManager::Instance()->ClearStats();
	
//	NMaterial* global = renderSystem->GetMaterialSystem()->GetMaterial(MATERIAL_FOR_REBIND);
//	DVASSERT(global);
//	
//	if(global)
//	{
//		global->Rebind();
//	}
	
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

		materialSystem->Draw();
	}
 
    //VI: need to call Setup2DDrawing in order to draw 2d to render targets correctly
    Setup2DDrawing();
	tilemaskEditorSystem->Draw();
    //VI: restore 3d camera state
    Setup3DDrawing();

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
    grassEditorSystem->ProcessCommand(command, redo);
	
	if(structureSystem)
		structureSystem->ProcessCommand(command, redo);

	particlesSystem->ProcessCommand(command, redo);

	if(editorLightSystem)
		editorLightSystem->ProcessCommand(command, redo);
	
	if(ownersSignatureSystem)
		ownersSignatureSystem->ProcessCommand(command, redo);

	materialSystem->ProcessCommand(command, redo);

    if (landscapeEditorDrawSystem)
        landscapeEditorDrawSystem->ProcessCommand(command, redo);
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

	return Color::White;
}

void SceneEditor2::SetShadowBlendMode(DAVA::ShadowPassBlendMode::eBlend blend)
{
	if(GetRenderSystem())
	{
		GetRenderSystem()->SetShadowBlendMode(blend);
	}
}

DAVA::ShadowPassBlendMode::eBlend SceneEditor2::GetShadowBlendMode() const
{
	if(GetRenderSystem())
	{
		return GetRenderSystem()->GetShadowBlendMode();
	}

	return DAVA::ShadowPassBlendMode::MODE_BLEND_COUNT;
}

const RenderManager::Stats & SceneEditor2::GetRenderStats() const
{
    return renderStats;
}

void SceneEditor2::DisableTools(int32 toolFlags, bool saveChanges /*= true*/)
{
	if (toolFlags & LANDSCAPE_TOOL_CUSTOM_COLOR )
	{
		Exec(new ActionDisableCustomColors(this, saveChanges));
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

    if(toolFlags & LANDSCAPE_TOOL_GRASS_EDITOR)
    {
        grassEditorSystem->EnableGrassEdit(false);
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

    if(toolFlags & LANDSCAPE_TOOL_GRASS_EDITOR)
    {
        res |= grassEditorSystem->IsEnabledGrassEdit();
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

    if(grassEditorSystem->IsEnabledGrassEdit())
    {
        toolFlags |= LANDSCAPE_TOOL_GRASS_EDITOR;
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

    clonedScene->SetGlobalMaterial(GetGlobalMaterial());

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

    if(materialSystem)
    {
        RemoveSystem(materialSystem);
        SafeDelete(materialSystem);
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

void SceneEditor2::Setup2DDrawing()
{
    RenderManager::Instance()->Setup2DMatrices();
}

void SceneEditor2::Setup3DDrawing()
{
    if (currentCamera)
    {
        currentCamera->SetupDynamicParameters();
    }
}


