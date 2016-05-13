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
#include "Scene/System/LandscapeEditorDrawSystem/CustomColorsProxy.h"
#include "Qt/Scene/SceneSignals.h"
#include "Main/QtUtils.h"

namespace LTTCLocal
{
bool TryEnableWithFunctions(SceneEditor2* editor, DAVA::uint32 allowedTools,
                            const LandscapeToolsToggleCommand::IsEnabledFunction& isEnabled,
                            const LandscapeToolsToggleCommand::EnableFunction& enable)
{
    if (editor == nullptr)
        return false;

    if (isEnabled())
        return false;

    DAVA::uint32 disableFlags = SceneEditor2::LANDSCAPE_TOOLS_ALL & (~allowedTools);
    editor->DisableToolsInstantly(disableFlags);
    if (editor->IsToolsEnabled(disableFlags))
    {
        DAVA::Logger::Error(ResourceEditor::LANDSCAPE_EDITOR_SYSTEM_DISABLE_EDITORS.c_str());
        return false;
    }

    LandscapeEditorDrawSystem::eErrorType enablingError = enable();
    if (enablingError == LandscapeEditorDrawSystem::LANDSCAPE_EDITOR_SYSTEM_NO_ERRORS)
    {
        editor->foliageSystem->SetFoliageVisible(false);
    }
    else
    {
        DAVA::Logger::Error(LandscapeEditorDrawSystem::GetDescriptionByError(enablingError).c_str());
        return false;
    }

    return true;
}

bool TryDisableWithFunctions(SceneEditor2* editor, const DAVA::String& error,
                             const LandscapeToolsToggleCommand::IsEnabledFunction& isEnabled,
                             const LandscapeToolsToggleCommand::DisableFunction& disable)
{
    if (editor == nullptr)
        return false;

    if (isEnabled())
    {
        if (disable())
        {
            editor->foliageSystem->SetFoliageVisible(true);
        }
        else
        {
            DAVA::Logger::Error(error.c_str());
            return false;
        }
    }

    return true;
}
}

/*
 * Common
 */
LandscapeToolsToggleCommand::LandscapeToolsToggleCommand(int identifier, SceneEditor2* _sceneEditor,
                                                         DAVA::uint32 _allowedTools, DAVA::String _disablingError)
    : Command2(identifier)
    , sceneEditor(_sceneEditor)
    , disablingError(_disablingError)
    , allowedTools(_allowedTools)
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

void LandscapeToolsToggleCommand::Redo()
{
    SaveEnabledToolsState();
    if (LTTCLocal::TryEnableWithFunctions(sceneEditor, allowedTools, isEnabledFunction, enableFunction))
    {
        OnEnabled();
    }
    SceneSignals::Instance()->EmitLandscapeEditorToggled(sceneEditor);
}

void LandscapeToolsToggleCommand::Undo()
{
    if (LTTCLocal::TryDisableWithFunctions(sceneEditor, disablingError, isEnabledFunction, disableFunction))
    {
        OnDisabled();
        ApplySavedState();
    }
    SceneSignals::Instance()->EmitLandscapeEditorToggled(sceneEditor);
}

void LandscapeToolsToggleCommand::OnEnabled()
{
}

void LandscapeToolsToggleCommand::OnDisabled()
{
}

/*
 * Ruler
 */
EnableRulerToolCommand::EnableRulerToolCommand(SceneEditor2* forSceneEditor)
    : LandscapeToolsToggleCommand(CMDID_RULER_TOOL_ENABLE, forSceneEditor, 0, ResourceEditor::RULER_TOOL_DISABLE_ERROR)
{
    isEnabledFunction = DAVA::MakeFunction(sceneEditor->rulerToolSystem, &RulerToolSystem::IsLandscapeEditingEnabled);
    enableFunction = DAVA::MakeFunction(sceneEditor->rulerToolSystem, &RulerToolSystem::EnableLandscapeEditing);
    disableFunction = DAVA::MakeFunction(sceneEditor->rulerToolSystem, &RulerToolSystem::DisableLandscapeEdititing);
}

/*
 * Tilemask
 */
