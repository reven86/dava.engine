#include "Scene/SceneEditor2.h"
#include "Scene/SceneSignals.h"

#include "Qt/Settings/SettingsManager.h"
#include "Deprecated/SceneValidator.h"
#include "Commands2/Base/RECommandStack.h"
#include "Commands2/Base/RECommandNotificationObject.h"
#include "Commands2/CustomColorsCommands2.h"
#include "Commands2/HeightmapEditorCommands2.h"
#include "Commands2/TilemaskEditorCommands.h"
#include "Commands2/LandscapeToolsToggleCommand.h"
#include "Project/ProjectManager.h"
#include "CommandLine/SceneExporter/SceneExporter.h"
#include "QtTools/ConsoleWidget/PointerSerializer.h"
#include "QtTools/DavaGLWidget/DavaRenderer.h"

// framework
#include "Scene3D/Entity.h"
#include "Scene3D/SceneFileV2.h"
#include "Scene3D/Systems/RenderUpdateSystem.h"
#include "Scene3D/Systems/StaticOcclusionSystem.h"
#include "Render/Highlevel/RenderBatchArray.h"
#include "Render/Highlevel/RenderPass.h"

#include "Scene/System/CameraSystem.h"
#include "Scene/System/CollisionSystem.h"
#include "Scene/System/HoodSystem.h"
#include "Scene/System/EditorLODSystem.h"
#include "Scene/System/EditorStatisticsSystem.h"
#include "Scene/System/VisibilityCheckSystem/VisibilityCheckSystem.h"
#include "Scene/System/EditorVegetationSystem.h"

#include <QShortcut>

namespace
{
const DAVA::FastName MATERIAL_FOR_REBIND = DAVA::FastName("Global");
}

