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

#include "Main/Request.h"
#include "Commands2/Base/CommandStack.h"
#include "Settings/SettingsManager.h"

//TODO: move all includes to .cpp file
#include "Scene/System/GridSystem.h"
#include "Scene/System/SelectionSystem.h"
#include "Scene/System/ModifSystem.h"
#include "Scene/System/LandscapeEditorDrawSystem.h"
#include "Scene/System/HeightmapEditorSystem.h"
#include "Scene/System/TilemaskEditorSystem.h"
#include "Scene/System/CustomColorsSystem.h"
#include "Scene/System/RulerToolSystem.h"
#include "Scene/System/StructureSystem.h"
#include "Scene/System/EditorParticlesSystem.h"
#include "Scene/System/EditorLightSystem.h"
#include "Scene/System/TextDrawSystem.h"
#include "Scene/System/DebugDrawSystem.h"
#include "Scene/System/BeastSystem.h"
#include "Scene/System/OwnersSignatureSystem.h"
#include "Scene/System/EditorMaterialSystem.h"
#include "Scene/System/WayEditSystem.h"
#include "Scene/System/PathSystem.h"

#include "Scene3D/Systems/StaticOcclusionBuildSystem.h"
#include "Scene3D/Systems/Controller/RotationControllerSystem.h"
#include "Scene3D/Systems/Controller/SnapToLandscapeControllerSystem.h"
#include "Scene3D/Systems/Controller/WASDControllerSystem.h"

class SceneCameraSystem;
class SceneCollisionSystem;
class HoodSystem;
class EditorLODSystem;
class EditorStatisticsSystem;
class FogSettingsChangedReceiver;
class VisibilityCheckSystem;

class SceneEditor2 : public DAVA::Scene
{
public:
    enum LandscapeTools : DAVA::uint32
    {
        LANDSCAPE_TOOL_CUSTOM_COLOR = 1 << 0,
        LANDSCAPE_TOOL_HEIGHTMAP_EDITOR = 1 << 1,
        LANDSCAPE_TOOL_TILEMAP_EDITOR = 1 << 2,
        LANDSCAPE_TOOL_RULER = 1 << 3,
        LANDSCAPE_TOOL_NOT_PASSABLE_TERRAIN = 1 << 4,

        LANDSCAPE_TOOLS_ALL = LANDSCAPE_TOOL_CUSTOM_COLOR | LANDSCAPE_TOOL_HEIGHTMAP_EDITOR | LANDSCAPE_TOOL_TILEMAP_EDITOR |
        LANDSCAPE_TOOL_RULER |
        LANDSCAPE_TOOL_NOT_PASSABLE_TERRAIN
    };

    SceneEditor2();
    ~SceneEditor2() override;

    // editor systems
    SceneCameraSystem* cameraSystem;
    SceneCollisionSystem* collisionSystem;
    SceneGridSystem* gridSystem;
    HoodSystem* hoodSystem;
    SceneSelectionSystem* selectionSystem;
    EntityModificationSystem* modifSystem;
    LandscapeEditorDrawSystem* landscapeEditorDrawSystem;
    HeightmapEditorSystem* heightmapEditorSystem;
    TilemaskEditorSystem* tilemaskEditorSystem;
    CustomColorsSystem* customColorsSystem;
    RulerToolSystem* rulerToolSystem;
    StructureSystem* structureSystem;
    EditorParticlesSystem* particlesSystem;
    EditorLightSystem* editorLightSystem;
    TextDrawSystem* textDrawSystem;
    DebugDrawSystem* debugDrawSystem;
    BeastSystem* beastSystem;
    OwnersSignatureSystem* ownersSignatureSystem;
    DAVA::StaticOcclusionBuildSystem* staticOcclusionBuildSystem;
    EditorMaterialSystem* materialSystem;
    EditorLODSystem* editorLODSystem = nullptr;
    EditorStatisticsSystem* editorStatisticsSystem = nullptr;
    VisibilityCheckSystem* visibilityCheckSystem = nullptr;

    DAVA::WASDControllerSystem* wasdSystem = nullptr;
    DAVA::RotationControllerSystem* rotationSystem = nullptr;
    DAVA::SnapToLandscapeControllerSystem* snapToLandscapeSystem = nullptr;

