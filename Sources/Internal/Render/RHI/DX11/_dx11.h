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

#include "Base/Platform.h"

    #if !defined(WIN32_LEAN_AND_MEAN)
        #define WIN32_LEAN_AND_MEAN
    #endif    
    #pragma warning( disable: 4005 )
#if !defined(__DAVAENGINE_WIN_UAP__)
    #define _WIN32_WINNT 0x0601
#endif
    #include <windows.h>

    #pragma warning( disable: 7 9 193 271 304 791 )
    #include <dxgi.h>
//    #include <d3d11.h>
    #include <d3d11_1.h>
    
    #include "../rhi_Type.h"

    #define RHI__FORCE_DX11_91 1

const char* D3D11ErrorText( HRESULT hr );

namespace rhi
{
struct InitParam;


DXGI_FORMAT             DX11_TextureFormat( TextureFormat format );

void                    InitializeRenderThreadDX11( uint32 frameCount );
void                    UninitializeRenderThreadDX11();

extern ID3D11Device*                _D3D11_Device;
extern IDXGISwapChain*              _D3D11_SwapChain;
extern ID3D11Texture2D*             _D3D11_SwapChainBuffer;
extern ID3D11RenderTargetView*      _D3D11_RenderTargetView;
extern ID3D11Texture2D*             _D3D11_DepthStencilBuffer;
extern ID3D11DepthStencilView*      _D3D11_DepthStencilView;
extern D3D_FEATURE_LEVEL            _D3D11_FeatureLevel;
extern ID3D11DeviceContext*         _D3D11_ImmediateContext;
extern ID3D11DeviceContext*         _D3D11_SecondaryContext;
extern DAVA::Mutex                  _D3D11_SecondaryContextSync;
extern ID3D11Debug*                 _D3D11_Debug;
extern ID3DUserDefinedAnnotation*   _D3D11_UserAnnotation;

// this hack need removed, when rhi_dx thread will synchronized with rander::reset
#ifdef __DAVAENGINE_WIN_UAP__
__DAVAENGINE_WIN_UAP_INCOMPLETE_IMPLEMENTATION__MARKER__
extern DAVA::Mutex need_synchronized;
#endif

extern InitParam                    _DX11_InitParam;

} // namespace rhi

