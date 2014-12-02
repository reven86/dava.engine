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
#include "Render/RenderManager.h"
#include <Render/RenderHelper.h>

#include "UI/UIControl.h"
#include "UI/UIControlBackground.h"

namespace DAVA
{

#if defined(__DAVAENGINE_IPHONE__) || defined(__DAVAENGINE_ANDROID__)
#define xGL_MAP_BUFF(x,y) glMapBufferOES(x,y)
#define xGL_UNMAP_BUFF(x) glUnmapBufferOES(x)
#define xGL_WRITE_FLAG GL_WRITE_ONLY_OES
#else
#define xGL_MAP_BUFF(x,y) glMapBuffer(x,y)
#define xGL_UNMAP_BUFF(x) glUnmapBuffer(x)
#define xGL_WRITE_FLAG GL_WRITE_ONLY
#endif

#define USE_MAPPING 0
#define USE_BATCHING true

VboPool::VboPool(uint32 size, uint8 count)
{
    vertexStride = sizeof(float32) * 8; //XYUVRGBA
    currentVertexBufferSize = size * vertexStride;
    currentIndexBufferSize = size * 2 * sizeof(uint16);
    for (int i = 0; i < count; ++i)
    {
        RenderDataObject* obj = new RenderDataObject();
#if USE_MAPPING
        obj->SetStream(EVF_VERTEX, TYPE_FLOAT, 2, vertexStride, 0);
        obj->SetStream(EVF_TEXCOORD0, TYPE_FLOAT, 2, vertexStride, 0);
        obj->SetStream(EVF_COLOR, TYPE_FLOAT, 4, vertexStride, 0);
        obj->BuildVertexBuffer(size, false);
        obj->SetIndices(EIF_16, 0, size * 2);
        obj->BuildIndexBuffer(false);
#endif
        dataObjects.push_back(obj);
    }
    currentDataObjectIndex = 0;
    currentDataObject = dataObjects[currentDataObjectIndex];
}

void VboPool::Next()
{
    currentDataObjectIndex = (currentDataObjectIndex + 1) % dataObjects.size();
    currentDataObject = dataObjects[currentDataObjectIndex];
}

void VboPool::SetVertexData(uint32 count, float32* data)
{
    currentDataObject->SetStream(EVF_VERTEX, TYPE_FLOAT, 2, vertexStride, data);
    currentDataObject->SetStream(EVF_TEXCOORD0, TYPE_FLOAT, 2, vertexStride, data + 2);
    currentDataObject->SetStream(EVF_COLOR, TYPE_FLOAT, 4, vertexStride, data + 4);
    currentDataObject->UpdateVertexBuffer(count);
    currentDataObject->SetStream(EVF_VERTEX, TYPE_FLOAT, 2, vertexStride, 0);
    currentDataObject->SetStream(EVF_TEXCOORD0, TYPE_FLOAT, 2, vertexStride, (void*)8);
    currentDataObject->SetStream(EVF_COLOR, TYPE_FLOAT, 4, vertexStride, (void*)16);
}

void VboPool::SetIndexData(uint32 count, uint8* data)
{
    currentDataObject->SetIndices(EIF_16, data, count);
    currentDataObject->UpdateIndexBuffer();
}

void VboPool::MapBuffers()
{
#if USE_MAPPING
    uint32 vbid = currentDataObject->GetVertexBufferID();
    uint32 ibid = currentDataObject->GetIndexBufferID();

    RenderManager::Instance()->HWglBindBuffer(GL_ARRAY_BUFFER, vbid);
    RenderManager::Instance()->HWglBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibid);

    RENDER_VERIFY(currentVertexBufferPointer = (float32*)xGL_MAP_BUFF(GL_ARRAY_BUFFER, xGL_WRITE_FLAG));
    RENDER_VERIFY(currentIndexBufferPointer = (uint16*)xGL_MAP_BUFF(GL_ELEMENT_ARRAY_BUFFER, xGL_WRITE_FLAG));
#endif
}

void VboPool::UnmapBuffers()
{
#if USE_MAPPING
    RENDER_VERIFY(xGL_UNMAP_BUFF(GL_ARRAY_BUFFER));
    currentVertexBufferPointer = 0;
    RENDER_VERIFY(xGL_UNMAP_BUFF(GL_ELEMENT_ARRAY_BUFFER));
    currentIndexBufferPointer = 0;
#endif
}

void VboPool::MapVertexBuffer()
{
#if USE_MAPPING
    uint32 vbid = currentDataObject->GetVertexBufferID();
    RENDER_VERIFY(RenderManager::Instance()->HWglBindBuffer(GL_ARRAY_BUFFER, vbid));
    RENDER_VERIFY(currentVertexBufferPointer = (float32*)xGL_MAP_BUFF(GL_ARRAY_BUFFER, xGL_WRITE_FLAG));
#endif
}

void VboPool::UnmapVertexBuffer()
{
#if USE_MAPPING
    RENDER_VERIFY(xGL_UNMAP_BUFF(GL_ARRAY_BUFFER));
    currentVertexBufferPointer = 0;
    RENDER_VERIFY(RenderManager::Instance()->HWglBindBuffer(GL_ARRAY_BUFFER, NULL));
#endif
}

void VboPool::MapIndexBuffer()
{
#if USE_MAPPING
    uint32 ibid = currentDataObject->GetIndexBufferID();
    RENDER_VERIFY(RenderManager::Instance()->HWglBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibid));
    RENDER_VERIFY(currentIndexBufferPointer = (uint16*)xGL_MAP_BUFF(GL_ELEMENT_ARRAY_BUFFER, xGL_WRITE_FLAG));
#endif
}

