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


#ifndef __DAVAENGINE_RENDER_RENDERSYSTEM_2D_H__
#define	__DAVAENGINE_RENDER_RENDERSYSTEM_2D_H__

#include "Base/BaseTypes.h"
#include "Base/Singleton.h"
#include "Render/RenderBase.h"
#include "Render/2D/Sprite.h"

#include "UI/UIControlBackground.h"

namespace DAVA
{

class Font;
class Sprite;
class TextBlock;
class UIGeometricData;
class VboPool;

struct RenderBatch2D
{
    explicit RenderBatch2D() { Reset(); }
    inline void Reset()
    {
        primitiveType = rhi::PRIMITIVE_TRIANGLELIST;
        clipRect = Rect(0, 0, -1, -1);
        transformedClipRect = Rect(0, 0, -1, -1);
        count = 0;
        indexOffset = 0;
        material = nullptr;
    }

    rhi::HTextureSet textureSetHandle;
    NMaterial * material;
    Rect clipRect;
    Rect transformedClipRect;
    rhi::PrimitiveType primitiveType;
    uint32 count;
    uint32 indexOffset;
};

struct TiledDrawData
{
    Vector< Vector2 > vertices;
    Vector< Vector2 > texCoords;
    Vector< uint16  > indeces;
    void GenerateTileData();
    void GenerateAxisData( float32 size, float32 spriteSize, float32 textureSize, float32 stretchCap, Vector< Vector3 > &axisData );

    Vector< Vector2 > transformedVertices;
    void GenerateTransformData();

    Sprite *sprite;
    int32 frame;
    Vector2 size;
    Vector2 stretchCap;
    Matrix3 transformMatr;
};

struct StretchDrawData
{
    Vector<Vector2> vertices;
    Vector<Vector2> transformedVertices;
    Vector<Vector2> texCoords;
    static const uint16 indeces[18 * 3];

    void GenerateStretchData();
    void GenerateTransformData();
    uint32 GetVertexInTrianglesCount() const;

    Sprite *sprite;
    int32 frame;
    Vector2 size;
    int32 type;
    Vector2 stretchCap;
    Matrix3 transformMatr;
};

class RenderSystem2D : public Singleton<RenderSystem2D>
{
public:
    static FastName FLAT_COLOR_SHADER;
    static FastName TEXTURE_FLAT_COLOR_SHADER;

    static ShaderDescriptor * FLAT_COLOR;
    static ShaderDescriptor * TEXTURE_MUL_FLAT_COLOR;
    static ShaderDescriptor * TEXTURE_MUL_FLAT_COLOR_ALPHA_TEST;
    static ShaderDescriptor * TEXTURE_MUL_FLAT_COLOR_IMAGE_A8;
    static ShaderDescriptor * TEXTURE_ADD_FLAT_COLOR;
    static ShaderDescriptor * TEXTURE_ADD_FLAT_COLOR_ALPHA_TEST;
    static ShaderDescriptor * TEXTURE_ADD_FLAT_COLOR_IMAGE_A8;
    static ShaderDescriptor * TEXTURE_MUL_COLOR;
    static ShaderDescriptor * TEXTURE_MUL_COLOR_ALPHA_TEST;
    static ShaderDescriptor * TEXTURE_MUL_COLOR_IMAGE_A8;
    static ShaderDescriptor * TEXTURE_ADD_COLOR;
    static ShaderDescriptor * TEXTURE_ADD_COLOR_ALPHA_TEST;
    static ShaderDescriptor * TEXTURE_ADD_COLOR_IMAGE_A8;


    RenderSystem2D();
    virtual ~RenderSystem2D();
    
    void Init();

    void Draw(Sprite * sprite, Sprite::DrawState * drawState = 0);
    void DrawStretched(Sprite * sprite, Sprite::DrawState * drawState, Vector2 streatchCap, UIControlBackground::eDrawType type, const UIGeometricData &gd, StretchDrawData ** pStreachData);
    void DrawTiled(Sprite * sprite, Sprite::DrawState * drawState, const Vector2& streatchCap, const UIGeometricData &gd, TiledDrawData ** pTiledData);