    WayEditSystem* wayEditSystem;
    PathSystem* pathSystem;

    // save/load
    DAVA::SceneFileV2::eError LoadScene(const DAVA::FilePath& path) override;
    DAVA::SceneFileV2::eError SaveScene(const DAVA::FilePath& pathname, bool saveForGame = false) override;
    DAVA::SceneFileV2::eError SaveScene();
    bool Export(const DAVA::eGPUFamily newGPU);

    const DAVA::FilePath& GetScenePath();
    void SetScenePath(const DAVA::FilePath& newScenePath);

    // commands
    bool CanUndo() const;
    bool CanRedo() const;

    void Undo();
    void Redo();

    void BeginBatch(const DAVA::String& text, DAVA::uint32 commandsCount = 1);
    void EndBatch();

    void Exec(Command2::Pointer&& command);
    void RemoveCommands(DAVA::int32 commandId);

    void ClearAllCommands();
    const CommandStack* GetCommandStack() const;

    // checks whether the scene changed since the last save
    bool IsLoaded() const;
    bool IsChanged() const;
    void SetChanged(bool changed);

    // enable/disable drawing custom HUD
    void SetHUDVisible(bool visible);
    bool IsHUDVisible() const;

    // DAVA events
    void Update(float timeElapsed) override;

    // this function should be called each time UI3Dview changes its position
    // viewport rect is used to calc. ray from camera to any 2d point on this viewport
    void SetViewportRect(const DAVA::Rect& newViewportRect);

    //Insert entity to begin of scene hierarchy to display editor entities at one place on top og scene tree
    void AddEditorEntity(Entity* editorEntity);

    const DAVA::RenderStats& GetRenderStats() const;

    void EnableToolsInstantly(DAVA::int32 toolFlags);
    void DisableToolsInstantly(DAVA::int32 toolFlags, bool saveChanges = true);
    bool IsToolsEnabled(DAVA::int32 toolFlags);
    DAVA::int32 GetEnabledTools();

    SceneEditor2* CreateCopyForExport(); //Need to prevent changes of original scene
    DAVA::Entity* Clone(DAVA::Entity* dstNode /* = NULL */) override;

    void Activate() override;
    void Deactivate() override;

    void EnableEditorSystems();

    DAVA::uint32 GetFramesCount() const;
    void ResetFramesCount();

    DAVA_DEPRECATED(void MarkAsChanged()); // for old material & particle editors

    INTROSPECTION(SceneEditor2,
                  MEMBER(cameraSystem, "CameraSystem", DAVA::I_VIEW | DAVA::I_EDIT)
                  MEMBER(collisionSystem, "Collision System", DAVA::I_VIEW | DAVA::I_EDIT)
                  MEMBER(selectionSystem, "Selection System", DAVA::I_VIEW | DAVA::I_EDIT)
                  MEMBER(gridSystem, "GridSystem", DAVA::I_VIEW | DAVA::I_EDIT)
                  MEMBER(materialSystem, "Material System", DAVA::I_VIEW | DAVA::I_EDIT)
                  )

protected:
    bool isLoaded = false;
    bool isHUDVisible = true;

    DAVA::FilePath curScenePath;
    CommandStack* commandStack = nullptr;
    DAVA::RenderStats renderStats;

    DAVA::Vector<DAVA::Entity*> editorEntities;

    void EditorCommandProcess(const Command2* command, bool redo);

    void Draw() override;

    void ExtractEditorEntities();
    void InjectEditorEntities();

    void RemoveSystems();

    bool wasChanged; //deprecated

    void Setup3DDrawing();

    DAVA::uint32 framesCount = 0;

private:
    friend struct EditorCommandNotify;

    class EditorCommandNotify : public CommandNotify
    {
    public:
        EditorCommandNotify(SceneEditor2* _editor);
        void Notify(const Command2* command, bool redo) override;
        void CleanChanged(bool clean) override;

    private:
        SceneEditor2* editor = nullptr;
    };
};

Q_DECLARE_METATYPE(SceneEditor2*)

void LookAtSelection(SceneEditor2* scene);
void RemoveSelection(SceneEditor2* scene);
void LockTransform(SceneEditor2* scene);
void UnlockTransform(SceneEditor2* scene);

#endif // __SCENE_EDITOR_PROXY_H__