SceneEditor2::SceneEditor2()
    : Scene()
    , commandStack(new RECommandStack())
{
    EditorCommandNotify* notify = new EditorCommandNotify(this);
    commandStack->SetNotify(notify);
    SafeRelease(notify);

    gridSystem = new SceneGridSystem(this);
    AddSystem(gridSystem, 0, SCENE_SYSTEM_REQUIRE_PROCESS, renderUpdateSystem);

    cameraSystem = new SceneCameraSystem(this);
    AddSystem(cameraSystem, MAKE_COMPONENT_MASK(DAVA::Component::CAMERA_COMPONENT), SCENE_SYSTEM_REQUIRE_PROCESS | SCENE_SYSTEM_REQUIRE_INPUT, transformSystem);

    rotationSystem = new DAVA::RotationControllerSystem(this);
    AddSystem(rotationSystem, MAKE_COMPONENT_MASK(DAVA::Component::CAMERA_COMPONENT) | MAKE_COMPONENT_MASK(DAVA::Component::ROTATION_CONTROLLER_COMPONENT), SCENE_SYSTEM_REQUIRE_PROCESS | SCENE_SYSTEM_REQUIRE_INPUT);

    snapToLandscapeSystem = new DAVA::SnapToLandscapeControllerSystem(this);
    AddSystem(snapToLandscapeSystem, MAKE_COMPONENT_MASK(DAVA::Component::CAMERA_COMPONENT) | MAKE_COMPONENT_MASK(DAVA::Component::SNAP_TO_LANDSCAPE_CONTROLLER_COMPONENT), SCENE_SYSTEM_REQUIRE_PROCESS);

    wasdSystem = new DAVA::WASDControllerSystem(this);
    AddSystem(wasdSystem, MAKE_COMPONENT_MASK(DAVA::Component::CAMERA_COMPONENT) | MAKE_COMPONENT_MASK(DAVA::Component::WASD_CONTROLLER_COMPONENT), SCENE_SYSTEM_REQUIRE_PROCESS);

    collisionSystem = new SceneCollisionSystem(this);
    AddSystem(collisionSystem, 0, SCENE_SYSTEM_REQUIRE_PROCESS | SCENE_SYSTEM_REQUIRE_INPUT, renderUpdateSystem);

    hoodSystem = new HoodSystem(this, cameraSystem);
    AddSystem(hoodSystem, 0, SCENE_SYSTEM_REQUIRE_PROCESS | SCENE_SYSTEM_REQUIRE_INPUT, renderUpdateSystem);

    modifSystem = new EntityModificationSystem(this, collisionSystem, cameraSystem, hoodSystem);
    AddSystem(modifSystem, 0, SCENE_SYSTEM_REQUIRE_INPUT, renderUpdateSystem);

    selectionSystem = new SceneSelectionSystem(this);
    AddSystem(selectionSystem, 0, SCENE_SYSTEM_REQUIRE_PROCESS | SCENE_SYSTEM_REQUIRE_INPUT, renderUpdateSystem);

    landscapeEditorDrawSystem = new LandscapeEditorDrawSystem(this);
    AddSystem(landscapeEditorDrawSystem, 0, SCENE_SYSTEM_REQUIRE_PROCESS, renderUpdateSystem);
    landscapeEditorDrawSystem->EnableSystem();

    heightmapEditorSystem = new HeightmapEditorSystem(this);
    AddSystem(heightmapEditorSystem, 0, SCENE_SYSTEM_REQUIRE_PROCESS | SCENE_SYSTEM_REQUIRE_INPUT, renderUpdateSystem);

    tilemaskEditorSystem = new TilemaskEditorSystem(this);
    AddSystem(tilemaskEditorSystem, 0, SCENE_SYSTEM_REQUIRE_PROCESS | SCENE_SYSTEM_REQUIRE_INPUT, renderUpdateSystem);

    customColorsSystem = new CustomColorsSystem(this);
    AddSystem(customColorsSystem, 0, SCENE_SYSTEM_REQUIRE_PROCESS | SCENE_SYSTEM_REQUIRE_INPUT, renderUpdateSystem);

    rulerToolSystem = new RulerToolSystem(this);
    AddSystem(rulerToolSystem, 0, SCENE_SYSTEM_REQUIRE_PROCESS | SCENE_SYSTEM_REQUIRE_INPUT, renderUpdateSystem);

    structureSystem = new StructureSystem(this);
    AddSystem(structureSystem, 0, SCENE_SYSTEM_REQUIRE_PROCESS, renderUpdateSystem);

    particlesSystem = new EditorParticlesSystem(this);
    AddSystem(particlesSystem, MAKE_COMPONENT_MASK(DAVA::Component::PARTICLE_EFFECT_COMPONENT), 0, renderUpdateSystem);

    textDrawSystem = new TextDrawSystem(this, cameraSystem);
    AddSystem(textDrawSystem, 0, SCENE_SYSTEM_REQUIRE_PROCESS, renderUpdateSystem);

    editorLightSystem = new EditorLightSystem(this);
    AddSystem(editorLightSystem, MAKE_COMPONENT_MASK(DAVA::Component::LIGHT_COMPONENT), SCENE_SYSTEM_REQUIRE_PROCESS, renderUpdateSystem);

    debugDrawSystem = new DebugDrawSystem(this);
    AddSystem(debugDrawSystem, 0);

    beastSystem = new BeastSystem(this);
    AddSystem(beastSystem, 0);

    ownersSignatureSystem = new OwnersSignatureSystem(this);
    AddSystem(ownersSignatureSystem, 0);

    staticOcclusionBuildSystem = new DAVA::StaticOcclusionBuildSystem(this);
    AddSystem(staticOcclusionBuildSystem, MAKE_COMPONENT_MASK(DAVA::Component::STATIC_OCCLUSION_COMPONENT) | MAKE_COMPONENT_MASK(DAVA::Component::TRANSFORM_COMPONENT), SCENE_SYSTEM_REQUIRE_PROCESS, renderUpdateSystem);

    materialSystem = new EditorMaterialSystem(this);
    AddSystem(materialSystem, MAKE_COMPONENT_MASK(DAVA::Component::RENDER_COMPONENT), SCENE_SYSTEM_REQUIRE_PROCESS, renderUpdateSystem);

    wayEditSystem = new WayEditSystem(this, selectionSystem, collisionSystem);
    AddSystem(wayEditSystem, MAKE_COMPONENT_MASK(DAVA::Component::WAYPOINT_COMPONENT), SCENE_SYSTEM_REQUIRE_PROCESS | SCENE_SYSTEM_REQUIRE_INPUT);
    structureSystem->AddDelegate(wayEditSystem);

    pathSystem = new PathSystem(this);
    AddSystem(pathSystem, MAKE_COMPONENT_MASK(DAVA::Component::PATH_COMPONENT), SCENE_SYSTEM_REQUIRE_PROCESS);
    modifSystem->AddDelegate(pathSystem);
    modifSystem->AddDelegate(wayEditSystem);

    editorLODSystem = new EditorLODSystem(this);
    AddSystem(editorLODSystem, MAKE_COMPONENT_MASK(DAVA::Component::LOD_COMPONENT), SCENE_SYSTEM_REQUIRE_PROCESS);

    editorStatisticsSystem = new EditorStatisticsSystem(this);
    AddSystem(editorStatisticsSystem, MAKE_COMPONENT_MASK(DAVA::Component::RENDER_COMPONENT), SCENE_SYSTEM_REQUIRE_PROCESS);

    visibilityCheckSystem = new VisibilityCheckSystem(this);
    AddSystem(visibilityCheckSystem, MAKE_COMPONENT_MASK(DAVA::Component::VISIBILITY_CHECK_COMPONENT), SCENE_SYSTEM_REQUIRE_PROCESS);

    editorVegetationSystem = new EditorVegetationSystem(this);
    AddSystem(editorVegetationSystem, MAKE_COMPONENT_MASK(DAVA::Component::RENDER_COMPONENT), 0);

    if (DAVA::Renderer::GetOptions()->IsOptionEnabled(DAVA::RenderOptions::DEBUG_DRAW_STATIC_OCCLUSION) && !staticOcclusionDebugDrawSystem)
    {
        staticOcclusionDebugDrawSystem = new StaticOcclusionDebugDrawSystem(this);
        AddSystem(staticOcclusionDebugDrawSystem, MAKE_COMPONENT_MASK(Component::STATIC_OCCLUSION_COMPONENT), 0, renderUpdateSystem);
    }

    selectionSystem->AddDelegate(modifSystem);
    selectionSystem->AddDelegate(hoodSystem);
    selectionSystem->AddDelegate(wayEditSystem);

    SceneSignals::Instance()->EmitOpened(this);

    wasChanged = false;
}

