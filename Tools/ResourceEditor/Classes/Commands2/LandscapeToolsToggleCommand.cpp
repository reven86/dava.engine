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
#include "Main/QtUtils.h"
#include "Qt/Scene/SceneSignals.h"

LandscapeToolsToggleCommand::LandscapeToolsToggleCommand(int identifier, SceneEditor2* _sceneEditor)
    : Command2(identifier)
    , sceneEditor(_sceneEditor)
{
}

DAVA::Entity* LandscapeToolsToggleCommand::GetEntity() const
{
    return sceneEditor;
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
    if (sceneEditor == nullptr)
    {
        return;
    }

    bool enabled = sceneEditor->rulerToolSystem->IsLandscapeEditingEnabled();
    if (enabled)
    {
        return;
    }

    sceneEditor->DisableTools(SceneEditor2::LANDSCAPE_TOOLS_ALL);

    bool success = !sceneEditor->IsToolsEnabled(SceneEditor2::LANDSCAPE_TOOLS_ALL);

    if (!success)
    {
        ShowErrorDialog(ResourceEditor::LANDSCAPE_EDITOR_SYSTEM_DISABLE_EDITORS);
    }

    LandscapeEditorDrawSystem::eErrorType enablingError = sceneEditor->rulerToolSystem->EnableLandscapeEditing();
    if (enablingError != LandscapeEditorDrawSystem::LANDSCAPE_EDITOR_SYSTEM_NO_ERRORS)
    {
        ShowErrorDialog(LandscapeEditorDrawSystem::GetDescriptionByError(enablingError));
    }

    if (success && (LandscapeEditorDrawSystem::LANDSCAPE_EDITOR_SYSTEM_NO_ERRORS == enablingError))
    {
        sceneEditor->foliageSystem->SetFoliageVisible(false);
    }

    SceneSignals::Instance()->EmitRulerToolToggled(sceneEditor);
}

void EnableRulerToolCommand::Undo()
{
    if (sceneEditor == nullptr)
    {
        return;
    }

    bool disabled = !sceneEditor->rulerToolSystem->IsLandscapeEditingEnabled();
    if (disabled)
    {
        return;
    }

    disabled = sceneEditor->rulerToolSystem->DisableLandscapeEdititing();
    if (!disabled)
    {
        ShowErrorDialog(ResourceEditor::RULER_TOOL_DISABLE_ERROR);
    }

    if (disabled)
    {
        sceneEditor->foliageSystem->SetFoliageVisible(true);
    }

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
    if (sceneEditor == nullptr)
    {
        return;
    }

    bool enabled = sceneEditor->tilemaskEditorSystem->IsLandscapeEditingEnabled();
    if (enabled)
    {
        return;
    }

    sceneEditor->DisableTools(SceneEditor2::LANDSCAPE_TOOLS_ALL);

    bool toolEnabled = sceneEditor->IsToolsEnabled(SceneEditor2::LANDSCAPE_TOOLS_ALL);
    if (toolEnabled)
    {
        ShowErrorDialog(ResourceEditor::LANDSCAPE_EDITOR_SYSTEM_DISABLE_EDITORS);
    }

    LandscapeEditorDrawSystem::eErrorType enablingError = sceneEditor->tilemaskEditorSystem->EnableLandscapeEditing();
    if (enablingError != LandscapeEditorDrawSystem::LANDSCAPE_EDITOR_SYSTEM_NO_ERRORS)
    {
        ShowErrorDialog(LandscapeEditorDrawSystem::GetDescriptionByError(enablingError));
    }

    if (!toolEnabled && (LandscapeEditorDrawSystem::LANDSCAPE_EDITOR_SYSTEM_NO_ERRORS == enablingError))
    {
        sceneEditor->foliageSystem->SetFoliageVisible(false);
    }

    SceneSignals::Instance()->EmitTilemaskEditorToggled(sceneEditor);
}

