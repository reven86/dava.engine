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

#include "LandscapeEditorSystem.h"


#include "Commands2/MetaObjModifyCommand.h"
#include "LandscapeEditorDrawSystem.h"

#include "Render/UniqueStateSet.h"

class TilemaskEditorSystem : public LandscapeEditorSystem
{
public:
    enum eTilemaskDrawType
    {
        TILEMASK_DRAW_NORMAL = 0,
        TILEMASK_DRAW_COPY_PASTE,

        TILEMASK_DRAW_TYPES_COUNT
    };

    TilemaskEditorSystem(DAVA::Scene* scene);
    virtual ~TilemaskEditorSystem();

    LandscapeEditorDrawSystem::eErrorType EnableLandscapeEditing();
    bool DisableLandscapeEdititing();

    void Process(DAVA::float32 timeElapsed) override;
    void Input(DAVA::UIEvent* event) override;
    void Draw();

    void SetBrushSize(DAVA::int32 brushSize);
    DAVA::int32 GetBrushSize();
    void SetStrength(DAVA::float32 strength);
    DAVA::float32 GetStrength();
    void SetToolImage(const DAVA::FilePath& toolImagePath, DAVA::int32 index);
    DAVA::int32 GetToolImage();
    void SetTileTexture(DAVA::uint32 tileTexture);
    DAVA::uint32 GetTileTextureIndex();

    DAVA::uint32 GetTileTextureCount() const;
    DAVA::Texture* GetTileTexture();
    DAVA::Color GetTileColor(DAVA::uint32 index);
    void SetTileColor(DAVA::int32 index, const DAVA::Color& color);

    void SetDrawingType(eTilemaskDrawType type);
    eTilemaskDrawType GetDrawingType();

protected:
    DAVA::uint32 curToolSize;

    DAVA::Texture* toolImageTexture;
    DAVA::Texture* landscapeTilemaskTexture;

    DAVA::uint32 tileTextureNum;

    DAVA::NMaterial* editorMaterial;

    eTilemaskDrawType drawingType;
    eTilemaskDrawType activeDrawingType;
    DAVA::float32 strength;
    DAVA::FilePath toolImagePath;
    DAVA::int32 toolImageIndex;

    rhi::HVertexBuffer quadBuffer;
    rhi::Packet quadPacket;
    DAVA::uint32 quadVertexLayoutID;

    DAVA::Vector2 copyPasteFrom;
    DAVA::Vector2 copyPasteOffset;

    DAVA::Rect updatedRectAccumulator;

    bool editingIsEnabled;

    DAVA::Texture* toolTexture;
    bool toolSpriteUpdated;

    bool needCreateUndo;

    const DAVA::FastName& textureLevel;

    void UpdateBrushTool();
    void UpdateToolImage();

    void AddRectToAccumulator(const DAVA::Rect& rect);
    void ResetAccumulatorRect();
    DAVA::Rect GetUpdatedRect();

    void CreateMaskTexture();
    void CreateMaskFromTexture(DAVA::Texture* texture);

    void CreateUndoPoint();

    void InitSprites();

    void FinishEditing();
};

#endif /* defined(__RESOURCEEDITORQT__TILEMASKEDITORSYSTEM__) */
