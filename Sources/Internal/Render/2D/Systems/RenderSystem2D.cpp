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
#include "Render/DynamicBufferAllocator.h"

#include "Render/ShaderCache.h"

namespace DAVA
{

static const uint32 MAX_VERTICES = 1024;
static const uint32 MAX_INDECES = MAX_VERTICES * 2;
static const uint32 VBO_FORMAT = EVF_VERTEX | EVF_TEXCOORD0 | EVF_COLOR;
static const uint32 VBO_STRIDE = GetVertexSize(VBO_FORMAT);
static const float32 SEGMENT_LENGTH = 15.0f;

const FastName RenderSystem2D::RENDER_PASS_NAME("2d");
const FastName RenderSystem2D::FLAG_COLOR_OP("COLOR_OP");

NMaterial* RenderSystem2D::DEFAULT_2D_COLOR_MATERIAL = nullptr;
NMaterial* RenderSystem2D::DEFAULT_2D_TEXTURE_MATERIAL = nullptr;
NMaterial* RenderSystem2D::DEFAULT_2D_TEXTURE_NOBLEND_MATERIAL = nullptr;
NMaterial* RenderSystem2D::DEFAULT_2D_TEXTURE_ALPHA8_MATERIAL = nullptr;
NMaterial* RenderSystem2D::DEFAULT_2D_TEXTURE_GRAYSCALE_MATERIAL = nullptr;

RenderSystem2D::RenderSystem2D() 
    : currentVertexBuffer(nullptr)
    , currentIndexBuffer(nullptr)
    , indexIndex(0)
    , vertexIndex(0)
    , spriteClipping(false)
    , spriteIndexCount(0)
    , spriteVertexCount(0)    
    , prevFrameErrorsFlags(NO_ERRORS)
    , currFrameErrorsFlags(NO_ERRORS)
    , highlightControlsVerticesLimit(0)
    , renderTargetWidth(0)
    , renderTargetHeight(0)
{
}

void RenderSystem2D::Init()
{
    DEFAULT_2D_COLOR_MATERIAL = new NMaterial();
    DEFAULT_2D_COLOR_MATERIAL->SetFXName(FastName("~res:/Materials/2d.Color.material"));
    DEFAULT_2D_COLOR_MATERIAL->PreBuildMaterial(RENDER_PASS_NAME);

    DEFAULT_2D_TEXTURE_MATERIAL = new NMaterial();
    DEFAULT_2D_TEXTURE_MATERIAL->SetFXName(FastName("~res:/Materials/2d.Textured.material"));
    DEFAULT_2D_TEXTURE_MATERIAL->PreBuildMaterial(RENDER_PASS_NAME);
    
    DEFAULT_2D_TEXTURE_NOBLEND_MATERIAL = new NMaterial();
    DEFAULT_2D_TEXTURE_NOBLEND_MATERIAL->SetFXName(FastName("~res:/Materials/2d.Textured.material"));
    DEFAULT_2D_TEXTURE_NOBLEND_MATERIAL->AddFlag(NMaterialFlagName::FLAG_BLENDING, BLENDING_NONE);
    DEFAULT_2D_TEXTURE_NOBLEND_MATERIAL->PreBuildMaterial(RENDER_PASS_NAME);
    
    DEFAULT_2D_TEXTURE_ALPHA8_MATERIAL = new NMaterial();
    DEFAULT_2D_TEXTURE_ALPHA8_MATERIAL->SetFXName(FastName("~res:/Materials/2d.Textured.Alpha8.material"));
    DEFAULT_2D_TEXTURE_ALPHA8_MATERIAL->PreBuildMaterial(RENDER_PASS_NAME);

    DEFAULT_2D_TEXTURE_GRAYSCALE_MATERIAL = new NMaterial();
    DEFAULT_2D_TEXTURE_GRAYSCALE_MATERIAL->SetFXName(FastName("~res:/Materials/2d.Textured.Grayscale.material"));
    DEFAULT_2D_TEXTURE_GRAYSCALE_MATERIAL->PreBuildMaterial(RENDER_PASS_NAME);

    rhi::VertexLayout layout;
    layout.AddElement(rhi::VS_POSITION, 0, rhi::VDT_FLOAT, 3);
    layout.AddElement(rhi::VS_TEXCOORD, 0, rhi::VDT_FLOAT, 2);
    layout.AddElement(rhi::VS_COLOR, 0, rhi::VDT_UINT8N, 4);

    currentVertexBuffer = nullptr;
    currentIndexBuffer = nullptr;
    currentIndexBase = 0;
    currentPacket.primitiveCount = 0;
    currentPacket.vertexStreamCount = 1;
    currentPacket.options = 0;
    currentPacket.vertexLayoutUID = rhi::VertexLayout::UniqueId(layout);

    vertexIndex = 0;
    indexIndex = 0;

    lastMaterial = nullptr;
    lastUsedCustomWorldMatrix = false;
    lastCustomWorldMatrix = Matrix4::IDENTITY;
    lastCustomMatrixSematic = 1;
    lastClip = Rect(0, 0, -1, -1);
    
}

RenderSystem2D::~RenderSystem2D()
{
    SafeRelease(DEFAULT_2D_COLOR_MATERIAL);
    SafeRelease(DEFAULT_2D_TEXTURE_MATERIAL);
    SafeRelease(DEFAULT_2D_TEXTURE_NOBLEND_MATERIAL);
    SafeRelease(DEFAULT_2D_TEXTURE_ALPHA8_MATERIAL);
    SafeRelease(DEFAULT_2D_TEXTURE_GRAYSCALE_MATERIAL);
}

void RenderSystem2D::BeginFrame()
{
    currentClip.x = 0;
    currentClip.y = 0;
    currentClip.dx = -1;
    currentClip.dy = -1;

    defaultSpriteDrawState.Reset();
    defaultSpriteDrawState.material = DEFAULT_2D_COLOR_MATERIAL;    

    rhi::RenderPassConfig renderPass2DConfig;
    renderPass2DConfig.priority = PRIORITY_MAIN_2D;
    renderPass2DConfig.colorBuffer[0].loadAction = rhi::LOADACTION_LOAD;
    renderPass2DConfig.colorBuffer[0].storeAction = rhi::STOREACTION_STORE;
    renderPass2DConfig.depthStencilBuffer.loadAction = rhi::LOADACTION_CLEAR;
    renderPass2DConfig.depthStencilBuffer.storeAction = rhi::STOREACTION_NONE;
    renderPass2DConfig.viewport.x = renderPass2DConfig.viewport.y = 0;
    renderPass2DConfig.viewport.width = Renderer::GetFramebufferWidth();
    renderPass2DConfig.viewport.height = Renderer::GetFramebufferHeight();

    pass2DHandle = rhi::AllocateRenderPass(renderPass2DConfig, 1, &packetList2DHandle);
    currentPacketListHandle = packetList2DHandle;

    rhi::BeginRenderPass(pass2DHandle);
    rhi::BeginPacketList(currentPacketListHandle);

    ShaderDescriptorCache::ClearDynamicBindigs();
    Setup2DMatrices();
}

void RenderSystem2D::EndFrame()
{
    Flush();
    prevFrameErrorsFlags = currFrameErrorsFlags;
    currFrameErrorsFlags = 0;

    rhi::EndPacketList(currentPacketListHandle);
    rhi::EndRenderPass(pass2DHandle);
}

void RenderSystem2D::BeginRenderTargetPass(Texture * target, bool needClear /* = true */, const Color& clearColor /* = Color::Clear */, int32 priority /* = PRIORITY_SERVICE_2D */)
{
    DVASSERT(!IsRenderTargetPass());
    DVASSERT(target);
    DVASSERT(target->GetWidth() && target->GetHeight())

    Flush();

    rhi::RenderPassConfig renderTargetPassConfig;
    renderTargetPassConfig.colorBuffer[0].texture = target->handle;
    renderTargetPassConfig.colorBuffer[0].clearColor[0] = clearColor.r;
    renderTargetPassConfig.colorBuffer[0].clearColor[1] = clearColor.g;
    renderTargetPassConfig.colorBuffer[0].clearColor[2] = clearColor.b;
    renderTargetPassConfig.colorBuffer[0].clearColor[3] = clearColor.a;
    renderTargetPassConfig.priority = priority;
    renderTargetPassConfig.viewport.width = target->GetWidth();
    renderTargetPassConfig.viewport.height = target->GetHeight();
    renderTargetPassConfig.depthStencilBuffer.texture = rhi::InvalidHandle;
    renderTargetPassConfig.colorBuffer[0].storeAction = rhi::STOREACTION_STORE;
    if(needClear)
        renderTargetPassConfig.colorBuffer[0].loadAction = rhi::LOADACTION_CLEAR;
    else
        renderTargetPassConfig.colorBuffer[0].loadAction = rhi::LOADACTION_LOAD;
    
    passTargetHandle = rhi::AllocateRenderPass(renderTargetPassConfig, 1, &currentPacketListHandle);

    rhi::BeginRenderPass(passTargetHandle);
    rhi::BeginPacketList(currentPacketListHandle);

    renderTargetWidth = target->GetWidth();
    renderTargetHeight = target->GetHeight();

    ShaderDescriptorCache::ClearDynamicBindigs();
    Setup2DMatrices();
}

void RenderSystem2D::EndRenderTargetPass()
{
    DVASSERT(IsRenderTargetPass());

    Flush();

    rhi::EndPacketList(currentPacketListHandle);
    rhi::EndRenderPass(passTargetHandle);

    currentPacketListHandle = packetList2DHandle;

    renderTargetWidth = 0;
    renderTargetHeight = 0;

    ShaderDescriptorCache::ClearDynamicBindigs();
    Setup2DMatrices();
}

void RenderSystem2D::Setup2DMatrices()
{
    if (IsRenderTargetPass())
    {
        if (rhi::DeviceCaps().isUpperLeftRTOrigin)
        {
            projMatrix.glOrtho(0.0f, (float32)renderTargetWidth,
                (float32)renderTargetHeight, 0.f,
                -1.0f, 1.0f, rhi::DeviceCaps().isZeroBaseClipRange);
        }
        else
        {
            projMatrix.glOrtho(0.0f, (float32)renderTargetWidth,
                0.0f, (float32)renderTargetHeight,
                -1.0f, 1.0f, rhi::DeviceCaps().isZeroBaseClipRange);
        }
    }
    else
    {
        projMatrix.glOrtho(0.0f, (float32)Renderer::GetFramebufferWidth(), (float32)Renderer::GetFramebufferHeight(), 0.0f, -1.0f, 1.0f, rhi::DeviceCaps().isZeroBaseClipRange);
    }

    if (rhi::DeviceCaps().isCenterPixelMapping)
    {
        // Make translation by half pixel for DirectX systems
        static Matrix4 pixelMappingMatrix = Matrix4::MakeTranslation(Vector3(-0.5f, -0.5f, 0.f));
        projMatrix = virtualToPhysicalMatrix * pixelMappingMatrix * projMatrix;
    }
    else
    {
        projMatrix = virtualToPhysicalMatrix * projMatrix;
    }

    projMatrixSemantic += 8; //cause eight is beautiful
    //actually, is not +=1 cause DynamicParams for UPADATE_ALWAYS_SEMANTIC increment by one last binded value.
    //TODO: need to rethink semantic for projection matrix in RenderSystem2D, or maybe need to rethink semantics for DynamicParams
}

void RenderSystem2D::ScreenSizeChanged()
{
    Matrix4 glTranslate, glScale;

    Vector2 scale = VirtualCoordinatesSystem::Instance()->ConvertVirtualToPhysical(Vector2(1.f, 1.f));
    Vector2 realDrawOffset = VirtualCoordinatesSystem::Instance()->GetPhysicalDrawOffset();

    glTranslate.glTranslate(realDrawOffset.x, realDrawOffset.y, 0.0f);
    glScale.glScale(scale.x, scale.y, 1.0f);

    virtualToPhysicalMatrix = glScale * glTranslate;
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
        //RHI_COMPLETE - Mikhail please review this
        Rect screen(0.0f, 0.0f, 
			IsRenderTargetPass() ? (float32)renderTargetWidth : VirtualCoordinatesSystem::Instance()->GetVirtualScreenSize().dx, 
			IsRenderTargetPass() ? (float32)renderTargetHeight : VirtualCoordinatesSystem::Instance()->GetVirtualScreenSize().dy);
        Rect res = screen.Intersection(rect);
        if (res.dx == 0)
        {
            res.x = 0;
            res.dx = 1;
        }
        if (res.dy == 0)
        {
            res.y = 0;
            res.dy = 1;
        }
        SetClip(res);
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

Rect RenderSystem2D::TransformClipRect(const Rect & rect, const Matrix4 & transformMatrix)
{
    Vector3 clipTopLeftCorner(rect.x, rect.y, 0.f);
    Vector3 clipBottomRightCorner(rect.x + rect.dx, rect.y + rect.dy, 0.f);
    clipTopLeftCorner = clipTopLeftCorner * transformMatrix;
    clipBottomRightCorner = clipBottomRightCorner * transformMatrix;
    Rect resRect = Rect(Vector2(clipTopLeftCorner.data), Vector2((clipBottomRightCorner - clipTopLeftCorner).data));
    if (resRect.x < 0.f)
    {
        resRect.x = 0;
    }
    if (resRect.y < 0.f)
    {
        resRect.y = 0;
    }
    return resRect;
}

void RenderSystem2D::SetSpriteClipping(bool clipping)
{
    spriteClipping = clipping;
}

bool RenderSystem2D::IsPreparedSpriteOnScreen(Sprite::DrawState * drawState)
{
    Rect clipRect = currentClip;
    if (clipRect.dx == -1)
    {
        clipRect.dx = (float32)(IsRenderTargetPass() ? renderTargetWidth : VirtualCoordinatesSystem::Instance()->GetVirtualScreenSize().dx);
    }
    if (clipRect.dy == -1)
    {
        clipRect.dy = (float32)(IsRenderTargetPass() ? renderTargetHeight : VirtualCoordinatesSystem::Instance()->GetVirtualScreenSize().dy);
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
    
    if (currentPacket.primitiveCount > 0)
    {
        rhi::AddPacket(currentPacketListHandle, currentPacket);

#if defined(__DAVAENGINE_RENDERSTATS__)
        ++Renderer::GetRenderStats().packets2d;
#endif
    }

    currentVertexBuffer = nullptr;
    currentIndexBuffer = nullptr;

    currentPacket.vertexStream[0] = rhi::HVertexBuffer();
    currentPacket.vertexCount = 0;
    currentPacket.baseVertex = 0;
    currentPacket.indexBuffer = rhi::HIndexBuffer();
    currentIndexBase = 0;
    currentPacket.primitiveCount = 0;

    vertexIndex = 0;
    indexIndex = 0;
    lastMaterial = nullptr;
}

void RenderSystem2D::DrawPacket(rhi::Packet& packet)
{
    if (currentClip.dx == 0.f || currentClip.dy == 0.f)
    {
        // Ignore draw if clip has zero width or height
        return;
    }
    Flush();
    if (currentClip.dx > 0.f && currentClip.dy > 0.f)
    {
        const Rect& transformedClipRect = TransformClipRect(currentClip, virtualToPhysicalMatrix);
        packet.scissorRect.x = (int16)transformedClipRect.x;
        packet.scissorRect.y = (int16)transformedClipRect.y;
        packet.scissorRect.width = (int16)ceilf(transformedClipRect.dx);
        packet.scissorRect.height = (int16)ceilf(transformedClipRect.dy);
        packet.options |= rhi::Packet::OPT_OVERRIDE_SCISSOR;
    }
    rhi::AddPacket(currentPacketListHandle, packet);

#if defined(__DAVAENGINE_RENDERSTATS__)
    ++Renderer::GetRenderStats().packets2d;
#endif
}

void RenderSystem2D::PushBatch(const BatchDescriptor& batchDesc)
{
    DVASSERT_MSG(batchDesc.vertexCount > 0 && batchDesc.vertexStride > 0 && batchDesc.vertexPointer != nullptr, "Incorrect vertex position data");
    DVASSERT_MSG(batchDesc.indexCount > 0 && batchDesc.indexPointer != nullptr, "Incorrect index data");
    DVASSERT_MSG(batchDesc.material != nullptr, "Incorrect material");
    DVASSERT_MSG((batchDesc.samplerStateHandle != rhi::InvalidHandle && batchDesc.textureSetHandle != rhi::InvalidHandle) ||
                (batchDesc.samplerStateHandle == rhi::InvalidHandle && batchDesc.textureSetHandle == rhi::InvalidHandle), 
                "Incorrect textureSet or samplerState handle");

    DVASSERT_MSG(batchDesc.texCoordPointer == nullptr || batchDesc.texCoordStride > 0, "Incorrect vertex texture coordinates data");
    DVASSERT_MSG(batchDesc.colorPointer == nullptr || batchDesc.colorStride > 0, "Incorrect vertex color data");

    if (currentClip.dx == 0.f || currentClip.dy == 0.f)
    {
        // Ignore draw if clip has zero width or height.
        // For disable clip and this check use Rect(0,0,-1,-1)
        return;
    }

#if defined(__DAVAENGINE_RENDERSTATS__)
    ++Renderer::GetRenderStats().batches2d;
#endif

    if ((vertexIndex + batchDesc.vertexCount > MAX_VERTICES) || (indexIndex + batchDesc.indexCount > MAX_INDECES))
    {
        // Buffer overflow. Switch to next VBO.
        Flush();

        // TODO: Make draw for big buffers (bigger than buffers in pool)
        // Draw immediately if batch is too big to buffer
        if (batchDesc.vertexCount > MAX_VERTICES || batchDesc.indexCount > MAX_INDECES)
        {
            if (((prevFrameErrorsFlags & BUFFER_OVERFLOW_ERROR) != BUFFER_OVERFLOW_ERROR))
            {
                Logger::Warning("PushBatch: Too much vertices (%d of %d)! Direct draw.", batchDesc.vertexCount, MAX_VERTICES);
            }
            currFrameErrorsFlags |= BUFFER_OVERFLOW_ERROR;

            // Begin create vertex and index buffers
            DVASSERT(currentVertexBuffer == nullptr && currentIndexBuffer == nullptr);
            if (currentVertexBuffer == nullptr && currentIndexBuffer == nullptr)
            {
                DynamicBufferAllocator::AllocResultVB vertexBuffer = DynamicBufferAllocator::AllocateVertexBuffer(VBO_STRIDE, batchDesc.vertexCount);
                DynamicBufferAllocator::AllocResultIB indexBuffer = DynamicBufferAllocator::AllocateIndexBuffer(batchDesc.indexCount);
                currentVertexBuffer = reinterpret_cast<BatchVertex*>(vertexBuffer.data);
                currentIndexBuffer = indexBuffer.data;

                currentPacket.vertexStream[0] = vertexBuffer.buffer;
                currentPacket.vertexCount = vertexBuffer.allocatedVertices;
                currentPacket.baseVertex = vertexBuffer.baseVertex;
                currentPacket.indexBuffer = indexBuffer.buffer;
                currentIndexBase = indexBuffer.baseIndex;
            }
            // End create vertex and index buffers
        }
    }

    // Begin create vertex and index buffers
    if (currentVertexBuffer == nullptr && currentIndexBuffer == nullptr)
    {
        DynamicBufferAllocator::AllocResultVB vertexBuffer = DynamicBufferAllocator::AllocateVertexBuffer(VBO_STRIDE, MAX_VERTICES);
        DynamicBufferAllocator::AllocResultIB indexBuffer = DynamicBufferAllocator::AllocateIndexBuffer(MAX_INDECES);
        currentVertexBuffer = reinterpret_cast<BatchVertex*>(vertexBuffer.data);
        currentIndexBuffer = indexBuffer.data;

        currentPacket.vertexStream[0] = vertexBuffer.buffer;
        currentPacket.vertexCount = vertexBuffer.allocatedVertices;
        currentPacket.baseVertex = vertexBuffer.baseVertex;
        currentPacket.indexBuffer = indexBuffer.buffer;
        currentIndexBase = indexBuffer.baseIndex;
}
    // End create vertex and index buffers

    // Begin define draw color
    Color useColor = batchDesc.singleColor;
    if (highlightControlsVerticesLimit > 0
        && batchDesc.vertexCount > highlightControlsVerticesLimit
        && Renderer::GetOptions()->IsOptionEnabled(RenderOptions::HIGHLIGHT_HARD_CONTROLS))
    {
        // Highlight too big controls with magenta color
        static Color magenta = Color(1.f, 0.f, 1.f, 1.f);
        useColor = magenta;
    }
    // End define draw color

    // Begin fill vertex and index buffers
    uint32 vi = vertexIndex;
    uint32 ii = indexIndex;
    if (batchDesc.texCoordPointer)
    {
        for (uint32 i = 0; i < batchDesc.vertexCount; ++i)
        {
            BatchVertex& v = currentVertexBuffer[vi++];
            v.pos.x = batchDesc.vertexPointer[i * batchDesc.vertexStride];
            v.pos.y = batchDesc.vertexPointer[i * batchDesc.vertexStride + 1];
            v.pos.z = 0.f; // axis Z, empty but need for EVF_VERTEX format
            v.uv.x = batchDesc.texCoordPointer[i * batchDesc.texCoordStride];
            v.uv.y = batchDesc.texCoordPointer[i * batchDesc.texCoordStride + 1];
            v.color = rhi::NativeColorRGBA(useColor.r, useColor.g, useColor.b, useColor.a);
        }
    }
    else
    {
        for (uint32 i = 0; i < batchDesc.vertexCount; ++i)
        {
            BatchVertex& v = currentVertexBuffer[vi++];
            v.pos.x = batchDesc.vertexPointer[i * batchDesc.vertexStride];
            v.pos.y = batchDesc.vertexPointer[i * batchDesc.vertexStride + 1];
            v.pos.z = 0.f; // axis Z, empty but need for EVF_VERTEX format
            v.uv.x = 0.f;
            v.uv.y = 0.f;
            v.color = rhi::NativeColorRGBA(useColor.r, useColor.g, useColor.b, useColor.a);
        }
    }
    for (uint32 i = 0; i < batchDesc.indexCount; ++i)
    {
        currentIndexBuffer[ii++] = vertexIndex + batchDesc.indexPointer[i];
    }
    // End fill vertex and index buffers

    // Begin check world matrix
    bool needUpdateWorldMatrix = false;
    bool useCustomWorldMatrix = batchDesc.worldMatrix != nullptr;
    if (!useCustomWorldMatrix && !lastUsedCustomWorldMatrix) // Equal and False
    {
        // Skip check world matrices. Use Matrix4::IDENTITY. (the most frequent option)
    }
    else if (useCustomWorldMatrix && lastUsedCustomWorldMatrix) // Equal and True
    {
        if (lastCustomWorldMatrix != *batchDesc.worldMatrix) // Update only if matrices not equal
        {
            needUpdateWorldMatrix = true;
            lastCustomWorldMatrix = *batchDesc.worldMatrix;
            lastCustomMatrixSematic++;
        }
    }
    else // Not equal
    {
        needUpdateWorldMatrix = true;
        if (useCustomWorldMatrix)
        {
            lastCustomWorldMatrix = *batchDesc.worldMatrix;
            lastCustomMatrixSematic++;
        }
    }
    // End check world matrix

    // Begin new packet
    if (currentPacket.textureSet != batchDesc.textureSetHandle
        || currentPacket.primitiveType != batchDesc.primitiveType
        || lastMaterial != batchDesc.material
        || lastClip != currentClip
        || needUpdateWorldMatrix)
    {
        if (currentPacket.primitiveCount > 0)
        {
            rhi::AddPacket(currentPacketListHandle, currentPacket);
            currentPacket.primitiveCount = 0;

#if defined(__DAVAENGINE_RENDERSTATS__)
            ++Renderer::GetRenderStats().packets2d;
#endif
        }

        if (useCustomWorldMatrix)
        {
            Renderer::GetDynamicBindings().SetDynamicParam(DynamicBindings::PARAM_WORLD, &lastCustomWorldMatrix, static_cast<pointer_size>(lastCustomMatrixSematic));
        }
        else
        {
            Renderer::GetDynamicBindings().SetDynamicParam(DynamicBindings::PARAM_WORLD, &Matrix4::IDENTITY, reinterpret_cast<pointer_size>(&Matrix4::IDENTITY));
        }
        Renderer::GetDynamicBindings().SetDynamicParam(DynamicBindings::PARAM_PROJ, &projMatrix, static_cast<pointer_size>(projMatrixSemantic));
        Renderer::GetDynamicBindings().SetDynamicParam(DynamicBindings::PARAM_VIEW, &Matrix4::IDENTITY, reinterpret_cast<pointer_size>(&Matrix4::IDENTITY));

        if (currentClip.dx > 0.f && currentClip.dy > 0.f)
        {
            const Rect& transformedClipRect = TransformClipRect(currentClip, virtualToPhysicalMatrix);
            currentPacket.scissorRect.x = (int16)transformedClipRect.x;
            currentPacket.scissorRect.y = (int16)transformedClipRect.y;
            currentPacket.scissorRect.width = (int16)ceilf(transformedClipRect.dx);
            currentPacket.scissorRect.height = (int16)ceilf(transformedClipRect.dy);
            currentPacket.options |= rhi::Packet::OPT_OVERRIDE_SCISSOR;
        }
        else
        {
            currentPacket.options &= ~rhi::Packet::OPT_OVERRIDE_SCISSOR;
        }
        lastClip = currentClip;
        
        currentPacket.primitiveType = batchDesc.primitiveType;
        currentPacket.startIndex = currentIndexBase + indexIndex;
        
        DVASSERT(batchDesc.material);
        lastMaterial = batchDesc.material;
        lastMaterial->BindParams(currentPacket);
        currentPacket.textureSet = batchDesc.textureSetHandle;
        currentPacket.samplerState = batchDesc.samplerStateHandle;
    }
    // End new packet

    switch (currentPacket.primitiveType)
    {
    case rhi::PRIMITIVE_LINELIST:
        currentPacket.primitiveCount += batchDesc.indexCount / 2;
        break;
    case rhi::PRIMITIVE_TRIANGLELIST:
        currentPacket.primitiveCount += batchDesc.indexCount / 3;
        break;
    case rhi::PRIMITIVE_TRIANGLESTRIP:
    	currentPacket.primitiveCount += batchDesc.indexCount - 2;
    	break;
    }

    indexIndex += batchDesc.indexCount;
    vertexIndex += batchDesc.vertexCount;
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
        DVASSERT(spriteVertexCount > 2); // Clip polygon should contain 3 points or more
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
            float32 coordX = state->position.x - state->pivotPoint.x * state->scale.x;
            float32 coordY = state->position.y - state->pivotPoint.y * state->scale.y;
            clipRect = Rect(  sprite->GetRectOffsetValueForFrame( frame, Sprite::X_OFFSET_TO_ACTIVE ) * state->scale.x + coordX
                            , sprite->GetRectOffsetValueForFrame( frame, Sprite::Y_OFFSET_TO_ACTIVE ) * state->scale.y + coordY
                            , sprite->GetRectOffsetValueForFrame( frame, Sprite::ACTIVE_WIDTH  ) * state->scale.x
                            , sprite->GetRectOffsetValueForFrame( frame, Sprite::ACTIVE_HEIGHT ) * state->scale.y );
        }
        else
        {
            float32 coordX = state->position.x - state->pivotPoint.x;
            float32 coordY = state->position.y - state->pivotPoint.y;
            clipRect = Rect(  sprite->GetRectOffsetValueForFrame( frame, Sprite::X_OFFSET_TO_ACTIVE ) + coordX
                            , sprite->GetRectOffsetValueForFrame( frame, Sprite::Y_OFFSET_TO_ACTIVE ) + coordY
                            , sprite->GetRectOffsetValueForFrame( frame, Sprite::ACTIVE_WIDTH )
                            , sprite->GetRectOffsetValueForFrame( frame, Sprite::ACTIVE_HEIGHT ) );
        }
        IntersectClipRect(clipRect);
    }

    BatchDescriptor batch;
    batch.material = state->GetMaterial();
    batch.textureSetHandle = sprite->GetTextureHandle(frame);
    batch.samplerStateHandle = sprite->GetTexture(frame)->samplerStateHandle;
    batch.singleColor = color;
    batch.vertexCount = spriteVertexCount;
    batch.indexCount = spriteIndexCount;
    batch.vertexStride = 2;
    batch.texCoordStride = 2;

    if (!sprite->clipPolygon)
    {
        batch.vertexPointer = spriteTempVertices;
        batch.texCoordPointer = sprite->texCoords[frame];
        batch.indexPointer = spriteIndeces;
    }
    else
    {
        batch.vertexPointer = (float32*)spriteClippedVertices.data();
        batch.texCoordPointer = (float32*)spriteClippedTexCoords.data();
        batch.indexPointer = spriteClippedIndecex.data();
    }
    PushBatch(batch);

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

    BatchDescriptor batch;
    batch.singleColor = color;
    batch.textureSetHandle = sprite->GetTextureHandle(frame);
    batch.samplerStateHandle = sprite->GetTexture(frame)->samplerStateHandle;
    batch.material = state->GetMaterial();
    batch.vertexCount = spriteVertexCount;
    batch.indexCount = spriteIndexCount;
    batch.vertexStride = 2;
    batch.texCoordStride = 2;
    batch.vertexPointer = (float32*)sd.transformedVertices.data();
    batch.texCoordPointer = (float32*)sd.texCoords.data();
    batch.indexPointer = sd.indeces;

    PushBatch(batch);
	
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

    BatchDescriptor batch;
    batch.singleColor = color;
    batch.material = state->GetMaterial();
    batch.textureSetHandle = sprite->GetTextureHandle(frame);
    batch.samplerStateHandle = sprite->GetTexture(frame)->samplerStateHandle;
    batch.vertexCount = spriteVertexCount;
    batch.indexCount = spriteIndexCount;
    batch.vertexStride = 2;
    batch.texCoordStride = 2;
    batch.vertexPointer = (float32*)td.transformedVertices.data();
    batch.texCoordPointer = (float32*)td.texCoords.data();
    batch.indexPointer = td.indeces.data();

    PushBatch(batch);

    if (!pTiledData)
    {
        SafeDelete(tiledData);
    }
}

/* RenderSyste2D Draw Helper Functions */

void RenderSystem2D::FillRect(const Rect & rect, const Color& color)
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

