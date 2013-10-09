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



#ifndef __RESOURCEEDITORQT__HEIGHTMAPEDITORCOMMANDS__
#define __RESOURCEEDITORQT__HEIGHTMAPEDITORCOMMANDS__

#include "DAVAEngine.h"
#include "Command.h"
#include "../SceneEditor/LandscapeEditorHeightmap.h"

class HeightmapModificationCommand: public Command
{
public:
	HeightmapModificationCommand(Command::eCommandType type,
								 const Rect& updatedRect,
								 CommandList::eCommandId id);

protected:
	Rect updatedRect;

	static String TimeString();
	static FilePath SaveHeightmap(Heightmap* heightmap);
	static LandscapeEditorHeightmap* GetEditor();
	static void UpdateLandscapeHeightmap(const FilePath & filename);
};


class HeightmapProxy;

class CommandModifyHeightmap: public HeightmapModificationCommand
{
public:
	CommandModifyHeightmap(HeightmapProxy* heightmapProxy,
						   Heightmap* originalHeightmap,
						   const Rect& updatedRect);
	virtual ~CommandModifyHeightmap();

protected:
	HeightmapProxy* heightmapProxy;
	uint16* undoRegion;
	uint16* redoRegion;

	virtual void Execute();
	virtual void Cancel();

	uint16* GetHeightmapRegion(Heightmap* heightmap);
	void ApplyHeightmapRegion(uint16* region);
};


class CommandDrawHeightmap: public HeightmapModificationCommand
{
public:
	DAVA_DEPRECATED(CommandDrawHeightmap(Heightmap* originalHeightmap, // DEPRECATED : using SceneDataManager(QOBJECT)
						 Heightmap* newHeightmap,
						 const Rect& updatedRect));
	virtual ~CommandDrawHeightmap();
	
protected:
	FilePath undoFilename;
	FilePath redoFilename;

	virtual void Execute();
	virtual void Cancel();
};

class CommandCopyPasteHeightmap: public HeightmapModificationCommand
{
public:
	DAVA_DEPRECATED(CommandCopyPasteHeightmap(bool copyHeightmap, bool copyTilemap, //DEPRECATED: using of SceneEditorScreenMain, SceneDataManager...
							  Heightmap* originalHeightmap, Heightmap* newHeightmap,
							  Image* originalTilemap, Image* newTilemap,
							  const FilePath& tilemapSavedPath, const Rect& updatedRect));
	virtual ~CommandCopyPasteHeightmap();

protected:
	FilePath heightmapUndoFilename;
	FilePath heightmapRedoFilename;

	Image* tilemapUndoImage;
	Image* tilemapRedoImage;
	FilePath tilemapSavedPathname;

	Landscape* landscape;

	bool heightmap;
	bool tilemap;

	virtual void Execute();
	virtual void Cancel();

	void UpdateLandscapeTilemap(Image* image);
	LandscapeEditorBase* GetActiveEditor();
};

#endif /* defined(__RESOURCEEDITORQT__HEIGHTMAPEDITORCOMMANDS__) */