SceneEditor2::~SceneEditor2()
{
    RenderContextGuard guard;
    commandStack.reset();
    RemoveSystems();

    SceneSignals::Instance()->EmitClosed(this);
}

DAVA::SceneFileV2::eError SceneEditor2::LoadScene(const DAVA::FilePath& path)
{
    RenderContextGuard guard;
    DAVA::SceneFileV2::eError ret = Scene::LoadScene(path);
    if (ret == DAVA::SceneFileV2::ERROR_NO_ERROR)
    {
        for (DAVA::int32 i = 0, e = GetScene()->GetChildrenCount(); i < e; ++i)
        {
            structureSystem->CheckAndMarkSolid(GetScene()->GetChild(i));
        }
        curScenePath = path;
        isLoaded = true;
    }

    SceneValidator::ExtractEmptyRenderObjects(this);
    SceneValidator::Instance()->ValidateScene(this, path);

    SceneSignals::Instance()->EmitLoaded(this);

    return ret;
}

DAVA::SceneFileV2::eError SceneEditor2::SaveScene(const DAVA::FilePath& path, bool saveForGame /*= false*/)
{
    RenderContextGuard guard;
    bool cameraLightState = false;
    if (editorLightSystem != nullptr)
    {
        cameraLightState = editorLightSystem->GetCameraLightEnabled();
        editorLightSystem->SetCameraLightEnabled(false);
    }

    ExtractEditorEntities();

    DAVA::ScopedPtr<DAVA::Texture> tilemaskTexture(nullptr);
    bool needToRestoreTilemask = false;
    if (landscapeEditorDrawSystem)
    { //dirty magic to work with new saving of materials and FBO landscape texture
        tilemaskTexture = SafeRetain(landscapeEditorDrawSystem->GetTileMaskTexture());

        needToRestoreTilemask = landscapeEditorDrawSystem->SaveTileMaskTexture();
        landscapeEditorDrawSystem->ResetTileMaskTexture();
    }

    DAVA::SceneFileV2::eError err = Scene::SaveScene(path, saveForGame);
    if (DAVA::SceneFileV2::ERROR_NO_ERROR == err)
    {
        curScenePath = path;
        isLoaded = true;

        // mark current position in command stack as clean
        wasChanged = false;
        commandStack->SetClean();
    }

    if (needToRestoreTilemask)
    {
        landscapeEditorDrawSystem->SetTileMaskTexture(tilemaskTexture);
    }

    InjectEditorEntities();

    if (editorLightSystem != nullptr)
    {
        editorLightSystem->SetCameraLightEnabled(cameraLightState);
    }

    SceneSignals::Instance()->EmitSaved(this);

    return err;
}

