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
#include "Scene3D/Systems/RenderUpdateSystem.h"
#include "Render/Highlevel/RenderBatchArray.h"
#include "Render/Highlevel/RenderPass.h"

#include "Scene/System/CameraSystem.h"
#include "Scene/System/CollisionSystem.h"
#include "Scene/System/HoodSystem.h"
#include "Scene3D/Entity.h"
#include "Scene/System/EditorLODSystem.h"


#include <QShortcut>


namespace
{
    const FastName MATERIAL_FOR_REBIND = FastName( "Global" );
}


SceneEditor2::SceneEditor2()
    : Scene()
    , wasdSystem(nullptr)
    , rotationSystem(nullptr)
    , snapToLandscapeSystem(nullptr)
    , isLoaded(false)
    , isHUDVisible(true)
{
    EditorCommandNotify *notify = new EditorCommandNotify(this);
    commandStack.SetNotify(notify);
    SafeRelease(notify);

    gridSystem = new SceneGridSystem(this);
    AddSystem(gridSystem, 0, SCENE_SYSTEM_REQUIRE_PROCESS, renderUpdateSystem);

    cameraSystem = new SceneCameraSystem(this);
    AddSystem(cameraSystem, MAKE_COMPONENT_MASK(DAVA::Component::CAMERA_COMPONENT), SCENE_SYSTEM_REQUIRE_PROCESS | SCENE_SYSTEM_REQUIRE_INPUT, transformSystem);

    rotationSystem = new RotationControllerSystem(this);
    AddSystem(rotationSystem, MAKE_COMPONENT_MASK(Component::CAMERA_COMPONENT) | MAKE_COMPONENT_MASK(Component::ROTATION_CONTROLLER_COMPONENT), SCENE_SYSTEM_REQUIRE_PROCESS | SCENE_SYSTEM_REQUIRE_INPUT);

    snapToLandscapeSystem = new SnapToLandscapeControllerSystem(this);
    AddSystem(snapToLandscapeSystem, MAKE_COMPONENT_MASK(Component::CAMERA_COMPONENT) | MAKE_COMPONENT_MASK(Component::SNAP_TO_LANDSCAPE_CONTROLLER_COMPONENT), SCENE_SYSTEM_REQUIRE_PROCESS);

    wasdSystem = new WASDControllerSystem(this);
    AddSystem(wasdSystem, MAKE_COMPONENT_MASK(Component::CAMERA_COMPONENT) | MAKE_COMPONENT_MASK(Component::WASD_CONTROLLER_COMPONENT), SCENE_SYSTEM_REQUIRE_PROCESS);

    collisionSystem = new SceneCollisionSystem(this);
    AddSystem(collisionSystem, 0, SCENE_SYSTEM_REQUIRE_PROCESS | SCENE_SYSTEM_REQUIRE_INPUT, renderUpdateSystem);

    hoodSystem = new HoodSystem(this, cameraSystem);
    AddSystem(hoodSystem, 0, SCENE_SYSTEM_REQUIRE_PROCESS | SCENE_SYSTEM_REQUIRE_INPUT, renderUpdateSystem);

    selectionSystem = new SceneSelectionSystem(this, collisionSystem, hoodSystem);
    AddSystem(selectionSystem, 0, SCENE_SYSTEM_REQUIRE_PROCESS | SCENE_SYSTEM_REQUIRE_INPUT, renderUpdateSystem);

    modifSystem = new EntityModificationSystem(this, collisionSystem, cameraSystem, hoodSystem);
    AddSystem(modifSystem, 0, SCENE_SYSTEM_REQUIRE_PROCESS | SCENE_SYSTEM_REQUIRE_INPUT, renderUpdateSystem);

    landscapeEditorDrawSystem = new LandscapeEditorDrawSystem(this);
    AddSystem(landscapeEditorDrawSystem, 0, SCENE_SYSTEM_REQUIRE_PROCESS, renderUpdateSystem);

    heightmapEditorSystem = new HeightmapEditorSystem(this);
    AddSystem(heightmapEditorSystem, 0, SCENE_SYSTEM_REQUIRE_PROCESS | SCENE_SYSTEM_REQUIRE_INPUT, renderUpdateSystem);

    tilemaskEditorSystem = new TilemaskEditorSystem(this);
    AddSystem(tilemaskEditorSystem, 0, SCENE_SYSTEM_REQUIRE_PROCESS | SCENE_SYSTEM_REQUIRE_INPUT, renderUpdateSystem);

    customColorsSystem = new CustomColorsSystem(this);
    AddSystem(customColorsSystem, 0, SCENE_SYSTEM_REQUIRE_PROCESS | SCENE_SYSTEM_REQUIRE_INPUT, renderUpdateSystem);

    visibilityToolSystem = new VisibilityToolSystem(this);
    AddSystem(visibilityToolSystem, 0, SCENE_SYSTEM_REQUIRE_PROCESS | SCENE_SYSTEM_REQUIRE_INPUT, renderUpdateSystem);

    rulerToolSystem = new RulerToolSystem(this);
    AddSystem(rulerToolSystem, 0, SCENE_SYSTEM_REQUIRE_PROCESS | SCENE_SYSTEM_REQUIRE_INPUT, renderUpdateSystem);

    structureSystem = new StructureSystem(this);
    AddSystem(structureSystem, 0, SCENE_SYSTEM_REQUIRE_PROCESS, renderUpdateSystem);

    particlesSystem = new EditorParticlesSystem(this);
    AddSystem(particlesSystem, MAKE_COMPONENT_MASK(DAVA::Component::PARTICLE_EFFECT_COMPONENT), 0, renderUpdateSystem);

    textDrawSystem = new TextDrawSystem(this, cameraSystem);
    AddSystem(textDrawSystem, 0, SCENE_SYSTEM_REQUIRE_PROCESS, renderUpdateSystem);

    editorLightSystem = new EditorLightSystem(this);
    AddSystem(editorLightSystem, MAKE_COMPONENT_MASK(Component::LIGHT_COMPONENT), SCENE_SYSTEM_REQUIRE_PROCESS, renderUpdateSystem);

    debugDrawSystem = new DebugDrawSystem(this);
    AddSystem(debugDrawSystem, 0);

    beastSystem = new BeastSystem(this);
    AddSystem(beastSystem, 0);

    ownersSignatureSystem = new OwnersSignatureSystem(this);
    AddSystem(ownersSignatureSystem, 0);

    staticOcclusionBuildSystem = new StaticOcclusionBuildSystem(this);
    AddSystem(staticOcclusionBuildSystem, MAKE_COMPONENT_MASK(Component::STATIC_OCCLUSION_COMPONENT) | MAKE_COMPONENT_MASK(Component::TRANSFORM_COMPONENT), SCENE_SYSTEM_REQUIRE_PROCESS, renderUpdateSystem);

    materialSystem = new EditorMaterialSystem(this);
    AddSystem(materialSystem, MAKE_COMPONENT_MASK(Component::RENDER_COMPONENT), SCENE_SYSTEM_REQUIRE_PROCESS, renderUpdateSystem);

    wayEditSystem = new WayEditSystem(this, selectionSystem, collisionSystem);
    AddSystem(wayEditSystem, MAKE_COMPONENT_MASK(Component::WAYPOINT_COMPONENT), SCENE_SYSTEM_REQUIRE_PROCESS | SCENE_SYSTEM_REQUIRE_INPUT);
    structureSystem->AddDelegate(wayEditSystem);

    pathSystem = new PathSystem(this);
    AddSystem(pathSystem, MAKE_COMPONENT_MASK(Component::PATH_COMPONENT), SCENE_SYSTEM_REQUIRE_PROCESS);
    modifSystem->AddDelegate(pathSystem);
    modifSystem->AddDelegate(wayEditSystem);

    editorLODSystem = new EditorLODSystem(this);
    AddSystem(editorLODSystem, MAKE_COMPONENT_MASK(Component::LOD_COMPONENT), SCENE_SYSTEM_REQUIRE_PROCESS);

    float32 * clearColor = renderSystem->GetMainRenderPass()->GetPassConfig().colorBuffer[0].clearColor;
    clearColor[0] = clearColor[1] = clearColor[2] = .3f;
    clearColor[3] = 1.f;

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

	SceneValidator::ExtractEmptyRenderObjectsAndShowErrors(this);
    SceneValidator::Instance()->ValidateSceneAndShowErrors(this, path);
    
	SceneSignals::Instance()->EmitLoaded(this);

	return ret;
}

