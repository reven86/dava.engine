#pragma once



    #if !defined(WIN32_LEAN_AND_MEAN)
        #define WIN32_LEAN_AND_MEAN
    #endif    
    #pragma warning( disable: 4005 )
    #include <windows.h>

    #pragma warning( disable: 7 9 193 271 304 791 )
    #include <dxgi.h>
    #include <d3d11.h>
    
    #include "../rhi_Type.h"



const char* D3D11ErrorText( HRESULT hr );

namespace rhi
{
struct InitParam;


DXGI_FORMAT             DX11_TextureFormat( TextureFormat format );

void                    InitializeRenderThreadDX11();
void                    UninitializeRenderThreadDX11();

extern ID3D11Device*            _D3D11_Device;
extern IDXGISwapChain*          _D3D11_SwapChain;
extern ID3D11Texture2D*         _D3D11_SwapChainBuffer;
extern ID3D11RenderTargetView*  _D3D11_RenderTargetView;
extern ID3D11Texture2D*         _D3D11_DepthStencilBuffer;
extern ID3D11DepthStencilView*  _D3D11_DepthStencilView;
extern D3D_FEATURE_LEVEL        _D3D11_FeatureLevel;
extern ID3D11DeviceContext*     _D3D11_ImmediateContext;
extern ID3D11Debug*             _D3D11_Debug;

extern InitParam                _DX11_InitParam;

} // namespace rhi