void VboPool::UnmapIndexBuffer()
{
#if USE_MAPPING
    RENDER_VERIFY(xGL_UNMAP_BUFF(GL_ELEMENT_ARRAY_BUFFER));
    currentIndexBufferPointer = 0;
    RENDER_VERIFY(RenderManager::Instance()->HWglBindBuffer(GL_ELEMENT_ARRAY_BUFFER, NULL));
#endif
}

void VboPool::RenewBuffers(uint32 size)
{
}

RenderDataObject* VboPool::GetDataObject() const
{
    return currentDataObject;
}

float32* VboPool::GetVertexBufferPointer() const
{
    return currentVertexBufferPointer;
}

uint16* VboPool::GetIndexBufferPointer() const
{
    return currentIndexBufferPointer;
}

uint32 VboPool::GetVertexBufferSize() const
{
    return currentVertexBufferSize;
}

uint32 VboPool::GetIndexBufferSize() const
{
    return currentIndexBufferSize;
}

RenderSystem2D::RenderSystem2D()
    : pool(NULL)
{
    useBatching = USE_BATCHING;
    spriteRenderObject = new RenderDataObject();
    spriteVertexStream = spriteRenderObject->SetStream(EVF_VERTEX, TYPE_FLOAT, 2, 0, 0);
    spriteTexCoordStream  = spriteRenderObject->SetStream(EVF_TEXCOORD0, TYPE_FLOAT, 2, 0, 0);
    spriteColorStream = spriteRenderObject->SetStream(EVF_COLOR, TYPE_FLOAT, 4, 0, 0);

#if !USE_MAPPING
    vertexBuffer2.resize(4096 * 32); //2048 points (XY UV RGBA)
    indexBuffer2.resize(8192);
#endif
}

RenderSystem2D::~RenderSystem2D()
{
	SafeDelete(pool);
	SafeRelease(spriteRenderObject);
}

void RenderSystem2D::Reset()
{
	// Create pool on first BeginFrame call
	if(NULL == pool)
	{
		pool = new VboPool(4096, 10);
	}

	currentClip.x = 0;
	currentClip.y = 0;
	currentClip.dx = -1;
	currentClip.dy = -1;

    Setup2DMatrices();

    defaultSpriteDrawState.Reset();
    defaultSpriteDrawState.renderState = RenderState::RENDERSTATE_2D_BLEND;
    defaultSpriteDrawState.shader = RenderManager::TEXTURE_MUL_FLAT_COLOR;

    batches.clear();
    batches.reserve(1024);
    currentBatch.Reset();

    vertexIndex = 0;
    indexIndex = 0;
}

void RenderSystem2D::Setup2DMatrices()
{
    RenderManager::SetDynamicParam(PARAM_WORLD, &Matrix4::IDENTITY, UPDATE_SEMANTIC_ALWAYS);
    RenderManager::SetDynamicParam(PARAM_VIEW, &viewMatrix, UPDATE_SEMANTIC_ALWAYS);
}

void RenderSystem2D::ScreenSizeChanged()
{
    Matrix4 glTranslate, glScale;

    Vector2 scale = VirtualCoordinatesSystem::Instance()->ConvertVirtualToPhysical(Vector2(1.f, 1.f));
    Vector2 realDrawOffset = VirtualCoordinatesSystem::Instance()->GetPhysicalDrawOffset();

    glTranslate.glTranslate(realDrawOffset.x, realDrawOffset.y, 0.0f);
    glScale.glScale(scale.x, scale.y, 1.0f);
    viewMatrix = glScale * glTranslate;
}

void RenderSystem2D::SetClip(const Rect &rect)
{
    currentClip = rect;
    RenderManager::Instance()->SetHWClip(VirtualCoordinatesSystem::Instance()->ConvertVirtualToPhysical(currentClip));
}

void RenderSystem2D::RemoveClip()
{
    currentClip = Rect(0,0,-1,-1);
    RenderManager::Instance()->SetHWClip(currentClip);
}

void RenderSystem2D::ClipRect(const Rect &rect)
{
    Rect r = currentClip;
    if(r.dx < 0)
    {
        r.dx = (float32)VirtualCoordinatesSystem::Instance()->GetVirtualScreenSize().dx;
    }
    if(r.dy < 0)
    {
        r.dy = (float32)VirtualCoordinatesSystem::Instance()->GetVirtualScreenSize().dy;
    }

    r = r.Intersection(rect);

    currentClip = r;
    RenderManager::Instance()->SetHWClip(VirtualCoordinatesSystem::Instance()->ConvertVirtualToPhysical(currentClip));
}

void RenderSystem2D::ClipPush()
{
    clipStack.push(currentClip);
}

void RenderSystem2D::ClipPop()
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

void RenderSystem2D::Flush()
{
    /*
    Called on each EndFrame, particle draw, screen transitions preparing, screen borders draw
    */

    if (!useBatching)
    {
        return;
    }

    if (currentBatch.count > 0)
    {
        batches.push_back(currentBatch);
        currentBatch.Reset();
    }

    if (batches.empty())
    {
        vertexIndex = 0;
        indexIndex = 0;
        return;
    }

    spritePrimitiveToDraw = PRIMITIVETYPE_TRIANGLELIST;

#if !USE_MAPPING
    pool->SetVertexData(vertexIndex, &vertexBuffer2[0]);
    pool->SetIndexData(indexIndex, (uint8*)&indexBuffer2[0]);
#else
    // Need set streams because on pool constructor we can't set they (deadlock)
    pool->GetDataObject()->SetStream(EVF_VERTEX, TYPE_FLOAT, 2, 32, 0);
    pool->GetDataObject()->SetStream(EVF_TEXCOORD0, TYPE_FLOAT, 2, 32, (void*)8);
    pool->GetDataObject()->SetStream(EVF_COLOR, TYPE_FLOAT, 4, 32, (void*)16);
#endif
    RenderManager::Instance()->SetRenderData(pool->GetDataObject());

    Vector<RenderBatch2D>::iterator it = batches.begin();
    Vector<RenderBatch2D>::iterator eit = batches.end();
    for (; it != eit; ++it)
    {
        const RenderBatch2D& batch = *it;

        bool clip = batch.clipRect.dx != -1;
        if (clip)
        {
            ClipPush();
            SetClip(batch.clipRect);
        }

        RENDERER_UPDATE_STATS(spriteDrawCount++);

        RenderManager::Instance()->SetRenderState(batch.renderState);
        RenderManager::Instance()->SetTextureState(batch.textureHandle);
        RenderManager::Instance()->SetRenderEffect(batch.shader);
        RenderManager::Instance()->DrawElements(spritePrimitiveToDraw, batch.count, EIF_16, (void*)(batch.indexOffset * 2));

        if (clip)
        {
            ClipPop();
        }
    }

    RenderManager::Instance()->SetRenderData(NULL);

    batches.clear();

    vertexIndex = 0;
    indexIndex = 0;

    pool->Next();
}