void SceneEditor2::ExtractEditorEntities()
{
    DVASSERT(editorEntities.size() == 0);

    DAVA::Vector<DAVA::Entity*> allEntities;
    GetChildNodes(allEntities);

    DAVA::size_type count = allEntities.size();
    for (DAVA::size_type i = 0; i < count; ++i)
    {
        if (allEntities[i]->GetName().find("editor.") != DAVA::String::npos)
        {
            allEntities[i]->Retain();
            editorEntities.push_back(allEntities[i]);

            allEntities[i]->GetParent()->RemoveNode(allEntities[i]);
        }
    }
}

void SceneEditor2::InjectEditorEntities()
{
    bool isSelectionEnabled = false;
    if (selectionSystem != nullptr)
    {
        isSelectionEnabled = selectionSystem->IsSystemEnabled();
        selectionSystem->EnableSystem(false);
    }

    for (DAVA::int32 i = static_cast<DAVA::int32>(editorEntities.size()) - 1; i >= 0; i--)
    {
        AddEditorEntity(editorEntities[i]);
        editorEntities[i]->Release();
    }

    editorEntities.clear();

    if (selectionSystem != nullptr)
    {
        selectionSystem->EnableSystem(isSelectionEnabled);
    }
}

DAVA::SceneFileV2::eError SceneEditor2::SaveScene()
{
    return SaveScene(curScenePath);
}

bool SceneEditor2::Export(const SceneExporter::Params& exportingParams)
{
    DAVA::ScopedPtr<SceneEditor2> clonedScene(CreateCopyForExport());
    if (clonedScene)
    {
        SceneExporter exporter;
        exporter.SetExportingParams(exportingParams);

        const DAVA::FilePath& scenePathname = GetScenePath();
        DAVA::FilePath newScenePathname = exportingParams.dataFolder + scenePathname.GetRelativePathname(exportingParams.dataSourceFolder);
        DAVA::FileSystem::Instance()->CreateDirectory(newScenePathname.GetDirectory(), true);

        SceneExporter::ExportedObjectCollection exportedObjects;
        bool sceneExported = exporter.ExportScene(clonedScene, scenePathname, exportedObjects);
        bool objectExported = exporter.ExportObjects(exportedObjects);

        return (sceneExported && objectExported);
    }

    return false;
}

const DAVA::FilePath& SceneEditor2::GetScenePath()
{
    return curScenePath;
}

void SceneEditor2::SetScenePath(const DAVA::FilePath& newScenePath)
{
    curScenePath = newScenePath;
}

bool SceneEditor2::CanUndo() const
{
    return commandStack->CanUndo();
}

bool SceneEditor2::CanRedo() const
{
    return commandStack->CanRedo();
}

DAVA::String SceneEditor2::GetUndoText() const
{
    const DAVA::Command* undoCommand = commandStack->GetUndoCommand();
    if (undoCommand != nullptr)
    {
        return undoCommand->GetDescription();
    }
    return DAVA::String();
}