    BatchDescriptor batch;
    batch.singleColor = color;
    batch.material = DEFAULT_2D_COLOR_MATERIAL;
    batch.vertexCount = 4;
    batch.indexCount = 6;
    batch.vertexStride = 2;
    batch.texCoordStride = 2;
    batch.vertexPointer = spriteTempVertices;
    batch.indexPointer = indices;
    PushBatch(batch);
}

void RenderSystem2D::DrawRect(const Rect & rect, const Color& color)
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

    BatchDescriptor batch;
    batch.singleColor = color;
    batch.material = DEFAULT_2D_COLOR_MATERIAL;
    batch.vertexCount = 4;
    batch.indexCount = 8;
    batch.vertexStride = 2;
    batch.texCoordStride = 2;
    batch.vertexPointer = spriteTempVertices;
    batch.indexPointer = indices;
    batch.primitiveType = rhi::PRIMITIVE_LINELIST;
    PushBatch(batch);
}

void RenderSystem2D::DrawGrid(const Rect & rect, const Vector2& gridSize, const Color& color)
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

    BatchDescriptor batch;
    batch.singleColor = color;
    batch.material = DEFAULT_2D_COLOR_MATERIAL;
    batch.vertexCount = curVertexIndex / 2;
    batch.indexCount = curVertexIndex;
    batch.vertexStride = 2;
    batch.texCoordStride = 2;
    batch.vertexPointer = gridVertices.data();
    batch.indexPointer = indices.data();
    batch.primitiveType = rhi::PRIMITIVE_LINELIST;
    PushBatch(batch);
}

