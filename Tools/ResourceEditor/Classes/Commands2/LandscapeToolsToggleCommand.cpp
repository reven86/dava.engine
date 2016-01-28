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

#include "Commands2/LandscapeToolsToggleCommand.h"
#include "Scene/SceneEditor2.h"
#include "Qt/Scene/SceneSignals.h"
#include "Qt/Scene/System/LandscapeEditorDrawSystem/CustomColorsProxy.h"
#include "Main/QtUtils.h"

namespace LTTCLocal
{
template <typename System, typename CheckFunction, typename EnableFunction>
bool TryEnableWithFunctions(SceneEditor2* editor, System SceneEditor2::*system, DAVA::uint32 disableFlags,
                            CheckFunction check, EnableFunction enable)
{
    if (editor == nullptr)
        return false;

    if (((editor->*system)->*check)())
        return false;

    editor->DisableToolsInstantly(disableFlags);
    if (editor->IsToolsEnabled(disableFlags))
    {
        ShowErrorDialog(ResourceEditor::LANDSCAPE_EDITOR_SYSTEM_DISABLE_EDITORS);
        return false;
    }

    LandscapeEditorDrawSystem::eErrorType enablingError = ((editor->*system)->*enable)();
    if (enablingError == LandscapeEditorDrawSystem::LANDSCAPE_EDITOR_SYSTEM_NO_ERRORS)
    {
        editor->foliageSystem->SetFoliageVisible(false);
    }
    else
    {
        ShowErrorDialog(LandscapeEditorDrawSystem::GetDescriptionByError(enablingError));
        return false;
    }

    return true;
}

template <typename System, typename CheckFunction, typename DisableFunction, typename... Arg>
bool TryDisableWithFunctions(SceneEditor2* editor, System SceneEditor2::*system, const String& error, CheckFunction check, DisableFunction disable, Arg... arg)
{
    if (editor == nullptr)
        return false;

    if (((editor->*system)->*check)())
    {
        if (((editor->*system)->*disable)(arg...))
        {
            editor->foliageSystem->SetFoliageVisible(true);
        }
        else
        {
            ShowErrorDialog(error);
            return false;
        }
    }

    return true;
}

template <class System>
bool TryEnable(SceneEditor2* editor, System* SceneEditor2::*system, DAVA::uint32 disableFlags)
{
    return TryEnableWithFunctions(editor, system, disableFlags, &System::IsLandscapeEditingEnabled, &System::EnableLandscapeEditing);
}

template <class System, typename... Arg>
bool TryDisable(SceneEditor2* editor, System* SceneEditor2::*system, const String& error, Arg... arg)
{
    return TryDisableWithFunctions(editor, system, error, &System::IsLandscapeEditingEnabled, &System::DisableLandscapeEdititing, arg...);
}
}

/*
 * Common
 */
LandscapeToolsToggleCommand::LandscapeToolsToggleCommand(int identifier, SceneEditor2* _sceneEditor)
    : Command2(identifier)
    , sceneEditor(_sceneEditor)
{
}

DAVA::Entity* LandscapeToolsToggleCommand::GetEntity() const
{
    return sceneEditor;
}

void LandscapeToolsToggleCommand::SaveEnabledToolsState()
{
    if (sceneEditor != nullptr)
    {
        enabledTools = sceneEditor->GetEnabledTools();
    }
}

void LandscapeToolsToggleCommand::ApplySavedState()
{
    if (sceneEditor != nullptr)
    {
        sceneEditor->EnableToolsInstantly(enabledTools);
    }
}

/*
 * Ruler
 */
EnableRulerToolCommand::EnableRulerToolCommand(SceneEditor2* forSceneEditor)
    : LandscapeToolsToggleCommand(CMDID_RULER_TOOL_ENABLE, forSceneEditor)
{
}

void EnableRulerToolCommand::Redo()
{
    SaveEnabledToolsState();
    LTTCLocal::TryEnable(sceneEditor, &SceneEditor2::rulerToolSystem, SceneEditor2::LANDSCAPE_TOOLS_ALL);

    SceneSignals::Instance()->EmitRulerToolToggled(sceneEditor);
}

void EnableRulerToolCommand::Undo()
{
    LTTCLocal::TryDisable(sceneEditor, &SceneEditor2::rulerToolSystem, ResourceEditor::RULER_TOOL_DISABLE_ERROR);
    ApplySavedState();

    SceneSignals::Instance()->EmitRulerToolToggled(sceneEditor);
}

/*
 * Tilemask
 */
EnableTilemaskEditorCommand::EnableTilemaskEditorCommand(SceneEditor2* forSceneEditor)
    : LandscapeToolsToggleCommand(CMDID_TILEMASK_EDITOR_ENABLE, forSceneEditor)
{
}