void RenderSystem2D::PushBatch(UniqueHandle state, UniqueHandle texture, Shader* shader, Rect const& clip)
{
    Shader * convShader = shader;

    if (RenderManager::TEXTURE_MUL_FLAT_COLOR == shader)
    {
        convShader = RenderManager::TEXTURE_MUL_COLOR;
    }
    else if (RenderManager::TEXTURE_MUL_FLAT_COLOR_ALPHA_TEST == shader)
    {
        convShader = RenderManager::TEXTURE_MUL_COLOR_ALPHA_TEST;
    }
    else if (RenderManager::TEXTURE_MUL_FLAT_COLOR_IMAGE_A8 == shader)
    {
        convShader = RenderManager::TEXTURE_MUL_COLOR_IMAGE_A8;
    }

    if (currentBatch.renderState != state
        || currentBatch.textureHandle != texture
        || currentBatch.shader != convShader
        || currentBatch.clipRect != currentClip)
    {
        if (currentBatch.count > 0)
        {
            batches.push_back(currentBatch);
        }
        currentBatch.Reset();

        currentBatch.renderState = state;
        currentBatch.textureHandle = texture;
        currentBatch.shader = convShader;
        currentBatch.clipRect = currentClip;
        currentBatch.indexOffset = indexIndex;
    }

    currentBatch.count += spriteIndexCount;
    indexIndex += spriteIndexCount;
    vertexIndex += spriteVertexCount;
}


