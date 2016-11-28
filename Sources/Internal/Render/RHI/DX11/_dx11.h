#pragma once

#include "Base/Platform.h"

#if !defined(__DAVAENGINE_WIN_UAP__)
#define _WIN32_WINNT 0x0601
#endif

#include <windows.h>

#include <dxgi.h>
#include <d3d11_1.h>

#if defined(__DAVAENGINE_WIN_UAP__)
#include <DXGI1_3.h>
#endif

#include "../rhi_Type.h"
#include "Logger/Logger.h"
#include "Debug/DVAssert.h"

#define RHI_DX11__FORCE_9X_PROFILE 0

namespace rhi
{
bool DX11_CheckResult(HRESULT, const char* call, const char* fileName, const DAVA::uint32 line);
void DX11_ProcessCallResult(HRESULT hr, const char* call, const char* fileName, const DAVA::uint32 line);

const char* DX11_GetErrorText(HRESULT hr);
uint32 DX11_GetMaxSupportedMultisampleCount(ID3D11Device* device);
DXGI_FORMAT DX11_TextureFormat(TextureFormat format);
D3D11_COMPARISON_FUNC DX11_CmpFunc(CmpFunc func);
D3D11_STENCIL_OP DX11_StencilOp(StencilOperation op);
D3D11_TEXTURE_ADDRESS_MODE DX11_TextureAddrMode(TextureAddrMode mode);
D3D11_FILTER DX11_TextureFilter(TextureFilter min_filter, TextureFilter mag_filter, TextureMipFilter mip_filter, DAVA::uint32 anisotropy);
D3D11_BLEND DX11_BlendOp(BlendOp op);

extern ID3D11Device* _D3D11_Device;
extern DAVA::Mutex _D3D11_DeviceLock;
extern IDXGISwapChain* _D3D11_SwapChain;
extern ID3D11Texture2D* _D3D11_SwapChainBuffer;
extern ID3D11RenderTargetView* _D3D11_RenderTargetView;
extern ID3D11Texture2D* _D3D11_DepthStencilBuffer;
extern ID3D11DepthStencilView* _D3D11_DepthStencilView;
extern D3D_FEATURE_LEVEL _D3D11_FeatureLevel;
extern ID3D11DeviceContext* _D3D11_ImmediateContext;
extern ID3D11DeviceContext* _D3D11_SecondaryContext;
extern DAVA::Mutex _D3D11_SecondaryContextSync;
extern ID3D11Debug* _D3D11_Debug;
extern ID3DUserDefinedAnnotation* _D3D11_UserAnnotation;
extern InitParam _DX11_InitParam;
extern DWORD _DX11_RenderThreadId;
extern bool _DX11_UseHardwareCommandBuffers;

} // namespace rhi