void RenderSystem2D::DrawLine(const Vector2 &start, const Vector2 &end, const Color& color)
{
    static uint16 indices[2] = { 0, 1 };
    spriteTempVertices[0] = start.x;
    spriteTempVertices[1] = start.y;
    spriteTempVertices[2] = end.x;
    spriteTempVertices[3] = end.y;

    BatchDescriptor batch;
    batch.singleColor = color;
    batch.material = DEFAULT_2D_COLOR_MATERIAL;
    batch.vertexCount = 2;
    batch.indexCount = 2;
    batch.vertexStride = 2;
    batch.texCoordStride = 2;
    batch.vertexPointer = spriteTempVertices;
    batch.indexPointer = indices;
    batch.primitiveType = rhi::PRIMITIVE_LINELIST;
    PushBatch(batch);
}

void RenderSystem2D::DrawLine(const Vector2 &start, const Vector2 &end, float32 lineWidth, const Color& color)
{
    // TODO: Create list of lines for emulating line with width >1px
    static uint16 indices[2] = { 0, 1 };
    spriteTempVertices[0] = start.x;
    spriteTempVertices[1] = start.y;
    spriteTempVertices[2] = end.x;
    spriteTempVertices[3] = end.y;

    BatchDescriptor batch;
    batch.singleColor = color;
    batch.material = DEFAULT_2D_COLOR_MATERIAL;
    batch.vertexCount = 2;
    batch.indexCount = 2;
    batch.vertexStride = 2;
    batch.texCoordStride = 2;
    batch.vertexPointer = spriteTempVertices;
    batch.indexPointer = indices;
    batch.primitiveType = rhi::PRIMITIVE_LINELIST;
    PushBatch(batch);
}