void EnableTilemaskEditorCommand::Undo()
{
    if ((sceneEditor == nullptr) || !sceneEditor->tilemaskEditorSystem->IsLandscapeEditingEnabled())
    {
        return;
    }

    if (sceneEditor->tilemaskEditorSystem->DisableLandscapeEdititing())
    {
        sceneEditor->foliageSystem->SetFoliageVisible(true);
    }
    else
    {
        ShowErrorDialog(ResourceEditor::TILEMASK_EDITOR_DISABLE_ERROR);
    }

    SceneSignals::Instance()->EmitTilemaskEditorToggled(sceneEditor);
}

/*
 * Not passable
 */

EnableNotPassableCommand::EnableNotPassableCommand(SceneEditor2* forSceneEditor)
    : LandscapeToolsToggleCommand(CMDID_NOT_PASSABLE_TERRAIN_ENABLE, forSceneEditor)
{
}

void EnableNotPassableCommand::Redo()
{
    if (sceneEditor == nullptr)
    {
        return;
    }

    bool enabled = sceneEditor->landscapeEditorDrawSystem->IsNotPassableTerrainEnabled();
    if (enabled)
    {
        return;
    }

    sceneEditor->DisableTools(SceneEditor2::LANDSCAPE_TOOLS_ALL & ~SceneEditor2::LANDSCAPE_TOOL_HEIGHTMAP_EDITOR);

    bool success = !sceneEditor->IsToolsEnabled(SceneEditor2::LANDSCAPE_TOOLS_ALL &
                                                ~SceneEditor2::LANDSCAPE_TOOL_HEIGHTMAP_EDITOR);
    if (!success)
    {
        ShowErrorDialog(ResourceEditor::LANDSCAPE_EDITOR_SYSTEM_DISABLE_EDITORS);
    }

    LandscapeEditorDrawSystem::eErrorType enablingError = sceneEditor->landscapeEditorDrawSystem->EnableNotPassableTerrain();
    if (enablingError != LandscapeEditorDrawSystem::LANDSCAPE_EDITOR_SYSTEM_NO_ERRORS)
    {
        ShowErrorDialog(LandscapeEditorDrawSystem::GetDescriptionByError(enablingError));
    }

    if (success &&
        LandscapeEditorDrawSystem::LANDSCAPE_EDITOR_SYSTEM_NO_ERRORS == enablingError)
    {
        sceneEditor->foliageSystem->SetFoliageVisible(false);
    }

    SceneSignals::Instance()->EmitNotPassableTerrainToggled(sceneEditor);
}

void EnableNotPassableCommand::Undo()
{
    if (sceneEditor == nullptr)
    {
        return;
    }

    bool disabled = !sceneEditor->landscapeEditorDrawSystem->IsNotPassableTerrainEnabled();
    if (disabled)
    {
        return;
    }

    sceneEditor->landscapeEditorDrawSystem->DisableNotPassableTerrain();
    if (!disabled && !sceneEditor->heightmapEditorSystem->IsLandscapeEditingEnabled())
    {
        sceneEditor->foliageSystem->SetFoliageVisible(true);
    }

    SceneSignals::Instance()->EmitNotPassableTerrainToggled(sceneEditor);
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
    if (sceneEditor == nullptr)
    {
        return;
    }

    bool enabled = sceneEditor->heightmapEditorSystem->IsLandscapeEditingEnabled();
    if (enabled)
    {
        return;
    }

    sceneEditor->DisableTools(SceneEditor2::LANDSCAPE_TOOLS_ALL & ~SceneEditor2::LANDSCAPE_TOOL_NOT_PASSABLE_TERRAIN);

    bool success = !sceneEditor->IsToolsEnabled(SceneEditor2::LANDSCAPE_TOOLS_ALL & ~SceneEditor2::LANDSCAPE_TOOL_NOT_PASSABLE_TERRAIN);

    if (!success)
    {
        ShowErrorDialog(ResourceEditor::LANDSCAPE_EDITOR_SYSTEM_DISABLE_EDITORS);
    }

    LandscapeEditorDrawSystem::eErrorType enablingError = sceneEditor->heightmapEditorSystem->EnableLandscapeEditing();
    if (enablingError != LandscapeEditorDrawSystem::LANDSCAPE_EDITOR_SYSTEM_NO_ERRORS)
    {
        ShowErrorDialog(LandscapeEditorDrawSystem::GetDescriptionByError(enablingError));
    }

    if (success &&
        LandscapeEditorDrawSystem::LANDSCAPE_EDITOR_SYSTEM_NO_ERRORS == enablingError)
    {
        sceneEditor->foliageSystem->SetFoliageVisible(false);
    }

    SceneSignals::Instance()->EmitHeightmapEditorToggled(sceneEditor);
}

