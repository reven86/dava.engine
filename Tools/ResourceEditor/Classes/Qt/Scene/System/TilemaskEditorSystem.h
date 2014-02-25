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



#ifndef __RESOURCEEDITORQT__TILEMASKEDITORSYSTEM__
#define __RESOURCEEDITORQT__TILEMASKEDITORSYSTEM__

#include "DAVAEngine.h"
#include "Commands2/MetaObjModifyCommand.h"
#include "LandscapeEditorDrawSystem.h"

#include "Render/UniqueStateSet.h"

class SceneCollisionSystem;
class SceneSelectionSystem;
class EntityModificationSystem;

using namespace DAVA;

class TilemaskEditorSystem: public DAVA::SceneSystem
{
public:
	enum eTilemaskDrawType
	{
		TILEMASK_DRAW_NORMAL = 0,
		TILEMASK_DRAW_COPY_PASTE,
		
		TILEMASK_DRAW_TYPES_COUNT
	};

	TilemaskEditorSystem(Scene* scene);
	virtual ~TilemaskEditorSystem();
	
	LandscapeEditorDrawSystem::eErrorType EnableLandscapeEditing();
	bool DisableLandscapeEdititing();
	bool IsLandscapeEditingEnabled() const;
	
	void Update(DAVA::float32 timeElapsed);
	void ProcessUIEvent(DAVA::UIEvent *event);
	void Draw();
	
	void SetBrushSize(int32 brushSize);
	int32 GetBrushSize();
	void SetStrength(float32 strength);
	float32 GetStrength();
	void SetToolImage(const FilePath& toolImagePath, int32 index);
	int32 GetToolImage();
	void SetTileTexture(uint32 tileTexture);
	uint32 GetTileTextureIndex();

	uint32 GetTileTextureCount() const;
	Texture* GetTileTexture(int32 index);
	Color GetTileColor(int32 index);
	void SetTileColor(int32 index, const Color& color);

	void SetDrawingType(eTilemaskDrawType type);
	eTilemaskDrawType GetDrawingType();

protected:
	bool enabled;
	
	SceneCollisionSystem* collisionSystem;
	SceneSelectionSystem* selectionSystem;
	EntityModificationSystem* modifSystem;
	LandscapeEditorDrawSystem* drawSystem;
	
	int32 landscapeSize;
	Texture* cursorTexture;
	uint32 cursorSize;
	uint32 curToolSize;
	Image* toolImage;
	Sprite* toolImageSprite;
	uint32 tileTextureNum;

	eTilemaskDrawType drawingType;
	eTilemaskDrawType activeDrawingType;
	float32 strength;
	FilePath toolImagePath;
	int32 toolImageIndex;
	
	bool isIntersectsLandscape;
	Vector2 cursorPosition;
	Vector2 prevCursorPos;
	Vector2 copyPasteFrom;
	Vector2 copyPasteTo;
	
	Rect updatedRectAccumulator;
	
	bool editingIsEnabled;
	
	Sprite* stencilSprite;
	Sprite* toolSprite;
	bool toolSpriteUpdated;

	eBlendMode srcBlendMode;
	eBlendMode dstBlendMode;
	Shader* tileMaskEditorShader;
	Shader* tileMaskCopyPasteShader;

	bool needCreateUndo;

	Landscape::eTextureLevel textureLevel;

	void UpdateCursorPosition();
	void UpdateToolImage(bool force = false);
	void UpdateBrushTool();
	Image* CreateToolImage(int32 sideSize, const FilePath& filePath);
	
	void AddRectToAccumulator(const Rect& rect);
	void ResetAccumulatorRect();
	Rect GetUpdatedRect();
	
	void CreateMaskTexture();
	void CreateMaskFromTexture(Texture* texture);

	void CreateUndoPoint();

	LandscapeEditorDrawSystem::eErrorType IsCanBeEnabled();

	void InitSprites();

	void FinishEditing();

	MetaObjModifyCommand* CreateTileColorCommand(Landscape::eTextureLevel level,
												 const Color& color);
	
	UniqueHandle noBlendDrawState;
};

#endif /* defined(__RESOURCEEDITORQT__TILEMASKEDITORSYSTEM__) */
