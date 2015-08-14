#include "win_dx11.h"
#include "_dx11.h"

#if defined(__DAVAENGINE_WIN32__)

using namespace rhi;

long init_device_and_swapchain_win32(void* window, int width, int height)
{

	DWORD                   flags           = 0;
    #if RHI__FORCE_DX11_91
    D3D_FEATURE_LEVEL       feature[]       = { D3D_FEATURE_LEVEL_9_3, D3D_FEATURE_LEVEL_9_2, D3D_FEATURE_LEVEL_9_1 };
    #else
    D3D_FEATURE_LEVEL       feature[]       = { /*D3D_FEATURE_LEVEL_11_1, */D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_9_1 };
    #endif
    DXGI_SWAP_CHAIN_DESC    swapchain_desc  = {0};

    #if 1
    flags |= D3D11_CREATE_DEVICE_DEBUG;
    flags |= D3D11_CREATE_DEVICE_PREVENT_INTERNAL_THREADING_OPTIMIZATIONS;
    #endif

    swapchain_desc.BufferDesc.Width                     = width;
    swapchain_desc.BufferDesc.Height                    = height;
    swapchain_desc.BufferDesc.Format                    = DXGI_FORMAT_B8G8R8A8_UNORM;
    swapchain_desc.BufferDesc.RefreshRate.Numerator     = 0;
    swapchain_desc.BufferDesc.RefreshRate.Denominator   = 0;
    swapchain_desc.BufferDesc.ScanlineOrdering          = DXGI_MODE_SCANLINE_ORDER_PROGRESSIVE;
    swapchain_desc.BufferDesc.Scaling                   = DXGI_MODE_SCALING_CENTERED;

    swapchain_desc.SampleDesc.Count                     = 1;
    swapchain_desc.SampleDesc.Quality                   = 0;

    swapchain_desc.BufferUsage                          = DXGI_USAGE_RENDER_TARGET_OUTPUT;//DXGI_USAGE_BACK_BUFFER;
    swapchain_desc.BufferCount                          = 2;

    swapchain_desc.OutputWindow                         = (HWND)window;
    swapchain_desc.Windowed                             = TRUE;

    swapchain_desc.SwapEffect                           = DXGI_SWAP_EFFECT_DISCARD;
    swapchain_desc.Flags                                = 0;
    
    HANDLE hr = D3D11CreateDeviceAndSwapChain
    ( 
        NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, 
        flags, feature, countof(feature), D3D11_SDK_VERSION, &swapchain_desc,
        &_D3D11_SwapChain, &_D3D11_Device, &_D3D11_FeatureLevel, &_D3D11_ImmediateContext
    );

	if (SUCCEEDED(hr))
	{
		hr = _D3D11_SwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)(&_D3D11_SwapChainBuffer));

		if (SUCCEEDED(hr))
		{
			hr = _D3D11_Device->QueryInterface(__uuidof(ID3D11Debug), (void**)(&_D3D11_Debug));

			hr = _D3D11_ImmediateContext->QueryInterface(__uuidof(ID3DUserDefinedAnnotation), (void**)(&_D3D11_UserAnnotation));
		}

		hr = _D3D11_Device->CreateRenderTargetView(_D3D11_SwapChainBuffer, 0, &_D3D11_RenderTargetView);

		D3D11_TEXTURE2D_DESC    ds_desc = { 0 };

		ds_desc.Width = _DX11_InitParam.width;
		ds_desc.Height = _DX11_InitParam.height;
		ds_desc.MipLevels = 1;
		ds_desc.ArraySize = 1;
		ds_desc.Format = (_D3D11_FeatureLevel == D3D_FEATURE_LEVEL_11_0) ? DXGI_FORMAT_D32_FLOAT : DXGI_FORMAT_D24_UNORM_S8_UINT;
		ds_desc.SampleDesc.Count = 1;
		ds_desc.SampleDesc.Quality = 0;
		ds_desc.Usage = D3D11_USAGE_DEFAULT;
		ds_desc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
		ds_desc.CPUAccessFlags = 0;
		ds_desc.MiscFlags = 0;

		hr = _D3D11_Device->CreateTexture2D(&ds_desc, 0, &_D3D11_DepthStencilBuffer);
		hr = _D3D11_Device->CreateDepthStencilView(_D3D11_DepthStencilBuffer, 0, &_D3D11_DepthStencilView);
	}

	return hr;
}

#endif