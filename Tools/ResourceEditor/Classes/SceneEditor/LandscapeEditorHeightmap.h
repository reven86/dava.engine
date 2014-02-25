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



#ifndef __LANDSCAPE_EDITOR_HEIGHTMAP_H__
#define __LANDSCAPE_EDITOR_HEIGHTMAP_H__

#include "DAVAEngine.h"
#include "LandscapeEditorBase.h"
#include "LandscapeToolsPanel.h"
#include "LandscapeEditorPropertyControl.h"

using namespace DAVA;

class EditorScene;
class EditorHeightmap;
class EditorLandscape;
class LandscapesController;
class CommandDrawHeightmap;
class LandscapeEditorHeightmap
    :   public LandscapeEditorBase
    ,   public LandscapeEditorPropertyControlDelegate
{
protected:
    virtual ~LandscapeEditorHeightmap();
public:
    
    LandscapeEditorHeightmap(LandscapeEditorDelegate *newDelegate, EditorBodyControl *parentControl, const Rect &toolsRect); 
    
    virtual void Update(float32 timeElapsed);

    virtual NodesPropertyControl *GetPropertyControl(const Rect &rect);
    
    //Tools Panel delegate
    virtual void OnToolSelected(LandscapeTool *newTool);
    virtual void OnShowGrid(bool show);

    //LE property control delegate
    virtual void LandscapeEditorSettingsChanged(LandscapeEditorSettings *settings);
    virtual void TextureWillChanged(const String &forKey);
    virtual void TextureDidChanged(const String &forKey);

	Heightmap* GetHeightmap();
	void UpdateHeightmap(Heightmap* heightmap, Rect rect = Rect(-1, -1, -1, -1));

	virtual void UpdateLandscapeTilemap(Texture* texture);
protected:
	void StoreOriginalHeightmap();
	void CreateUndoPoint();
	void CreateHeightmapUndo();
	void CreateCopyPasteUndo();

    virtual void InputAction(int32 phase, bool intersects);
    virtual void HideAction();
    virtual void ShowAction();
    virtual void SaveTextureAction(const FilePath &pathToFile);
	virtual void UpdateCursor();

    bool CopyPasteBegin();
    virtual void RecreateHeightmapNode();

    
	void UpdateTileMaskTool(float32 timeElapsed);
	void UpdateBrushTool(float32 timeElapsed);
	void UpdateCopypasteTool(float32 timeElapsed);

    
    void UpdateToolImage();
    float32 GetDropperHeight();
    
    void UpdateHeightmap(const Rect &updatedRect);
    
    
    bool editingIsEnabled;
    
	bool wasTileMaskToolUpdate;

    LandscapesController *landscapesController;
    
    Image *toolImage;
    Image *toolImageTile;
    float32 prevToolSize;
    
    Vector2 copyFromCenter;
    Vector2 copyToCenter;
    Image *tilemaskImage;
    FilePath tilemaskPathname;
    bool tilemaskWasChanged;
    Texture *tilemaskTexture;
    
    void CreateTilemaskImage();
    Image *CreateToolImage(int32 sideSize);

	Rect updatedRectAccumulator;
	Heightmap* oldHeightmap;
	Image* oldTilemap;

	void AddRectToAccumulator(const Rect& rect);
	void ResetAccumulatorRect();
	Rect GetUpdatedRect();
};


#endif //__LANDSCAPE_EDITOR_HEIGHTMAP_H__
