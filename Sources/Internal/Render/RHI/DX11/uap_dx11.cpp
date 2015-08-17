#include "uap_dx11.h"
#include "_dx11.h"

#if defined(__DAVAENGINE_WIN_UAP__)


#include <agile.h>
#include <Windows.ui.xaml.media.dxinterop.h>

using namespace rhi;

long init_device_and_swapchain_uap(void* panel, int width, int height)
{
	Windows::UI::Xaml::Controls::SwapChainPanel^ swapChainPanel = reinterpret_cast<Windows::UI::Xaml::Controls::SwapChainPanel^>(panel);

	HRESULT                 hr;
	DWORD                   flags = 0;
#if RHI__FORCE_DX11_91
	D3D_FEATURE_LEVEL       feature[] = { D3D_FEATURE_LEVEL_9_3, D3D_FEATURE_LEVEL_9_2, D3D_FEATURE_LEVEL_9_1 };
#else
	D3D_FEATURE_LEVEL       feature[] = { /*D3D_FEATURE_LEVEL_11_1, */D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_9_1 };
#endif
	DXGI_SWAP_CHAIN_DESC1    swapchain_desc = { 0 };

	Microsoft::WRL::ComPtr<ID3D11Device> d3dDevice;
	Microsoft::WRL::ComPtr<ID3D11DeviceContext> d3dContext;
	Microsoft::WRL::ComPtr<IDXGISwapChain1> swapChain;

#if 0
	flags |= D3D11_CREATE_DEVICE_DEBUG;
	flags |= D3D11_CREATE_DEVICE_PREVENT_INTERNAL_THREADING_OPTIMIZATIONS;
#endif

	hr = D3D11CreateDevice(
		NULL,
		D3D_DRIVER_TYPE_HARDWARE,
		NULL,
		flags,
		feature,
		ARRAYSIZE(feature),
		D3D11_SDK_VERSION,
		&d3dDevice,
		&_D3D11_FeatureLevel,
		&d3dContext
		);

	if (FAILED(hr))
	{
		return hr;
	}

	Microsoft::WRL::ComPtr<IDXGIDevice> dxgiDevice;
	d3dDevice.As(&dxgiDevice);

	if(width < 0)
	{
		width = swapChainPanel->ActualWidth;
	}
	if(height < 0)
	{
		height = swapChainPanel->ActualHeight;
	}

	swapchain_desc.Width = width;
	swapchain_desc.Height = height;
	swapchain_desc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;           // this is the most common swap chain format
	swapchain_desc.Stereo = false;
	swapchain_desc.SampleDesc.Count = 1;                          // don't use multi-sampling
	swapchain_desc.SampleDesc.Quality = 0;
	swapchain_desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapchain_desc.BufferCount = 2;
	swapchain_desc.Scaling = DXGI_SCALING_STRETCH;
	swapchain_desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL; // we recommend using this swap effect for all applications
	swapchain_desc.Flags = 0;

	// get the DXGI adapter
	Microsoft::WRL::ComPtr<IDXGIAdapter> dxgiAdapter;
	dxgiDevice->GetAdapter(&dxgiAdapter);

	// get the DXGI factory
	Microsoft::WRL::ComPtr<IDXGIFactory2> dxgiFactory;
	dxgiAdapter->GetParent(__uuidof(IDXGIFactory2), &dxgiFactory);
	// create swap chain by calling CreateSwapChainForComposition
	hr = dxgiFactory->CreateSwapChainForComposition(
		d3dDevice.Get(),
		&swapchain_desc,
		nullptr,		// allow on any display 
		&swapChain
		);

	if (SUCCEEDED(hr))
	{
// 		Microsoft::WRL::ComPtr<ISwapChainPanelNative> swapChainNative;
// 		IInspectable* panelInspectable = (IInspectable*) reinterpret_cast<IInspectable*>(swapChainPanel);
// 		panelInspectable->QueryInterface(__uuidof(ISwapChainPanelNative), (void **)&swapChainNative);
// 		swapChainNative->SetSwapChain(swapChain.Get());

		swapChainPanel->Dispatcher->RunAsync(::Windows::UI::Core::CoreDispatcherPriority::High, ref new Windows::UI::Core::DispatchedHandler([=]()
		{
			// Get backing native interface for SwapChainPanel
			Microsoft::WRL::ComPtr<ISwapChainPanelNative> panelNative;
			reinterpret_cast<IUnknown*>(swapChainPanel)->QueryInterface(IID_PPV_ARGS(&panelNative));
			panelNative->SetSwapChain(swapChain.Get());
		}, Platform::CallbackContext::Any));

		Microsoft::WRL::ComPtr<ID3D11Texture2D> dxgiBackBuffer;
		hr = swapChain->GetBuffer(0, IID_PPV_ARGS(&dxgiBackBuffer));// __uuidof(ID3D11Texture2D), (void**)(&_D3D11_SwapChainBuffer));

		if (SUCCEEDED(hr))
		{
// 			hr = d3dDevice->QueryInterface(__uuidof(ID3D11Debug), (void**)(&_D3D11_Debug));
// 			hr = d3dContext->QueryInterface(__uuidof(ID3DUserDefinedAnnotation), (void**)(&_D3D11_UserAnnotation));
		}

		hr = d3dDevice->CreateRenderTargetView(dxgiBackBuffer.Get(), 0, &_D3D11_RenderTargetView);

		D3D11_TEXTURE2D_DESC    ds_desc = { 0 };

		ds_desc.Width = width;
		ds_desc.Height = height;
		ds_desc.MipLevels = 1;
		ds_desc.ArraySize = 1;
		ds_desc.Format = (_D3D11_FeatureLevel == D3D_FEATURE_LEVEL_11_0) ? DXGI_FORMAT_D32_FLOAT : DXGI_FORMAT_D24_UNORM_S8_UINT;
		ds_desc.SampleDesc.Count = 1;
		ds_desc.SampleDesc.Quality = 0;
		ds_desc.Usage = D3D11_USAGE_DEFAULT;
		ds_desc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
		ds_desc.CPUAccessFlags = 0;
		ds_desc.MiscFlags = 0;

		hr = d3dDevice->CreateTexture2D(&ds_desc, 0, &_D3D11_DepthStencilBuffer);
		hr = d3dDevice->CreateDepthStencilView(_D3D11_DepthStencilBuffer, 0, &_D3D11_DepthStencilView);

		_D3D11_SwapChainBuffer = reinterpret_cast<ID3D11Texture2D*>(dxgiBackBuffer.Get());
		_D3D11_SwapChain = reinterpret_cast<IDXGISwapChain*>(swapChain.Get());
		_D3D11_Device = reinterpret_cast<ID3D11Device*>(d3dDevice.Get());
		_D3D11_ImmediateContext = reinterpret_cast<ID3D11DeviceContext*>(d3dContext.Get());
	}

	return hr;
}

#endif