DAVA::String SceneEditor2::GetRedoText() const
{
    const DAVA::Command* redoCommand = commandStack->GetRedoCommand();
    if (redoCommand != nullptr)
    {
        return redoCommand->GetDescription();
    }
    return DAVA::String();
}

void SceneEditor2::Undo()
{
    if (commandStack->CanUndo())
    {
        commandStack->Undo();
    }
}

void SceneEditor2::Redo()
{
    if (commandStack->CanRedo())
    {
        commandStack->Redo();
    }
}

void SceneEditor2::BeginBatch(const DAVA::String& text, DAVA::uint32 commandsCount /*= 1*/)
{
    commandStack->BeginBatch(text, commandsCount);
}

void SceneEditor2::EndBatch()
{
    commandStack->EndBatch();
}

void SceneEditor2::ActivateCommandStack()
{
    commandStack->Activate();
}

void SceneEditor2::Exec(std::unique_ptr<DAVA::Command>&& command)
{
    if (command)
    {
        commandStack->Exec(std::move(command));
    }
}

void SceneEditor2::RemoveCommands(DAVA::uint32 commandId)
{
    commandStack->RemoveCommands(commandId);
}

void SceneEditor2::ClearAllCommands()
{
    commandStack->Clear();
}

const RECommandStack* SceneEditor2::GetCommandStack() const
{
    return commandStack.get();
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
    return ((!commandStack->IsClean()) || wasChanged);
}

void SceneEditor2::SetChanged()
{
    commandStack->SetChanged();
}

void SceneEditor2::Update(float timeElapsed)
{
    ++framesCount;

    Scene::Update(timeElapsed);

    renderStats = DAVA::Renderer::GetRenderStats();
}

void SceneEditor2::SetViewportRect(const DAVA::Rect& newViewportRect)
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

        if (collisionSystem)
            collisionSystem->Draw();
    }

    tilemaskEditorSystem->Draw();

    //VI: restore 3d camera state
    Setup3DDrawing();

    if (isHUDVisible)
    {
        particlesSystem->Draw();
        debugDrawSystem->Draw();
        wayEditSystem->Draw();
        pathSystem->Draw();
        visibilityCheckSystem->Draw();

        // should be last
        selectionSystem->Draw();
        hoodSystem->Draw();
        textDrawSystem->Draw();
    }
}

void SceneEditor2::EditorCommandProcess(const RECommandNotificationObject& commandNotification)
{
    if (commandNotification.IsEmpty())
    {
        return;
    }

    if (collisionSystem)
    {
        collisionSystem->ProcessCommand(commandNotification);
    }
    if (structureSystem)
    {
        structureSystem->ProcessCommand(commandNotification);
    }

    particlesSystem->ProcessCommand(commandNotification);
    materialSystem->ProcessCommand(commandNotification);

    if (landscapeEditorDrawSystem)
    {
        landscapeEditorDrawSystem->ProcessCommand(commandNotification);
    }

    pathSystem->ProcessCommand(commandNotification);
    wayEditSystem->ProcessCommand(commandNotification);
    editorLODSystem->ProcessCommand(commandNotification);

    if (selectionSystem)
    {
        selectionSystem->ProcessCommand(commandNotification);
    }
}

void SceneEditor2::AddEditorEntity(Entity* editorEntity)
{
    if (GetChildrenCount())
    {
        InsertBeforeNode(editorEntity, GetChild(0));
    }
    else
    {
        AddNode(editorEntity);
    }
}

SceneEditor2::EditorCommandNotify::EditorCommandNotify(SceneEditor2* _editor)
    : editor(_editor)
{
}

void SceneEditor2::EditorCommandNotify::Notify(const RECommandNotificationObject& commandNotification)
{
    if (nullptr != editor)
    {
        editor->EditorCommandProcess(commandNotification);
        SceneSignals::Instance()->EmitCommandExecuted(editor, commandNotification);
    }
}

