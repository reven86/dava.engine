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


#include "VisibilityToolActions.h"
#include "../Qt/Scene/System/LandscapeEditorDrawSystem/VisibilityToolProxy.h"
#include "../Qt/Scene/SceneEditor2.h"
#include "../Qt/Scene/SceneSignals.h"

#include "../Qt/Main/QtUtils.h"

ActionEnableVisibilityTool::ActionEnableVisibilityTool(SceneEditor2* forSceneEditor)
:	CommandAction(CMDID_VISIBILITY_TOOL_ENABLE)
,	sceneEditor(forSceneEditor)
{
}

void ActionEnableVisibilityTool::Redo()
{
	if (sceneEditor == NULL)
	{
		return;
	}
	
	bool enabled = sceneEditor->visibilityToolSystem->IsLandscapeEditingEnabled();
	if (enabled)
	{
		return;
	}

	sceneEditor->DisableTools(SceneEditor2::LANDSCAPE_TOOLS_ALL);

	bool success = !sceneEditor->IsToolsEnabled(SceneEditor2::LANDSCAPE_TOOLS_ALL);
	
	if (!success )
	{
		ShowErrorDialog(ResourceEditor::LANDSCAPE_EDITOR_SYSTEM_DISABLE_EDITORS);
	}
	
	LandscapeEditorDrawSystem::eErrorType enablingError = sceneEditor->visibilityToolSystem->EnableLandscapeEditing();
	if (enablingError != LandscapeEditorDrawSystem::LANDSCAPE_EDITOR_SYSTEM_NO_ERRORS)
	{
		ShowErrorDialog(LandscapeEditorDrawSystem::GetDescriptionByError(enablingError));
	}
    
    if(success &&
       LandscapeEditorDrawSystem::LANDSCAPE_EDITOR_SYSTEM_NO_ERRORS == enablingError)
    {
        sceneEditor->foliageSystem->SetFoliageVisible(false);
    }
    
	SceneSignals::Instance()->EmitVisibilityToolToggled(sceneEditor);
}

ActionDisableVisibilityTool::ActionDisableVisibilityTool(SceneEditor2* forSceneEditor)
:	CommandAction(CMDID_VISIBILITY_TOOL_DISABLE)
,	sceneEditor(forSceneEditor)
{
}

void ActionDisableVisibilityTool::Redo()
{
	if (sceneEditor == NULL)
	{
		return;
	}
	
	bool disabled = !sceneEditor->visibilityToolSystem->IsLandscapeEditingEnabled();
	if (disabled)
	{
		return;
	}

	disabled = sceneEditor->visibilityToolSystem->DisableLandscapeEdititing();
	if (!disabled)
	{
		ShowErrorDialog(ResourceEditor::VISIBILITY_TOOL_DISABLE_ERROR);
	}
    
    if(disabled)
    {
        sceneEditor->foliageSystem->SetFoliageVisible(true);
    }

	SceneSignals::Instance()->EmitVisibilityToolToggled(sceneEditor);
}
