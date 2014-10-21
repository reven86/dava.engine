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


#include "Render/2D/RenderSystem2D/RenderSystem2D.h"
#include "VirtualCoordinatesSystem.h"
#include "Render/RenderManager.h"
#include <Render/RenderHelper.h>

#include "UI/UIControl.h"
#include "UI/UIControlBackground.h"

namespace DAVA
{

RenderSystem2D::RenderSystem2D()
{
    useBatching = true;
    spriteRenderObject = new RenderDataObject();
    spriteVertexStream = spriteRenderObject->SetStream(EVF_VERTEX, TYPE_FLOAT, 2, 0, 0);
    spriteTexCoordStream  = spriteRenderObject->SetStream(EVF_TEXCOORD0, TYPE_FLOAT, 2, 0, 0);
    spriteColorStream = spriteRenderObject->SetStream(EVF_COLOR, TYPE_FLOAT, 4, 0, 0);

    //vertexBuffer2.reserve(2048 * (2 + 2 + 4)); //2048 points (XY UV RGBA)
    //indexBuffer2.reserve(2048);
}

RenderSystem2D::~RenderSystem2D()
{
	SafeRelease(spriteRenderObject);
}

void RenderSystem2D::Reset()
{
	currentClip.x = 0;
	currentClip.y = 0;
	currentClip.dx = -1;
	currentClip.dy = -1;
    
    Setup2DMatrices();

    batches.clear();
    batches.reserve(1024);
    currentBatch.Reset();
    //indexBuffer2.clear();
    //vertexBuffer2.clear();
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
    
    float32 scale = VirtualCoordinates::GetVirtualToPhysicalFactor();
    Vector2 realDrawOffset = VirtualCoordinatesSystem::Instance()->GetPhysicalDrawOffset();
    
    glTranslate.glTranslate(realDrawOffset.x, realDrawOffset.y, 0.0f);
    glScale.glScale(scale, scale, 1.0f);
    viewMatrix = glScale * glTranslate;
}

void RenderSystem2D::SetClip(const Rect &rect)
{
    currentClip = rect;
    RenderManager::Instance()->SetHWClip(VirtualCoordinates::ConvertVirtualToPhysical(currentClip));
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
        r.dx = (float32)ScreenSizes::GetVirtualScreenSize().dx;
    }
    if(r.dy < 0)
    {
        r.dy = (float32)ScreenSizes::GetVirtualScreenSize().dy;
    }
    
    r = r.Intersection(rect);
    
    currentClip = r;
    RenderManager::Instance()->SetHWClip(VirtualCoordinates::ConvertVirtualToPhysical(currentClip));
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
        return;
    }

    spriteVertexStream->Set(TYPE_FLOAT, 2, 0, vertexBuffer);
    spriteTexCoordStream->Set(TYPE_FLOAT, 2, 0, texBuffer);
    spriteColorStream->Set(TYPE_FLOAT, 4, 0, colorBuffer);
    spritePrimitiveToDraw = PRIMITIVETYPE_TRIANGLELIST; 
    RenderManager::Instance()->SetRenderData(spriteRenderObject);

    Vector<RenderBatch2D> copy;
    batches.swap(copy);
    Vector<RenderBatch2D>::iterator it = copy.begin();
    Vector<RenderBatch2D>::iterator eit = copy.end();
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
        RenderManager::Instance()->DrawElements(spritePrimitiveToDraw, batch.count, EIF_32, &indexBuffer[batch.indeces]);

        if (clip)
        {
            ClipPop();
        }
    }
}

