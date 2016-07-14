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
        ShowErrorDialog(ResourceEditor::LANDSCAPE_EDITOR_SYSTEM_DISABLE_EDITORS);
        return false;
    }

    LandscapeEditorDrawSystem::eErrorType enablingError = enable();
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
            ShowErrorDialog(error);
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
                                                         const DAVA::String& commandDescr, bool isEnabling,
                                                         DAVA::uint32 _allowedTools, DAVA::String _disablingError)
    : CommandWithoutExecute(identifier, commandDescr + ((isEnabling == true) ? "Enabled" : "Disabled"))
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
EnableRulerToolCommand::EnableRulerToolCommand(SceneEditor2* forSceneEditor, bool isEnabling)
    : LandscapeToolsToggleCommand(CMDID_RULER_TOOL_ENABLE, forSceneEditor, "Ruler Tool ", isEnabling,
                                  0, ResourceEditor::RULER_TOOL_DISABLE_ERROR)
{
    isEnabledFunction = DAVA::MakeFunction(sceneEditor->rulerToolSystem, &RulerToolSystem::IsLandscapeEditingEnabled);
    enableFunction = DAVA::MakeFunction(sceneEditor->rulerToolSystem, &RulerToolSystem::EnableLandscapeEditing);
    disableFunction = DAVA::MakeFunction(sceneEditor->rulerToolSystem, &RulerToolSystem::DisableLandscapeEdititing);
}

/*
 * Tilemask
 */
EnableTilemaskEditorCommand::EnableTilemaskEditorCommand(SceneEditor2* forSceneEditor, bool isEnabling)
    : LandscapeToolsToggleCommand(CMDID_TILEMASK_EDITOR_ENABLE, forSceneEditor, "Tilemask Editor ", isEnabling,
                                  0, ResourceEditor::TILEMASK_EDITOR_DISABLE_ERROR)
{
    isEnabledFunction = DAVA::MakeFunction(sceneEditor->tilemaskEditorSystem, &TilemaskEditorSystem::IsLandscapeEditingEnabled);
    enableFunction = DAVA::MakeFunction(sceneEditor->tilemaskEditorSystem, &TilemaskEditorSystem::EnableLandscapeEditing);
    disableFunction = DAVA::MakeFunction(sceneEditor->tilemaskEditorSystem, &TilemaskEditorSystem::DisableLandscapeEdititing);
}

/*
 * Heightmap editor
 */
EnableHeightmapEditorCommand::EnableHeightmapEditorCommand(SceneEditor2* forSceneEditor, bool isEnabling)
    : LandscapeToolsToggleCommand(CMDID_HEIGHTMAP_EDITOR_ENABLE, forSceneEditor, "Heightmap Editor ", isEnabling,
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
EnableCustomColorsCommand::EnableCustomColorsCommand(SceneEditor2* forSceneEditor, bool _saveChanges, bool isEnabling)
    : LandscapeToolsToggleCommand(CMDID_CUSTOM_COLORS_ENABLE, forSceneEditor, "Custom Colors Editor ", isEnabling, 0, ResourceEditor::CUSTOM_COLORS_DISABLE_ERROR)
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
        ShowErrorDialog(LandscapeEditorDrawSystem::GetDescriptionByError(LandscapeEditorDrawSystem::LANDSCAPE_EDITOR_SYSTEM_CUSTOMCOLORS_ABSENT));
        drawSystem->GetCustomColorsProxy()->ResetLoadedState();
    }
}

/*
 * Not passable - special case
 */
EnableNotPassableCommand::EnableNotPassableCommand(SceneEditor2* forSceneEditor, bool isEnabling)
    : LandscapeToolsToggleCommand(CMDID_NOT_PASSABLE_TERRAIN_ENABLE, forSceneEditor, "Not Passable Editor ", isEnabling,
                                  SceneEditor2::LANDSCAPE_TOOL_HEIGHTMAP_EDITOR,
                                  ResourceEditor::NOT_PASSABLE_TERRAIN_DISABLE_ERROR)
{
    isEnabledFunction = DAVA::MakeFunction(sceneEditor->landscapeEditorDrawSystem, &LandscapeEditorDrawSystem::IsNotPassableTerrainEnabled);
    enableFunction = DAVA::MakeFunction(sceneEditor->landscapeEditorDrawSystem, &LandscapeEditorDrawSystem::EnableNotPassableTerrain);
    disableFunction = DAVA::MakeFunction(sceneEditor->landscapeEditorDrawSystem, &LandscapeEditorDrawSystem::DisableNotPassableTerrain);
}
