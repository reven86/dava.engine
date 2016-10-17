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
#include <d3d11_1.h>

#if defined(__DAVAENGINE_WIN_UAP__)
    #include <DXGI1_3.h>
#endif

#include "../rhi_Type.h"
#include "Logger/Logger.h"

#define RHI_DX11__FORCE_9X_PROFILE 0
#define RHI_DX11__USE_DEFERRED_CONTEXTS 1
#define RHI_DX11__ENABLE_ERROR_CHECK 1

#if RHI_DX11__ENABLE_ERROR_CHECK
#define CHECK_HR(hr) \
    if (FAILED(hr)) \
    { \
        DAVA::Logger::Error("D3D11Error at %s: %d\n%s", __FILE__, __LINE__, DX11_GetErrorText(hr)); \
    } 
#else
#define CHECK_HR(hr) hr
#endif

#define DX11_GET_DEVICE (_D3D11_Device.load(std::memory_order_relaxed))

#define DX11_DEVICE_CALL(F, HR) \
do { \
    if (DX11_GET_DEVICE) \
    {\
        HRESULT callResult = DX11_GET_DEVICE->F; \
        if ((callResult == DXGI_ERROR_DEVICE_REMOVED) || (callResult == DXGI_ERROR_DEVICE_RESET)) \
        { \
            DX11_InvalidateDevice(DX11_GET_DEVICE->GetDeviceRemovedReason()); \
        } \
        else if (FAILED(callResult))\
        { \
            DAVA::Logger::Error("DX11 Device call %s\nat %s [%u] failed:\n%s", #F, __FILE__, __LINE__, DX11_GetErrorText(callResult)); \
        } \
        HR = callResult; \
    }\
} while (DX11_GET_DEVICE == nullptr);

namespace rhi
{
struct InitParam;

DXGI_FORMAT DX11_TextureFormat(TextureFormat format);
uint32 DX11_GetMaxSupportedMultisampleCount(ID3D11Device* device);
const char* DX11_GetErrorText(HRESULT hr);

void InitializeRenderThreadDX11(uint32 frameCount);
void UninitializeRenderThreadDX11();
void DX11_InvalidateDevice(HRESULT hr);

extern std::atomic<ID3D11Device*> _D3D11_Device;

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

} // namespace rhi
