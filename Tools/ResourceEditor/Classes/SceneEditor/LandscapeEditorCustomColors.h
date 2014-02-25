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



#ifndef __LANDSCAPE_EDITOR_CUSTOM_COLOR_H__
#define __LANDSCAPE_EDITOR_CUSTOM_COLOR_H__

#include "DAVAEngine.h"
#include "LandscapeEditorBase.h"
#include "LandscapeToolsPanel.h"
#include "LandscapeEditorPropertyControl.h"


using namespace DAVA;

class EditorHeightmap;
class CommandDrawCustomColors;
class LandscapeEditorCustomColors
    :   public LandscapeEditorBase
    ,   public LandscapeEditorPropertyControlDelegate

{
protected:
    virtual ~LandscapeEditorCustomColors();
public:
    LandscapeEditorCustomColors(LandscapeEditorDelegate *newDelegate, EditorBodyControl *parentControl, const Rect &toolsRect);
    
	virtual void Draw(const UIGeometricData &geometricData);

    virtual NodesPropertyControl *GetPropertyControl(const Rect &rect);
    
    virtual bool SetScene(EditorScene *newScene);
	virtual void SaveTexture();
    
    //LE property control delegate
    virtual void LandscapeEditorSettingsChanged(LandscapeEditorSettings *settings);
    virtual void TextureWillChanged(const String &forKey);
    virtual void TextureDidChanged(const String &forKey);
    

	void SetColor(const Color &newColor);
	void SetRadius(int radius);
	void SaveColorLayer(const FilePath &pathName);
	void LoadColorLayer(const FilePath &pathName);
	FilePath GetCurrentSaveFileName();

	void ClearSceneResources();

	Image* StoreState();
	void RestoreState(Image* image);

	virtual void UpdateLandscapeTilemap(Texture* texture);

protected:

    virtual void InputAction(int32 phase, bool intersects);
    virtual void HideAction();
    virtual void ShowAction();
    virtual void SaveTextureAction(const FilePath &pathToFile);
	virtual void UpdateCursor();

	void StoreOriginalState();
	void CreateUndoPoint();

	FilePath GetScenePath();
	String GetRelativePathToScenePath(const FilePath& absolutePath);
	FilePath GetAbsolutePathFromScenePath(const String& relativePath);
	String GetRelativePathToProjectPath(const FilePath& absolutePath);
	FilePath GetAbsolutePathFromProjectPath(const String& relativePath);
	void StoreSaveFileName(const FilePath& fileName);

	void LoadTextureAction(const FilePath& pathToFile);

    virtual void RecreateHeightmapNode();
	void UpdateCircleTexture(bool setTransparent);


	void PrepareRenderLayers();

    void PerformLandscapeDraw();
    void DrawCircle(Vector<Vector<bool> >& matrixForCircle) ;
    uint8*	DrawFilledCircleWithFormat(uint32 radius, DAVA::PixelFormat format, bool setTransparent);

	bool wasTileMaskToolUpdate;

    LandscapeEditorSettings *settings;

    Color paintColor;

    bool editingIsEnabled;

	Texture*	circleTexture;
	Sprite*		colorSprite;
	Texture*	texSurf;

	int32		radius;

	bool		isCursorTransparent;
	bool		isFogEnabled;

	bool unsavedChanges;

	Image* originalTexture;
};


#endif //__LANDSCAPE_EDITOR_CUSTOM_COLOR_H__