void RenderSystem2D::Draw(Sprite * sprite, Sprite::DrawState * drawState /* = 0 */)
{
    if(!RenderManager::Instance()->GetOptions()->IsOptionEnabled(RenderOptions::SPRITE_DRAW))
	{
		return;
	}

    Setup2DMatrices();

    Sprite::DrawState * state = drawState;
    if (!state)
    {
        state = &defaultSpriteDrawState;
    }

	float32 x, y;
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

    x = state->position.x - state->pivotPoint.x * state->scale.x;
    y = state->position.y - state->pivotPoint.y * state->scale.y;

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
                    spriteTempVertices[2] = spriteTempVertices[6] = VirtualCoordinatesSystem::Instance()->ConvertPhysicalToVirtualX(floorf(VirtualCoordinatesSystem::Instance()->ConvertVirtualToPhysicalX(frameVertices[frame][0] * scaleX + x) + 0.5f));//x2
                    spriteTempVertices[5] = spriteTempVertices[7] = VirtualCoordinatesSystem::Instance()->ConvertPhysicalToVirtualY(floorf(VirtualCoordinatesSystem::Instance()->ConvertVirtualToPhysicalY(frameVertices[frame][1] * scaleY + y) + 0.5f));//y2
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
                    spriteTempVertices[2] = spriteTempVertices[6] = VirtualCoordinatesSystem::Instance()->ConvertPhysicalToVirtualX(floorf(VirtualCoordinatesSystem::Instance()->ConvertVirtualToPhysicalX(frameVertices[frame][0] + x) + 0.5f));//x2
                    spriteTempVertices[5] = spriteTempVertices[7] = VirtualCoordinatesSystem::Instance()->ConvertPhysicalToVirtualY(floorf(VirtualCoordinatesSystem::Instance()->ConvertVirtualToPhysicalY(frameVertices[frame][1] + y) + 0.5f));//y2
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
                        spriteTempVertices[2] = spriteTempVertices[6] = VirtualCoordinatesSystem::Instance()->ConvertPhysicalToVirtualX(floorf(VirtualCoordinatesSystem::Instance()->ConvertVirtualToPhysicalX(frameVertices[frame][0] * scaleX + x) + 0.5f));//x2
                        spriteTempVertices[0] = spriteTempVertices[4] = (frameVertices[frame][2] - frameVertices[frame][0]) * scaleX + spriteTempVertices[2];//x1
                        spriteTempVertices[1] = spriteTempVertices[3] = VirtualCoordinatesSystem::Instance()->ConvertPhysicalToVirtualY(floorf(VirtualCoordinatesSystem::Instance()->ConvertVirtualToPhysicalY(frameVertices[frame][1] * scaleY + y) + 0.5f));//y1
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
                        spriteTempVertices[2] = spriteTempVertices[6] = VirtualCoordinatesSystem::Instance()->ConvertPhysicalToVirtualX(floorf(VirtualCoordinatesSystem::Instance()->ConvertVirtualToPhysicalX(frameVertices[frame][0] + x) + 0.5f));//x2
                        spriteTempVertices[0] = spriteTempVertices[4] = (frameVertices[frame][2] - frameVertices[frame][0]) + spriteTempVertices[2];//x1
                        spriteTempVertices[1] = spriteTempVertices[3] = VirtualCoordinatesSystem::Instance()->ConvertPhysicalToVirtualY(floorf(VirtualCoordinatesSystem::Instance()->ConvertVirtualToPhysicalY(frameVertices[frame][1] + y) + 0.5f));//y1
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
                        spriteTempVertices[0] = spriteTempVertices[4] = VirtualCoordinatesSystem::Instance()->ConvertPhysicalToVirtualX(floorf(VirtualCoordinatesSystem::Instance()->ConvertVirtualToPhysicalX(frameVertices[frame][0] * scaleX + x) + 0.5f));//x1
                        spriteTempVertices[5] = spriteTempVertices[7] = VirtualCoordinatesSystem::Instance()->ConvertPhysicalToVirtualY(floorf(VirtualCoordinatesSystem::Instance()->ConvertVirtualToPhysicalY(frameVertices[frame][1] * scaleY + y) + 0.5f));//y2
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
                        spriteTempVertices[0] = spriteTempVertices[4] = VirtualCoordinatesSystem::Instance()->ConvertPhysicalToVirtualX(floorf(VirtualCoordinatesSystem::Instance()->ConvertVirtualToPhysicalX(frameVertices[frame][0] + x) + 0.5f));//x1
                        spriteTempVertices[5] = spriteTempVertices[7] = VirtualCoordinatesSystem::Instance()->ConvertPhysicalToVirtualY(floorf(VirtualCoordinatesSystem::Instance()->ConvertVirtualToPhysicalY(frameVertices[frame][1] + y) + 0.5f));//y2
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
                spriteTempVertices[0] = spriteTempVertices[4] = VirtualCoordinatesSystem::Instance()->ConvertPhysicalToVirtualX(floorf(VirtualCoordinatesSystem::Instance()->ConvertVirtualToPhysicalX(frameVertices[frame][0] * scaleX + x) + 0.5f));//x1
                spriteTempVertices[1] = spriteTempVertices[3] = VirtualCoordinatesSystem::Instance()->ConvertPhysicalToVirtualY(floorf(VirtualCoordinatesSystem::Instance()->ConvertVirtualToPhysicalY(frameVertices[frame][1] * scaleY + y) + 0.5f));//y1
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
                spriteTempVertices[0] = spriteTempVertices[4] = VirtualCoordinatesSystem::Instance()->ConvertPhysicalToVirtualX(floorf(VirtualCoordinatesSystem::Instance()->ConvertVirtualToPhysicalX(frameVertices[frame][0] + x) + 0.5f));//x1
                spriteTempVertices[1] = spriteTempVertices[3] = VirtualCoordinatesSystem::Instance()->ConvertPhysicalToVirtualY(floorf(VirtualCoordinatesSystem::Instance()->ConvertVirtualToPhysicalY(frameVertices[frame][1] + y) + 0.5f));//y1
                spriteTempVertices[2] = spriteTempVertices[6] = (frameVertices[frame][2] - frameVertices[frame][0]) + spriteTempVertices[0];//x2
                spriteTempVertices[5] = spriteTempVertices[7] = (frameVertices[frame][5] - frameVertices[frame][1]) + spriteTempVertices[1];//y2
            }

        }

    }

    if(!sprite->clipPolygon)
    {
        if(sprite->flags & Sprite::EST_ROTATE)
        {
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

        spriteVertexCount = 4;

        if (useBatching)
        {
            uint32 vi = vertexIndex * 8;
            uint32 ii = indexIndex;
#if USE_MAPPING
            pool->MapVertexBuffer();
            float32* vb = pool->GetVertexBufferPointer();
#else
            float32 * vb = &vertexBuffer2.front();
#endif

            spriteIndexCount = 6;

            const Color c = RenderManager::Instance()->GetColor();
            for (int32 i = 0; i < spriteVertexCount; ++i)
            {
                vb[vi++] = spriteTempVertices[i * 2];
                vb[vi++] = spriteTempVertices[i * 2 + 1];
                vb[vi++] = sprite->texCoords[frame][i * 2];
                vb[vi++] = sprite->texCoords[frame][i * 2 + 1];
                vb[vi++] = c.r;
                vb[vi++] = c.g;
                vb[vi++] = c.b;
                vb[vi++] = c.a;
            }

#if USE_MAPPING
            pool->UnmapVertexBuffer();
            pool->MapIndexBuffer();
            uint16* ib = pool->GetIndexBufferPointer();
#else
            uint16* ib = &indexBuffer2.front();
#endif

            static uint32 spriteIndeces[] = { 0, 1, 2, 1, 3, 2 };
            for (int32 i = 0; i < spriteIndexCount; ++i)
            {
                //indexBuffer2.push_back(vertexIndex + spriteIndeces[i]);
                ib[ii++] = vertexIndex + spriteIndeces[i];
            }

#if USE_MAPPING
            pool->UnmapIndexBuffer();
#endif
        }
        else
        {
            spriteVertexStream->Set(TYPE_FLOAT, 2, 0, spriteTempVertices);
            spriteTexCoordStream->Set(TYPE_FLOAT, 2, 0, sprite->texCoords[frame]);
            spritePrimitiveToDraw = PRIMITIVETYPE_TRIANGLESTRIP;
			DVASSERT(spriteVertexStream->pointer != 0);
			DVASSERT(spriteTexCoordStream->pointer != 0);
        }

    }
    else
    {
        spriteClippedVertices.clear();
        spriteClippedVertices.reserve(sprite->clipPolygon->GetPointCount());
        spriteClippedTexCoords.clear();
        spriteClippedTexCoords.reserve(sprite->clipPolygon->GetPointCount());

        Texture * t = sprite->GetTexture(frame);
        Vector2 virtualTexSize = VirtualCoordinatesSystem::Instance()->ConvertResourceToVirtual(Vector2((float32)t->width, (float32)t->height), sprite->GetResourceSizeIndex());
        float32 adjWidth = 1.f / virtualTexSize.x;
        float32 adjHeight = 1.f / virtualTexSize.y;

        if (useBatching)
        {
            uint32 vi = vertexIndex * 8;
            uint32 ii = indexIndex;
#if USE_MAPPING
            pool->MapVertexBuffer();
            float32* vb = pool->GetVertexBufferPointer();
#else
            float32 * vb = &vertexBuffer2.front();
#endif

            spriteVertexCount = sprite->clipPolygon->GetPointCount();
            DVASSERT(spriteVertexCount >= 2);
            spriteIndexCount = (spriteVertexCount - 2) * 3;

            const Color c = RenderManager::Instance()->GetColor();
            for (int32 i = 0; i < spriteVertexCount; ++i)
            {

                const Vector2 &point = sprite->clipPolygon->GetPoints()[i];
                Vector2 texCoord((point.x - frameVertices[frame][0]) * adjWidth, (point.y - frameVertices[frame][1]) * adjHeight);
                if (sprite->flags & Sprite::EST_SCALE)
                {
                    vb[vi++] = point.x * scaleX + x;
                    vb[vi++] = point.y * scaleY + y;

                }
                else
                {
                    vb[vi++] = point.x + x;
                    vb[vi++] = point.y + y;
                }
                vb[vi++] = sprite->texCoords[frame][0] + texCoord.x;
                vb[vi++] = sprite->texCoords[frame][1] + texCoord.y;
                vb[vi++] = c.r;
                vb[vi++] = c.g;
                vb[vi++] = c.b;
                vb[vi++] = c.a;
            }

#if USE_MAPPING
            pool->UnmapVertexBuffer();
            pool->MapIndexBuffer();
            uint16* ib = pool->GetIndexBufferPointer();
#else
            uint16* ib = &indexBuffer2.front();
#endif

            for (int32 i = 2; i < spriteVertexCount; ++i)
            {
                ib[ii++] = vertexIndex;
                ib[ii++] = vertexIndex + i - 1;
                ib[ii++] = vertexIndex + i;
            }

#if USE_MAPPING
            pool->UnmapIndexBuffer();
#endif
        }
        else
        {
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
                    spriteClippedVertices.push_back( point + pos );
                }
            }

            for (int32 i = 0; i < sprite->clipPolygon->GetPointCount(); ++i)
            {
                const Vector2 &point = sprite->clipPolygon->GetPoints()[i];
                Vector2 texCoord((point.x - frameVertices[frame][0]) * adjWidth, (point.y - frameVertices[frame][1]) * adjHeight);
                spriteClippedTexCoords.push_back( Vector2( sprite->texCoords[frame][0] + texCoord.x, sprite->texCoords[frame][1] + texCoord.y ) );
            }

            spriteVertexStream->Set(TYPE_FLOAT, 2, 0, &spriteClippedVertices.front());
            spriteTexCoordStream->Set(TYPE_FLOAT, 2, 0, &spriteClippedTexCoords.front());
            spritePrimitiveToDraw = PRIMITIVETYPE_TRIANGLEFAN;
            spriteVertexCount = sprite->clipPolygon->pointCount;
			DVASSERT(spriteVertexStream->pointer != 0);
			DVASSERT(spriteTexCoordStream->pointer != 0);
        }
    }

    Rect clipRect(0,0,-1,-1);
	if(sprite->clipPolygon)
	{
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
	}
    else
    {
        clipRect = currentClip;
    }

    if (useBatching)
    {
        PushBatch(state->renderState, sprite->GetTextureHandle(state->frame), state->shader, clipRect);
    }
    else
    {
        if (sprite->clipPolygon)
        {
            ClipPush();
            ClipRect(clipRect);
        }

        RENDERER_UPDATE_STATS(spriteDrawCount++);

        RenderManager::Instance()->SetRenderState(state->renderState);
        RenderManager::Instance()->SetTextureState(sprite->GetTextureHandle(state->frame));
        RenderManager::Instance()->SetRenderData(spriteRenderObject);
        RenderManager::Instance()->SetRenderEffect(state->shader);
        RenderManager::Instance()->DrawArrays(spritePrimitiveToDraw, 0, spriteVertexCount);

        if (sprite->clipPolygon)
        {
            ClipPop();
        }
    }

}