void RenderSystem2D::Draw(Sprite * sprite, Sprite::DrawState * state)
{
    if(!RenderManager::Instance()->GetOptions()->IsOptionEnabled(RenderOptions::SPRITE_DRAW))
	{
		return;
	}
    
    Setup2DMatrices();
    
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
                    spriteTempVertices[2] = spriteTempVertices[6] = floorf((frameVertices[frame][0] * scaleX + x) * VirtualCoordinates::GetVirtualToPhysicalFactor() + 0.5f) * VirtualCoordinates::GetPhysicalToVirtualFactor();//x2
                    spriteTempVertices[5] = spriteTempVertices[7] = floorf((frameVertices[frame][1] * scaleY + y) * VirtualCoordinates::GetVirtualToPhysicalFactor() + 0.5f) * VirtualCoordinates::GetPhysicalToVirtualFactor();//y2
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
                    spriteTempVertices[2] = spriteTempVertices[6] = floorf((frameVertices[frame][0] + x) * VirtualCoordinates::GetVirtualToPhysicalFactor() + 0.5f) * VirtualCoordinates::GetPhysicalToVirtualFactor();//x2
                    spriteTempVertices[5] = spriteTempVertices[7] = floorf((frameVertices[frame][1] + y) * VirtualCoordinates::GetVirtualToPhysicalFactor() + 0.5f) * VirtualCoordinates::GetPhysicalToVirtualFactor();//y2
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
                        spriteTempVertices[2] = spriteTempVertices[6] = floorf((frameVertices[frame][0] * scaleX + x) * VirtualCoordinates::GetVirtualToPhysicalFactor() + 0.5f) * VirtualCoordinates::GetPhysicalToVirtualFactor();//x2
                        spriteTempVertices[0] = spriteTempVertices[4] = (frameVertices[frame][2] - frameVertices[frame][0]) * scaleX + spriteTempVertices[2];//x1
                        spriteTempVertices[1] = spriteTempVertices[3] = floorf((frameVertices[frame][1] * scaleY + y) * VirtualCoordinates::GetVirtualToPhysicalFactor() + 0.5f) * VirtualCoordinates::GetPhysicalToVirtualFactor();//y1
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
                        spriteTempVertices[2] = spriteTempVertices[6] = floorf((frameVertices[frame][0] + x) * VirtualCoordinates::GetVirtualToPhysicalFactor() + 0.5f) * VirtualCoordinates::GetPhysicalToVirtualFactor();//x2
                        spriteTempVertices[0] = spriteTempVertices[4] = (frameVertices[frame][2] - frameVertices[frame][0]) + spriteTempVertices[2];//x1
                        spriteTempVertices[1] = spriteTempVertices[3] = floorf((frameVertices[frame][1] + y) * VirtualCoordinates::GetVirtualToPhysicalFactor() + 0.5f) * VirtualCoordinates::GetPhysicalToVirtualFactor();//y1
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
                        spriteTempVertices[0] = spriteTempVertices[4] = floorf((frameVertices[frame][0] * scaleX + x) * VirtualCoordinates::GetVirtualToPhysicalFactor() + 0.5f) * VirtualCoordinates::GetPhysicalToVirtualFactor();//x1
                        spriteTempVertices[5] = spriteTempVertices[7] = floorf((frameVertices[frame][1] * scaleY + y) * VirtualCoordinates::GetVirtualToPhysicalFactor() + 0.5f) * VirtualCoordinates::GetPhysicalToVirtualFactor();//y2
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
                        spriteTempVertices[0] = spriteTempVertices[4] = floorf((frameVertices[frame][0] + x) * VirtualCoordinates::GetVirtualToPhysicalFactor() + 0.5f) * VirtualCoordinates::GetPhysicalToVirtualFactor();//x1
                        spriteTempVertices[5] = spriteTempVertices[7] = floorf((frameVertices[frame][1] + y) * VirtualCoordinates::GetVirtualToPhysicalFactor() + 0.5f) * VirtualCoordinates::GetPhysicalToVirtualFactor();//y2
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
                spriteTempVertices[0] = spriteTempVertices[4] = floorf((frameVertices[frame][0] * scaleX + x) * VirtualCoordinates::GetVirtualToPhysicalFactor() + 0.5f) * VirtualCoordinates::GetPhysicalToVirtualFactor();//x1
                spriteTempVertices[1] = spriteTempVertices[3] = floorf((frameVertices[frame][1] * scaleY + y) * VirtualCoordinates::GetVirtualToPhysicalFactor() + 0.5f) * VirtualCoordinates::GetPhysicalToVirtualFactor();//y1
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
                spriteTempVertices[0] = spriteTempVertices[4] = floorf((frameVertices[frame][0] + x) * VirtualCoordinates::GetVirtualToPhysicalFactor() + 0.5f) * VirtualCoordinates::GetPhysicalToVirtualFactor();//x1
                spriteTempVertices[1] = spriteTempVertices[3] = floorf((frameVertices[frame][1] + y) * VirtualCoordinates::GetVirtualToPhysicalFactor() + 0.5f) * VirtualCoordinates::GetPhysicalToVirtualFactor();//y1
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
        
        if (useBatching)
        {
            for (uint8 i = 0; i < 8; ++i)
            {
                vertexBuffer[vertexIndex * 2 + i] = spriteTempVertices[i];
                texBuffer[vertexIndex * 2 + i] = sprite->texCoords[frame][i];
            }
            const Color c = RenderManager::Instance()->GetColor();
            for (uint8 i = 0; i < 16; ++i)
            {
                colorBuffer[vertexIndex * 4 + i] = c.color[i % 4];
            }
            static uint32 spriteIndeces[] = { 0, 1, 2, 1, 3, 2 };
            DVASSERT((indexIndex + 6) < (MAX_VERTEXES * 3 / 2));
            for (uint8 i = 0; i < 6; ++i)
            {
                indexBuffer[indexIndex + i] = vertexIndex + spriteIndeces[i];
            }
            spriteIndexCount = 6;
        }
        else
        {
            spriteVertexStream->Set(TYPE_FLOAT, 2, 0, spriteTempVertices);
            spriteTexCoordStream->Set(TYPE_FLOAT, 2, 0, sprite->texCoords[frame]);
            spritePrimitiveToDraw = PRIMITIVETYPE_TRIANGLESTRIP;
			DVASSERT(spriteVertexStream->pointer != 0);
			DVASSERT(spriteTexCoordStream->pointer != 0);
        }
        spriteVertexCount = 4;
    }
    else
    {
        spriteClippedVertices.clear();
        spriteClippedVertices.reserve(sprite->clipPolygon->GetPointCount());
        spriteClippedTexCoords.clear();
        spriteClippedTexCoords.reserve(sprite->clipPolygon->GetPointCount());
        
        Texture * t = sprite->GetTexture(frame);
        Vector2 virtualTexSize = VirtualCoordinates::ConvertResourceToVirtual(Vector2((float32)t->width, (float32)t->height), sprite->GetResourceSizeIndex());
        float32 adjWidth = 1.f / virtualTexSize.x;
        float32 adjHeight = 1.f / virtualTexSize.y;
        
        if (useBatching)
        {
            int32 count = sprite->clipPolygon->GetPointCount();

            if (sprite->flags & Sprite::EST_SCALE)
            {
                for (int32 i = 0; i < count; ++i)
                {
                    const Vector2 &point = sprite->clipPolygon->GetPoints()[i];
                    vertexBuffer[vertexIndex * 2 + i * 2 + 0] = point.x*scaleX + x;
                    vertexBuffer[vertexIndex * 2 + i * 2 + 1] = point.y*scaleY + y;
                }
            }
            else
            {
                //Vector2 pos(x, y);
                for (int32 i = 0; i < count; ++i)
                {
                    const Vector2 &point = sprite->clipPolygon->GetPoints()[i];
                    vertexBuffer[vertexIndex * 2 + i * 2 + 0] = point.x + x;
                    vertexBuffer[vertexIndex * 2 + i * 2 + 1] = point.y + y;
                }
            }

            for (int32 i = 0; i < count; ++i)
            {
                const Vector2 &point = sprite->clipPolygon->GetPoints()[i];
                Vector2 texCoord((point.x - frameVertices[frame][0]) * adjWidth, (point.y - frameVertices[frame][1]) * adjHeight);
                texBuffer[vertexIndex * 2 + i * 2 + 0] = sprite->texCoords[frame][0] + texCoord.x;
                texBuffer[vertexIndex * 2 + i * 2 + 1] = sprite->texCoords[frame][1] + texCoord.y;
            }

            const Color c = RenderManager::Instance()->GetColor();
            for (int32 i = 0; i < count; ++i)
            {
                colorBuffer[(vertexIndex + i) * 4 + 0] = c.r;
                colorBuffer[(vertexIndex + i) * 4 + 1] = c.g;
                colorBuffer[(vertexIndex + i) * 4 + 2] = c.b;
                colorBuffer[(vertexIndex + i) * 4 + 3] = c.a;
            }

            DVASSERT((indexIndex + count * 3) < (MAX_VERTEXES * 3 / 2));
            uint32 idx = 0;
            for (int32 i = 2; i < count; ++i)
            {
                indexBuffer[indexIndex + idx + 0] = vertexIndex + 0;
                indexBuffer[indexIndex + idx + 1] = vertexIndex + i - 1;
                indexBuffer[indexIndex + idx + 2] = vertexIndex + i;
                idx += 3;
            }

            spriteIndexCount = idx;
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
        Shader * shader = state->shader;

        if (RenderManager::TEXTURE_MUL_FLAT_COLOR == shader)
        {
            shader = RenderManager::TEXTURE_MUL_COLOR;
        }
        else if (RenderManager::TEXTURE_MUL_FLAT_COLOR_ALPHA_TEST == shader)
        {
            shader = RenderManager::TEXTURE_MUL_COLOR_ALPHA_TEST;
        }
        else if (RenderManager::TEXTURE_MUL_FLAT_COLOR_IMAGE_A8 == shader)
        {
            shader = RenderManager::TEXTURE_MUL_COLOR_IMAGE_A8;
        }

        if (currentBatch.renderState != state->renderState
            || currentBatch.textureHandle != sprite->GetTextureHandle(state->frame)
            || currentBatch.shader != shader
            || currentBatch.clipRect != clipRect)
        {
            if (currentBatch.count > 0)
            {
                batches.push_back(currentBatch);
            }
            currentBatch.Reset();
        
            currentBatch.renderState = state->renderState;
            currentBatch.textureHandle = sprite->GetTextureHandle(state->frame);
            currentBatch.shader = shader;
            currentBatch.clipRect = clipRect;
            currentBatch.indeces = indexIndex;
        }
        
        currentBatch.count += spriteIndexCount;
        indexIndex += spriteIndexCount;
        vertexIndex += spriteVertexCount;

        DVASSERT(indexIndex < (MAX_VERTEXES * 3 / 2));
        DVASSERT(vertexIndex < MAX_VERTEXES);

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

    const float32 resMulFactor = 1.0f / VirtualCoordinates::GetResourceToVirtualFactor(sprite->GetResourceSizeIndex());
//	if (spr->IsUseContentScale())
//	{
        texDx *= resMulFactor;
        texDy *= resMulFactor;
//	}

    const float32 leftCap  = realLeftStretchCap   * resMulFactor;
    const float32 rightCap = realRightStretchCap  * resMulFactor;
    const float32 topCap   = realTopStretchCap    * resMulFactor;
    const float32 bottomCap= realBottomStretchCap * resMulFactor;

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

    static uint32 indeces[18 * 3] =
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
		for(uint8 i = 0; i < 32; ++i)
		{
			vertexBuffer[vertexIndex * 2 + i] = vertices[i];
			texBuffer[vertexIndex * 2 + i] = texCoords[i];
		}
		spriteVertexCount = 16;

		const Color c = RenderManager::Instance()->GetColor();
        for (uint8 i = 0; i < 64; ++i)
        {
            colorBuffer[vertexIndex * 4 + i] = c.color[i % 4];
        }

        DVASSERT(indexIndex + 54 < (MAX_VERTEXES * 3 / 2));
        for(uint8 i = 0; i < 54; ++i)
		{
			indexBuffer[indexIndex + i] = vertexIndex + indeces[i];
		}
		spriteIndexCount = 54;
		
		if (currentBatch.renderState != state->renderState
            || currentBatch.textureHandle != sprite->GetTextureHandle(state->frame)
            || currentBatch.shader != RenderManager::TEXTURE_MUL_COLOR
            || currentBatch.clipRect != currentClip)
        {
            if (currentBatch.count > 0)
            {
                batches.push_back(currentBatch);
            }
            currentBatch.Reset();
        
            currentBatch.renderState = state->renderState;
            currentBatch.textureHandle = sprite->GetTextureHandle(state->frame);
            currentBatch.shader = RenderManager::TEXTURE_MUL_COLOR;
            currentBatch.clipRect = currentClip;
            currentBatch.indeces = indexIndex;
        }
        
        currentBatch.count += spriteIndexCount;
        indexIndex += spriteIndexCount;
        vertexIndex += spriteVertexCount;

        DVASSERT(indexIndex < (MAX_VERTEXES * 3 / 2));
        DVASSERT(vertexIndex < MAX_VERTEXES);
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
		RenderManager::Instance()->DrawElements(PRIMITIVETYPE_TRIANGLELIST, vertInTriCount, EIF_32, indeces);
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
		spriteVertexCount = td.transformedVertices.size();
		Memcpy(&vertexBuffer[vertexIndex * 2], &td.transformedVertices[0], sizeof(float32) * spriteVertexCount * 2);
		Memcpy(&texBuffer[vertexIndex * 2], &td.texCoords[0], sizeof(float32) * spriteVertexCount * 2);

		const Color c = RenderManager::Instance()->GetColor();
        for (int32 i = 0; i < spriteVertexCount; ++i)
        {
            Memcpy(&colorBuffer[(vertexIndex + i) * 4], c.color, sizeof(float32) * 4);
        }

        spriteIndexCount = td.indeces.size();
        for (int32 i = 0; i < spriteIndexCount; ++i)
        {
            indexBuffer[indexIndex + i] = vertexIndex + td.indeces[i];
        }
		
		if (currentBatch.renderState != state->renderState
            || currentBatch.textureHandle != sprite->GetTextureHandle(state->frame)
            || currentBatch.shader != RenderManager::TEXTURE_MUL_COLOR
            || currentBatch.clipRect != currentClip)
        {
            if (currentBatch.count > 0)
            {
                batches.push_back(currentBatch);
            }
            currentBatch.Reset();
        
            currentBatch.renderState = state->renderState;
            currentBatch.textureHandle = sprite->GetTextureHandle(state->frame);
            currentBatch.shader = RenderManager::TEXTURE_MUL_COLOR;
            currentBatch.clipRect = currentClip;
            currentBatch.indeces = indexIndex;
        }
        
        currentBatch.count += spriteIndexCount;
        indexIndex += spriteIndexCount;
        vertexIndex += spriteVertexCount;

        DVASSERT(indexIndex < (MAX_VERTEXES * 3 / 2));
        DVASSERT(vertexIndex < MAX_VERTEXES);

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
		RenderManager::Instance()->DrawElements(PRIMITIVETYPE_TRIANGLELIST, td.indeces.size(), EIF_32, &td.indeces[0]);
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
    float32 spriteResToVirFactor = VirtualCoordinates::GetResourceToVirtualFactor(sprite->GetResourceSizeIndex());
    GenerateAxisData( size.x, sprite->GetRectOffsetValueForFrame(frame, Sprite::ACTIVE_WIDTH), (float32)texture->GetWidth() * spriteResToVirFactor, stretchCap.x, cellsWidth );

    Vector< Vector3 > cellsHeight;
    GenerateAxisData( size.y, sprite->GetRectOffsetValueForFrame(frame, Sprite::ACTIVE_HEIGHT), (float32)texture->GetHeight() * spriteResToVirFactor, stretchCap.y, cellsHeight );

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