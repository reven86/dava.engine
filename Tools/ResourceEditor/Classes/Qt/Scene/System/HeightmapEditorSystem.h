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


#ifndef __RESOURCEEDITORQT__HEIGHTMAPEDITORSYSTEM__
#define __RESOURCEEDITORQT__HEIGHTMAPEDITORSYSTEM__

#include "LandscapeEditorSystem.h"
#include "LandscapeEditorDrawSystem.h"

#include "Render/UniqueStateSet.h"

class HoodSystem;

using namespace DAVA;

class HeightmapEditorSystem: public LandscapeEditorSystem
{
public:
	enum eHeightmapDrawType
	{
		HEIGHTMAP_DRAW_ABSOLUTE = 0,
		HEIGHTMAP_DRAW_RELATIVE,
		HEIGHTMAP_DRAW_AVERAGE,
		HEIGHTMAP_DRAW_ABSOLUTE_DROPPER,
		HEIGHTMAP_DROPPER,
		HEIGHTMAP_COPY_PASTE,
		
		HEIGHTMAP_DRAW_TYPES_COUNT
	};
	
	HeightmapEditorSystem(Scene* scene);
	virtual ~HeightmapEditorSystem();
	
	LandscapeEditorDrawSystem::eErrorType EnableLandscapeEditing();
	bool DisableLandscapeEdititing();
	
	virtual void Process(DAVA::float32 timeElapsed);
	virtual void Input(DAVA::UIEvent *event);
	
	void SetBrushSize(int32 brushSize);
	int32 GetBrushSize();
	void SetStrength(float32 strength);
	float32 GetStrength();
	void SetAverageStrength(float32 averageStrength);
	float32 GetAverageStrength();
	void SetToolImage(const FilePath& toolImagePath, int32 index);
    int32 GetToolImageIndex();
    void SetDrawingType(eHeightmapDrawType type);
    eHeightmapDrawType GetDrawingType();

    void SetDropperHeight(float32 height);
	float32 GetDropperHeight();

protected:
    
    Vector2 GetHeightmapPositionFromCursor() const;
    
protected:
	Texture* squareTexture;
	uint32 curToolSize;
    Image* curToolImage;

    eHeightmapDrawType drawingType;
    float32 strength;
    float32 averageStrength;
	bool inverseDrawingEnabled;
	FilePath toolImagePath;
	int32 toolImageIndex;

	float32 curHeight;
	Vector2 copyPasteFrom;
	Vector2 copyPasteTo;
	
	Rect heightmapUpdatedRect;

	bool editingIsEnabled;
	
	Heightmap* originalHeightmap;

    eHeightmapDrawType activeDrawingType;

    void UpdateToolImage();
    void UpdateBrushTool(float32 timeElapsed);

    void AddRectToAccumulator(Rect& accumulator, const Rect& rect);
    void ResetAccumulatorRect(Rect& accumulator);
	Rect GetHeightmapUpdatedRect();
	
	void StoreOriginalHeightmap();
	void CreateHeightmapUndo();

	void FinishEditing();
};

#endif /* defined(__RESOURCEEDITORQT__HEIGHTMAPEDITORSYSTEM__) */