void RenderSystem2D::DrawStretched(Sprite * sprite, Sprite::DrawState * state, Vector2 streatchCap, Rect drawRect, UIControlBackground::eDrawType type)
{
    if (!sprite)return;
	if (!RenderManager::Instance()->GetOptions()->IsOptionEnabled(RenderOptions::SPRITE_DRAW))
    {
        return;
    }

	int32 frame = Clamp(state->frame, 0, sprite->frameCount - 1);

    UniqueHandle textureHandle = sprite->GetTextureHandle(frame);
    Texture* texture = sprite->GetTexture(frame);

    float32 texX = sprite->GetRectOffsetValueForFrame(frame, Sprite::X_POSITION_IN_TEXTURE);
    float32 texY = sprite->GetRectOffsetValueForFrame(frame, Sprite::Y_POSITION_IN_TEXTURE);
    float32 texDx = sprite->GetRectOffsetValueForFrame(frame, Sprite::ACTIVE_WIDTH);
    float32 texDy = sprite->GetRectOffsetValueForFrame(frame, Sprite::ACTIVE_HEIGHT);
    float32 texOffX = sprite->GetRectOffsetValueForFrame(frame, Sprite::X_OFFSET_TO_ACTIVE);
    float32 texOffY = sprite->GetRectOffsetValueForFrame(frame, Sprite::Y_OFFSET_TO_ACTIVE);

    const float32 spriteWidth = sprite->GetWidth();
    const float32 spriteHeight = sprite->GetHeight();

    const float32 leftOffset  = streatchCap.x - texOffX;
    const float32 rightOffset = streatchCap.x - ( spriteWidth - texDx - texOffX );
    const float32 topOffset   = streatchCap.y  - texOffY;
    const float32 bottomOffset= streatchCap.y  - ( spriteHeight - texDy - texOffY );

    const float32 realLeftStretchCap  = Max( 0.0f, leftOffset );
    const float32 realRightStretchCap = Max( 0.0f, rightOffset );
    const float32 realTopStretchCap   = Max( 0.0f, topOffset );
    const float32 realBottomStretchCap= Max( 0.0f, bottomOffset );

    const float32 scaleFactorX = drawRect.dx / spriteWidth;
    const float32 scaleFactorY = drawRect.dy / spriteHeight;
    float32 x = drawRect.x + Max( 0.0f, -leftOffset ) * scaleFactorX;
    float32 y = drawRect.y + Max( 0.0f, -topOffset  ) * scaleFactorY;
    float32 dx = drawRect.dx - ( Max( 0.0f, -leftOffset ) + Max( 0.0f, -rightOffset  ) ) * scaleFactorX;
    float32 dy = drawRect.dy - ( Max( 0.0f, -topOffset  ) + Max( 0.0f, -bottomOffset ) ) * scaleFactorY;

    texDx = VirtualCoordinatesSystem::Instance()->ConvertVirtualToResourceX(texDx, sprite->GetResourceSizeIndex());
    texDy = VirtualCoordinatesSystem::Instance()->ConvertVirtualToResourceY(texDy, sprite->GetResourceSizeIndex());

    const float32 leftCap = VirtualCoordinatesSystem::Instance()->ConvertVirtualToResourceX(realLeftStretchCap, sprite->GetResourceSizeIndex());
    const float32 rightCap = VirtualCoordinatesSystem::Instance()->ConvertVirtualToResourceX(realRightStretchCap, sprite->GetResourceSizeIndex());
    const float32 topCap = VirtualCoordinatesSystem::Instance()->ConvertVirtualToResourceY(realTopStretchCap, sprite->GetResourceSizeIndex());
    const float32 bottomCap = VirtualCoordinatesSystem::Instance()->ConvertVirtualToResourceY(realBottomStretchCap, sprite->GetResourceSizeIndex());

    float32 vertices[16 * 2];
    float32 texCoords[16 * 2];

    float32 textureWidth = (float32)texture->GetWidth();
    float32 textureHeight = (float32)texture->GetHeight();

    int32 vertInTriCount = 18;

    switch (type)
    {
	case UIControlBackground::DRAW_STRETCH_HORIZONTAL:
        {
            float32 ddy = (spriteHeight - dy);
            y -= ddy * 0.5f;
            dy += ddy;

            vertices[0] = vertices[8]  = x;
            vertices[1] = vertices[3]  = vertices[5]  = vertices[7]  = y;
            vertices[4] = vertices[12] = x + dx - realRightStretchCap;

            vertices[2] = vertices[10] = x + realLeftStretchCap;
            vertices[9] = vertices[11] = vertices[13] = vertices[15] = y + dy;
            vertices[6] = vertices[14] = x + dx;

            texCoords[0] = texCoords[8]  = texX / textureWidth;
            texCoords[1] = texCoords[3]  = texCoords[5]  = texCoords[7]  = texY / textureHeight;
            texCoords[4] = texCoords[12] = (texX + texDx - rightCap) / textureWidth;

            texCoords[2] = texCoords[10] = (texX + leftCap) / textureWidth;
            texCoords[9] = texCoords[11] = texCoords[13] = texCoords[15] = (texY + texDy) / textureHeight;
            texCoords[6] = texCoords[14] = (texX + texDx) / textureWidth;
        }
        break;
        case UIControlBackground::DRAW_STRETCH_VERTICAL:
        {
            float32 ddx = (spriteWidth - dx);
            x -= ddx * 0.5f;
            dx += ddx;

            vertices[0] = vertices[2]  = vertices[4]  = vertices[6]  = x;
            vertices[8] = vertices[10] = vertices[12] = vertices[14] = x + dx;

            vertices[1] = vertices[9]  = y;
            vertices[3] = vertices[11] = y + realTopStretchCap;
            vertices[5] = vertices[13] = y + dy - realBottomStretchCap;
            vertices[7] = vertices[15] = y + dy;

            texCoords[0] = texCoords[2]  = texCoords[4]  = texCoords[6]  = texX / textureWidth;
            texCoords[8] = texCoords[10] = texCoords[12] = texCoords[14] = (texX + texDx) / textureWidth;

            texCoords[1] = texCoords[9]  = texY / textureHeight;
            texCoords[3] = texCoords[11] = (texY + topCap) / textureHeight;
            texCoords[5] = texCoords[13] = (texY + texDy - bottomCap) / textureHeight;
            texCoords[7] = texCoords[15] = (texY + texDy) / textureHeight;
        }
        break;
        case UIControlBackground::DRAW_STRETCH_BOTH:
        {
            vertInTriCount = 18 * 3;

            vertices[0] = vertices[8]  = vertices[16] = vertices[24] = x;
            vertices[2] = vertices[10] = vertices[18] = vertices[26] = x + realLeftStretchCap;
            vertices[4] = vertices[12] = vertices[20] = vertices[28] = x + dx - realRightStretchCap;
            vertices[6] = vertices[14] = vertices[22] = vertices[30] = x + dx;

            vertices[1] = vertices[3]  = vertices[5]  = vertices[7]  = y;
            vertices[9] = vertices[11] = vertices[13] = vertices[15] = y + realTopStretchCap;
            vertices[17]= vertices[19] = vertices[21] = vertices[23] = y + dy - realBottomStretchCap;
            vertices[25]= vertices[27] = vertices[29] = vertices[31] = y + dy;

            texCoords[0] = texCoords[8]  = texCoords[16] = texCoords[24] = texX / textureWidth;
            texCoords[2] = texCoords[10] = texCoords[18] = texCoords[26] = (texX + leftCap) / textureWidth;
            texCoords[4] = texCoords[12] = texCoords[20] = texCoords[28] = (texX + texDx - rightCap) / textureWidth;
            texCoords[6] = texCoords[14] = texCoords[22] = texCoords[30] = (texX + texDx) / textureWidth;

            texCoords[1]  = texCoords[3]  = texCoords[5]  = texCoords[7]  = texY / textureHeight;
            texCoords[9]  = texCoords[11] = texCoords[13] = texCoords[15] = (texY + topCap) / textureHeight;
            texCoords[17] = texCoords[19] = texCoords[21] = texCoords[23] = (texY + texDy - bottomCap)  / textureHeight;
            texCoords[25] = texCoords[27] = texCoords[29] = texCoords[31] = (texY + texDy) / textureHeight;
        }
        break;
        default: break;
    }

    static uint16 indeces[18 * 3] =
    {
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

	if (useBatching)
	{
        uint32 vi = vertexIndex * 8;
        uint32 ii = indexIndex;
#if USE_MAPPING
        pool->MapVertexBuffer();
        float32* vb = pool->GetVertexBufferPointer();
#else
        float32 * vb = &vertexBuffer2.front();
#endif

        spriteVertexCount = 16;
        spriteIndexCount = vertInTriCount;

        const Color c = RenderManager::Instance()->GetColor();
        for (int32 i = 0; i < spriteVertexCount; ++i)
		{
            vb[vi++] = vertices[i * 2];
            vb[vi++] = vertices[i * 2 + 1];
            vb[vi++] = texCoords[i * 2];
            vb[vi++] = texCoords[i * 2 + 1];
            vb[vi++] = c.r;
            vb[vi++] = c.g;
            vb[vi++] = c.b;
            vb[vi++] = c.a;
		}

#if USE_MAPPING
        pool->UnmapVertexBuffer();
        pool->MapIndexBuffer();
        uint16* ib = pool->GetIndexBufferPointer();
#else
        uint16* ib = &indexBuffer2.front();
#endif

        for (int32 i = 0; i < spriteIndexCount; ++i)
		{
            ib[ii++] = vertexIndex + indeces[i];
		}

#if USE_MAPPING
        pool->UnmapIndexBuffer();
#endif

        PushBatch(state->renderState, sprite->GetTextureHandle(state->frame), RenderManager::TEXTURE_MUL_FLAT_COLOR, currentClip);
	}
	else
	{
        RENDERER_UPDATE_STATS(spriteDrawCount++);

		spriteVertexStream->Set(TYPE_FLOAT, 2, 0, vertices);
		spriteTexCoordStream->Set(TYPE_FLOAT, 2, 0, texCoords);
		RenderManager::Instance()->SetTextureState(textureHandle);
		RenderManager::Instance()->SetRenderState(state->renderState);
		RenderManager::Instance()->SetRenderEffect(RenderManager::TEXTURE_MUL_FLAT_COLOR);
		RenderManager::Instance()->SetRenderData(spriteRenderObject);
		RenderManager::Instance()->DrawElements(PRIMITIVETYPE_TRIANGLELIST, vertInTriCount, EIF_16, indeces);
	}
}

void RenderSystem2D::DrawTiled(Sprite * sprite, Sprite::DrawState * state, const Vector2& stretchCapVector, const UIGeometricData &gd, TiledDrawData ** pTiledData)
{
    if (!sprite)return;
    if (!RenderManager::Instance()->GetOptions()->IsOptionEnabled(RenderOptions::SPRITE_DRAW))
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

    UniqueHandle textureHandle = sprite->GetTextureHandle(frame);

    stretchCap.x = Min( stretchCap.x * 0.5f, stretchCapVector.x );
    stretchCap.y = Min( stretchCap.y * 0.5f, stretchCapVector.y );

    bool needGenerateData = false;

	TiledDrawData * tiledData = 0;
	if( pTiledData )
	{
		tiledData = *pTiledData;
	}
    if( !tiledData )
    {
        tiledData = new TiledDrawData();
        needGenerateData = true;
    }
    else
    {
        needGenerateData |= stretchCap != tiledData->stretchCap;
        needGenerateData |= frame != tiledData->frame;
        needGenerateData |= sprite != tiledData->sprite;
        needGenerateData |= size != tiledData->size;
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


	if (useBatching)
	{
        uint32 vi = vertexIndex * 8;
        uint32 ii = indexIndex;
#if USE_MAPPING
        pool->MapVertexBuffer();
        float32* vb = pool->GetVertexBufferPointer();
#else
        float32 * vb = &vertexBuffer2.front();
#endif

		spriteVertexCount = td.transformedVertices.size();
        spriteIndexCount = td.indeces.size();

        const Color c = RenderManager::Instance()->GetColor();
        for (int32 i = 0; i < spriteVertexCount; ++i)
        {
            vb[vi++] = (td.transformedVertices[i].x);
            vb[vi++] = (td.transformedVertices[i].y);
            vb[vi++] = (td.texCoords[i].x);
            vb[vi++] = (td.texCoords[i].y);
            vb[vi++] = (c.r);
            vb[vi++] = (c.g);
            vb[vi++] = (c.b);
            vb[vi++] = (c.a);
        }

#if USE_MAPPING
        pool->UnmapVertexBuffer();
        pool->MapIndexBuffer();
        uint16* ib = pool->GetIndexBufferPointer();
#else
        uint16* ib = &indexBuffer2.front();
#endif

		for (int32 i = 0; i < spriteIndexCount; ++i)
        {
            ib[ii++] = vertexIndex + td.indeces[i];
        }

#if USE_MAPPING
        pool->UnmapIndexBuffer();
#endif

        PushBatch(state->renderState, sprite->GetTextureHandle(state->frame), RenderManager::TEXTURE_MUL_FLAT_COLOR, currentClip);

	}
	else
	{
        RENDERER_UPDATE_STATS(spriteDrawCount++);

		spriteVertexStream->Set(TYPE_FLOAT, 2, 0, &td.transformedVertices[0]);
		spriteTexCoordStream->Set(TYPE_FLOAT, 2, 0, &td.texCoords[0]);

		RenderManager::Instance()->SetTextureState(textureHandle);
		RenderManager::Instance()->SetRenderState(state->renderState);
		RenderManager::Instance()->SetRenderEffect(RenderManager::TEXTURE_MUL_FLAT_COLOR);
		RenderManager::Instance()->SetRenderData(spriteRenderObject);
		RenderManager::Instance()->DrawElements(PRIMITIVETYPE_TRIANGLELIST, td.indeces.size(), EIF_16, &td.indeces[0]);
	}
}

void RenderSystem2D::DrawFilled(Sprite * sprite, Sprite::DrawState * state, const UIGeometricData& gd)
{
	Flush();
	RenderManager::Instance()->SetTextureState(RenderState::TEXTURESTATE_EMPTY);
    if( gd.angle != 0.0f )
    {
        Polygon2 poly;
        gd.GetPolygon( poly );
        RenderHelper::Instance()->FillPolygon( poly, state->renderState );
    }
    else
    {
        RenderHelper::Instance()->FillRect( gd.GetUnrotatedRect(), state->renderState );
    }
}

void TiledDrawData::GenerateTileData()
{
    Texture *texture = sprite->GetTexture(frame);

    Vector< Vector3 > cellsWidth;
    GenerateAxisData( size.x, sprite->GetRectOffsetValueForFrame(frame, Sprite::ACTIVE_WIDTH), VirtualCoordinatesSystem::Instance()->ConvertResourceToVirtualX((float32)texture->GetWidth(), sprite->GetResourceSizeIndex()), stretchCap.x, cellsWidth );

    Vector< Vector3 > cellsHeight;
    GenerateAxisData( size.y, sprite->GetRectOffsetValueForFrame(frame, Sprite::ACTIVE_HEIGHT), VirtualCoordinatesSystem::Instance()->ConvertResourceToVirtualX((float32)texture->GetHeight(), sprite->GetResourceSizeIndex()), stretchCap.y, cellsHeight );

    int32 vertexCount = 4 * cellsHeight.size() * cellsWidth.size();
    vertices.resize( vertexCount );
    transformedVertices.resize( vertexCount );
    texCoords.resize( vertexCount );

    int32 indecesCount = 6 * cellsHeight.size() * cellsWidth.size();
    indeces.resize( indecesCount );

    int32 offsetIndex = 0;
    const float32 * textCoords = sprite->GetTextureCoordsForFrame(frame);
    Vector2 trasformOffset;
    const Vector2 tempTexCoordsPt( textCoords[0], textCoords[1] );
    for( uint32 row = 0; row < cellsHeight.size(); ++row )
    {
        Vector2 cellSize( 0.0f, cellsHeight[row].x );
        Vector2 texCellSize( 0.0f, cellsHeight[row].y );
        Vector2 texTrasformOffset( 0.0f, cellsHeight[row].z );
        trasformOffset.x = 0.0f;

        for( uint32 column = 0; column < cellsWidth.size(); ++column, ++offsetIndex )
        {
            cellSize.x = cellsWidth[column].x;
            texCellSize.x = cellsWidth[column].y;
            texTrasformOffset.x = cellsWidth[column].z;

            int32 vertIndex = offsetIndex*4;
            vertices[vertIndex + 0] = trasformOffset;
            vertices[vertIndex + 1] = trasformOffset + Vector2( cellSize.x, 0.0f );
            vertices[vertIndex + 2] = trasformOffset + Vector2( 0.0f, cellSize.y );
            vertices[vertIndex + 3] = trasformOffset + cellSize;

            const Vector2 texel = tempTexCoordsPt + texTrasformOffset;
            texCoords[vertIndex + 0] = texel;
            texCoords[vertIndex + 1] = texel + Vector2( texCellSize.x, 0.0f );
            texCoords[vertIndex + 2] = texel + Vector2( 0.0f, texCellSize.y );
            texCoords[vertIndex + 3] = texel + texCellSize;

            int32 indecesIndex = offsetIndex*6;
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

    if( centerSize > 0.0f )
    {
        gridSize = (int32)ceilf( ( size - sideSize * 2.0f ) / centerSize );
        const float32 tileAreaSize = size - sideSize * 2.0f;
        partSize = tileAreaSize - floorf( tileAreaSize / centerSize ) * centerSize;
    }

    if( sideSize > 0.0f )
        gridSize += 2;

      axisData.resize( gridSize );

    int32 beginOffset = 0;
    int32 endOffset = 0;
    if( sideSize > 0.0f )
    {
        axisData.front() = Vector3( sideSize, sideTexSize, 0.0f );
        axisData.back() = Vector3( sideSize, sideTexSize, sideTexSize + centerTexSize );
        beginOffset = 1;
        endOffset = 1;
    }

    if( partSize > 0.0f )
    {
        ++endOffset;
        const int32 index = gridSize - endOffset;
        axisData[index].x = partSize;
        axisData[index].y = partSize / textureSize;
        axisData[index].z = sideTexSize;
    }

    if( centerSize > 0.0f )
    {
        std::fill( axisData.begin() + beginOffset, axisData.begin() + gridSize - endOffset, Vector3( centerSize, centerTexSize, sideTexSize ) );
    }
}

void TiledDrawData::GenerateTransformData()
{
    for( uint32 index = 0; index < vertices.size(); ++index )
    {
        transformedVertices[index] = vertices[index] * transformMatr;
    }
}


};
