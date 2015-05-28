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


#include "RenderSystem2D.h"
#include "VirtualCoordinatesSystem.h"

#include "UI/UIControl.h"
#include "UI/UIControlBackground.h"

#include "Render/Renderer.h"

namespace DAVA
{

#ifndef BATCHING_DEBUG
// Enable buffer checks (reuse VBO which already used in triple buffer)
#define BATCHING_DEBUG 0
#endif

static const uint32 MAX_VERTICES = 1024;
static const uint32 MAX_INDECES = MAX_VERTICES * 2;
static const uint32 VBO_POOL_SIZE = 40;
static const uint32 RESERVED_BATCHES = 1024;
static const uint32 VBO_FORMAT = EVF_VERTEX | EVF_TEXCOORD0 | EVF_COLOR;
#if BATCHING_DEBUG
static const uint32 VBO_USING_FRAME_LIFE = 3; // "triple buffer"
#endif
static const float32 SEGMENT_LENGTH = 15.0f;

NMaterial* RenderSystem2D::DEFAULT_2D_COLOR_MATERIAL = nullptr;
NMaterial* RenderSystem2D::DEFAULT_2D_TEXTURE_MATERIAL = nullptr;
NMaterial* RenderSystem2D::DEFAULT_2D_TEXTURE_ALPHA8_MATERIAL = nullptr;

class VboPool
{
public:
    VboPool(uint32 verticesCount, uint32 format, uint32 indicesCount, uint8 buffersCount);
    ~VboPool();

    void ReleaseBuffers();
    void HardReset(uint32 verticesCount, uint32 indicesCount, uint8 buffersCount);

    void Next();

#if BATCHING_DEBUG
    void NextFrame();
    void CheckForBufferReuse();
    void EnableBuffersLoopingWarning(int8 framesCount);
#endif

    rhi::HVertexBuffer GetVertexBuffer();
    rhi::HIndexBuffer GetIndexBuffer();

    uint32 GetVerticesLimit() const;
    uint32 GetIndicesLimit() const;
    uint32 GetVertexStride() const;
    uint32 GetVertexLayoutID() const;

private:
    rhi::HVertexBuffer currentVertexBuffer;
    rhi::HIndexBuffer currentIndexBuffer;
    Vector<rhi::HVertexBuffer> vertexBuffers;
    Vector<rhi::HIndexBuffer> indexBuffers;