void EnableHeightmapEditorCommand::Undo()
{
    if (sceneEditor == nullptr)
    {
        return;
    }

    bool disabled = !sceneEditor->heightmapEditorSystem->IsLandscapeEditingEnabled();
    if (disabled)
    {
        return;
    }

    disabled = sceneEditor->heightmapEditorSystem->DisableLandscapeEdititing();
    if (!disabled)
    {
        ShowErrorDialog(ResourceEditor::HEIGHTMAP_EDITOR_DISABLE_ERROR);
    }

    if (disabled)
    {
        if (!sceneEditor->landscapeEditorDrawSystem->IsNotPassableTerrainEnabled())
        {
            sceneEditor->foliageSystem->SetFoliageVisible(true);
        }

        sceneEditor->foliageSystem->SyncFoliageWithLandscape();
    }

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
    if (sceneEditor == nullptr)
    {
        return;
    }

    bool enabled = sceneEditor->customColorsSystem->IsLandscapeEditingEnabled();
    if (enabled)
    {
        return;
    }

    sceneEditor->DisableTools(SceneEditor2::LANDSCAPE_TOOLS_ALL);

    bool success = !sceneEditor->IsToolsEnabled(SceneEditor2::LANDSCAPE_TOOLS_ALL);
    if (!success)
    {
        ShowErrorDialog(ResourceEditor::LANDSCAPE_EDITOR_SYSTEM_DISABLE_EDITORS);
    }

    LandscapeEditorDrawSystem::eErrorType enablingError = sceneEditor->customColorsSystem->EnableLandscapeEditing();
    if (enablingError != LandscapeEditorDrawSystem::LANDSCAPE_EDITOR_SYSTEM_NO_ERRORS)
    {
        ShowErrorDialog(LandscapeEditorDrawSystem::GetDescriptionByError(enablingError));
    }
    else
    {
        if (!sceneEditor->landscapeEditorDrawSystem->GetCustomColorsProxy()->IsTextureLoaded())
        {
            ShowErrorDialog(LandscapeEditorDrawSystem::GetDescriptionByError(LandscapeEditorDrawSystem::LANDSCAPE_EDITOR_SYSTEM_CUSTOMCOLORS_ABSENT));
            sceneEditor->landscapeEditorDrawSystem->GetCustomColorsProxy()->ResetLoadedState();
        }

        if (success &&
            LandscapeEditorDrawSystem::LANDSCAPE_EDITOR_SYSTEM_NO_ERRORS == enablingError)
        {
            sceneEditor->foliageSystem->SetFoliageVisible(false);
        }
    }

    SceneSignals::Instance()->EmitCustomColorsToggled(sceneEditor);
}

void EnableCustomColorsCommand::Undo()
{
    if (sceneEditor == nullptr)
    {
        return;
    }

    bool disabled = !sceneEditor->customColorsSystem->IsLandscapeEditingEnabled();
    if (disabled)
    {
        return;
    }

    bool success = sceneEditor->customColorsSystem->DisableLandscapeEdititing(saveChanges);
    if (!success)
    {
        ShowErrorDialog(ResourceEditor::CUSTOM_COLORS_DISABLE_ERROR);
    }

    if (success)
    {
        sceneEditor->foliageSystem->SetFoliageVisible(true);
    }

    SceneSignals::Instance()->EmitCustomColorsToggled(sceneEditor);
}