void SceneEditor2::EditorCommandNotify::CleanChanged(bool clean)
{
    if (nullptr != editor)
    {
        SceneSignals::Instance()->EmitModifyStatusChanged(editor, !clean);
    }
}

void SceneEditor2::EditorCommandNotify::CanUndoChanged(bool canUndo)
{
    SceneSignals::Instance()->CanUndoStateChanged(canUndo);
}

void SceneEditor2::EditorCommandNotify::CanRedoChanged(bool canRedo)
{
    SceneSignals::Instance()->CanRedoStateChanged(canRedo);
}

void SceneEditor2::EditorCommandNotify::UndoTextChanged(const DAVA::String& undoText)
{
    SceneSignals::Instance()->UndoTextChanged(undoText);
}

void SceneEditor2::EditorCommandNotify::RedoTextChanged(const DAVA::String& redoText)
{
    SceneSignals::Instance()->RedoTextChanged(redoText);
}

const DAVA::RenderStats& SceneEditor2::GetRenderStats() const
{
    return renderStats;
}

void SceneEditor2::EnableToolsInstantly(DAVA::int32 toolFlags)
{
    if (toolFlags & LANDSCAPE_TOOL_CUSTOM_COLOR)
    {
        EnableCustomColorsCommand(this, true).Redo();
    }

    if (toolFlags & LANDSCAPE_TOOL_HEIGHTMAP_EDITOR)
    {
        EnableHeightmapEditorCommand(this).Redo();
    }

    if (toolFlags & LANDSCAPE_TOOL_TILEMAP_EDITOR)
    {
        EnableTilemaskEditorCommand(this).Redo();
    }

    if (toolFlags & LANDSCAPE_TOOL_RULER)
    {
        EnableRulerToolCommand(this).Redo();
    }

    if (toolFlags & LANDSCAPE_TOOL_NOT_PASSABLE_TERRAIN)
    {
        EnableNotPassableCommand(this).Redo();
    }
}

void SceneEditor2::DisableToolsInstantly(DAVA::int32 toolFlags, bool saveChanges /*= true*/)
{
    if (toolFlags & LANDSCAPE_TOOL_CUSTOM_COLOR)
    {
        EnableCustomColorsCommand(this, saveChanges).Undo();
    }

    if (toolFlags & LANDSCAPE_TOOL_HEIGHTMAP_EDITOR)
    {
        EnableHeightmapEditorCommand(this).Undo();
    }

    if (toolFlags & LANDSCAPE_TOOL_TILEMAP_EDITOR)
    {
        EnableTilemaskEditorCommand(this).Undo();
    }

    if (toolFlags & LANDSCAPE_TOOL_RULER)
    {
        EnableRulerToolCommand(this).Undo();
    }

    if (toolFlags & LANDSCAPE_TOOL_NOT_PASSABLE_TERRAIN)
    {
        EnableNotPassableCommand(this).Undo();
    }
}