    uint8 currentBufferIndex;
    uint32 vertexStride;
    uint32 verticesLimit;
    uint32 indicesLimit;
    uint32 vertexLayoutID;

#if BATCHING_DEBUG
    int8 defaultVboFrameLife;
    int8 currentFrame;
    Vector<int8> vboFrameLifes;
#endif
};

rhi::HVertexBuffer VboPool::GetVertexBuffer()
{
    return currentVertexBuffer;
}

rhi::HIndexBuffer VboPool::GetIndexBuffer()
{
    return currentIndexBuffer;
}

inline uint32 VboPool::GetVerticesLimit() const
{
    return verticesLimit;
}

inline uint32 VboPool::GetIndicesLimit() const
{
    return indicesLimit;
}

inline uint32 VboPool::GetVertexStride() const
{
    return vertexStride;
}

inline uint32 VboPool::GetVertexLayoutID() const
{
    return vertexLayoutID;
}

VboPool::VboPool(uint32 verticesCount, uint32 format, uint32 indicesCount, uint8 buffersCount)
    : verticesLimit(0)
    , indicesLimit(0)
#if BATCHING_DEBUG
    , defaultVboFrameLife(0)
    , currentFrame(0)
    , vboFrameLifes()
#endif
{
    //TODO: create vertex layout from incoming format
    vertexStride = GetVertexSize(format);

    rhi::VertexLayout vLayout;
    vLayout.AddElement(rhi::VS_POSITION, 0, rhi::VDT_FLOAT, 3);
    vLayout.AddElement(rhi::VS_TEXCOORD, 0, rhi::VDT_FLOAT, 2);
    vLayout.AddElement(rhi::VS_COLOR, 0, rhi::VDT_UINT8N, 4);
    vertexLayoutID = rhi::VertexLayout::UniqueId(vLayout);

    HardReset(verticesCount, indicesCount, buffersCount);
}

VboPool::~VboPool()
{
    ReleaseBuffers();
}

void VboPool::ReleaseBuffers()
{
    for(auto vb : vertexBuffers)
    {
        rhi::DeleteVertexBuffer(vb);
    }
    vertexBuffers.clear();

    for (auto ib : indexBuffers)
    {
        rhi::DeleteIndexBuffer(ib);
    }
    indexBuffers.clear();
}

void VboPool::HardReset(uint32 verticesCount, uint32 indicesCount, uint8 buffersCount)
{
    if(verticesLimit == verticesCount && indicesLimit == indicesCount && vertexBuffers.size() == buffersCount)
    {
        return;
    }
    // Destroy exist buffers
    ReleaseBuffers();
    // Create new buffers
    verticesLimit = verticesCount;
    indicesLimit = indicesCount;
    for (uint8 i = 0; i < buffersCount; ++i)
    {
        rhi::HVertexBuffer vBuffer = rhi::CreateVertexBuffer(verticesCount * vertexStride);
        rhi::HIndexBuffer iBuffer = rhi::CreateIndexBuffer(indicesCount * sizeof(uint16));

        vertexBuffers.push_back(vBuffer);
        indexBuffers.push_back(iBuffer);
    }
    // Set current buffer
    currentBufferIndex = 0;
    currentVertexBuffer = vertexBuffers[currentBufferIndex];
    currentIndexBuffer = indexBuffers[currentBufferIndex];
}
    
void VboPool::Next()
{
    currentBufferIndex = (currentBufferIndex + 1) % vertexBuffers.size();
    currentVertexBuffer = vertexBuffers[currentBufferIndex];
    currentIndexBuffer = indexBuffers[currentBufferIndex];
}

#if BATCHING_DEBUG
void VboPool::EnableBuffersLoopingWarning(int8 framesCount)
{
    if(defaultVboFrameLife != framesCount)
    {
        vboFrameLifes.clear();
        defaultVboFrameLife = framesCount;
        if(defaultVboFrameLife > 0)
        {
            vboFrameLifes.resize(dataObjects.size(), -1);
        }
    }
}

void VboPool::CheckForBufferReuse()
{
    if(defaultVboFrameLife > 0)
    {
        if(vboFrameLifes[currentDataObjectIndex] > 0)
        {
            // Check if we try reuse buffer which can be used in triple buffer (or n-frames per buffer)
            Logger::Warning("RenderSystem2D: Reuse buffer (%d) which is still being processed", currentDataObjectIndex);
        }
        vboFrameLifes[currentDataObjectIndex] = defaultVboFrameLife;
    }
}

void VboPool::NextFrame()
{
    if(defaultVboFrameLife > 0)
    {
        currentFrame = ++currentFrame % defaultVboFrameLife;
        std::for_each(vboFrameLifes.begin(), vboFrameLifes.end(), [](int8 &val){
            if(val > 0)
            {
                val--;
            }
        });
    }
}
#endif //BATCHING_DEBUG

RenderSystem2D::RenderSystem2D() 
    : vboTemp(NULL)
    , iboTemp(NULL)
    , spriteClipping(true)
    , pool(NULL)
    , indexIndex(0)
    , vertexIndex(0)
    , spriteIndexCount(0)
    , spriteVertexCount(0)
    , spritePrimitiveToDraw(PRIMITIVETYPE_TRIANGLELIST)
    , prevFrameErrorsFlags(NO_ERRORS)
    , currFrameErrorsFlags(NO_ERRORS)
    , highlightControlsVerticesLimit(0)
    , renderTargetWidth(0)
    , renderTargetHeight(0)
{
}

void RenderSystem2D::Init()
{
    if(!pool)
    {
        pool = new VboPool(MAX_VERTICES, VBO_FORMAT, MAX_INDECES, VBO_POOL_SIZE);
#if BATCHING_DEBUG
        pool->EnableBuffersLoopingWarning(VBO_USING_FRAME_LIFE);
#endif
        vboTemp.resize(MAX_VERTICES * GetVertexSize(VBO_FORMAT));
        iboTemp.resize(MAX_INDECES);
        // Render data object for drawing big batches
    }

    DEFAULT_2D_COLOR_MATERIAL = new NMaterial();
    DEFAULT_2D_COLOR_MATERIAL->SetFXName(FastName("~res:/Materials/2d.Color.material"));
    DEFAULT_2D_COLOR_MATERIAL->PreBuildMaterial(FastName("2d"));

    DEFAULT_2D_TEXTURE_MATERIAL = new NMaterial();
    DEFAULT_2D_TEXTURE_MATERIAL->SetFXName(FastName("~res:/Materials/2d.Textured.material"));
    DEFAULT_2D_TEXTURE_MATERIAL->PreBuildMaterial(FastName("2d"));

    DEFAULT_2D_TEXTURE_ALPHA8_MATERIAL = new NMaterial();
    DEFAULT_2D_TEXTURE_ALPHA8_MATERIAL->SetFXName(FastName("~res:/Materials/2d.Textured.material"));
    DEFAULT_2D_TEXTURE_ALPHA8_MATERIAL->AddFlag(FastName("IMAGE_A8"), 1);
    DEFAULT_2D_TEXTURE_ALPHA8_MATERIAL->PreBuildMaterial(FastName("2d"));

}

RenderSystem2D::~RenderSystem2D()
{
    SafeDelete(pool);
    
    SafeRelease(DEFAULT_2D_COLOR_MATERIAL);
    SafeRelease(DEFAULT_2D_TEXTURE_MATERIAL);
    SafeRelease(DEFAULT_2D_TEXTURE_ALPHA8_MATERIAL);
}

void RenderSystem2D::BeginFrame()
{
    currentClip.x = 0;
    currentClip.y = 0;
    currentClip.dx = -1;
    currentClip.dy = -1;

    Setup2DMatrices();

    defaultSpriteDrawState.Reset();
    defaultSpriteDrawState.material = DEFAULT_2D_COLOR_MATERIAL;    

    batches.clear();
    batches.reserve(RESERVED_BATCHES);
    currentBatch.Reset();
    vertexIndex = 0;
    indexIndex = 0;
#if BATCHING_DEBUG
    pool->NextFrame();
#endif

    rhi::RenderPassConfig renderPass2DConfig;
    renderPass2DConfig.priority = PRIORITY_MAIN_2D;
    renderPass2DConfig.colorBuffer[0].loadAction = rhi::LOADACTION_NONE;
    pass2DHandle = rhi::AllocateRenderPass(renderPass2DConfig, 1, &packetList2DHandle);

    rhi::BeginRenderPass(pass2DHandle);
    rhi::BeginPacketList(packetList2DHandle);
}

void RenderSystem2D::EndFrame()
{
    Flush();
    prevFrameErrorsFlags = currFrameErrorsFlags;
    currFrameErrorsFlags = 0;

    rhi::EndPacketList(packetList2DHandle);
    rhi::EndRenderPass(pass2DHandle);
}

void RenderSystem2D::BeginRenderTargetPass(Texture * target)
{
    DVASSERT(!renderTargetWidth);
    DVASSERT(target);
    DVASSERT(target->GetWidth() && target->GetHeight())

    Flush();

    //RHI_COMPLETE: set viewport
    rhi::RenderPassConfig renderTargetPassConfig;
    renderTargetPassConfig.priority = PRIORITY_SERVICE_2D;
    passTargetHandle = rhi::AllocateRenderPass(renderTargetPassConfig, 1, &packetListTargetHandle);

    rhi::BeginRenderPass(passTargetHandle);
    rhi::BeginPacketList(packetListTargetHandle);

    renderTargetWidth = target->GetWidth();
    renderTargetHeight = target->GetHeight();
}

void RenderSystem2D::EndRenderTargetPass()
{
    Flush();

    rhi::EndPacketList(packetListTargetHandle);
    rhi::EndRenderPass(passTargetHandle);

    renderTargetWidth = 0;
    renderTargetHeight = 0;
}

void RenderSystem2D::Setup2DProjection()
{
    if (renderTargetWidth)
    {
        projMatrix.glOrtho(0.0f, (float32)renderTargetWidth,
                           0.0f, (float32)renderTargetHeight,
                          -1.0f, 1.0f);
    }
    else
    {
        projMatrix.glOrtho(0.0f, (float32)Renderer::GetFramebufferWidth(), (float32)Renderer::GetFramebufferHeight(), 0.0f, -1.0f, 1.0f);
    }

    projMatrix = virtualToPhysicalMatrix * projMatrix;
    Renderer::GetDynamicBindings().SetDynamicParam(DynamicBindings::PARAM_PROJ, &projMatrix, DynamicBindings::UPDATE_SEMANTIC_ALWAYS);

    projMatrix.glOrtho(0.0f, (float32)1024, (float32)768, 0.0f, -1.0f, 1.0f);
    projMatrix = virtualToPhysicalMatrix * projMatrix;
    Renderer::GetDynamicBindings().SetDynamicParam(DynamicBindings::PARAM_PROJ, &projMatrix, DynamicBindings::UPDATE_SEMANTIC_ALWAYS);
}

void RenderSystem2D::Setup2DMatrices()
{
    Renderer::GetDynamicBindings().SetDynamicParam(DynamicBindings::PARAM_WORLD, &Matrix4::IDENTITY, (pointer_size)&Matrix4::IDENTITY);
    Renderer::GetDynamicBindings().SetDynamicParam(DynamicBindings::PARAM_VIEW, &Matrix4::IDENTITY, (pointer_size)&Matrix4::IDENTITY);
    Setup2DProjection();
}

void RenderSystem2D::ScreenSizeChanged()
{
    Matrix4 glTranslate, glScale;

    Vector2 scale = VirtualCoordinatesSystem::Instance()->ConvertVirtualToPhysical(Vector2(1.f, 1.f));
    Vector2 realDrawOffset = VirtualCoordinatesSystem::Instance()->GetPhysicalDrawOffset();

    glTranslate.glTranslate(realDrawOffset.x, realDrawOffset.y, 0.0f);
    glScale.glScale(scale.x, scale.y, 1.0f);

    virtualToPhysicalMatrix = glScale * glTranslate;

    Setup2DProjection();
}

void RenderSystem2D::SetClip(const Rect &rect)
{
    if ((currentClip == rect) || (currentClip.dx < 0 && rect.dx < 0) || (currentClip.dy < 0 && rect.dy < 0))
    {
        return;
    }
    currentClip = rect;
}

void RenderSystem2D::RemoveClip()
{
    SetClip(Rect(0.f, 0.f, -1.f, -1.f));
}

void RenderSystem2D::IntersectClipRect(const Rect &rect)
{
    if (currentClip.dx < 0 || currentClip.dy < 0)
    {
        SetClip(rect);
    }
    else
    {
        SetClip(currentClip.Intersection(rect));
    }
}

void RenderSystem2D::PushClip()
{
    clipStack.push(currentClip);
}

void RenderSystem2D::PopClip()
{
    if(clipStack.empty())
    {
        Rect r(0, 0, -1, -1);
        SetClip(r);
    }
    else
    {
        Rect r = clipStack.top();
        SetClip(r);
    }
    clipStack.pop();
}

Rect RenderSystem2D::TransformClipRect(const Rect & rect)
{
    DynamicBindings & dynamicParams = Renderer::GetDynamicBindings();
    Matrix4 transformMx = dynamicParams.GetDynamicParamMatrix(DynamicBindings::PARAM_VIEW) * virtualToPhysicalMatrix;

    Vector3 clipTopLeftCorner(currentClip.x, currentClip.y, 0.f);
    Vector3 clipBottomRightCorner(currentClip.x + currentClip.dx, currentClip.y + currentClip.dy, 0.f);

    clipTopLeftCorner = clipTopLeftCorner * transformMx;
    clipBottomRightCorner = clipBottomRightCorner * transformMx;

    Rect transformedClip(Vector2(clipTopLeftCorner.data), Vector2((clipBottomRightCorner - clipTopLeftCorner).data));

    return transformedClip;
}

void RenderSystem2D::SetSpriteClipping(bool clipping)
{
    spriteClipping = clipping;
}

bool RenderSystem2D::IsPreparedSpriteOnScreen(Sprite::DrawState * drawState)
{
#if RHI_COMPLETE
    if (RenderManager::Instance()->GetRenderTarget() != nullptr)
        return true;
#endif RHI_COMPLETE

    Rect clipRect = currentClip;
    if (clipRect.dx == -1)
    {
        clipRect.dx = (float32)VirtualCoordinatesSystem::Instance()->GetVirtualScreenSize().dx;
    }
    if (clipRect.dy == -1)
    {
        clipRect.dy = (float32)VirtualCoordinatesSystem::Instance()->GetVirtualScreenSize().dy;
    }

    float32 left = Min(Min(spriteTempVertices[0], spriteTempVertices[2]), Min(spriteTempVertices[4], spriteTempVertices[6]));
    float32 right = Max(Max(spriteTempVertices[0], spriteTempVertices[2]), Max(spriteTempVertices[4], spriteTempVertices[6]));
    float32 top = Min(Min(spriteTempVertices[1], spriteTempVertices[3]), Min(spriteTempVertices[5], spriteTempVertices[7]));
    float32 bottom = Max(Max(spriteTempVertices[1], spriteTempVertices[3]), Max(spriteTempVertices[5], spriteTempVertices[7]));

    const Rect spriteRect(left, top, right - left, bottom - top);
    return clipRect.RectIntersects(spriteRect);
}
  
void RenderSystem2D::Flush()
{
    /*
    Called on each EndFrame, particle draw, screen transitions preparing, screen borders draw
    */

    if (vertexIndex == 0 && indexIndex == 0)
    {
        return;
    }
    
    if (currentBatch.count > 0)
    {
        batches.push_back(currentBatch);
        currentBatch.Reset();
    }

    DVASSERT(!batches.empty());

    spritePrimitiveToDraw = PRIMITIVETYPE_TRIANGLELIST;

#if BATCHING_DEBUG
    pool->CheckForBufferReuse();
#endif

    rhi::UpdateVertexBuffer(pool->GetVertexBuffer(), &vboTemp.front(), 0, vertexIndex * pool->GetVertexStride());
    rhi::UpdateIndexBuffer(pool->GetIndexBuffer(), &iboTemp.front(), 0, indexIndex * sizeof(iboTemp[0]));

    Vector<RenderBatch2D>::iterator it = batches.begin();
    Vector<RenderBatch2D>::iterator eit = batches.end();
    for (; it != eit; ++it)
    {
        const RenderBatch2D& batch = *it;

        rhi::Packet packet;
        packet.vertexStreamCount = 1;
        packet.vertexStream[0] = pool->GetVertexBuffer();
        packet.indexBuffer = pool->GetIndexBuffer();
        packet.startIndex = batch.indexOffset;
        packet.primitiveType = batch.primitiveType;
        packet.vertexLayoutUID = pool->GetVertexLayoutID();
        packet.vertexCount = vertexIndex;

#if RHI_COMPLETE
        packet.clip = clip;
        Rect2i clip = Rect2i((int32)batch.transformedClipRect.x,
            (int32)batch.transformedClipRect.y,
            (int32)ceilf(batch.transformedClipRect.dx),
            (int32)ceilf(batch.transformedClipRect.dy));

#endif // RHI_COMPLETE

        switch (packet.primitiveType)
        {
        case rhi::PRIMITIVE_LINELIST:
            packet.primitiveCount = batch.count / 2;
            break;
        case rhi::PRIMITIVE_TRIANGLEFAN:
            DVASSERT(batch.count > 2);
            packet.primitiveCount = batch.count - 2;
            break;
        case rhi::PRIMITIVE_TRIANGLELIST:
            packet.primitiveCount = batch.count / 3;
            break;
        }

        batch.material->BindParams(packet);
        packet.fragmentTextureSet = batch.textureSetHandle;

        rhi::AddPacket(renderTargetWidth ? packetListTargetHandle : packetList2DHandle, packet);
    }

    batches.clear();

    vertexIndex = 0;
    indexIndex = 0;
    pool->Next();

}

void RenderSystem2D::HardResetBatchingBuffers(uint32 verticesCount, uint32 indicesCount, uint8 buffersCount)
{
    vboTemp.resize(verticesCount * GetVertexSize(VBO_FORMAT));
    iboTemp.resize(indicesCount);
    pool->HardReset(verticesCount, indicesCount, buffersCount);
}

void RenderSystem2D::PushBatch(NMaterial * material, rhi::HTextureSet texture, Rect const& clip,
    uint32 vertexCount, const float32* vertexPointer, const float32* texCoordPointer,
    uint32 indexCount, const uint16* indexPointer,
    const Color& color, const rhi::PrimitiveType primitiveType)
{
    if ((vertexIndex + vertexCount > pool->GetVerticesLimit()) || (indexIndex + indexCount > pool->GetIndicesLimit()))
    {
        // Buffer overflow. Switch to next VBO.
        Flush();

        // TODO: Make draw for big buffers (bigger than buffers in pool)
#if RHI_COMPLETE
        // Draw immediately if batch is too big to buffer
        if(vertexCount > pool->GetVerticesLimit() || indexCount > pool->GetIndicesLimit())
        {
            if( ((prevFrameErrorsFlags & BUFFER_OVERFLOW_ERROR) != BUFFER_OVERFLOW_ERROR) )
            {
                Logger::Warning("PushBatch: Vertices overhead (%d of %d)! Direct draw.", vertexCount, pool->GetVerticesLimit());
            }
            currFrameErrorsFlags |= BUFFER_OVERFLOW_ERROR;

            spriteRenderObject->SetStream(EVF_VERTEX, TYPE_FLOAT, 2, 0, vertexPointer);
            spriteRenderObject->SetStream(EVF_TEXCOORD0, TYPE_FLOAT, 2, 0, texCoordPointer);

            UpdateClip();

            if(RenderManager::Instance()->GetOptions()->IsOptionEnabled(RenderOptions::HIGHLIGHT_HARD_CONTROLS))
            {
                // Highlight too big controls with red color
                static Color red = Color(1.f, 0.f, 0.f, 1.f);
                RenderManager::Instance()->SetColor(red);
            }
            else
            {
                RenderManager::Instance()->SetColor(color);
            }

            RenderManager::Instance()->SetRenderState(state);
            RenderManager::Instance()->SetTextureState(texture);
            RenderManager::Instance()->SetRenderData(spriteRenderObject);
            RenderManager::Instance()->SetRenderEffect(shader);

            void* indeces = reinterpret_cast<void*>(const_cast<uint16*>(indexPointer));
            RenderManager::Instance()->DrawElements(PRIMITIVETYPE_TRIANGLELIST, indexCount, EIF_16, indeces);
            return;
        }
#endif //RHI_COMPLETE

    }

    Color useColor = color;
    if(highlightControlsVerticesLimit > 0
            && vertexCount > highlightControlsVerticesLimit
            && Renderer::GetOptions()->IsOptionEnabled(RenderOptions::HIGHLIGHT_HARD_CONTROLS))
    {
        // Highlight too big controls with magenta color
        static Color magenta = Color(1.f, 0.f, 1.f, 1.f);
        useColor = magenta;
    }

    uint32 vi = vertexIndex * 6;
    uint32 ii = indexIndex;
    for (uint32 i = 0; i < vertexCount; ++i)
    {
        vboTemp[vi++] = vertexPointer[i * 2];
        vboTemp[vi++] = vertexPointer[i * 2 + 1];
        vboTemp[vi++] = 0.f; // axe Z, empty but need for EVF_VERTEX format
        vboTemp[vi++] = texCoordPointer ? texCoordPointer[i * 2] : 0.f;
        vboTemp[vi++] = texCoordPointer ? texCoordPointer[i * 2 + 1] : 0.f;
        *(uint32*)(&vboTemp[vi++]) = useColor.GetRGBA();
    }
    for (uint32 i = 0; i < indexCount; ++i)
    {
        iboTemp[ii++] = vertexIndex + indexPointer[i];
    }
    
    if (currentBatch.textureSetHandle != texture
        || currentBatch.material != material
        || currentBatch.clipRect != clip
        || currentBatch.primitiveType != primitiveType)
    {
        if (currentBatch.count > 0)
        {
            batches.push_back(currentBatch);
        }
        currentBatch.Reset();

        currentBatch.primitiveType = primitiveType;
        currentBatch.textureSetHandle = texture;
        currentBatch.material = material;
        currentBatch.clipRect = clip;
        currentBatch.transformedClipRect = TransformClipRect(clip);
        currentBatch.indexOffset = indexIndex;
    }

    currentBatch.count += indexCount;
    indexIndex += indexCount;
    vertexIndex += vertexCount;
}


void RenderSystem2D::Draw(Sprite * sprite, Sprite::DrawState * drawState, const Color& color)
{
    if(!Renderer::GetOptions()->IsOptionEnabled(RenderOptions::SPRITE_DRAW))
	{
		return;
	}

    static uint16 spriteIndeces[] = { 0, 1, 2, 1, 3, 2 };
    Vector<uint16> spriteClippedIndecex;

    Sprite::DrawState * state = drawState;
    if (!state)
    {
        state = &defaultSpriteDrawState;
    }

	float32 scaleX = 1.0f;
    float32 scaleY = 1.0f;

    sprite->flags = 0;
    if (state->flags != 0)
    {
        sprite->flags |= Sprite::EST_MODIFICATION;
    }

    if(state->scale.x != 1.f || state->scale.y != 1.f)
    {
        sprite->flags |= Sprite::EST_SCALE;
        scaleX = state->scale.x;
        scaleY = state->scale.y;
    }

    if(state->angle != 0.f) sprite->flags |= Sprite::EST_ROTATE;

    int32 frame = Clamp(state->frame, 0, sprite->frameCount - 1);

    float32 x = state->position.x - state->pivotPoint.x * state->scale.x;
    float32 y = state->position.y - state->pivotPoint.y * state->scale.y;

    float32 **frameVertices = sprite->frameVertices;
    float32 **rectsAndOffsets = sprite->rectsAndOffsets;
    Vector2 spriteSize = sprite->size;

    if(sprite->flags & Sprite::EST_MODIFICATION)
    {
        if((state->flags & (ESM_HFLIP | ESM_VFLIP)) == (ESM_HFLIP | ESM_VFLIP))
        {//HFLIP|VFLIP
            if(sprite->flags & Sprite::EST_SCALE)
            {//SCALE
                x += (spriteSize.dx - rectsAndOffsets[frame][2] - rectsAndOffsets[frame][4] * 2) * scaleX;
                y += (spriteSize.dy - rectsAndOffsets[frame][3] - rectsAndOffsets[frame][5] * 2) * scaleY;
                if(!state || !state->usePerPixelAccuracy || (sprite->flags & Sprite::EST_ROTATE))
                {
                    spriteTempVertices[2] = spriteTempVertices[6] = frameVertices[frame][0] * scaleX + x;//x2 do not change this sequence. This is because of the cache reason
                    spriteTempVertices[1] = spriteTempVertices[3] = frameVertices[frame][5] * scaleY + y;//y1
                    spriteTempVertices[0] = spriteTempVertices[4] = frameVertices[frame][2] * scaleX + x;//x1
                    spriteTempVertices[5] = spriteTempVertices[7] = frameVertices[frame][1] * scaleY + y;//y2
                }
                else
                {
                    spriteTempVertices[2] = spriteTempVertices[6] = VirtualCoordinatesSystem::Instance()->AlignVirtualToPhysicalX(frameVertices[frame][0] * scaleX + x);//x2
                    spriteTempVertices[5] = spriteTempVertices[7] = VirtualCoordinatesSystem::Instance()->AlignVirtualToPhysicalY(frameVertices[frame][1] * scaleY + y);//y2
                    spriteTempVertices[0] = spriteTempVertices[4] = (frameVertices[frame][2] - frameVertices[frame][0]) * scaleX + spriteTempVertices[2];//x1
                    spriteTempVertices[1] = spriteTempVertices[3] = (frameVertices[frame][5] - frameVertices[frame][1]) * scaleY + spriteTempVertices[5];//y1
                }
            }
            else
            {//NOT SCALE
                x += (spriteSize.dx - rectsAndOffsets[frame][2] - rectsAndOffsets[frame][4] * 2);
                y += (spriteSize.dy - rectsAndOffsets[frame][3] - rectsAndOffsets[frame][5] * 2);
                if(!state || !state->usePerPixelAccuracy || (sprite->flags & Sprite::EST_ROTATE))
                {
                    spriteTempVertices[2] = spriteTempVertices[6] = frameVertices[frame][0] + x;//x2 do not change this sequence. This is because of the cache reason
                    spriteTempVertices[1] = spriteTempVertices[3] = frameVertices[frame][5] + y;//y1
                    spriteTempVertices[0] = spriteTempVertices[4] = frameVertices[frame][2] + x;//x1
                    spriteTempVertices[5] = spriteTempVertices[7] = frameVertices[frame][1] + y;//y2
                }
                else
                {
                    spriteTempVertices[2] = spriteTempVertices[6] = VirtualCoordinatesSystem::Instance()->AlignVirtualToPhysicalX(frameVertices[frame][0] + x);//x2
                    spriteTempVertices[5] = spriteTempVertices[7] = VirtualCoordinatesSystem::Instance()->AlignVirtualToPhysicalY(frameVertices[frame][1] + y);//y2
                    spriteTempVertices[0] = spriteTempVertices[4] = (frameVertices[frame][2] - frameVertices[frame][0]) + spriteTempVertices[2];//x1
                    spriteTempVertices[1] = spriteTempVertices[3] = (frameVertices[frame][5] - frameVertices[frame][1]) + spriteTempVertices[5];//y1
                }
            }
        }
        else
        {
            if(state->flags & ESM_HFLIP)
            {//HFLIP
                if(sprite->flags & Sprite::EST_SCALE)
                {//SCALE
                    x += (spriteSize.dx - rectsAndOffsets[frame][2] - rectsAndOffsets[frame][4] * 2) * scaleX;
                    if(!state || !state->usePerPixelAccuracy || (sprite->flags & Sprite::EST_ROTATE))
                    {
                        spriteTempVertices[0] = spriteTempVertices[4] = frameVertices[frame][2] * scaleX + x;//x1
                        spriteTempVertices[5] = spriteTempVertices[7] = frameVertices[frame][5] * scaleY + y;//y2
                        spriteTempVertices[1] = spriteTempVertices[3] = frameVertices[frame][1] * scaleX + y;//y1 //WEIRD: maybe scaleY should be used?
                        spriteTempVertices[2] = spriteTempVertices[6] = frameVertices[frame][0] * scaleX + x;//x2
                    }
                    else
                    {
                        spriteTempVertices[2] = spriteTempVertices[6] = VirtualCoordinatesSystem::Instance()->AlignVirtualToPhysicalX(frameVertices[frame][0] * scaleX + x);//x2
                        spriteTempVertices[0] = spriteTempVertices[4] = (frameVertices[frame][2] - frameVertices[frame][0]) * scaleX + spriteTempVertices[2];//x1
                        spriteTempVertices[1] = spriteTempVertices[3] = VirtualCoordinatesSystem::Instance()->AlignVirtualToPhysicalY(frameVertices[frame][1] * scaleY + y);//y1
                        spriteTempVertices[5] = spriteTempVertices[7] = (frameVertices[frame][5] - frameVertices[frame][1]) * scaleY + spriteTempVertices[1];//y2
                    }
                }
                else
                {//NOT SCALE
                    x += (spriteSize.dx - rectsAndOffsets[frame][2] - rectsAndOffsets[frame][4] * 2);
                    if(!state || !state->usePerPixelAccuracy || (sprite->flags & Sprite::EST_ROTATE))
                    {
                        spriteTempVertices[0] = spriteTempVertices[4] = frameVertices[frame][2] + x;//x1
                        spriteTempVertices[5] = spriteTempVertices[7] = frameVertices[frame][5] + y;//y2
                        spriteTempVertices[1] = spriteTempVertices[3] = frameVertices[frame][1] + y;//y1
                        spriteTempVertices[2] = spriteTempVertices[6] = frameVertices[frame][0] + x;//x2
                    }
                    else
                    {
                        spriteTempVertices[2] = spriteTempVertices[6] = VirtualCoordinatesSystem::Instance()->AlignVirtualToPhysicalX(frameVertices[frame][0] + x);//x2
                        spriteTempVertices[0] = spriteTempVertices[4] = (frameVertices[frame][2] - frameVertices[frame][0]) + spriteTempVertices[2];//x1
                        spriteTempVertices[1] = spriteTempVertices[3] = VirtualCoordinatesSystem::Instance()->AlignVirtualToPhysicalY(frameVertices[frame][1] + y);//y1
                        spriteTempVertices[5] = spriteTempVertices[7] = (frameVertices[frame][5] - frameVertices[frame][1]) + spriteTempVertices[1];//y2
                    }
                }
            }
            else
            {//VFLIP
                if(sprite->flags & Sprite::EST_SCALE)
                {//SCALE
                    y += (spriteSize.dy - rectsAndOffsets[frame][3] - rectsAndOffsets[frame][5] * 2) * scaleY;
                    if(!state || !state->usePerPixelAccuracy || (sprite->flags & Sprite::EST_ROTATE))
                    {
                        spriteTempVertices[0] = spriteTempVertices[4] = frameVertices[frame][0] * scaleX + x;//x1
                        spriteTempVertices[5] = spriteTempVertices[7] = frameVertices[frame][1] * scaleY + y;//y2
                        spriteTempVertices[1] = spriteTempVertices[3] = frameVertices[frame][5] * scaleY + y;//y1
                        spriteTempVertices[2] = spriteTempVertices[6] = frameVertices[frame][2] * scaleX + x;//x2
                    }
                    else
                    {
                        spriteTempVertices[0] = spriteTempVertices[4] = VirtualCoordinatesSystem::Instance()->AlignVirtualToPhysicalX(frameVertices[frame][0] * scaleX + x);//x1
                        spriteTempVertices[5] = spriteTempVertices[7] = VirtualCoordinatesSystem::Instance()->AlignVirtualToPhysicalY(frameVertices[frame][1] * scaleY + y);//y2
                        spriteTempVertices[2] = spriteTempVertices[6] = (frameVertices[frame][2] - frameVertices[frame][0]) * scaleX + spriteTempVertices[0];//x2
                        spriteTempVertices[1] = spriteTempVertices[3] = (frameVertices[frame][5] - frameVertices[frame][1]) * scaleY + spriteTempVertices[5];//y1
                    }
                }
                else
                {//NOT SCALE
                    y += (spriteSize.dy - rectsAndOffsets[frame][3] - rectsAndOffsets[frame][5] * 2);
                    if(!state || !state->usePerPixelAccuracy || (sprite->flags & Sprite::EST_ROTATE))
                    {
                        spriteTempVertices[0] = spriteTempVertices[4] = frameVertices[frame][0] + x;//x1
                        spriteTempVertices[5] = spriteTempVertices[7] = frameVertices[frame][1] + y;//y2
                        spriteTempVertices[1] = spriteTempVertices[3] = frameVertices[frame][5] + y;//y1
                        spriteTempVertices[2] = spriteTempVertices[6] = frameVertices[frame][2] + x;//x2
                    }
                    else
                    {
                        spriteTempVertices[0] = spriteTempVertices[4] = VirtualCoordinatesSystem::Instance()->AlignVirtualToPhysicalX(frameVertices[frame][0] + x);//x1
                        spriteTempVertices[5] = spriteTempVertices[7] = VirtualCoordinatesSystem::Instance()->AlignVirtualToPhysicalY(frameVertices[frame][1] + y);//y2
                        spriteTempVertices[2] = spriteTempVertices[6] = (frameVertices[frame][2] - frameVertices[frame][0]) + spriteTempVertices[0];//x2
                        spriteTempVertices[1] = spriteTempVertices[3] = (frameVertices[frame][5] - frameVertices[frame][1]) + spriteTempVertices[5];//y1
                    }
                }
            }
        }

    }
    else
    {//NO MODIFERS
        if(sprite->flags & Sprite::EST_SCALE)
        {//SCALE
            if(!state || !state->usePerPixelAccuracy || (sprite->flags & Sprite::EST_ROTATE))
            {
                spriteTempVertices[0] = spriteTempVertices[4] = frameVertices[frame][0] * scaleX + x;//x1
                spriteTempVertices[5] = spriteTempVertices[7] = frameVertices[frame][5] * scaleY + y;//y2
                spriteTempVertices[1] = spriteTempVertices[3] = frameVertices[frame][1] * scaleY + y;//y1
                spriteTempVertices[2] = spriteTempVertices[6] = frameVertices[frame][2] * scaleX + x;//x2 do not change this sequence. This is because of the cache reason
            }
            else
            {
                spriteTempVertices[0] = spriteTempVertices[4] = VirtualCoordinatesSystem::Instance()->AlignVirtualToPhysicalX(frameVertices[frame][0] * scaleX + x);//x1
                spriteTempVertices[1] = spriteTempVertices[3] = VirtualCoordinatesSystem::Instance()->AlignVirtualToPhysicalY(frameVertices[frame][1] * scaleY + y);//y1
                spriteTempVertices[2] = spriteTempVertices[6] = (frameVertices[frame][2] - frameVertices[frame][0]) * scaleX + spriteTempVertices[0];//x2
                spriteTempVertices[5] = spriteTempVertices[7] = (frameVertices[frame][5] - frameVertices[frame][1]) * scaleY + spriteTempVertices[1];//y2
            }
        }
        else
        {//NOT SCALE
            if(!state || !state->usePerPixelAccuracy || (sprite->flags & Sprite::EST_ROTATE))
            {
                spriteTempVertices[0] = spriteTempVertices[4] = frameVertices[frame][0] + x;//x1
                spriteTempVertices[5] = spriteTempVertices[7] = frameVertices[frame][5] + y;//y2
                spriteTempVertices[1] = spriteTempVertices[3] = frameVertices[frame][1] + y;//y1
                spriteTempVertices[2] = spriteTempVertices[6] = frameVertices[frame][2] + x;//x2 do not change this sequence. This is because of the cache reason
            }
            else
            {
                spriteTempVertices[0] = spriteTempVertices[4] = VirtualCoordinatesSystem::Instance()->AlignVirtualToPhysicalX(frameVertices[frame][0] + x);//x1
                spriteTempVertices[1] = spriteTempVertices[3] = VirtualCoordinatesSystem::Instance()->AlignVirtualToPhysicalY(frameVertices[frame][1] + y);//y1
                spriteTempVertices[2] = spriteTempVertices[6] = (frameVertices[frame][2] - frameVertices[frame][0]) + spriteTempVertices[0];//x2
                spriteTempVertices[5] = spriteTempVertices[7] = (frameVertices[frame][5] - frameVertices[frame][1]) + spriteTempVertices[1];//y2
            }

        }

    }

    if(!sprite->clipPolygon)
    {
        if(sprite->flags & Sprite::EST_ROTATE)
        {
            //SLOW CODE
            //			glPushMatrix();
            //			glTranslatef(drawCoord.x, drawCoord.y, 0);
            //			glRotatef(RadToDeg(rotateAngle), 0.0f, 0.0f, 1.0f);
            //			glTranslatef(-drawCoord.x, -drawCoord.y, 0);
            //			RenderManager::Instance()->DrawArrays(PRIMITIVETYPE_TRIANGLESTRIP, 0, 4);
            //			glPopMatrix();
            
            // Optimized code
            float32 sinA = sinf(state->angle);
            float32 cosA = cosf(state->angle);
            for(int32 k = 0; k < 4; ++k)
            {
                float32 x = spriteTempVertices[(k << 1)] - state->position.x;
                float32 y = spriteTempVertices[(k << 1) + 1] - state->position.y;

                float32 nx = (x) * cosA  - (y) * sinA + state->position.x;
                float32 ny = (x) * sinA  + (y) * cosA + state->position.y;

                spriteTempVertices[(k << 1)] = nx;
                spriteTempVertices[(k << 1) + 1] = ny;
            }
        }

        if (spriteClipping && !IsPreparedSpriteOnScreen(state))
        {
            // Skip draw for sprites out of screen
            return;
        }

        spriteVertexCount = 4;
        spriteIndexCount = 6;
    }
    else
    {
        spriteClippedVertices.clear();
        spriteClippedVertices.reserve(sprite->clipPolygon->GetPointCount());
        spriteClippedTexCoords.clear();
        spriteClippedTexCoords.reserve(sprite->clipPolygon->GetPointCount());

        Texture * t = sprite->GetTexture(frame);

        Vector2 virtualTexSize = Vector2((float32)t->width, (float32)t->height);
        if (sprite->type == Sprite::SPRITE_FROM_FILE)
        {
            virtualTexSize = VirtualCoordinatesSystem::Instance()->ConvertResourceToVirtual(virtualTexSize, sprite->GetResourceSizeIndex());
        }
        else if (!sprite->textureInVirtualSpace)
        {
            virtualTexSize = VirtualCoordinatesSystem::Instance()->ConvertPhysicalToVirtual(virtualTexSize);
        }

        float32 adjWidth = 1.f / virtualTexSize.x;
        float32 adjHeight = 1.f / virtualTexSize.y;

        if (sprite->flags & Sprite::EST_SCALE)
        {
            for (int32 i = 0; i < sprite->clipPolygon->GetPointCount(); ++i)
            {
                const Vector2 &point = sprite->clipPolygon->GetPoints()[i];
                spriteClippedVertices.push_back(Vector2(point.x*scaleX + x, point.y*scaleY + y));
            }
        }
        else
        {
            Vector2 pos(x, y);
            for (int32 i = 0; i < sprite->clipPolygon->GetPointCount(); ++i)
            {
                const Vector2 &point = sprite->clipPolygon->GetPoints()[i];
                spriteClippedVertices.push_back(point + pos);
            }
        }

        for (int32 i = 0; i < sprite->clipPolygon->GetPointCount(); ++i)
        {
            const Vector2 &point = sprite->clipPolygon->GetPoints()[i];
            Vector2 texCoord((point.x - frameVertices[frame][0]) * adjWidth, (point.y - frameVertices[frame][1]) * adjHeight);
            spriteClippedTexCoords.push_back(Vector2(sprite->texCoords[frame][0] + texCoord.x, sprite->texCoords[frame][1] + texCoord.y));
        }

        spriteVertexCount = sprite->clipPolygon->GetPointCount();
        DVASSERT(spriteVertexCount >= 2);
        spriteIndexCount = (spriteVertexCount - 2) * 3;

        spriteClippedIndecex.clear();
        spriteClippedIndecex.reserve(spriteIndexCount);

        for (int32 i = 2; i < spriteVertexCount; ++i)
        {
            spriteClippedIndecex.push_back(0);
            spriteClippedIndecex.push_back(i - 1);
            spriteClippedIndecex.push_back(i);
        }
    }
    
    if(sprite->clipPolygon)
    {
        PushClip();
        Rect clipRect;
        if( sprite->flags & Sprite::EST_SCALE )
        {
            float32 x = state->position.x - state->pivotPoint.x * state->scale.x;
            float32 y = state->position.y - state->pivotPoint.y * state->scale.y;
            clipRect = Rect(  sprite->GetRectOffsetValueForFrame( state->frame, Sprite::X_OFFSET_TO_ACTIVE ) * state->scale.x + x
                            , sprite->GetRectOffsetValueForFrame( state->frame, Sprite::Y_OFFSET_TO_ACTIVE ) * state->scale.y + y
                            , sprite->GetRectOffsetValueForFrame( state->frame, Sprite::ACTIVE_WIDTH  ) * state->scale.x
                            , sprite->GetRectOffsetValueForFrame( state->frame, Sprite::ACTIVE_HEIGHT ) * state->scale.y );
        }
        else
        {
            float32 x = state->position.x - state->pivotPoint.x;
            float32 y = state->position.y - state->pivotPoint.y;
            clipRect = Rect(  sprite->GetRectOffsetValueForFrame( state->frame, Sprite::X_OFFSET_TO_ACTIVE ) + x
                            , sprite->GetRectOffsetValueForFrame( state->frame, Sprite::Y_OFFSET_TO_ACTIVE ) + y
                            , sprite->GetRectOffsetValueForFrame( state->frame, Sprite::ACTIVE_WIDTH )
                            , sprite->GetRectOffsetValueForFrame( state->frame, Sprite::ACTIVE_HEIGHT ) );
        }
        IntersectClipRect(clipRect);
    }

    if (!sprite->clipPolygon)
    {
        PushBatch(state->GetMaterial(), sprite->GetTextureHandle(state->frame), currentClip,
            spriteVertexCount, spriteTempVertices, sprite->texCoords[frame], 
            spriteIndexCount, spriteIndeces,
            color);
    }
    else
    {
        PushBatch(state->GetMaterial(), sprite->GetTextureHandle(state->frame), currentClip,
            spriteVertexCount, (float32*)&spriteClippedVertices.front(), (float32*)&spriteClippedTexCoords.front(),
            spriteIndexCount, spriteIndexCount ? &spriteClippedIndecex.front() : NULL,
            color);
    }

    if (sprite->clipPolygon)
    {
        PopClip();
    }

}

void RenderSystem2D::DrawStretched(Sprite * sprite, Sprite::DrawState * state, Vector2 stretchCapVector, UIControlBackground::eDrawType type, const UIGeometricData &gd, StretchDrawData ** pStreachData, const Color& color)
{
    if (!sprite)return;
	if (!Renderer::GetOptions()->IsOptionEnabled(RenderOptions::SPRITE_DRAW))
    {
        return;
    }

    int32 frame = Clamp(state->frame, 0, sprite->frameCount - 1);
    const Vector2 &size = gd.size;

    if (stretchCapVector.x < 0.0f || stretchCapVector.y < 0.0f ||
        size.x <= 0.0f || size.y <= 0.0f)
        return;

    Vector2 stretchCap(Min(size.x * 0.5f, stretchCapVector.x),
        Min(size.y * 0.5f, stretchCapVector.y));

    rhi::HTextureSet textureHandle = sprite->GetTextureHandle(frame);

    bool needGenerateData = false;
    StretchDrawData * stretchData = 0;
    if(pStreachData)
    {
        stretchData = *pStreachData;
        if (!stretchData)
        {
            stretchData = new StretchDrawData();
            needGenerateData = true;
            *pStreachData = stretchData;
        }
        else
        {
            needGenerateData |= sprite != stretchData->sprite;
            needGenerateData |= frame != stretchData->frame;
            needGenerateData |= gd.size != stretchData->size;
            needGenerateData |= type != stretchData->type;
            needGenerateData |= stretchCap != stretchData->stretchCap;
        }
    }
    else
    {
        stretchData = new StretchDrawData();
        needGenerateData = true;
    }
    
    StretchDrawData &sd = *stretchData;

    if (needGenerateData)
    {
        sd.sprite = sprite;
        sd.frame = frame;
        sd.size = gd.size;
        sd.type = type;
        sd.stretchCap = stretchCap;
        sd.GenerateStretchData();
    }

    Matrix3 transformMatr;
    gd.BuildTransformMatrix(transformMatr);

    if (needGenerateData || sd.transformMatr != transformMatr)
    {
        sd.transformMatr = transformMatr;
        sd.GenerateTransformData();
    }

    spriteVertexCount = (int32)sd.transformedVertices.size();
    spriteIndexCount = sd.GetVertexInTrianglesCount();

    //RHI_COMPLETE: in old code was predfined shader TEXTURE_MUL_FLAT_COLOR. Need take it into account in sprite materials
    PushBatch(state->GetMaterial(), textureHandle, currentClip,
        spriteVertexCount, (float32*)&sd.transformedVertices.front(), (float32*)&sd.texCoords.front(),
        spriteIndexCount, sd.indeces,
        color);
	
    if (!pStreachData)
    {
        SafeDelete(stretchData);
    }
}

void RenderSystem2D::DrawTiled(Sprite * sprite, Sprite::DrawState * state, const Vector2& stretchCapVector, const UIGeometricData &gd, TiledDrawData ** pTiledData, const Color& color)
{
    if (!sprite)return;
    if (!Renderer::GetOptions()->IsOptionEnabled(RenderOptions::SPRITE_DRAW))
    {
        return;
    }

	int32 frame = Clamp(state->frame, 0, sprite->frameCount - 1);

    const Vector2 &size = gd.size;

    if( stretchCapVector.x < 0.0f || stretchCapVector.y < 0.0f ||
        size.x <= 0.0f || size.y <= 0.0f )
        return;

    Vector2 stretchCap( Min( size.x, sprite->GetRectOffsetValueForFrame(frame, Sprite::ACTIVE_WIDTH) ),
                        Min( size.y, sprite->GetRectOffsetValueForFrame(frame, Sprite::ACTIVE_HEIGHT) ) );

    rhi::HTextureSet textureHandle = sprite->GetTextureHandle(frame);

    stretchCap.x = Min( stretchCap.x * 0.5f, stretchCapVector.x );
    stretchCap.y = Min( stretchCap.y * 0.5f, stretchCapVector.y );

    bool needGenerateData = false;

	TiledDrawData * tiledData = 0;
    if (pTiledData)
	{
		tiledData = *pTiledData;
        if (!tiledData)
        {
            tiledData = new TiledDrawData();
            needGenerateData = true;
            *pTiledData = tiledData;
        }
        else
        {
            needGenerateData |= stretchCap != tiledData->stretchCap;
            needGenerateData |= frame != tiledData->frame;
            needGenerateData |= sprite != tiledData->sprite;
            needGenerateData |= size != tiledData->size;
        }
	}
    else
    {
        tiledData = new TiledDrawData();
        needGenerateData = true;
    }
    
    TiledDrawData &td = *tiledData;

    if( needGenerateData )
    {
        td.stretchCap = stretchCap;
        td.size = size;
        td.frame = frame;
        td.sprite = sprite;
        td.GenerateTileData();
    }

    Matrix3 transformMatr;
    gd.BuildTransformMatrix( transformMatr );

    if( needGenerateData || td.transformMatr != transformMatr )
    {
        td.transformMatr = transformMatr;
        td.GenerateTransformData();
    }

    spriteVertexCount = (int32)td.transformedVertices.size();
    spriteIndexCount = (int32)td.indeces.size();

    //RHI_COMPLETE: in old code was predfined shader TEXTURE_MUL_FLAT_COLOR. Need take it into account in sprite materials
    PushBatch(state->GetMaterial(), textureHandle, currentClip,
        spriteVertexCount, (float32*)&td.transformedVertices.front(), (float32*)&td.texCoords.front(),
        spriteIndexCount, &td.indeces.front(),
        color);

    if (!pTiledData)
    {
        SafeDelete(tiledData);
    }
}

/* RenderSyste2D Draw Helper Functions */

void RenderSystem2D::FillRect(const Rect & rect, NMaterial *material, const Color& color)
{
    if (!Renderer::GetOptions()->IsOptionEnabled(RenderOptions::SPRITE_DRAW))
    {
        return;
    }

    static uint16 indices[6] = { 0, 1, 2, 1, 3, 2 };
    spriteTempVertices[0] = rect.x;
    spriteTempVertices[1] = rect.y;
    spriteTempVertices[2] = rect.x + rect.dx;
    spriteTempVertices[3] = rect.y;
    spriteTempVertices[4] = rect.x;
    spriteTempVertices[5] = rect.y + rect.dy;
    spriteTempVertices[6] = rect.x + rect.dx;
    spriteTempVertices[7] = rect.y + rect.dy;

    PushBatch(DEFAULT_2D_COLOR_MATERIAL, rhi::HTextureSet(), Rect(), 4, spriteTempVertices, nullptr, 6, indices, color);
}

void RenderSystem2D::DrawRect(const Rect & rect, NMaterial *material, const Color& color)
{
    static uint16 indices[8] = { 0, 1, 1, 2, 2, 3, 3, 0 };
    spriteTempVertices[0] = rect.x;
    spriteTempVertices[1] = rect.y;
    spriteTempVertices[2] = rect.x + rect.dx;
    spriteTempVertices[3] = rect.y;
    spriteTempVertices[4] = rect.x + rect.dx;
    spriteTempVertices[5] = rect.y + rect.dy;
    spriteTempVertices[6] = rect.x;
    spriteTempVertices[7] = rect.y + rect.dy;

    PushBatch(DEFAULT_2D_COLOR_MATERIAL, rhi::HTextureSet(), Rect(), 4, spriteTempVertices, nullptr, 8, indices, color, rhi::PRIMITIVE_LINELIST);
}

void RenderSystem2D::DrawGrid(const Rect & rect, const Vector2& gridSize, const Color& color, NMaterial *material)
{
    // TODO! review with Ivan/Victor whether it is not performance problem!
    Vector<float32> gridVertices;
    int32 verLinesCount = (int32)ceilf(rect.dx / gridSize.x);
    int32 horLinesCount = (int32)ceilf(rect.dy / gridSize.y);
    gridVertices.resize((horLinesCount + verLinesCount) * 4);

    float32 curPos = 0;
    int32 curVertexIndex = 0;
    for (int i = 0; i < horLinesCount; i++)
    {
        gridVertices[curVertexIndex++] = rect.x;
        gridVertices[curVertexIndex++] = rect.y + curPos;
        gridVertices[curVertexIndex++] = rect.x + rect.dx;
        gridVertices[curVertexIndex++] = rect.y + curPos;
    }

    curPos = 0.0f;
    for (int i = 0; i < verLinesCount; i++)
    {
        gridVertices[curVertexIndex++] = rect.x + curPos;
        gridVertices[curVertexIndex++] = rect.y;
        gridVertices[curVertexIndex++] = rect.x + curPos;
        gridVertices[curVertexIndex++] = rect.y + rect.dy;
    }

    Vector<uint16> indices;
    for (int i = 0; i < curVertexIndex; ++i)
    {
        indices.push_back(i);
    }

    PushBatch(DEFAULT_2D_COLOR_MATERIAL, rhi::HTextureSet(), Rect(), curVertexIndex / 2, gridVertices.data(), nullptr, curVertexIndex, indices.data(), color, rhi::PRIMITIVE_LINELIST);
}

void RenderSystem2D::DrawLine(const Vector2 &start, const Vector2 &end, NMaterial *materia, const Color& color)
{
    static uint16 indices[2] = { 0, 1 };
    spriteTempVertices[0] = start.x;
    spriteTempVertices[1] = start.y;
    spriteTempVertices[2] = end.x;
    spriteTempVertices[3] = end.y;
    PushBatch(DEFAULT_2D_COLOR_MATERIAL, rhi::HTextureSet(), Rect(), 2, spriteTempVertices, nullptr, 2, indices, color, rhi::PRIMITIVE_LINELIST);
}

void RenderSystem2D::DrawLine(const Vector2 &start, const Vector2 &end, float32 lineWidth, NMaterial *material, const Color& color)
{
    // TODO: Create list of lines for emulating line with width >1px
    static uint16 indices[2] = { 0, 1 };
    spriteTempVertices[0] = start.x;
    spriteTempVertices[1] = start.y;
    spriteTempVertices[2] = end.x;
    spriteTempVertices[3] = end.y;
    PushBatch(DEFAULT_2D_COLOR_MATERIAL, rhi::HTextureSet(), Rect(), 2, spriteTempVertices, nullptr, 2, indices, color, rhi::PRIMITIVE_LINELIST);
}

void RenderSystem2D::DrawLines(const Vector<float32>& linePoints, NMaterial *material, const Color& color)
{
    auto ptCount = linePoints.size() / 2;
    Vector<uint16> indices;
    for (auto i = 0U; i < ptCount - 1; ++i)
    {
        indices.push_back(i);
        indices.push_back(i + 1);
    }
    RenderSystem2D::Instance()->PushBatch(DEFAULT_2D_COLOR_MATERIAL, rhi::HTextureSet(), Rect(), ptCount, linePoints.data(), nullptr, 2, indices.data(), color, rhi::PRIMITIVE_LINELIST);
}

void RenderSystem2D::DrawCircle(const Vector2 & center, float32 radius, NMaterial *material, const Color& color)
{
    Polygon2 pts;
    float32 angle = Min(PI / 6.0f, SEGMENT_LENGTH / radius);// maximum angle 30 degrees
    int ptsCount = (int)(2 * PI / angle) + 1;

    pts.points.reserve(ptsCount);
    for (int k = 0; k < ptsCount; ++k)
    {
        float32 angle = ((float)k / (ptsCount - 1)) * 2 * PI;
        float32 sinA = sinf(angle);
        float32 cosA = cosf(angle);
        Vector2 pos = center - Vector2(sinA * radius, cosA * radius);

        pts.AddPoint(pos);
    }

    DrawPolygon(pts, false, material, color);
}

void RenderSystem2D::DrawPolygon(const Polygon2 & polygon, bool closed, NMaterial *material, const Color& color)
{
    auto ptCount = polygon.GetPointCount();
    if (ptCount >= 2)
    {
        Vector<uint16> indices;
        auto i = 0U;
        for (; i < ptCount - 1; ++i)
        {
            indices.push_back(i);
            indices.push_back(i + 1);
        }
        if (closed)
        {
            indices.push_back(i);
            indices.push_back(0);
        }
        auto pointsPtr = static_cast<const float32*>(static_cast<const void*>(polygon.GetPoints()));
        PushBatch(DEFAULT_2D_COLOR_MATERIAL, rhi::HTextureSet(), Rect(), ptCount, pointsPtr, nullptr, indices.size(), &indices[0], color, rhi::PRIMITIVE_LINELIST);
    }
}

void RenderSystem2D::FillPolygon(const Polygon2 & polygon, NMaterial *material, const Color& color)
{
    auto ptCount = polygon.GetPointCount();
    if (ptCount >= 3)
    {
        Vector<uint16> indices;
        for (auto i = 1U; i < ptCount - 1; ++i)
        {
            indices.push_back(0);
            indices.push_back(i);
            indices.push_back(i + 1);
        }
        auto pointsPtr = static_cast<const float32*>(static_cast<const void*>(polygon.GetPoints()));
        PushBatch(DEFAULT_2D_COLOR_MATERIAL, rhi::HTextureSet(), Rect(), ptCount, pointsPtr, nullptr, indices.size(), &indices[0], color);
    }
}

void RenderSystem2D::DrawPolygonTransformed(const Polygon2 & polygon, bool closed, const Matrix3 & transform, NMaterial *material, const Color& color)
{
    Polygon2 copyPoly = polygon;
    copyPoly.Transform(transform);
    DrawPolygon(copyPoly, closed, material, color);
}

void RenderSystem2D::DrawTexture(Texture * texture, NMaterial *material, const Rect & _dstRect /* = Rect(0.f, 0.f, -1.f, -1.f) */, const Rect & _srcRect /* = Rect(0.f, 0.f, -1.f, -1.f) */)
{
    if (!texture)
        return;

    Rect destRect(_dstRect);
#if RHI_COMPLETE
    if (destRect.dx < 0.f || destRect.dy < 0.f)
    {
        Size2i targetSize;
        Texture * currentRenderTarget = RenderManager::Instance()->GetRenderTarget();
        if (currentRenderTarget)
        {
            targetSize = Size2i(currentRenderTarget->GetWidth(), currentRenderTarget->GetHeight());
        }
        else
        {
            targetSize = RenderManager::Instance()->GetFramebufferSize();
        }
        destRect.dx = (float32)targetSize.dx;
        destRect.dy = (float32)targetSize.dy;
    }
#endif

    spriteTempVertices[0] = spriteTempVertices[4] = destRect.x;//x1
    spriteTempVertices[5] = spriteTempVertices[7] = destRect.y;//y2
    spriteTempVertices[1] = spriteTempVertices[3] = destRect.y + destRect.dy;//y1
    spriteTempVertices[2] = spriteTempVertices[6] = destRect.x + destRect.dx;//x2

    Vector2 textureSize = Vector2((float32)texture->GetWidth(), (float32)texture->GetHeight());

    Rect relativeSrcRect;
    relativeSrcRect.x = _srcRect.x / textureSize.dx;
    relativeSrcRect.y = _srcRect.y / textureSize.dy;
    relativeSrcRect.dx = (_srcRect.dx < 0.f) ? 1.f : _srcRect.dx / textureSize.dx;
    relativeSrcRect.dy = (_srcRect.dy < 0.f) ? 1.f : _srcRect.dy / textureSize.dy;

    float32 texCoords[8];
    texCoords[0] = texCoords[4] = relativeSrcRect.x;//x1
    texCoords[5] = texCoords[7] = relativeSrcRect.y;//y2
    texCoords[1] = texCoords[3] = relativeSrcRect.y + relativeSrcRect.dy;//y1
    texCoords[2] = texCoords[6] = relativeSrcRect.x + relativeSrcRect.dx;//x2

    rhi::HTextureSet htexture;
#if RHI_COMPLETE
    TextureStateData textureStateData;
    textureStateData.SetTexture(0, texture);
    UniqueHandle textureState = RenderManager::Instance()->CreateTextureState(textureStateData);
#endif

    static uint16 indices[6] = { 0, 1, 2, 1, 3, 2 };
    PushBatch(DEFAULT_2D_COLOR_MATERIAL, htexture, Rect(), 4, spriteTempVertices, texCoords, 6, indices, Color::White, rhi::PRIMITIVE_LINELIST);
}

/* TiledDrawData Implementation */

void TiledDrawData::GenerateTileData()
{
    Texture *texture = sprite->GetTexture(frame);

    Vector< Vector3 > cellsWidth;
    GenerateAxisData(size.x, sprite->GetRectOffsetValueForFrame(frame, Sprite::ACTIVE_WIDTH),
        VirtualCoordinatesSystem::Instance()->ConvertResourceToVirtualX((float32)texture->GetWidth(), sprite->GetResourceSizeIndex()), stretchCap.x, cellsWidth);

    Vector< Vector3 > cellsHeight;
    GenerateAxisData(size.y, sprite->GetRectOffsetValueForFrame(frame, Sprite::ACTIVE_HEIGHT),
        VirtualCoordinatesSystem::Instance()->ConvertResourceToVirtualY((float32)texture->GetHeight(), sprite->GetResourceSizeIndex()), stretchCap.y, cellsHeight);

    int32 vertexCount = (int32)(4 * cellsHeight.size() * cellsWidth.size());
    if (vertexCount >= std::numeric_limits<uint16>::max())
    {
        vertices.clear();
        transformedVertices.clear();
        texCoords.clear();
        Logger::Error("[TiledDrawData::GenerateTileData] tile background too big!");
        return;
    }
    vertices.resize(vertexCount);
    transformedVertices.resize(vertexCount);
    texCoords.resize(vertexCount);

    int32 indecesCount = (int32)(6 * cellsHeight.size() * cellsWidth.size());
    indeces.resize(indecesCount);

    int32 offsetIndex = 0;
    const float32 * textCoords = sprite->GetTextureCoordsForFrame(frame);
    Vector2 trasformOffset;
    const Vector2 tempTexCoordsPt(textCoords[0], textCoords[1]);
    for (uint32 row = 0; row < cellsHeight.size(); ++row)
    {
        Vector2 cellSize(0.0f, cellsHeight[row].x);
        Vector2 texCellSize(0.0f, cellsHeight[row].y);
        Vector2 texTrasformOffset(0.0f, cellsHeight[row].z);
        trasformOffset.x = 0.0f;

        for (uint32 column = 0; column < cellsWidth.size(); ++column, ++offsetIndex)
        {
            cellSize.x = cellsWidth[column].x;
            texCellSize.x = cellsWidth[column].y;
            texTrasformOffset.x = cellsWidth[column].z;

            int32 vertIndex = offsetIndex * 4;
            vertices[vertIndex + 0] = trasformOffset;
            vertices[vertIndex + 1] = trasformOffset + Vector2(cellSize.x, 0.0f);
            vertices[vertIndex + 2] = trasformOffset + Vector2(0.0f, cellSize.y);
            vertices[vertIndex + 3] = trasformOffset + cellSize;

            const Vector2 texel = tempTexCoordsPt + texTrasformOffset;
            texCoords[vertIndex + 0] = texel;
            texCoords[vertIndex + 1] = texel + Vector2(texCellSize.x, 0.0f);
            texCoords[vertIndex + 2] = texel + Vector2(0.0f, texCellSize.y);
            texCoords[vertIndex + 3] = texel + texCellSize;

            int32 indecesIndex = offsetIndex * 6;
            indeces[indecesIndex + 0] = vertIndex;
            indeces[indecesIndex + 1] = vertIndex + 1;
            indeces[indecesIndex + 2] = vertIndex + 2;

            indeces[indecesIndex + 3] = vertIndex + 1;
            indeces[indecesIndex + 4] = vertIndex + 3;
            indeces[indecesIndex + 5] = vertIndex + 2;

            trasformOffset.x += cellSize.x;
        }
        trasformOffset.y += cellSize.y;
    }
}

void TiledDrawData::GenerateAxisData( float32 size, float32 spriteSize, float32 textureSize, float32 stretchCap, Vector< Vector3 > &axisData )
{
    int32 gridSize = 0;

    float32 sideSize = stretchCap;
    float32 sideTexSize = sideSize / textureSize;

    float32 centerSize = spriteSize - sideSize * 2.0f;
    float32 centerTexSize = centerSize / textureSize;

    float32 partSize = 0.0f;

    if (centerSize > 0.0f)
    {
        gridSize = (int32)ceilf((size - sideSize * 2.0f) / centerSize);
        const float32 tileAreaSize = size - sideSize * 2.0f;
        partSize = tileAreaSize - floorf(tileAreaSize / centerSize) * centerSize;
    }

    if (sideSize > 0.0f)
        gridSize += 2;

    axisData.resize(gridSize);

    int32 beginOffset = 0;
    int32 endOffset = 0;
    if (sideSize > 0.0f)
    {
        axisData.front() = Vector3(sideSize, sideTexSize, 0.0f);
        axisData.back() = Vector3(sideSize, sideTexSize, sideTexSize + centerTexSize);
        beginOffset = 1;
        endOffset = 1;
    }

    if (partSize > 0.0f)
    {
        ++endOffset;
        const int32 index = gridSize - endOffset;
        axisData[index].x = partSize;
        axisData[index].y = partSize / textureSize;
        axisData[index].z = sideTexSize;
    }

    if (centerSize > 0.0f)
    {
        std::fill(axisData.begin() + beginOffset, axisData.begin() + gridSize - endOffset, Vector3(centerSize, centerTexSize, sideTexSize));
    }
}

void TiledDrawData::GenerateTransformData()
{
    const uint32 size = (uint32)vertices.size();
    for( uint32 index = 0; index < size; ++index )
    {
        transformedVertices[index] = vertices[index] * transformMatr;
    }
}

const uint16 StretchDrawData::indeces[18 * 3] = {
    0, 1, 4,
    1, 5, 4,
    1, 2, 5,
    2, 6, 5,
    2, 3, 6,
    3, 7, 6,

    4, 5, 8,
    5, 9, 8,
    5, 6, 9,
    6, 10, 9,
    6, 7, 10,
    7, 11, 10,

    8, 9, 12,
    9, 12, 13,
    9, 10, 13,
    10, 14, 13,
    10, 11, 14,
    11, 15, 14
};

uint32 StretchDrawData::GetVertexInTrianglesCount() const
{
    switch (type)
    {
    case UIControlBackground::DRAW_STRETCH_HORIZONTAL:
    case UIControlBackground::DRAW_STRETCH_VERTICAL:
        return 18;
    case UIControlBackground::DRAW_STRETCH_BOTH:
        return 18 * 3;
    default:
        DVASSERT(0);
        return 0;
    }
}

void StretchDrawData::GenerateTransformData()
{
    const uint32 size = (uint32)vertices.size();
    for (uint32 index = 0; index < size; ++index)
    {
        transformedVertices[index] = vertices[index] * transformMatr;
    }
}

void StretchDrawData::GenerateStretchData()
{
    const Vector2 sizeInTex(sprite->GetRectOffsetValueForFrame(frame, Sprite::ACTIVE_WIDTH), sprite->GetRectOffsetValueForFrame(frame, Sprite::ACTIVE_HEIGHT));
    const Vector2 offsetInTex(sprite->GetRectOffsetValueForFrame(frame, Sprite::X_OFFSET_TO_ACTIVE), sprite->GetRectOffsetValueForFrame(frame, Sprite::Y_OFFSET_TO_ACTIVE));
    const Vector2 &spriteSize = sprite->GetSize();

    const Vector2 xyLeftTopCap(offsetInTex - stretchCap);
    const Vector2 xyRightBottomCap(spriteSize - sizeInTex - offsetInTex - stretchCap);

    const Vector2 xyRealLeftTopCap(Max(0.0f, -xyLeftTopCap.x), Max(0.0f, -xyLeftTopCap.y));
    const Vector2 xyRealRightBottomCap(Max(0.0f, -xyRightBottomCap.x), Max(0.0f, -xyRightBottomCap.y));

    const Vector2 xyNegativeLeftTopCap(Max(0.0f, xyLeftTopCap.x), Max(0.0f, xyLeftTopCap.y));

    const Vector2 scaleFactor = (size - stretchCap*2.0f) / (spriteSize - stretchCap*2.0f);

    Vector2 xyPos;
    Vector2 xySize;

    if (UIControlBackground::DRAW_STRETCH_BOTH == type || UIControlBackground::DRAW_STRETCH_HORIZONTAL == type)
    {
        xySize.x = xyRealLeftTopCap.x + xyRealRightBottomCap.x + (sizeInTex.x - xyRealLeftTopCap.x - xyRealRightBottomCap.x) * scaleFactor.x;
        xyPos.x = stretchCap.x + xyNegativeLeftTopCap.x * scaleFactor.x - xyRealLeftTopCap.x;
    }
    else
    {
        xySize.x = sizeInTex.x;
        xyPos.x = offsetInTex.x + (size.x - spriteSize.x) * 0.5f;
    }

    if (UIControlBackground::DRAW_STRETCH_BOTH == type || UIControlBackground::DRAW_STRETCH_VERTICAL == type)
    {
        xySize.y = xyRealLeftTopCap.y + xyRealRightBottomCap.y + (sizeInTex.y - xyRealLeftTopCap.y - xyRealRightBottomCap.y) * scaleFactor.y;
        xyPos.y = stretchCap.y + xyNegativeLeftTopCap.y * scaleFactor.y - xyRealLeftTopCap.y;
    }
    else
    {
        xySize.y = sizeInTex.y;
        xyPos.y = offsetInTex.y + (size.y - spriteSize.y) * 0.5f;
    }

    const Texture* texture = sprite->GetTexture(frame);
    const Vector2 textureSize((float32)texture->GetWidth(), (float32)texture->GetHeight());

    const Vector2 uvPos(sprite->GetRectOffsetValueForFrame(frame, Sprite::X_POSITION_IN_TEXTURE) / textureSize.x,
        sprite->GetRectOffsetValueForFrame(frame, Sprite::Y_POSITION_IN_TEXTURE) / textureSize.y);

    VirtualCoordinatesSystem * vcs = VirtualCoordinatesSystem::Instance();

    const Vector2 uvSize = vcs->ConvertVirtualToResource(Vector2(sprite->GetRectOffsetValueForFrame(frame, Sprite::ACTIVE_WIDTH), sprite->GetRectOffsetValueForFrame(frame, Sprite::ACTIVE_HEIGHT)),
        sprite->GetResourceSizeIndex()
        ) / textureSize;
    const Vector2 uvLeftTopCap = vcs->ConvertVirtualToResource(xyRealLeftTopCap, sprite->GetResourceSizeIndex()) / textureSize;
    const Vector2 uvRightBottomCap = vcs->ConvertVirtualToResource(xyRealRightBottomCap, sprite->GetResourceSizeIndex()) / textureSize;

    switch (type)
    {
    case UIControlBackground::DRAW_STRETCH_HORIZONTAL:
    {
        vertices.resize(8);
        transformedVertices.resize(8);
        texCoords.resize(8);

        vertices[0] = Vector2(xyPos.x, xyPos.y);
        vertices[1] = Vector2(xyPos.x + xyRealLeftTopCap.x, xyPos.y);
        vertices[2] = Vector2(xyPos.x + xySize.x - xyRealRightBottomCap.x, xyPos.y);
        vertices[3] = Vector2(xyPos.x + xySize.x, xyPos.y);

        vertices[4] = Vector2(xyPos.x, xyPos.y + xySize.y);
        vertices[5] = Vector2(xyPos.x + xyRealLeftTopCap.x, xyPos.y + xySize.y);
        vertices[6] = Vector2(xyPos.x + xySize.x - xyRealRightBottomCap.x, xyPos.y + xySize.y);
        vertices[7] = Vector2(xyPos.x + xySize.x, xyPos.y + xySize.y);

        texCoords[0] = Vector2(uvPos.x, uvPos.y);
        texCoords[1] = Vector2(uvPos.x + uvLeftTopCap.x, uvPos.y);
        texCoords[2] = Vector2(uvPos.x + uvSize.x - uvRightBottomCap.x, uvPos.y);
        texCoords[3] = Vector2(uvPos.x + uvSize.x, uvPos.y);

        texCoords[4] = Vector2(uvPos.x, uvPos.y + uvSize.y);
        texCoords[5] = Vector2(uvPos.x + uvLeftTopCap.x, uvPos.y + uvSize.y);
        texCoords[6] = Vector2(uvPos.x + uvSize.x - uvRightBottomCap.x, uvPos.y + uvSize.y);
        texCoords[7] = Vector2(uvPos.x + uvSize.x, uvPos.y + uvSize.y);
    }
    break;
    case UIControlBackground::DRAW_STRETCH_VERTICAL:
    {
        vertices.resize(8);
        transformedVertices.resize(8);
        texCoords.resize(8);

        vertices[0] = Vector2(xyPos.x, xyPos.y);
        vertices[1] = Vector2(xyPos.x, xyPos.y + xyRealLeftTopCap.y);
        vertices[2] = Vector2(xyPos.x, xyPos.y + xySize.y - xyRealRightBottomCap.y);
        vertices[3] = Vector2(xyPos.x, xyPos.y + xySize.y);

        vertices[4] = Vector2(xyPos.x + xySize.x, xyPos.y);
        vertices[5] = Vector2(xyPos.x + xySize.x, xyPos.y + xyRealLeftTopCap.y);
        vertices[6] = Vector2(xyPos.x + xySize.x, xyPos.y + xySize.y - xyRealRightBottomCap.y);
        vertices[7] = Vector2(xyPos.x + xySize.x, xyPos.y + xySize.y);

        texCoords[0] = Vector2(uvPos.x, uvPos.y);
        texCoords[1] = Vector2(uvPos.x, uvPos.y + uvLeftTopCap.y);
        texCoords[2] = Vector2(uvPos.x, uvPos.y + uvSize.y - uvRightBottomCap.y);
        texCoords[3] = Vector2(uvPos.x, uvPos.y + uvSize.y);

        texCoords[4] = Vector2(uvPos.x + uvSize.x, uvPos.y);
        texCoords[5] = Vector2(uvPos.x + uvSize.x, uvPos.y + uvLeftTopCap.y);
        texCoords[6] = Vector2(uvPos.x + uvSize.x, uvPos.y + uvSize.y - uvRightBottomCap.y);
        texCoords[7] = Vector2(uvPos.x + uvSize.x, uvPos.y + uvSize.y);
    }
    break;
    case UIControlBackground::DRAW_STRETCH_BOTH:
    {
        vertices.resize(16);
        transformedVertices.resize(16);
        texCoords.resize(16);

        vertices[0] = Vector2(xyPos.x, xyPos.y);
        vertices[1] = Vector2(xyPos.x + xyRealLeftTopCap.x, xyPos.y);
        vertices[2] = Vector2(xyPos.x + xySize.x - xyRealRightBottomCap.x, xyPos.y);
        vertices[3] = Vector2(xyPos.x + xySize.x, xyPos.y);

        vertices[4] = Vector2(xyPos.x, xyPos.y + xyRealLeftTopCap.y);
        vertices[5] = Vector2(xyPos.x + xyRealLeftTopCap.x, xyPos.y + xyRealLeftTopCap.y);
        vertices[6] = Vector2(xyPos.x + xySize.x - xyRealRightBottomCap.x, xyPos.y + xyRealLeftTopCap.y);
        vertices[7] = Vector2(xyPos.x + xySize.x, xyPos.y + xyRealLeftTopCap.y);

        vertices[8] = Vector2(xyPos.x, xyPos.y + xySize.y - xyRealRightBottomCap.y);
        vertices[9] = Vector2(xyPos.x + xyRealLeftTopCap.x, xyPos.y + xySize.y - xyRealRightBottomCap.y);
        vertices[10] = Vector2(xyPos.x + xySize.x - xyRealRightBottomCap.x, xyPos.y + xySize.y - xyRealRightBottomCap.y);
        vertices[11] = Vector2(xyPos.x + xySize.x, xyPos.y + xySize.y - xyRealRightBottomCap.y);

        vertices[12] = Vector2(xyPos.x, xyPos.y + xySize.y);
        vertices[13] = Vector2(xyPos.x + xyRealLeftTopCap.x, xyPos.y + xySize.y);
        vertices[14] = Vector2(xyPos.x + xySize.x - xyRealRightBottomCap.x, xyPos.y + xySize.y);
        vertices[15] = Vector2(xyPos.x + xySize.x, xyPos.y + xySize.y);

        texCoords[0] = Vector2(uvPos.x, uvPos.y);
        texCoords[1] = Vector2(uvPos.x + uvLeftTopCap.x, uvPos.y);
        texCoords[2] = Vector2(uvPos.x + uvSize.x - uvRightBottomCap.x, uvPos.y);
        texCoords[3] = Vector2(uvPos.x + uvSize.x, uvPos.y);

        texCoords[4] = Vector2(uvPos.x, uvPos.y + uvLeftTopCap.y);
        texCoords[5] = Vector2(uvPos.x + uvLeftTopCap.x, uvPos.y + uvLeftTopCap.y);
        texCoords[6] = Vector2(uvPos.x + uvSize.x - uvRightBottomCap.x, uvPos.y + uvLeftTopCap.y);
        texCoords[7] = Vector2(uvPos.x + uvSize.x, uvPos.y + uvLeftTopCap.y);

        texCoords[8] = Vector2(uvPos.x, uvPos.y + uvSize.y - uvRightBottomCap.y);
        texCoords[9] = Vector2(uvPos.x + uvLeftTopCap.x, uvPos.y + uvSize.y - uvRightBottomCap.y);
        texCoords[10] = Vector2(uvPos.x + uvSize.x - uvRightBottomCap.x, uvPos.y + uvSize.y - uvRightBottomCap.y);
        texCoords[11] = Vector2(uvPos.x + uvSize.x, uvPos.y + uvSize.y - uvRightBottomCap.y);

        texCoords[12] = Vector2(uvPos.x, uvPos.y + uvSize.y);
        texCoords[13] = Vector2(uvPos.x + uvLeftTopCap.x, uvPos.y + uvSize.y);
        texCoords[14] = Vector2(uvPos.x + uvSize.x - uvRightBottomCap.x, uvPos.y + uvSize.y);
        texCoords[15] = Vector2(uvPos.x + uvSize.x, uvPos.y + uvSize.y);
    }
    break;
    }
}

};
