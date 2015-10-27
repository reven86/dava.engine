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

#pragma once

    #include "../rhi_Type.h"

    #if !defined(WIN32_LEAN_AND_MEAN)
        #define WIN32_LEAN_AND_MEAN
    #endif    
    #include <windows.h>

    #pragma warning(disable : 7 9 193 271 304 791)
    #include <d3d9.h>

const char* D3D9ErrorText(HRESULT hr);

namespace rhi
{
struct InitParam;

D3DFORMAT DX9_TextureFormat(TextureFormat format);

void InitializeRenderThreadDX9(uint32 frameCount);
void UninitializeRenderThreadDX9();

void AcquireDevice();
void ReleaseDevice();

extern IDirect3D9* _D3D9;
extern IDirect3DDevice9* _D3D9_Device;
extern unsigned _D3D9_Adapter;
extern IDirect3DSurface9* _D3D9_BackBuf;
extern IDirect3DSurface9* _D3D9_DepthBuf;

extern InitParam _DX9_InitParam;
extern D3DPRESENT_PARAMETERS _DX9_PresentParam;

} // namespace rhi