EnableTilemaskEditorCommand::EnableTilemaskEditorCommand(SceneEditor2* forSceneEditor)
    : LandscapeToolsToggleCommand(CMDID_TILEMASK_EDITOR_ENABLE, forSceneEditor, 0, ResourceEditor::TILEMASK_EDITOR_DISABLE_ERROR)
{
    isEnabledFunction = DAVA::MakeFunction(sceneEditor->tilemaskEditorSystem, &TilemaskEditorSystem::IsLandscapeEditingEnabled);
    enableFunction = DAVA::MakeFunction(sceneEditor->tilemaskEditorSystem, &TilemaskEditorSystem::EnableLandscapeEditing);
    disableFunction = DAVA::MakeFunction(sceneEditor->tilemaskEditorSystem, &TilemaskEditorSystem::DisableLandscapeEdititing);
}

/*
 * Heightmap editor
 */
EnableHeightmapEditorCommand::EnableHeightmapEditorCommand(SceneEditor2* forSceneEditor)
    : LandscapeToolsToggleCommand(CMDID_HEIGHTMAP_EDITOR_ENABLE, forSceneEditor,
                                  SceneEditor2::LANDSCAPE_TOOL_NOT_PASSABLE_TERRAIN,
                                  ResourceEditor::HEIGHTMAP_EDITOR_DISABLE_ERROR)
{
    isEnabledFunction = DAVA::MakeFunction(sceneEditor->heightmapEditorSystem, &HeightmapEditorSystem::IsLandscapeEditingEnabled);
    enableFunction = DAVA::MakeFunction(sceneEditor->heightmapEditorSystem, &HeightmapEditorSystem::EnableLandscapeEditing);
    disableFunction = DAVA::MakeFunction(sceneEditor->heightmapEditorSystem, &HeightmapEditorSystem::DisableLandscapeEdititing);
}

void EnableHeightmapEditorCommand::OnDisabled()
{
    sceneEditor->foliageSystem->SyncFoliageWithLandscape();
}

/*
 * Custom colors
 */
EnableCustomColorsCommand::EnableCustomColorsCommand(SceneEditor2* forSceneEditor, bool _saveChanges)
    : LandscapeToolsToggleCommand(CMDID_CUSTOM_COLORS_ENABLE, forSceneEditor, 0, ResourceEditor::CUSTOM_COLORS_DISABLE_ERROR)
{
    isEnabledFunction = DAVA::MakeFunction(sceneEditor->customColorsSystem, &CustomColorsSystem::IsLandscapeEditingEnabled);
    enableFunction = DAVA::MakeFunction(sceneEditor->customColorsSystem, &CustomColorsSystem::EnableLandscapeEditing);
    disableFunction = Bind(DAVA::MakeFunction(sceneEditor->customColorsSystem, &CustomColorsSystem::DisableLandscapeEdititing), _saveChanges);
}

void EnableCustomColorsCommand::OnEnabled()
{
    auto drawSystem = sceneEditor->landscapeEditorDrawSystem;
    if (drawSystem->GetCustomColorsProxy()->IsTextureLoaded() == false)
    {
        DAVA::Logger::Error(LandscapeEditorDrawSystem::GetDescriptionByError(LandscapeEditorDrawSystem::LANDSCAPE_EDITOR_SYSTEM_CUSTOMCOLORS_ABSENT).c_str());
        drawSystem->GetCustomColorsProxy()->ResetLoadedState();
    }
}

/*
 * Not passable - special case
 */
EnableNotPassableCommand::EnableNotPassableCommand(SceneEditor2* forSceneEditor)
    : LandscapeToolsToggleCommand(CMDID_NOT_PASSABLE_TERRAIN_ENABLE, forSceneEditor,
                                  SceneEditor2::LANDSCAPE_TOOL_HEIGHTMAP_EDITOR,
                                  ResourceEditor::NOT_PASSABLE_TERRAIN_DISABLE_ERROR)
{
    isEnabledFunction = DAVA::MakeFunction(sceneEditor->landscapeEditorDrawSystem, &LandscapeEditorDrawSystem::IsNotPassableTerrainEnabled);
    enableFunction = DAVA::MakeFunction(sceneEditor->landscapeEditorDrawSystem, &LandscapeEditorDrawSystem::EnableNotPassableTerrain);
    disableFunction = DAVA::MakeFunction(sceneEditor->landscapeEditorDrawSystem, &LandscapeEditorDrawSystem::DisableNotPassableTerrain);
}
