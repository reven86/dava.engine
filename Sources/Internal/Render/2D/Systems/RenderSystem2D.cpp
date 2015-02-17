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
#include "Render/ShaderCache.h"

namespace DAVA
{

FastName RenderSystem2D::FLAT_COLOR_SHADER("~res:/Shaders/renderer2dColor");
FastName RenderSystem2D::TEXTURE_FLAT_COLOR_SHADER("~res:/Shaders/renderer2dTexture");

Shader * RenderSystem2D::FLAT_COLOR = 0;
Shader * RenderSystem2D::TEXTURE_MUL_FLAT_COLOR = 0;
Shader * RenderSystem2D::TEXTURE_MUL_FLAT_COLOR_ALPHA_TEST = 0;
Shader * RenderSystem2D::TEXTURE_MUL_FLAT_COLOR_IMAGE_A8 = 0;
Shader * RenderSystem2D::TEXTURE_ADD_FLAT_COLOR = 0;
Shader * RenderSystem2D::TEXTURE_ADD_FLAT_COLOR_ALPHA_TEST = 0;
Shader * RenderSystem2D::TEXTURE_ADD_FLAT_COLOR_IMAGE_A8 = 0;

RenderSystem2D::RenderSystem2D() :
spriteRenderObject(0),
spriteVertexStream(0),
spriteTexCoordStream(0),
currentRenderTarget(0),
spriteClipping(true),
clipChanged(false)
{
}

void RenderSystem2D::Init()
{
    if (!spriteRenderObject) //used as flag 'isInited'
    {
        spriteRenderObject = new RenderDataObject();
        spriteVertexStream = spriteRenderObject->SetStream(EVF_VERTEX, TYPE_FLOAT, 2, 0, 0);
        spriteTexCoordStream = spriteRenderObject->SetStream(EVF_TEXCOORD0, TYPE_FLOAT, 2, 0, 0);

        FLAT_COLOR = SafeRetain(ShaderCache::Instance()->Get(FLAT_COLOR_SHADER, FastNameSet()));
        TEXTURE_MUL_FLAT_COLOR = SafeRetain(ShaderCache::Instance()->Get(TEXTURE_FLAT_COLOR_SHADER, FastNameSet()));

        FastNameSet set;
        set.Insert(FastName("ALPHA_TEST_ENABLED"));
        TEXTURE_MUL_FLAT_COLOR_ALPHA_TEST = SafeRetain(ShaderCache::Instance()->Get(TEXTURE_FLAT_COLOR_SHADER, set));

        set.clear();
        set.Insert(FastName("IMAGE_A8"));
        TEXTURE_MUL_FLAT_COLOR_IMAGE_A8 = SafeRetain(ShaderCache::Instance()->Get(TEXTURE_FLAT_COLOR_SHADER, set));

        set.clear();
        set.Insert(FastName("ADD_COLOR"));
        TEXTURE_ADD_FLAT_COLOR = SafeRetain(ShaderCache::Instance()->Get(TEXTURE_FLAT_COLOR_SHADER, set));

        set.clear();
        set.Insert(FastName("ADD_COLOR"));
        set.Insert(FastName("ALPHA_TEST_ENABLED"));
        TEXTURE_ADD_FLAT_COLOR_ALPHA_TEST = SafeRetain(ShaderCache::Instance()->Get(TEXTURE_FLAT_COLOR_SHADER, set));

        set.clear();
        set.Insert(FastName("ADD_COLOR"));
        set.Insert(FastName("IMAGE_A8"));
        TEXTURE_ADD_FLAT_COLOR_IMAGE_A8 = SafeRetain(ShaderCache::Instance()->Get(TEXTURE_FLAT_COLOR_SHADER, set));
    }
}

RenderSystem2D::~RenderSystem2D()
{
    SafeRelease(spriteRenderObject);

    SafeRelease(FLAT_COLOR);
    SafeRelease(TEXTURE_MUL_FLAT_COLOR);
    SafeRelease(TEXTURE_MUL_FLAT_COLOR_ALPHA_TEST);
    SafeRelease(TEXTURE_MUL_FLAT_COLOR_IMAGE_A8);
    SafeRelease(TEXTURE_ADD_FLAT_COLOR);
    SafeRelease(TEXTURE_ADD_FLAT_COLOR_ALPHA_TEST);
    SafeRelease(TEXTURE_ADD_FLAT_COLOR_IMAGE_A8);
}

void RenderSystem2D::Reset()
{
    currentClip.x = 0;
    currentClip.y = 0;
    currentClip.dx = -1;
    currentClip.dy = -1;
    
    Setup2DMatrices();

    defaultSpriteDrawState.Reset();
    defaultSpriteDrawState.renderState = RenderState::RENDERSTATE_2D_BLEND;
    defaultSpriteDrawState.shader = TEXTURE_MUL_FLAT_COLOR;
}

void RenderSystem2D::Setup2DProjection()
{
    Matrix4 glOrtho;
    if (currentRenderTarget)
    {
        glOrtho.glOrtho(0.0f, (float32)currentRenderTarget->GetTexture()->GetWidth(),
                        0.0f, (float32)currentRenderTarget->GetTexture()->GetHeight(),
                       -1.0f, 1.0f);
    }
    else
    {
        Size2i targetSize = RenderManager::Instance()->GetFramebufferSize();
        glOrtho.glOrtho(0.0f, (float32)targetSize.dx, (float32)targetSize.dy, 0.0f, -1.0f, 1.0f);
    }

    projMatrix = virtualToPhysicalMatrix * glOrtho;
    RenderManager::SetDynamicParam(PARAM_PROJ, &projMatrix, UPDATE_SEMANTIC_ALWAYS);
}

void RenderSystem2D::Setup2DMatrices()
{
    RenderManager::SetDynamicParam(PARAM_WORLD, &Matrix4::IDENTITY, (pointer_size)&Matrix4::IDENTITY);
    RenderManager::SetDynamicParam(PARAM_VIEW, &Matrix4::IDENTITY, (pointer_size)&Matrix4::IDENTITY);
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
    currentClip = rect;
    clipChanged = true;
}

void RenderSystem2D::RemoveClip()
{
    SetClip(Rect(0.f, 0.f, -1.f, -1.f));
}

void RenderSystem2D::ClipRect(const Rect &rect)
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

void RenderSystem2D::UpdateClip()
{
    if (clipChanged)
    {
        if (currentClip.dx < 0.f || currentClip.dy < 0.f) //disable clip
        {
            RenderManager::Instance()->SetClip(currentClip);
        }
        else
        {
            RenderManager::ComputeWorldViewMatrixIfRequired();
            Matrix4 transformMx = RenderManager::GetDynamicParamMatrix(PARAM_VIEW) * virtualToPhysicalMatrix;

            Vector3 clipTopLeftCorner(currentClip.x, currentClip.y, 0.f);
            Vector3 clipBottomRightCorner(currentClip.x + currentClip.dx, currentClip.y + currentClip.dy, 0.f);

            clipTopLeftCorner = clipTopLeftCorner * transformMx;
            clipBottomRightCorner = clipBottomRightCorner * transformMx;

            Rect transformedClip(Vector2(clipTopLeftCorner.data), Vector2((clipBottomRightCorner - clipTopLeftCorner).data));
            RenderManager::Instance()->SetClip(transformedClip);
        }
        clipChanged = false;
    }
}

void RenderSystem2D::PushRenderTarget()
{
    renderTargetStack.push(currentRenderTarget);
}

void RenderSystem2D::PopRenderTarget()
{
    DVASSERT(renderTargetStack.size());

    SetRenderTarget(renderTargetStack.top());
    renderTargetStack.pop();
}

void RenderSystem2D::SetRenderTarget(Sprite * target)
{
    currentRenderTarget = target;

    Rect viewport;
    if (currentRenderTarget)
    {
        viewport.dx = (float32)currentRenderTarget->GetTexture()->GetWidth();
        viewport.dy = (float32)currentRenderTarget->GetTexture()->GetHeight();
        RenderManager::Instance()->SetRenderTarget(currentRenderTarget->GetTexture());
    }
    else
    {
        Size2i framebufferSize = RenderManager::Instance()->GetFramebufferSize();
        viewport.dx = (float32)framebufferSize.dx;
        viewport.dy = (float32)framebufferSize.dy;
        RenderManager::Instance()->SetRenderTarget(0);
    }

    RenderManager::Instance()->SetViewport(viewport);

    Setup2DProjection();
}

void RenderSystem2D::SetSpriteClipping(bool clipping)
{
    spriteClipping = clipping;
}

void RenderSystem2D::Draw(Sprite * sprite, Sprite::DrawState * drawState /* = 0 */)
{
    if(!RenderManager::Instance()->GetOptions()->IsOptionEnabled(RenderOptions::SPRITE_DRAW))
    {
        return;
    }

    Sprite::DrawState * state = drawState;
    if (!state)
    {
        state = &defaultSpriteDrawState;
    }

    PrepareSpriteRenderData(sprite, state);

    bool spriteIsOnScreen = (sprite->clipPolygon) ? true : (!spriteClipping || IsPreparedSpriteOnScreen(state));
    if (spriteIsOnScreen)
    {
        if (sprite->clipPolygon)
        {
            PushClip();
            Rect clipRect;
            if (sprite->flags & Sprite::EST_SCALE)
            {
                float32 x = state->position.x - state->pivotPoint.x * state->scale.x;
                float32 y = state->position.y - state->pivotPoint.y * state->scale.y;
                clipRect = Rect(sprite->GetRectOffsetValueForFrame(state->frame, Sprite::X_OFFSET_TO_ACTIVE) * state->scale.x + x
                    , sprite->GetRectOffsetValueForFrame(state->frame, Sprite::Y_OFFSET_TO_ACTIVE) * state->scale.y + y
                    , sprite->GetRectOffsetValueForFrame(state->frame, Sprite::ACTIVE_WIDTH) * state->scale.x
                    , sprite->GetRectOffsetValueForFrame(state->frame, Sprite::ACTIVE_HEIGHT) * state->scale.y);
            }
            else
            {
                float32 x = state->position.x - state->pivotPoint.x;
                float32 y = state->position.y - state->pivotPoint.y;
                clipRect = Rect(sprite->GetRectOffsetValueForFrame(state->frame, Sprite::X_OFFSET_TO_ACTIVE) + x
                    , sprite->GetRectOffsetValueForFrame(state->frame, Sprite::Y_OFFSET_TO_ACTIVE) + y
                    , sprite->GetRectOffsetValueForFrame(state->frame, Sprite::ACTIVE_WIDTH)
                    , sprite->GetRectOffsetValueForFrame(state->frame, Sprite::ACTIVE_HEIGHT));
            }

            ClipRect(clipRect);
        }

        UpdateClip();

        RenderManager::Instance()->SetRenderState(state->renderState);
        RenderManager::Instance()->SetTextureState(sprite->GetTextureHandle(state->frame));
        RenderManager::Instance()->SetRenderData(spriteRenderObject);
        RenderManager::Instance()->SetRenderEffect(state->shader);
        RenderManager::Instance()->DrawArrays(spritePrimitiveToDraw, 0, spriteVertexCount);

        if (sprite->clipPolygon)
        {
            PopClip();
        }
    }
}

bool RenderSystem2D::IsPreparedSpriteOnScreen(Sprite::DrawState * drawState)
{
    if (RenderManager::Instance()->IsRenderTarget())
        return true;

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

void RenderSystem2D::PrepareSpriteRenderData(Sprite * sprite, Sprite::DrawState * state)
{
    DVASSERT(state);
    
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
                if(!state->usePerPixelAccuracy || (sprite->flags & Sprite::EST_ROTATE))
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
                if(!state->usePerPixelAccuracy || (sprite->flags & Sprite::EST_ROTATE))
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
                    if(!state->usePerPixelAccuracy || (sprite->flags & Sprite::EST_ROTATE))
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
                    if(!state->usePerPixelAccuracy || (sprite->flags & Sprite::EST_ROTATE))
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
                    if(!state->usePerPixelAccuracy || (sprite->flags & Sprite::EST_ROTATE))
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
                    if(!state->usePerPixelAccuracy || (sprite->flags & Sprite::EST_ROTATE))
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
            if(!state->usePerPixelAccuracy || (sprite->flags & Sprite::EST_ROTATE))
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
            if(!state->usePerPixelAccuracy || (sprite->flags & Sprite::EST_ROTATE))
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
        
        spriteVertexStream->Set(TYPE_FLOAT, 2, 0, spriteTempVertices);
        spriteTexCoordStream->Set(TYPE_FLOAT, 2, 0, sprite->texCoords[frame]);
        spritePrimitiveToDraw = PRIMITIVETYPE_TRIANGLESTRIP;
        spriteVertexCount = 4;
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
        
        if( sprite->flags & Sprite::EST_SCALE )
        {
            for(int32 i = 0; i < sprite->clipPolygon->GetPointCount(); ++i)
            {
                const Vector2 &point = sprite->clipPolygon->GetPoints()[i];
                spriteClippedVertices.push_back( Vector2( point.x*scaleX + x, point.y*scaleY + y ) );
            }
        }
        else
        {
            Vector2 pos(x, y);
            for(int32 i = 0; i < sprite->clipPolygon->GetPointCount(); ++i)
            {
                const Vector2 &point = sprite->clipPolygon->GetPoints()[i];
                spriteClippedVertices.push_back( point + pos );
            }
        }
        
        for( int32 i = 0; i < sprite->clipPolygon->GetPointCount(); ++i )
        {
            const Vector2 &point = sprite->clipPolygon->GetPoints()[i];
            
            Vector2 texCoord( ( point.x - frameVertices[frame][0] ) * adjWidth
                             , ( point.y - frameVertices[frame][1] ) * adjHeight );
            
            spriteClippedTexCoords.push_back( Vector2( sprite->texCoords[frame][0] + texCoord.x
                                                , sprite->texCoords[frame][1] + texCoord.y ) );
        }
        
        spriteVertexStream->Set(TYPE_FLOAT, 2, 0, &spriteClippedVertices.front());
        spriteTexCoordStream->Set(TYPE_FLOAT, 2, 0, &spriteClippedTexCoords.front());
        spritePrimitiveToDraw = PRIMITIVETYPE_TRIANGLEFAN;
        spriteVertexCount = sprite->clipPolygon->pointCount;
    }
    
    DVASSERT(spriteVertexStream->pointer != 0);
    DVASSERT(spriteTexCoordStream->pointer != 0);
}
};