void EnableTilemaskEditorCommand::Redo()
{
    SaveEnabledToolsState();
    LTTCLocal::TryEnable(sceneEditor, &SceneEditor2::tilemaskEditorSystem, SceneEditor2::LANDSCAPE_TOOLS_ALL);

    SceneSignals::Instance()->EmitTilemaskEditorToggled(sceneEditor);
}

void EnableTilemaskEditorCommand::Undo()
{
    LTTCLocal::TryDisable(sceneEditor, &SceneEditor2::tilemaskEditorSystem, ResourceEditor::TILEMASK_EDITOR_DISABLE_ERROR);
    ApplySavedState();

    SceneSignals::Instance()->EmitTilemaskEditorToggled(sceneEditor);
}

/*
 * Heightmap editor
 */
EnableHeightmapEditorCommand::EnableHeightmapEditorCommand(SceneEditor2* forSceneEditor)
    : LandscapeToolsToggleCommand(CMDID_HEIGHTMAP_EDITOR_ENABLE, forSceneEditor)
{
}

void EnableHeightmapEditorCommand::Redo()
{
    SaveEnabledToolsState();
    LTTCLocal::TryEnable(sceneEditor, &SceneEditor2::heightmapEditorSystem,
                         SceneEditor2::LANDSCAPE_TOOLS_ALL & ~SceneEditor2::LANDSCAPE_TOOL_NOT_PASSABLE_TERRAIN);
    SceneSignals::Instance()->EmitHeightmapEditorToggled(sceneEditor);
}

void EnableHeightmapEditorCommand::Undo()
{
    if (LTTCLocal::TryDisable(sceneEditor, &SceneEditor2::heightmapEditorSystem, ResourceEditor::HEIGHTMAP_EDITOR_DISABLE_ERROR))
    {
        sceneEditor->foliageSystem->SyncFoliageWithLandscape();
    }
    ApplySavedState();

    SceneSignals::Instance()->EmitHeightmapEditorToggled(sceneEditor);
}

/*
 * Custom colors
 */

EnableCustomColorsCommand::EnableCustomColorsCommand(SceneEditor2* forSceneEditor, bool _saveChanges)
    : LandscapeToolsToggleCommand(CMDID_CUSTOM_COLORS_ENABLE, forSceneEditor)
    , saveChanges(_saveChanges)
{
}

void EnableCustomColorsCommand::Redo()
{
    SaveEnabledToolsState();
    if (LTTCLocal::TryEnable(sceneEditor, &SceneEditor2::customColorsSystem, SceneEditor2::LANDSCAPE_TOOLS_ALL))
    {
        auto drawSystem = sceneEditor->landscapeEditorDrawSystem;
        if (drawSystem->GetCustomColorsProxy()->IsTextureLoaded() == false)
        {
            ShowErrorDialog(LandscapeEditorDrawSystem::GetDescriptionByError(LandscapeEditorDrawSystem::LANDSCAPE_EDITOR_SYSTEM_CUSTOMCOLORS_ABSENT));
            drawSystem->GetCustomColorsProxy()->ResetLoadedState();
        }
    }

    SceneSignals::Instance()->EmitCustomColorsToggled(sceneEditor);
}

void EnableCustomColorsCommand::Undo()
{
    LTTCLocal::TryDisable(sceneEditor, &SceneEditor2::customColorsSystem, ResourceEditor::CUSTOM_COLORS_DISABLE_ERROR, false);
    ApplySavedState();

    SceneSignals::Instance()->EmitCustomColorsToggled(sceneEditor);
}

/*
 * Not passable - special case
 */

EnableNotPassableCommand::EnableNotPassableCommand(SceneEditor2* forSceneEditor)
    : LandscapeToolsToggleCommand(CMDID_NOT_PASSABLE_TERRAIN_ENABLE, forSceneEditor)
{
}

void EnableNotPassableCommand::Redo()
{
    SaveEnabledToolsState();
    LTTCLocal::TryEnableWithFunctions(sceneEditor, &SceneEditor2::landscapeEditorDrawSystem,
                                      SceneEditor2::LANDSCAPE_TOOLS_ALL & ~SceneEditor2::LANDSCAPE_TOOL_HEIGHTMAP_EDITOR,
                                      &LandscapeEditorDrawSystem::IsNotPassableTerrainEnabled,
                                      &LandscapeEditorDrawSystem::EnableNotPassableTerrain);

    SceneSignals::Instance()->EmitNotPassableTerrainToggled(sceneEditor);
}

void EnableNotPassableCommand::Undo()
{
    LTTCLocal::TryDisableWithFunctions(sceneEditor, &SceneEditor2::landscapeEditorDrawSystem,
                                       ResourceEditor::NOT_PASSABLE_TERRAIN_DISABLE_ERROR,
                                       &LandscapeEditorDrawSystem::IsNotPassableTerrainEnabled,
                                       &LandscapeEditorDrawSystem::DisableNotPassableTerrain);
    ApplySavedState();

    SceneSignals::Instance()->EmitNotPassableTerrainToggled(sceneEditor);
}
