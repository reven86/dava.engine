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
    struct BatchDescriptor 
    {
        Color singleColor = Color::White; 
        uint32 vertexCount = 0;
        uint32 indexCount = 0;
        const float32* vertexPointer = nullptr;
        uint32 vertexStride = 0;
        const float32* texCoordPointer = nullptr;
        uint32 texCoordStride = 0;
        const uint32* colorPointer = nullptr;
        uint32 colorStride = 0;
        const uint16* indexPointer = nullptr;
        NMaterial * material = nullptr;
        rhi::HTextureSet textureSetHandle;
        rhi::HSamplerState samplerStateHandle;
        rhi::PrimitiveType primitiveType = rhi::PRIMITIVE_TRIANGLELIST;
        Matrix4* worldMatrix = nullptr;
    };

    enum ColorOperations
    {
        COLOR_MUL = 0,
        COLOR_ADD,
        ALPHA_MUL,
        ALPHA_ADD,
    };

    static const FastName RENDER_PASS_NAME;
    static const FastName FLAG_COLOR_OP;

    static NMaterial* DEFAULT_2D_COLOR_MATERIAL;
    static NMaterial* DEFAULT_2D_TEXTURE_MATERIAL;
    static NMaterial* DEFAULT_2D_TEXTURE_NOBLEND_MATERIAL;
    static NMaterial* DEFAULT_2D_TEXTURE_ALPHA8_MATERIAL;
    static NMaterial* DEFAULT_2D_TEXTURE_GRAYSCALE_MATERIAL;

    RenderSystem2D();
    virtual ~RenderSystem2D();
    
    void Init();

    void Draw(Sprite * sprite, Sprite::DrawState * drawState, const Color& color);
    void DrawStretched(Sprite * sprite, Sprite::DrawState * drawState, Vector2 streatchCap, UIControlBackground::eDrawType type, const UIGeometricData &gd, StretchDrawData ** pStreachData, const Color& color);
    void DrawTiled(Sprite * sprite, Sprite::DrawState * drawState, const Vector2& streatchCap, const UIGeometricData &gd, TiledDrawData ** pTiledData, const Color& color);

    void SetViewMatrix(const Matrix4& viewMatrix);

    /**
     * Destroy current buffers and create new.
     * @param verticesCount vertices count per buffer (size of buffer equals verticesCount*GetVertexSize(vertexFormat))
     * @param indicesCount indices count per buffer (size of buffer equals indicesCount*sizeof(uint16))
     * @param buffersCount buffers count
     */
    void HardResetBatchingBuffers(uint32 verticesCount, uint32 indicesCount, uint8 buffersCount);

    void PushBatch(const BatchDescriptor& batchDesc);
    
    /*
     *  note - it will flush currently batched!
     *  it will also modify packet to add current clip
     */
    void DrawPacket(rhi::Packet& packet);
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

    void ScreenSizeChanged();

    void SetSpriteClipping(bool clipping);

    void BeginRenderTargetPass(Texture * target, bool needClear = true, const Color& clearColor = Color::Clear, int32 priority = PRIORITY_SERVICE_2D);
    void EndRenderTargetPass();

    /* 2D DRAW HELPERS */

    /**
    \brief Draws line from pt1 to pt2
    \param pt1 starting point
    \param pt2 ending point
    \param color draw color
    */
    void DrawLine(const Vector2 & pt1, const Vector2 & pt2, const Color& color);

    /**
    \brief Draws line from pt1 to pt2
    \param pt1 starting point
    \param pt2 ending point
    \param color draw color
    */
    void DrawLine(const Vector2 &start, const Vector2 &end, float32 lineWidth, const Color& color);

    /**
    \brief Draws multiple lines.
    \param linePoints list of points in the format (startX, startY, endX, endY), (startX, startY, endX, endY)...
    \param color draw color
    */
    void DrawLines(const Vector<float32>& linePoints, const Color& color);

    /**
    \brief Draws given rect in 2D space
    \param pt1 starting point
    \param pt2 ending point
    \param color draw color
    */
    void DrawRect(const Rect & rect, const Color& color);

    /**
    \brief Fills given rect in 2D space
    \param pt1 starting point
    \param pt2 ending point
    \param color draw color
    */
    void FillRect(const Rect & rect, const Color& color);

    /**
    \brief Fills given rect in 2D space using four colors in corners
    \param pt1 starting point
    \param pt2 ending point
    \param xy top left color
    \param wy top right color
    \param xh bottom left color
    \param wh bottom right color
    */
    void FillGradientRect(const Rect& rect, const Color& xy, const Color& wy, const Color& xh, const Color& wh);

    /**
    \brief Draws grid in the given rect
    \param rect rect to fill grid with
    \param gridSize distance between grid lines
    \param color grid color
    */
    void DrawGrid(const Rect & rect, const Vector2& gridSize, const Color& color);

    /**
    \brief Draws circle in 2D space
    \param center center of the circle
    \param radius radius of the circle
    \param color draw color
    */
    void DrawCircle(const Vector2 & center, float32 radius, const Color& color);

    /**
    \brief Draws all concecutive lines from given polygon
    \param polygon the polygon we want to draw
    \param closed you should set this flag to true if you want to connect last point of polygon with first point
    \param color draw color
    */
    void DrawPolygon(const Polygon2 & polygon, bool closed, const Color& color);

    /**
    \brief Fill convex polygon with color. As all other draw functions this function use global color that can be set with RenderSystem2D::Instance()->SetColor function.
    \param polygon the polygon we want to draw
    \param color draw color
    */
    void FillPolygon(const Polygon2 & polygon, const Color& color);

    /**
    \brief Draws all concecutive lines from given polygon after transformation
    \param polygon the polygon we want to draw
    \param closed you should set this flag to true if you want to connect last point of polygon with first point
    \param transform transform that will be applied to polygon before it will be drawn
    \param color draw color
    */
    void DrawPolygonTransformed(const Polygon2 & polygon, bool closed, const Matrix3 & transform, const Color& color);

    void DrawTexture(Texture* texture, NMaterial* material, const Color& color, const Rect& dstRect = Rect(0.f, 0.f, -1.f, -1.f), const Rect& srcRect = Rect(0.f, 0.f, -1.f, -1.f));

