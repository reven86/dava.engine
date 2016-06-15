#pragma once

#include "Base/Platform.h"

    #if !defined(WIN32_LEAN_AND_MEAN)
        #define WIN32_LEAN_AND_MEAN
    #endif    
    #pragma warning(disable : 4005)
#if !defined(__DAVAENGINE_WIN_UAP__)
    #define _WIN32_WINNT 0x0601
#endif
    #include <windows.h>

    #pragma warning(disable : 7 9 193 271 304 791)
    #include <dxgi.h>
//    #include <d3d11.h>
    #include <d3d11_1.h>
#if defined(__DAVAENGINE_WIN_UAP__)
    #include <DXGI1_3.h>
#endif
    
    #include "../rhi_Type.h"

    #define RHI_DX11__FORCE_9X_PROFILE 0
    #define RHI_DX11__USE_DEFERRED_CONTEXTS 1

const char* D3D11ErrorText(HRESULT hr);

namespace rhi
{
struct InitParam;

DXGI_FORMAT DX11_TextureFormat(TextureFormat format);

void InitializeRenderThreadDX11(uint32 frameCount);
void UninitializeRenderThreadDX11();

extern ID3D11Device* _D3D11_Device;
extern IDXGISwapChain* _D3D11_SwapChain;
extern ID3D11Texture2D* _D3D11_SwapChainBuffer;
extern ID3D11Texture2D* _D3D11_SwapChainBufferCopy;
extern ID3D11RenderTargetView* _D3D11_RenderTargetView;
extern ID3D11Texture2D* _D3D11_DepthStencilBuffer;
extern ID3D11DepthStencilView* _D3D11_DepthStencilView;
extern D3D_FEATURE_LEVEL _D3D11_FeatureLevel;
extern ID3D11DeviceContext* _D3D11_ImmediateContext;
extern ID3D11DeviceContext* _D3D11_SecondaryContext;
extern DAVA::Mutex _D3D11_SecondaryContextSync;
extern ID3D11Debug* _D3D11_Debug;
extern ID3DUserDefinedAnnotation* _D3D11_UserAnnotation;

extern ScreenShotCallback _D3D11_PendingScreenshotCallback;
extern DAVA::Mutex _D3D11_ScreenshotCallbackSync;

extern InitParam _DX11_InitParam;

} // namespace rhi