bool SceneEditor2::IsToolsEnabled(DAVA::int32 toolFlags)
{
    bool res = false;

    if (toolFlags & LANDSCAPE_TOOL_CUSTOM_COLOR)
    {
        res |= customColorsSystem->IsLandscapeEditingEnabled();
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

DAVA::int32 SceneEditor2::GetEnabledTools()
{
    DAVA::int32 toolFlags = 0;

    if (customColorsSystem->IsLandscapeEditingEnabled())
    {
        toolFlags |= LANDSCAPE_TOOL_CUSTOM_COLOR;
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

DAVA::Entity* SceneEditor2::Clone(Entity* dstNode /*= NULL*/)
{
    if (!dstNode)
    {
        DVASSERT_MSG(DAVA::IsPointerToExactClass<SceneEditor2>(this), "Can clone only SceneEditor2");
        dstNode = new SceneEditor2();
    }

    return Scene::Clone(dstNode);
}

SceneEditor2* SceneEditor2::CreateCopyForExport()
{
    auto originalPath = curScenePath;
    auto tempName = DAVA::Format(".tmp_%llu.sc2", static_cast<DAVA::uint64>(time(nullptr)) ^ static_cast<DAVA::uint64>(reinterpret_cast<DAVA::pointer_size>(this)));

    SceneEditor2* ret = nullptr;
    DAVA::FilePath tmpScenePath = DAVA::FilePath::CreateWithNewExtension(curScenePath, tempName);
    if (DAVA::SceneFileV2::ERROR_NO_ERROR == SaveScene(tmpScenePath))
    {
        SceneEditor2* sceneCopy = new SceneEditor2();
        if (DAVA::SceneFileV2::ERROR_NO_ERROR == sceneCopy->LoadScene(tmpScenePath))
        {
            sceneCopy->RemoveSystems();
            ret = sceneCopy;
        }
        else
        {
            SafeRelease(sceneCopy);
        }

        DAVA::FileSystem::Instance()->DeleteFile(tmpScenePath);
    }

    curScenePath = originalPath; // because SaveScene overwrites curScenePath
    SceneSignals::Instance()->EmitUpdated(this);

    return ret;
}

void SceneEditor2::RemoveSystems()
{
    if (selectionSystem != nullptr)
    {
        RemoveSystem(selectionSystem);
        SafeDelete(selectionSystem);
    }

    if (editorLightSystem)
    {
        editorLightSystem->SetCameraLightEnabled(false);
        RemoveSystem(editorLightSystem);
        SafeDelete(editorLightSystem);
    }

    if (structureSystem)
    {
        RemoveSystem(structureSystem);
        SafeDelete(structureSystem);
    }

    if (landscapeEditorDrawSystem)
    {
        landscapeEditorDrawSystem->DisableSystem();
        RemoveSystem(landscapeEditorDrawSystem);
        SafeDelete(landscapeEditorDrawSystem);
    }

    if (collisionSystem)
    {
        RemoveSystem(collisionSystem);
        SafeDelete(collisionSystem);
    }

    if (materialSystem)
    {
        RemoveSystem(materialSystem);
        SafeDelete(materialSystem);
    }

    if (visibilityCheckSystem)
    {
        RemoveSystem(visibilityCheckSystem);
        SafeDelete(visibilityCheckSystem);
    }
}

void SceneEditor2::MarkAsChanged()
{
    if (!wasChanged)
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

void SceneEditor2::EnableEditorSystems()
{
    cameraSystem->EnableSystem();

    collisionSystem->EnableSystem();

    // must be last to enable selection after all systems add their entities
    selectionSystem->EnableSystem(true);
}

DAVA::uint32 SceneEditor2::GetFramesCount() const
{
    return framesCount;
}

void SceneEditor2::ResetFramesCount()
{
    framesCount = 0;
}

void LookAtSelection(SceneEditor2* scene)
{
    if (scene != nullptr)
    {
        scene->cameraSystem->MoveToSelection();
    }
}

void RemoveSelection(SceneEditor2* scene)
{
    if (scene == nullptr)
        return;

    const auto& selection = scene->selectionSystem->GetSelection();

    SelectableGroup objectsToRemove;
    for (const auto& item : selection.GetContent())
    {
        if (item.CanBeCastedTo<DAVA::Entity>())
        {
            DAVA::Entity* entity = item.AsEntity();
            if (entity->GetLocked() || entity->GetNotRemovable())
            {
                //Don't remove entity
                continue;
            }

            DAVA::Camera* camera = DAVA::GetCamera(entity);
            if (camera != nullptr && camera == scene->GetCurrentCamera())
            {
                //Don't remove current camera
                continue;
            }
        }

        objectsToRemove.Add(item.GetContainedObject(), item.GetBoundingBox());
    }

    if (objectsToRemove.IsEmpty() == false)
    {
        scene->structureSystem->Remove(objectsToRemove);
    }
}

void LockTransform(SceneEditor2* scene)
{
    if (scene != nullptr)
    {
        scene->modifSystem->LockTransform(scene->selectionSystem->GetSelection(), true);
    }
}

void UnlockTransform(SceneEditor2* scene)
{
    if (scene != nullptr)
    {
        scene->modifSystem->LockTransform(scene->selectionSystem->GetSelection(), false);
    }
}
