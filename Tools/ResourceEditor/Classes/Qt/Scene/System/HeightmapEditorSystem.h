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

class HeightmapEditorSystem : public LandscapeEditorSystem
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

    HeightmapEditorSystem(DAVA::Scene* scene);
    virtual ~HeightmapEditorSystem();

    LandscapeEditorDrawSystem::eErrorType EnableLandscapeEditing();
    bool DisableLandscapeEdititing();

    virtual void Process(DAVA::float32 timeElapsed);
    virtual void Input(DAVA::UIEvent* event);

    void SetBrushSize(DAVA::int32 brushSize);
    DAVA::int32 GetBrushSize();
    void SetStrength(DAVA::float32 strength);
    DAVA::float32 GetStrength();
    void SetAverageStrength(DAVA::float32 averageStrength);
    DAVA::float32 GetAverageStrength();
    void SetToolImage(const DAVA::FilePath& toolImagePath, DAVA::int32 index);
    DAVA::int32 GetToolImageIndex();
    void SetDrawingType(eHeightmapDrawType type);
    eHeightmapDrawType GetDrawingType();

    void SetDropperHeight(DAVA::float32 height);
    DAVA::float32 GetDropperHeight();

protected:
    DAVA::Vector2 GetHeightmapPositionFromCursor() const;

protected:
    DAVA::Texture* squareTexture = nullptr;
    DAVA::uint32 curToolSize = 30;
    DAVA::Image* curToolImage = nullptr;

    eHeightmapDrawType drawingType = HEIGHTMAP_DRAW_ABSOLUTE;
    DAVA::float32 strength = 15.f;
    DAVA::float32 averageStrength = 0.5f;
    bool inverseDrawingEnabled = false;
    DAVA::FilePath toolImagePath;
    DAVA::int32 toolImageIndex = 0;

    DAVA::float32 curHeight = 0.f;
    DAVA::Vector2 copyPasteFrom;
    DAVA::Vector2 copyPasteTo;

    DAVA::Rect heightmapUpdatedRect;

    bool editingIsEnabled = false;

    DAVA::Heightmap* originalHeightmap = nullptr;

    eHeightmapDrawType activeDrawingType;

    void UpdateToolImage();
    void UpdateBrushTool(DAVA::float32 timeElapsed);

    void AddRectToAccumulator(DAVA::Rect& accumulator, const DAVA::Rect& rect);
    void ResetAccumulatorRect(DAVA::Rect& accumulator);
    DAVA::Rect GetHeightmapUpdatedRect();

    void StoreOriginalHeightmap();
    void CreateHeightmapUndo();

    void FinishEditing();
};

#endif /* defined(__RESOURCEEDITORQT__HEIGHTMAPEDITORSYSTEM__) */
