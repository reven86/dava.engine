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


#ifndef __RHI_IMPL_H__
#define __RHI_IMPL_H__

    #include "rhi_Private.h"

namespace rhi
{
struct ResetParam;
struct RenderDeviceCaps;

struct
Dispatch
{
    //    void    (*impl_Initialize)();
    void (*impl_Reset)(const ResetParam&);
    void (*impl_Uninitialize)();
    void (*impl_Present)(Handle);
    Api (*impl_HostApi)();
    bool (*impl_NeedRestoreResources)();
    bool (*impl_TextureFormatSupported)(TextureFormat);

    void (*impl_SuspendRendering)();
    void (*impl_ResumeRendering)();
    void (*impl_InvalidateCache)();

    const RenderDeviceCaps& (*impl_DeviceCaps)();

    Handle (*impl_VertexBuffer_Create)(const VertexBuffer::Descriptor& desc);
    void (*impl_VertexBuffer_Delete)(Handle);
    bool (*impl_VertexBuffer_Update)(Handle, const void*, uint32, uint32);
    void* (*impl_VertexBuffer_Map)(Handle, uint32, uint32);
    void (*impl_VertexBuffer_Unmap)(Handle);
    bool (*impl_VertexBuffer_NeedRestore)(Handle);

    Handle (*impl_IndexBuffer_Create)(const IndexBuffer::Descriptor& desc);
    void (*impl_IndexBuffer_Delete)(Handle);
    bool (*impl_IndexBuffer_Update)(Handle, const void*, uint32, uint32);
    void* (*impl_IndexBuffer_Map)(Handle, uint32, uint32);
    void (*impl_IndexBuffer_Unmap)(Handle);
    bool (*impl_IndexBuffer_NeedRestore)(Handle);

    Handle (*impl_QueryBuffer_Create)(unsigned maxObjectCount);
    void (*impl_QueryBuffer_Reset)(Handle buf);
    void (*impl_QueryBuffer_Delete)(Handle buf);
    bool (*impl_QueryBuffer_IsReady)(Handle buf, uint32 objectIndex);
    int (*impl_QueryBuffer_Value)(Handle buf, uint32 objectIndex);

    Handle (*impl_PerfQuerySet_Create)(uint32 maxQueryCount);
    void (*impl_PerfQuerySet_Delete)(Handle set);
    void (*impl_PerfQuerySet_Reset)(Handle set);
    void (*impl_PerfQuerySet_SetCurrent)(Handle set);
    void (*impl_PerfQuerySet_GetStatus)(Handle set, bool* isReady, bool* isValid);
    bool (*impl_PerfQuerySet_IsValid)(Handle handle);
    bool (*impl_PerfQuerySet_GetFreq)(Handle set, uint64* freq);
    bool (*impl_PerfQuerySet_GetTimestamp)(Handle set, uint32 timestampIndex, uint64* time);
    bool (*impl_PerfQuerySet_GetFrameTimestamps)(Handle set, uint64* t0, uint64* t1);

    Handle (*impl_Texture_Create)(const Texture::Descriptor& desc);
    void (*impl_Texture_Delete)(Handle);
    void* (*impl_Texture_Map)(Handle, unsigned, TextureFace);
    void (*impl_Texture_Unmap)(Handle);
    void (*impl_Texture_Update)(Handle, const void*, uint32, TextureFace);
    bool (*impl_Texture_NeedRestore)(Handle);

    Handle (*impl_PipelineState_Create)(const PipelineState::Descriptor&);
    void (*impl_PipelineState_Delete)(Handle);
    Handle (*impl_PipelineState_CreateVertexConstBuffer)(Handle, uint32);
    Handle (*impl_PipelineState_CreateFragmentConstBuffer)(Handle, uint32);
    uint32 (*impl_PipelineState_VertexConstBufferCount)(Handle);
    uint32 (*impl_PipelineState_VertexConstCount)(Handle, uint32);
    bool (*impl_PipelineState_GetVertexConstInfo)(Handle, uint32, uint32, ProgConstInfo*);
    uint32 (*impl_PipelineState_FragmentConstBufferCount)(Handle);
    uint32 (*impl_PipelineState_FragmentConstCount)(Handle, uint32);
    bool (*impl_PipelineState_GetFragmentConstInfo)(Handle, uint32, uint32, ProgConstInfo*);

    uint32 (*impl_ConstBuffer_ConstCount)(Handle);
    bool (*impl_ConstBuffer_SetConst)(Handle, uint32, uint32, const float*);
    bool (*impl_ConstBuffer_SetConst1fv)(Handle, uint32, uint32, const float*, uint32);
    void (*impl_ConstBuffer_Delete)(Handle);

    Handle (*impl_DepthStencilState_Create)(const DepthStencilState::Descriptor&);
    void (*impl_DepthStencilState_Delete)(Handle);

    Handle (*impl_SamplerState_Create)(const SamplerState::Descriptor&);
    void (*impl_SamplerState_Delete)(Handle);

    Handle (*impl_Renderpass_Allocate)(const RenderPassConfig&, uint32, Handle*);
    void (*impl_Renderpass_Begin)(Handle);
    void (*impl_Renderpass_End)(Handle);

    Handle (*impl_SyncObject_Create)();
    void (*impl_SyncObject_Delete)(Handle);
    bool (*impl_SyncObject_IsSignaled)(Handle);

    void (*impl_CommandBuffer_Begin)(Handle);
    void (*impl_CommandBuffer_End)(Handle, Handle);
    void (*impl_CommandBuffer_SetPipelineState)(Handle, Handle, uint32 vdecl);
    void (*impl_CommandBuffer_SetCullMode)(Handle, CullMode);
    void (*impl_CommandBuffer_SetScissorRect)(Handle, ScissorRect);
    void (*impl_CommandBuffer_SetViewport)(Handle, Viewport);
    void (*impl_CommandBuffer_SetFillMode)(Handle, FillMode);
    void (*impl_CommandBuffer_SetVertexData)(Handle, Handle, uint32);
    void (*impl_CommandBuffer_SetVertexConstBuffer)(Handle, uint32, Handle);
    void (*impl_CommandBuffer_SetVertexTexture)(Handle, uint32, Handle);
    void (*impl_CommandBuffer_SetIndices)(Handle, Handle);
    void (*impl_CommandBuffer_SetQueryIndex)(Handle, uint32);
    void (*impl_CommandBuffer_SetQueryBuffer)(Handle, Handle);
    void (*impl_CommandBuffer_IssueTimestampQuery)(Handle, Handle, uint32);
    void (*impl_CommandBuffer_SetFragmentConstBuffer)(Handle, uint32, Handle);
    void (*impl_CommandBuffer_SetFragmentTexture)(Handle, uint32, Handle);
    void (*impl_CommandBuffer_SetDepthStencilState)(Handle, Handle);
    void (*impl_CommandBuffer_SetSamplerState)(Handle, const Handle);
    void (*impl_CommandBuffer_DrawPrimitive)(Handle, PrimitiveType, uint32);
    void (*impl_CommandBuffer_DrawIndexedPrimitive)(Handle, PrimitiveType, uint32, uint32, uint32, uint32);
    void (*impl_CommandBuffer_SetMarker)(Handle, const char*);
    void (*impl_CommandBuffer_SetSync)(Handle, Handle);
};

void SetDispatchTable(const Dispatch& dispatch);

//------------------------------------------------------------------------------

Size2i TextureExtents(Size2i size, uint32 level);
uint32 TextureStride(TextureFormat format, Size2i size, uint32 level);
uint32 TextureSize(TextureFormat format, uint32 width, uint32 height, uint32 level = 0);

} // namespace rhi


#endif // __RHI_IMPL_H__