void RenderSystem2D::DrawLines(const Vector<float32>& linePoints, const Color& color)
{
    auto ptCount = linePoints.size() / 2;
    Vector<uint16> indices;
    indices.reserve(ptCount);
    for (auto i = 0U; i < ptCount; ++i)
    {
        indices.push_back(i);
    }

    BatchDescriptor batch;
    batch.singleColor = color;
    batch.material = DEFAULT_2D_COLOR_MATERIAL;
    batch.vertexCount = ptCount;
    batch.indexCount = indices.size();
    batch.vertexStride = 2;
    batch.texCoordStride = 2;
    batch.vertexPointer = linePoints.data();
    batch.indexPointer = indices.data();
    batch.primitiveType = rhi::PRIMITIVE_LINELIST;
    PushBatch(batch);
}

void RenderSystem2D::DrawCircle(const Vector2 & center, float32 radius, const Color& color)
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

    DrawPolygon(pts, false, color);
}

void RenderSystem2D::DrawPolygon(const Polygon2 & polygon, bool closed, const Color& color)
{
    auto ptCount = polygon.GetPointCount();
    if (ptCount >= 2)
    {
        Vector<uint16> indices;
        indices.reserve(ptCount + 1);
        auto i = 0;
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

        BatchDescriptor batch;
        batch.singleColor = color;
        batch.material = DEFAULT_2D_COLOR_MATERIAL;
        batch.vertexCount = ptCount;
        batch.indexCount = indices.size();
        batch.vertexStride = 2;
        batch.texCoordStride = 2;
        batch.vertexPointer = pointsPtr;
        batch.indexPointer = indices.data();
        batch.primitiveType = rhi::PRIMITIVE_LINELIST;
        PushBatch(batch);
    }
}

