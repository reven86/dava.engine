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



#include "HeightmapEditorCommands.h"
#include "../SceneEditor/SceneEditorScreenMain.h"
#include "../Qt/Scene/SceneData.h"
#include "../Qt/Scene/SceneDataManager.h"
#include "../SceneEditor/EditorBodyControl.h"
#include "../LandscapeEditor/LandscapesController.h"

#include "../Qt/Scene/System/LandscapeEditorDrawSystem/HeightmapProxy.h"

#include "Utils/Random.h"

HeightmapModificationCommand::HeightmapModificationCommand(Command::eCommandType type,
														   const Rect& updatedRect,
														   CommandList::eCommandId id)
:	Command(type, id)
{
	this->updatedRect = updatedRect;
}

String HeightmapModificationCommand::TimeString()
{
    time_t now = time(0);
    tm* utcTime = localtime(&now);
	
    String timeString = Format("%04d.%02d.%02d_%02d_%02d_%02d",
							   utcTime->tm_year + 1900, utcTime->tm_mon + 1, utcTime->tm_mday,
							   utcTime->tm_hour, utcTime->tm_min, utcTime->tm_sec);
	
    return timeString;
}

FilePath HeightmapModificationCommand::SaveHeightmap(Heightmap* heightmap)
{
	FilePath documentsPath("~doc:/");

	FilePath folderPathname("~doc:/History/");
	FileSystem::Instance()->CreateDirectory(folderPathname);

	folderPathname = folderPathname + "Heightmap/";
	FileSystem::Instance()->CreateDirectory(folderPathname);
	
	FileList* fileList = new FileList(folderPathname);
	
	bool validFileName = false;
	String filename;
	String time = TimeString();
	uint32 num = 0;
	do
	{
		filename = time;
		if (num)
		{
			filename += Format(" (%d)", num);
		}
		filename += Heightmap::FileExtension();
		
		int32 i = 0;
		for (; i < fileList->GetCount(); ++i)
		{
			if (fileList->GetFilename(i) == filename)
			{
				++num;
				break;
			}
		}
		if (i >= fileList->GetCount())
			validFileName = true;
	} while (!validFileName);

	FilePath saveFileName = folderPathname + filename;
	heightmap->Save(saveFileName);
	
	SafeRelease(fileList);
	
	return saveFileName;
}

LandscapeEditorHeightmap* HeightmapModificationCommand::GetEditor()
{
	LandscapeEditorHeightmap* editor = NULL;
	
	SceneEditorScreenMain *screen = dynamic_cast<SceneEditorScreenMain *>(UIScreenManager::Instance()->GetScreen());
	if(screen)
	{
		editor = dynamic_cast<LandscapeEditorHeightmap*>(screen->FindCurrentBody()->bodyControl->GetLandscapeEditor(SceneEditorScreenMain::ELEMID_HEIGHTMAP));
	}
	
	return editor;
}

void HeightmapModificationCommand::UpdateLandscapeHeightmap(const FilePath & filename)
{
	SceneData *activeScene = SceneDataManager::Instance()->SceneGetActive();
	LandscapesController* landscapesController = activeScene->GetLandscapesController();
	
	Landscape* landscapeNode = landscapesController->GetCurrentLandscape();
	
	Heightmap* heightmap = new Heightmap();
	heightmap->Load(filename);
	
	landscapeNode->SetHeightmap(heightmap);
	heightmap->Save(landscapeNode->GetHeightmapPathname());

	SafeRelease(heightmap);
}


CommandDrawHeightmap::CommandDrawHeightmap(Heightmap* originalHeightmap, Heightmap* newHeightmap, const Rect& updatedRect)
:	HeightmapModificationCommand(COMMAND_UNDO_REDO, updatedRect, CommandList::ID_COMMAND_DRAW_HEIGHTMAP)
{
	commandName = "Heightmap Change";

	if (originalHeightmap && newHeightmap)
	{
		undoFilename = SaveHeightmap(originalHeightmap);
		redoFilename = SaveHeightmap(newHeightmap);
	}
}

CommandDrawHeightmap::~CommandDrawHeightmap()
{
	FileSystem::Instance()->DeleteFile(undoFilename);
	FileSystem::Instance()->DeleteFile(redoFilename);
}

void CommandDrawHeightmap::Execute()
{
	LandscapeEditorHeightmap* editor = GetEditor();
	if (editor == NULL)
	{
		SetState(STATE_INVALID);
		return;
	}

	if (editor->IsActive())
	{
		Heightmap* heightmap = editor->GetHeightmap();
		heightmap->Load(redoFilename);
		editor->UpdateHeightmap(heightmap, updatedRect);
	}
	else
	{
		UpdateLandscapeHeightmap(redoFilename);
	}
}

void CommandDrawHeightmap::Cancel()
{
	LandscapeEditorHeightmap* editor = GetEditor();
	if (!editor)
		return;

	if (editor->IsActive())
	{
		Heightmap* heightmap = editor->GetHeightmap();
		heightmap->Load(undoFilename);
		editor->UpdateHeightmap(heightmap, updatedRect);
	}
	else
	{
		UpdateLandscapeHeightmap(undoFilename);
	}
}