private:
    bool IsPreparedSpriteOnScreen(Sprite::DrawState * drawState);
    void Setup2DMatrices();

    Rect TransformClipRect(const Rect & rect, const Matrix4 & transformMatrix);

    inline bool IsRenderTargetPass() { return (currentPacketListHandle != packetList2DHandle); };

    Matrix4 virtualToPhysicalMatrix;
    Matrix4 projMatrix;
    Matrix4 viewMatrix;
    uint32 projMatrixSemantic;
    uint32 viewMatrixSemantic;
    std::stack<Rect> clipStack;
	Rect currentClip;

    Array<float32, 8> spriteTempVertices;
    Array<uint32, 4> spriteTempColors;
    Vector<Vector2> spriteClippedTexCoords;
    Vector<Vector2> spriteClippedVertices;
    
    int32 spriteVertexCount;
    int32 spriteIndexCount;

    Sprite::DrawState defaultSpriteDrawState;

    bool spriteClipping;

    struct BatchVertex
    {
        Vector3 pos;
        Vector2 uv;
        uint32 color;
    };
    
    BatchVertex * currentVertexBuffer;
    uint16 * currentIndexBuffer;
    rhi::Packet currentPacket;
    uint32 currentIndexBase;
    uint32 vertexIndex;
    uint32 indexIndex;
    NMaterial * lastMaterial;
    Rect lastClip;
    Matrix4 lastCustomWorldMatrix;
    bool lastUsedCustomWorldMatrix;
    uint32 lastCustomMatrixSematic;

    // Batching errors handling
    uint32 prevFrameErrorsFlags;
    uint32 currFrameErrorsFlags;
    enum ErrorFlag {
        NO_ERRORS = 0,
        BUFFER_OVERFLOW_ERROR = 1,
    };
    uint32 highlightControlsVerticesLimit;

    rhi::HRenderPass pass2DHandle;
    rhi::HPacketList packetList2DHandle;
    rhi::HRenderPass passTargetHandle;
    rhi::HPacketList currentPacketListHandle;

    int32 renderTargetWidth;
    int32 renderTargetHeight;
    
};

inline void RenderSystem2D::SetHightlightControlsVerticesLimit(uint32 verticesCount)
{
    highlightControlsVerticesLimit = verticesCount;
}

} // ns

#endif	/* __DAVAENGINE_RENDER_RENDERSYSTEM_2D_H__ */