    /**
     * Destroy current buffers and create new.
     * @param verticesCount vertices count per buffer (size of buffer equals verticesCount*GetVertexSize(vertexFormat))
     * @param indicesCount indices count per buffer (size of buffer equals indicesCount*sizeof(uint16))
     * @param buffersCount buffers count
     */
    void HardResetBatchingBuffers(uint32 verticesCount, uint32 indicesCount, uint8 buffersCount);

    void PushBatch(NMaterial * material, rhi::HTextureSet texture, const Rect& clip,
        uint32 vertexCount, const float32* vertexPointer, const float32* texCoordPointer,
        uint32 indexCount, const uint16* indexPointer,
        const Color& color, const rhi::PrimitiveType primitiveType = rhi::PRIMITIVE_TRIANGLELIST);
    
    /*!
     * Highlight controls which has vertices count bigger than verticesCount.
     * Work only with RenderOptions::HIGHLIGHT_BIG_CONTROLS option enabled.
     * @param verticesCount vertices limit
     */
    void SetHightlightControlsVerticesLimit(uint32 verticesCount);

    void BeginFrame();
    void EndFrame();
    void Flush();
    
    void SetClip(const Rect &rect);
	void IntersectClipRect(const Rect &rect);
	void RemoveClip();
    
	void PushClip();
	void PopClip();

    inline void SetColor(const Color & color);
    inline void SetColor(float32 r, float32 g, float32 b, float32 a);
    inline const Color & GetColor();

    void ScreenSizeChanged();

    void Setup2DMatrices();
    
    void SetSpriteClipping(bool clipping);

private:
    bool IsPreparedSpriteOnScreen(Sprite::DrawState * drawState);
    static ShaderDescriptor* GetShaderForBatching(ShaderDescriptor* inputShader);
    
    void Setup2DProjection();

    Rect TransformClipRect(const Rect & rect);

    Matrix4 virtualToPhysicalMatrix;
    Matrix4 projMatrix;
	std::stack<Rect> clipStack;
	Rect currentClip;

#if RHI_COMPLETE
    RenderDataObject * spriteRenderObject;
#endif RHI_COMPLETE

    float32 spriteTempVertices[8];
    Vector<Vector2> spriteClippedTexCoords;
    Vector<Vector2> spriteClippedVertices;
    ePrimitiveType spritePrimitiveToDraw;
    int32 spriteVertexCount;
    int32 spriteIndexCount;

    Sprite::DrawState defaultSpriteDrawState;

    Vector<float32> vboTemp;
    Vector<uint16> iboTemp;

    bool spriteClipping;
    
    Vector<RenderBatch2D> batches;
    RenderBatch2D currentBatch;
    uint32 vertexIndex;
    uint32 indexIndex;

    Color currentColor;

    VboPool* pool;

    // Batching errors handling
    uint32 prevFrameErrorsFlags;
    uint32 currFrameErrorsFlags;
    enum ErrorFlag {
        NO_ERRORS = 0,
        BUFFER_OVERFLOW_ERROR = 1,
    };
    uint32 highlightControlsVerticesLimit;

    rhi::RenderPassConfig renderPassConfig;
    rhi::HRenderPass currentPassHandle;
    rhi::HPacketList currentPacketListHandle;

};

inline void RenderSystem2D::SetHightlightControlsVerticesLimit(uint32 verticesCount)
{
    highlightControlsVerticesLimit = verticesCount;
}

inline void RenderSystem2D::SetColor(const Color & color)
{
    currentColor = color;
}

void RenderSystem2D::SetColor(float32 r, float32 g, float32 b, float32 a)
{
    SetColor(Color(r, g, b, a));
}

inline const Color & RenderSystem2D::GetColor()
{
    return currentColor;
}

} // ns

#endif	/* __DAVAENGINE_RENDER_RENDERSYSTEM_2D_H__ */

