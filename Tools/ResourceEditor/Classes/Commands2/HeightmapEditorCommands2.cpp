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


#include "HeightmapEditorCommands2.h"
#include "../Qt/Scene/System/LandscapeEditorDrawSystem/HeightmapProxy.h"
#include "../Qt/Scene/System/LandscapeEditorDrawSystem/LandscapeProxy.h"
#include "../Qt/Scene/SceneEditor2.h"
#include "../Qt/Scene/SceneSignals.h"

#include "../Qt/Main/QtUtils.h"

ActionEnableHeightmapEditor::ActionEnableHeightmapEditor(SceneEditor2* forSceneEditor)
:	CommandAction(CMDID_HEIGHTMAP_EDITOR_ENABLE)
,	sceneEditor(forSceneEditor)
{
}

void ActionEnableHeightmapEditor::Redo()
{
	if (sceneEditor == NULL)
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

	if (!success )
	{
		ShowErrorDialog(ResourceEditor::LANDSCAPE_EDITOR_SYSTEM_DISABLE_EDITORS);
	}
	
	LandscapeEditorDrawSystem::eErrorType enablingError = sceneEditor->heightmapEditorSystem->EnableLandscapeEditing();
	if (enablingError != LandscapeEditorDrawSystem::LANDSCAPE_EDITOR_SYSTEM_NO_ERRORS)
	{
		ShowErrorDialog(LandscapeEditorDrawSystem::GetDescriptionByError(enablingError));
	}
    
    if(success &&
       LandscapeEditorDrawSystem::LANDSCAPE_EDITOR_SYSTEM_NO_ERRORS == enablingError)
    {
        sceneEditor->foliageSystem->SetFoliageVisible(false);
    }

	SceneSignals::Instance()->EmitHeightmapEditorToggled(sceneEditor);
}

ActionDisableHeightmapEditor::ActionDisableHeightmapEditor(SceneEditor2* forSceneEditor)
:	CommandAction(CMDID_HEIGHTMAP_EDITOR_DISABLE)
,	sceneEditor(forSceneEditor)
{
}

void ActionDisableHeightmapEditor::Redo()
{
	if (sceneEditor == NULL)
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
    
    if(disabled)
    {
        if(!sceneEditor->landscapeEditorDrawSystem->IsNotPassableTerrainEnabled())
        {
            sceneEditor->foliageSystem->SetFoliageVisible(true);
        }
        
        sceneEditor->foliageSystem->SyncFoliageWithLandscape();
    }

	SceneSignals::Instance()->EmitHeightmapEditorToggled(sceneEditor);
}


ModifyHeightmapCommand::ModifyHeightmapCommand(HeightmapProxy* heightmapProxy,
											   Heightmap* originalHeightmap,
											   const Rect& updatedRect)
:	Command2(CMDID_HEIGHTMAP_MODIFY, "Height Map Change")
,	heightmapProxy(heightmapProxy)
{
	if (originalHeightmap && heightmapProxy)
	{
		this->updatedRect = updatedRect;
		undoRegion = GetHeightmapRegion(originalHeightmap);
		redoRegion = GetHeightmapRegion(heightmapProxy);
	}
}

ModifyHeightmapCommand::~ModifyHeightmapCommand()
{
	SafeDeleteArray(undoRegion);
	SafeDeleteArray(redoRegion);
}

Entity* ModifyHeightmapCommand::GetEntity() const
{
	return NULL;
}

void ModifyHeightmapCommand::Redo()
{
	ApplyHeightmapRegion(redoRegion);
}

void ModifyHeightmapCommand::Undo()
{
	ApplyHeightmapRegion(undoRegion);
}

uint16* ModifyHeightmapCommand::GetHeightmapRegion(Heightmap* heightmap)
{
	int32 size = heightmap->Size();
    int32 width = (int32)ceilf(updatedRect.dx);
    int32 height = (int32)ceilf(updatedRect.dy);
    int32 xOffset = (int32)floorf(updatedRect.x);
    int32 yOffset = (int32)floorf(updatedRect.y);

    DVASSERT((xOffset + width) <= size && (yOffset + height) <= size);

    uint16* newData = new uint16[width * height];
    uint16* oldData = heightmap->Data();
	
	for (int32 i = 0; i < height; ++i)
	{
		uint16* src = oldData + (yOffset + i) * size + xOffset;
		uint16* dst = newData + i * width;
		memcpy(dst, src, sizeof(uint16) * width);
	}
	
	return newData;
}

void ModifyHeightmapCommand::ApplyHeightmapRegion(uint16* region)
{
    int32 size = heightmapProxy->Size();
    int32 width = (int32)ceilf(updatedRect.dx);
    int32 height = (int32)ceilf(updatedRect.dy);
    int32 xOffset = (int32)floorf(updatedRect.x);
    int32 yOffset = (int32)floorf(updatedRect.y);

    DVASSERT((xOffset + width) <= size && (yOffset + height) <= size);

    uint16* data = heightmapProxy->Data();

    for (int32 i = 0; i < height; ++i)
	{
		uint16* src = region + i * width;
		uint16* dst = data + (yOffset + i) * size + xOffset;
		memcpy(dst, src, sizeof(uint16) * width);
	}
	
	heightmapProxy->UpdateRect(updatedRect);
}