void RenderSystem2D::FillPolygon(const Polygon2 & polygon, const Color& color)
{
    auto ptCount = polygon.GetPointCount();
    if (ptCount >= 3)
    {
        Vector<uint16> indices;
        for (auto i = 1; i < ptCount - 1; ++i)
        {
            indices.push_back(0);
            indices.push_back(i);
            indices.push_back(i + 1);
        }
        auto pointsPtr = static_cast<const float32*>(static_cast<const void*>(polygon.GetPoints()));

        BatchDescriptor batch;
        batch.singleColor = color;
        batch.material = DEFAULT_2D_COLOR_MATERIAL;
        batch.vertexCount = ptCount;
        batch.indexCount = indices.size();
        batch.vertexPointer = pointsPtr;
        batch.vertexStride = 2;
        batch.texCoordStride = 2;
        batch.indexPointer = indices.data();
        PushBatch(batch);
    }
}

void RenderSystem2D::DrawPolygonTransformed(const Polygon2 & polygon, bool closed, const Matrix3 & transform, const Color& color)
{
    Polygon2 copyPoly = polygon;
    copyPoly.Transform(transform);
    DrawPolygon(copyPoly, closed, color);
}

void RenderSystem2D::DrawTexture(rhi::HTextureSet htextureSet, rhi::HSamplerState hSamplerState, NMaterial *material, const Color & color, const Rect & _dstRect /* = Rect(0.f, 0.f, -1.f, -1.f) */, const Rect & _srcRect /* = Rect(0.f, 0.f, -1.f, -1.f) */)
{
    Rect destRect(_dstRect);
    if (destRect.dx < 0.f || destRect.dy < 0.f)
    {
        if (IsRenderTargetPass())
        {
            destRect.dx = (float32)renderTargetWidth;
            destRect.dy = (float32)renderTargetHeight;
        }
        else
        {
            destRect.dx = (float32)Renderer::GetFramebufferWidth();
            destRect.dy = (float32)Renderer::GetFramebufferHeight();
        }
        
        destRect = VirtualCoordinatesSystem::Instance()->ConvertPhysicalToVirtual(destRect);
    }

    spriteTempVertices[0] = spriteTempVertices[4] = destRect.x;//x1
    spriteTempVertices[5] = spriteTempVertices[7] = destRect.y;//y2
    spriteTempVertices[1] = spriteTempVertices[3] = destRect.y + destRect.dy;//y1
    spriteTempVertices[2] = spriteTempVertices[6] = destRect.x + destRect.dx;//x2

    Rect srcRect;
    srcRect.x = _srcRect.x;
    srcRect.y = _srcRect.y;
    srcRect.dx = (_srcRect.dx < 0.f) ? 1.f : _srcRect.dx;
    srcRect.dy = (_srcRect.dy < 0.f) ? 1.f : _srcRect.dy;

    float32 texCoords[8];
    texCoords[0] = texCoords[4] = srcRect.x;//x1
    texCoords[5] = texCoords[7] = srcRect.y;//y2
    texCoords[1] = texCoords[3] = srcRect.y + srcRect.dy;//y1
    texCoords[2] = texCoords[6] = srcRect.x + srcRect.dx;//x2

    static uint16 indices[6] = { 0, 1, 2, 1, 3, 2 };

    BatchDescriptor batch;
    batch.singleColor = color;
    batch.textureSetHandle = htextureSet;
    batch.samplerStateHandle = hSamplerState;
    batch.material = material;
    batch.vertexCount = 4;
    batch.indexCount = 6;
    batch.vertexStride = 2;
    batch.texCoordStride = 2;
    batch.vertexPointer = spriteTempVertices;
    batch.texCoordPointer = texCoords;
    batch.indexPointer = indices;
    PushBatch(batch);
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