SceneFileV2::eError SceneEditor2::Save(const DAVA::FilePath & path, bool saveForGame /*= false*/)
{
	ExtractEditorEntities();
    
    if(landscapeEditorDrawSystem)
    {
        landscapeEditorDrawSystem->SaveTileMaskTexture();
        landscapeEditorDrawSystem->ResetTileMaskTexture();
    }
    
	DAVA::SceneFileV2::eError err = Scene::SaveScene(path, saveForGame);
	if(DAVA::SceneFileV2::ERROR_NO_ERROR == err)
	{
		curScenePath = path;
		isLoaded = true;

		// mark current position in command stack as clean
		wasChanged = false;
		commandStack.SetClean(true);
	}

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
	
	FilePath projectPath(ProjectManager::Instance()->CurProjectPath());
	
	exporter.SetInFolder(projectPath + String("DataSource/3d/"));
    exporter.SetOutFolder(projectPath + String("Data/3d/"));
	exporter.SetGPUForExporting(newGPU);

	DAVA::VariantType quality = SettingsManager::Instance()->GetValue(Settings::General_CompressionQuality);
	exporter.SetCompressionQuality((DAVA::TextureConverter::eConvertQuality)quality.AsInt32());

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

const DAVA::FilePath & SceneEditor2::GetScenePath()
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

bool SceneEditor2::IsBatchStarted() const
{
    return commandStack.IsBatchStarted();
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
    renderStats = Renderer::GetRenderStats();
    Renderer::GetRenderStats().Reset();

    Scene::Update(timeElapsed);
}

void SceneEditor2::SetViewportRect(const DAVA::Rect &newViewportRect)
{
	cameraSystem->SetViewportRect(newViewportRect);
}

void SceneEditor2::Draw()
{
    Scene::Draw();

    if (isHUDVisible)
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
 
	tilemaskEditorSystem->Draw();
    //VI: restore 3d camera state
    Setup3DDrawing();

	if(isHUDVisible)
	{
		particlesSystem->Draw();
		debugDrawSystem->Draw();
        wayEditSystem->Draw();
        pathSystem->Draw();

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

	materialSystem->ProcessCommand(command, redo);

    if (landscapeEditorDrawSystem)
        landscapeEditorDrawSystem->ProcessCommand(command, redo);
    
    pathSystem->ProcessCommand(command, redo);
    wayEditSystem->ProcessCommand(command, redo);
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

const RenderStats& SceneEditor2::GetRenderStats() const
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
    SceneEditor2 *ret = nullptr;
    FilePath tmpScenePath = FilePath::CreateWithNewExtension(curScenePath, ".tmp_exported.sc2");
    if (SceneFileV2::ERROR_NO_ERROR == SaveScene(tmpScenePath))
    {
        SceneEditor2 *sceneCopy = new SceneEditor2();
        if (SceneFileV2::ERROR_NO_ERROR == sceneCopy->LoadScene(tmpScenePath))
        {
            sceneCopy->RemoveSystems();
            ret = sceneCopy;
        }

        FileSystem::Instance()->DeleteFile(tmpScenePath);
    }

    return ret;
}

void SceneEditor2::RemoveSystems()
{
	if(editorLightSystem)
	{
        editorLightSystem->SetCameraLightEnabled(false);
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

void SceneEditor2::Setup3DDrawing()
{
    if (drawCamera)
    {
        drawCamera->SetupDynamicParameters(false);
    }
}

void SceneEditor2::Activate()
{
    SceneSignals::Instance()->EmitActivated(this);
    Scene::Activate();
}

void SceneEditor2::Deactivate()
{
    Scene::Deactivate();
    SceneSignals::Instance()->EmitDeactivated(this);
}