CommandCopyPasteHeightmap::CommandCopyPasteHeightmap(bool copyHeightmap, bool copyTilemap, Heightmap* originalHeightmap, Heightmap* newHeightmap, Image* originalTilemap, Image* newTilemap, const FilePath& tilemapSavedPath, const Rect& updatedRect)
:	HeightmapModificationCommand(COMMAND_UNDO_REDO, updatedRect, CommandList::ID_COMMAND_COPY_PASTE_HEIGHTMAP)
,	heightmap(copyHeightmap)
,	tilemap(copyTilemap)
{
	commandName = "Heightmap Copy/Paste";

	tilemapUndoImage = NULL;
	tilemapRedoImage = NULL;

	if (copyHeightmap && originalHeightmap && newHeightmap)
	{
		heightmapUndoFilename = SaveHeightmap(originalHeightmap);
		heightmapRedoFilename = SaveHeightmap(newHeightmap);
	}

	if (copyTilemap && originalTilemap && newTilemap)
	{
        tilemapSavedPathname = FilePath::CreateWithNewExtension(tilemapSavedPath, ".png");
        
		tilemapUndoImage = SafeRetain(originalTilemap);
		tilemapRedoImage = SafeRetain(newTilemap);
	}
}

CommandCopyPasteHeightmap::~CommandCopyPasteHeightmap()
{
	FileSystem::Instance()->DeleteFile(heightmapUndoFilename);
	FileSystem::Instance()->DeleteFile(heightmapRedoFilename);
	SafeRelease(tilemapUndoImage);
	SafeRelease(tilemapRedoImage);
}

void CommandCopyPasteHeightmap::Execute()
{
	LandscapeEditorHeightmap* editor = GetEditor();
	if (!editor)
	{
		SetState(STATE_INVALID);
		return;
	}

	// Apply new heightmap
	if (heightmap)
	{
		if (editor->IsActive())
		{
			Heightmap* heightmap = editor->GetHeightmap();
			heightmap->Load(heightmapRedoFilename);
			editor->UpdateHeightmap(heightmap, updatedRect);
		}
		else
		{
			UpdateLandscapeHeightmap(heightmapRedoFilename);
		}
	}

	// Apply new tilemap
	if (tilemap)
	{
		UpdateLandscapeTilemap(tilemapRedoImage);
	}
}

void CommandCopyPasteHeightmap::Cancel()
{
	// Restore old heightmap
	if (heightmap)
	{
		LandscapeEditorHeightmap* editor = GetEditor();
		if (!editor)
			return;
		
		if (editor->IsActive())
		{
			Heightmap* heightmap = editor->GetHeightmap();
			heightmap->Load(heightmapUndoFilename);
			editor->UpdateHeightmap(heightmap, updatedRect);
		}
		else
		{
			UpdateLandscapeHeightmap(heightmapUndoFilename);
		}
	}

	// Restore old tilemap
	if (tilemap)
	{
		UpdateLandscapeTilemap(tilemapUndoImage);
	}
}

void CommandCopyPasteHeightmap::UpdateLandscapeTilemap(DAVA::Image *image)
{
	Texture* texture = Texture::CreateFromData(image->GetPixelFormat(), image->GetData(), image->GetWidth(), image->GetHeight(), false);
	texture->relativePathname = tilemapSavedPathname;
	texture->GenerateMipmaps();
	texture->SetWrapMode(Texture::WRAP_REPEAT, Texture::WRAP_REPEAT);

	LandscapeEditorBase* editor = GetActiveEditor();
	if (editor)
	{
		editor->UpdateLandscapeTilemap(texture);
	}
	else
	{
		SceneEditorScreenMain *screen = dynamic_cast<SceneEditorScreenMain *>(UIScreenManager::Instance()->GetScreen());
		EditorScene* scene = screen->FindCurrentBody()->bodyControl->GetScene();
		Landscape* landscape = FindLandscape(scene);

		landscape->SetTexture(Landscape::TEXTURE_TILE_MASK, texture);
		landscape->UpdateFullTiledTexture();
		ImageLoader::Save(image, tilemapSavedPathname);
	}

	SafeRelease(texture);
}

LandscapeEditorBase* CommandCopyPasteHeightmap::GetActiveEditor()
{
	LandscapeEditorBase* editor = NULL;
	
	SceneEditorScreenMain *screen = dynamic_cast<SceneEditorScreenMain *>(UIScreenManager::Instance()->GetScreen());
	if(screen)
	{
		editor = screen->FindCurrentBody()->bodyControl->GetCurrentLandscapeEditor();
	}
	
	return editor;
}

CommandModifyHeightmap::CommandModifyHeightmap(HeightmapProxy* heightmapProxy,
											   Heightmap* originalHeightmap,
											   const Rect& updatedRect)
:	HeightmapModificationCommand(COMMAND_UNDO_REDO, updatedRect, CommandList::ID_COMMAND_DRAW_HEIGHTMAP)
,	heightmapProxy(heightmapProxy)
{
	commandName = "Heightmap Change";
	
	if (originalHeightmap && heightmapProxy)
	{
		undoRegion = GetHeightmapRegion(originalHeightmap);
		redoRegion = GetHeightmapRegion(heightmapProxy);
	}
}

CommandModifyHeightmap::~CommandModifyHeightmap()
{
	SafeDeleteArray(undoRegion);
	SafeDeleteArray(redoRegion);
}

void CommandModifyHeightmap::Execute()
{
	ApplyHeightmapRegion(redoRegion);
}

void CommandModifyHeightmap::Cancel()
{
	ApplyHeightmapRegion(undoRegion);
}

uint16* CommandModifyHeightmap::GetHeightmapRegion(Heightmap* heightmap)
{
	int32 size = heightmap->Size();
	int32 width = (int32)updatedRect.dx;
	int32 height = (int32)updatedRect.dy;
	int32 xOffset = (int32)updatedRect.x;
	int32 yOffset = (int32)updatedRect.y;

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

void CommandModifyHeightmap::ApplyHeightmapRegion(uint16* region)
{
	int32 size = heightmapProxy->Size();
	int32 width = (int32)updatedRect.dx;
	int32 height = (int32)updatedRect.dy;
	int32 xOffset = (int32)updatedRect.x;
	int32 yOffset = (int32)updatedRect.y;

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
