#include "_dx11.h"
#include "rhi_DX11.h"
#include "../rhi_Public.h"

#if defined(__DAVAENGINE_WIN_UAP__)

#include "Concurrency/Thread.h"

#include "uap_dx11.h"

#include <agile.h>
#include <Windows.ui.xaml.media.dxinterop.h>
#include <DirectXMath.h>
#include <dxgi1_3.h>
#include <D3D11SDKLayers.h>

using namespace rhi;
using namespace Microsoft::WRL;

static Windows::UI::Xaml::Controls::SwapChainPanel ^ m_swapChainPanel = nullptr;
static ComPtr<ID3D11Device1> m_d3dDevice;
static ComPtr<IDXGIAdapter> m_dxgiAdapter;
static ComPtr<ID3D11DeviceContext1> m_d3dContext;
static ComPtr<ID3D11Debug> m_d3Debug;
static ComPtr<ID3DUserDefinedAnnotation> m_d3UserAnnotation;
static ComPtr<IDXGISwapChain1> m_swapChain;
static ComPtr<ID3D11Texture2D> m_swapChainBuffer;
static ComPtr<ID3D11RenderTargetView> m_d3dRenderTargetView;
static ComPtr<ID3D11Texture2D> m_d3dDepthStencilBuffer;
static ComPtr<ID3D11DepthStencilView> m_d3dDepthStencilView;
static D3D11_VIEWPORT m_screenViewport;
static D3D_FEATURE_LEVEL m_d3dFeatureLevel;

static Windows::Foundation::Size m_d3dRenderTargetSize;
static Windows::Foundation::Size m_backbufferSize;
static Windows::Foundation::Size m_backbufferScale;

DirectX::XMFLOAT4X4 m_orientationTransform3D;
Windows::Graphics::Display::DisplayOrientations m_nativeOrientation;
Windows::Graphics::Display::DisplayOrientations m_currentOrientation;
float m_dpi = 1.f;

namespace DAVA
{
namespace UWPWorkaround
{
bool enableSurfaceSizeWorkaround = false; //'workaround' for ATI HD ****G drivers
}
}

DXGI_MODE_ROTATION ComputeDisplayRotation();
void CreateDeviceResources();
bool CreateWindowSizeDependentResources();
void SetSwapChainPanel(Windows::UI::Xaml::Controls::SwapChainPanel ^ panel);
void SetBackBufferSize(const Windows::Foundation::Size& backbufferSize, const Windows::Foundation::Size& backbufferScale);
void SetCurrentOrientation(Windows::Graphics::Display::DisplayOrientations currentOrientation);
void ValidateDevice();
void HandleDeviceLost();
void Trim();
void Present();

// Constants used to calculate screen rotations.
namespace ScreenRotation
{
// 0-degree Z-rotation
static const DirectX::XMFLOAT4X4 Rotation0(
1.0f, 0.0f, 0.0f, 0.0f,
0.0f, 1.0f, 0.0f, 0.0f,
0.0f, 0.0f, 1.0f, 0.0f,
0.0f, 0.0f, 0.0f, 1.0f);

// 90-degree Z-rotation
static const DirectX::XMFLOAT4X4 Rotation90(
0.0f, 1.0f, 0.0f, 0.0f,
-1.0f, 0.0f, 0.0f, 0.0f,
0.0f, 0.0f, 1.0f, 0.0f,
0.0f, 0.0f, 0.0f, 1.0f);

// 180-degree Z-rotation
static const DirectX::XMFLOAT4X4 Rotation180(
-1.0f, 0.0f, 0.0f, 0.0f,
0.0f, -1.0f, 0.0f, 0.0f,
0.0f, 0.0f, 1.0f, 0.0f,
0.0f, 0.0f, 0.0f, 1.0f);

// 270-degree Z-rotation
static const DirectX::XMFLOAT4X4 Rotation270(
0.0f, -1.0f, 0.0f, 0.0f,
1.0f, 0.0f, 0.0f, 0.0f,
0.0f, 0.0f, 1.0f, 0.0f,
0.0f, 0.0f, 0.0f, 1.0f);
};

inline void ThrowIfFailed(HRESULT hr)
{
    if (FAILED(hr))
    {
        // Set a breakpoint on this line to catch Win32 API errors.
        throw Platform::Exception::CreateException(hr);
    }
}

#if defined(_DEBUG)
// Check for SDK Layer support.
inline bool SdkLayersAvailable()
{
    HRESULT hr = D3D11CreateDevice(
    nullptr,
    D3D_DRIVER_TYPE_NULL, // There is no need to create a real hardware device.
    0,
    D3D11_CREATE_DEVICE_DEBUG, // Check for the SDK layers.
    nullptr, // Any feature level will do.
    0,
    D3D11_SDK_VERSION, // Always set this to D3D11_SDK_VERSION for Windows Store apps.
    nullptr, // No need to keep the D3D device reference.
    nullptr, // No need to know the feature level.
    nullptr // No need to keep the D3D device context reference.
    );

    return SUCCEEDED(hr);
}
#endif

// This method determines the rotation between the display device's native Orientation and the
// current display orientation.
DXGI_MODE_ROTATION ComputeDisplayRotation()
{
    using Windows::Graphics::Display::DisplayOrientations;

    DXGI_MODE_ROTATION rotation = DXGI_MODE_ROTATION_UNSPECIFIED;

    // Note: NativeOrientation can only be Landscape or Portrait even though
    // the DisplayOrientations enum has other values.
    switch (m_nativeOrientation)
    {
    case DisplayOrientations::Landscape:
        switch (m_currentOrientation)
        {
        case DisplayOrientations::Landscape:
            rotation = DXGI_MODE_ROTATION_IDENTITY;
            break;

        case DisplayOrientations::Portrait:
            rotation = DXGI_MODE_ROTATION_ROTATE270;
            break;

        case DisplayOrientations::LandscapeFlipped:
            rotation = DXGI_MODE_ROTATION_ROTATE180;
            break;

        case DisplayOrientations::PortraitFlipped:
            rotation = DXGI_MODE_ROTATION_ROTATE90;
            break;
        }
        break;

    case DisplayOrientations::Portrait:
        switch (m_currentOrientation)
        {
        case DisplayOrientations::Landscape:
            rotation = DXGI_MODE_ROTATION_ROTATE90;
            break;

        case DisplayOrientations::Portrait:
            rotation = DXGI_MODE_ROTATION_IDENTITY;
            break;

        case DisplayOrientations::LandscapeFlipped:
            rotation = DXGI_MODE_ROTATION_ROTATE270;
            break;

        case DisplayOrientations::PortraitFlipped:
            rotation = DXGI_MODE_ROTATION_ROTATE180;
            break;
        }
        break;
    }
    return rotation;
}

void CreateDeviceResources()
{
    // This flag adds support for surfaces with a different color channel ordering
    // than the API default. It is required for compatibility with Direct2D.
    UINT creationFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;

#if defined(_DEBUG)
    if (SdkLayersAvailable())
    {
        // If the project is in a debug build, enable debugging via SDK Layers with this flag.
        creationFlags |= D3D11_CREATE_DEVICE_DEBUG;
    }
#endif

    // This array defines the set of DirectX hardware feature levels this app will support.
    // Note the ordering should be preserved.
    // Don't forget to declare your application's minimum required feature level in its
    // description.  All applications are assumed to support 9.1 unless otherwise stated.
    D3D_FEATURE_LEVEL featureLevels[] =
    {
#if !(RHI__FORCE_DX11_91)
        D3D_FEATURE_LEVEL_11_1,
        D3D_FEATURE_LEVEL_11_0,
#endif
        D3D_FEATURE_LEVEL_9_3,
        D3D_FEATURE_LEVEL_9_2,
        D3D_FEATURE_LEVEL_9_1
    };

    ComPtr<IDXGIAdapter> defAdapter;
    ComPtr<IDXGIFactory1> factory;
    std::vector<ComPtr<IDXGIAdapter>> adapter;
    const uint32 preferredVendorID[] =
    {
      0x10DE, // nVIDIA
      0x1002 /*,     // ATI
        0x4D4F4351  // Qualcomm*/
    };

    if (SUCCEEDED(CreateDXGIFactory1(__uuidof(IDXGIFactory1), (void**)&factory)))
    {
        IDXGIAdapter* a = NULL;

        for (UINT i = 0; factory->EnumAdapters(i, &a) != DXGI_ERROR_NOT_FOUND; ++i)
        {
            adapter.push_back(a);
        }
    }

    DAVA::Logger::Info("detected GPUs (%u) :", adapter.size());
    for (unsigned i = 0; i != adapter.size(); ++i)
    {
        DXGI_ADAPTER_DESC desc = { 0 };

        if (SUCCEEDED(adapter[i]->GetDesc(&desc)))
        {
            char info[128];

            ::WideCharToMultiByte(CP_ACP, WC_NO_BEST_FIT_CHARS, desc.Description, -1, info, countof(info) - 1, NULL, NULL);

            DAVA::Logger::Info("  adapter[%u]  \"%s\"  vendor= %04X  device= %04X", i, info, desc.VendorId, desc.DeviceId);

            if (!defAdapter)
            {
                for (unsigned k = 0; k != countof(preferredVendorID); ++k)
                {
                    if (desc.VendorId == preferredVendorID[k])
                    {
                        defAdapter = adapter[i];
                        break;
                    }
                }
            }
        }
    }

    // Create the Direct3D 11 API device object and a corresponding context.
    ComPtr<ID3D11Device> device;
    ComPtr<ID3D11DeviceContext> context;

    HRESULT hr = D3D11CreateDevice(
    defAdapter.Get(), // Specify nullptr to use the default adapter.
    (defAdapter) ? D3D_DRIVER_TYPE_UNKNOWN : D3D_DRIVER_TYPE_HARDWARE, // Create a device using the hardware graphics driver.
    0, // Should be 0 unless the driver is D3D_DRIVER_TYPE_SOFTWARE.
    creationFlags, // Set debug and Direct2D compatibility flags.
    featureLevels, // List of feature levels this app can support.
    ARRAYSIZE(featureLevels), // Size of the list above.
    D3D11_SDK_VERSION, // Always set this to D3D11_SDK_VERSION for Windows Store apps.
    &device, // Returns the Direct3D device created.
    &m_d3dFeatureLevel, // Returns feature level of device created.
    &context // Returns the device immediate context.
    );

    if (FAILED(hr))
    {
        hr = D3D11CreateDevice(
        nullptr, // Specify nullptr to use the default adapter.
        D3D_DRIVER_TYPE_HARDWARE, // Create a device using the hardware graphics driver.
        0, // Should be 0 unless the driver is D3D_DRIVER_TYPE_SOFTWARE.
        creationFlags, // Set debug and Direct2D compatibility flags.
        featureLevels, // List of feature levels this app can support.
        ARRAYSIZE(featureLevels), // Size of the list above.
        D3D11_SDK_VERSION, // Always set this to D3D11_SDK_VERSION for Windows Store apps.
        &device, // Returns the Direct3D device created.
        &m_d3dFeatureLevel, // Returns feature level of device created.
        &context // Returns the device immediate context.
        );
    }

#if 0
    if (FAILED(hr))
    {
        // If the initialization fails, fall back to the WARP device.
        // For more information on WARP, see: 
        // http://go.microsoft.com/fwlink/?LinkId=286690
        ThrowIfFailed(
            D3D11CreateDevice(
            nullptr,
            D3D_DRIVER_TYPE_WARP, // Create a WARP device instead of a hardware device.
            0,
            creationFlags,
            featureLevels,
            ARRAYSIZE(featureLevels),
            D3D11_SDK_VERSION,
            &device,
            &m_d3dFeatureLevel,
            &context
        )
        );
    }
#endif

    if (device.Get())
    {
        IDXGIDevice* dxgiDevice = NULL;
        m_dxgiAdapter = defAdapter.Get();

        if (!m_dxgiAdapter)
        {
            if (SUCCEEDED(device.Get()->QueryInterface(__uuidof(IDXGIDevice), (void**)(&dxgiDevice))))
                dxgiDevice->GetAdapter(&m_dxgiAdapter);
        }

        if (m_dxgiAdapter)
        {
            DXGI_ADAPTER_DESC desc = { 0 };

            if (SUCCEEDED(m_dxgiAdapter->GetDesc(&desc)))
            {
                char info[128];

                ::WideCharToMultiByte(CP_ACP, WC_NO_BEST_FIT_CHARS, desc.Description, -1, info, countof(info) - 1, NULL, NULL);

                DAVA::UWPWorkaround::enableSurfaceSizeWorkaround |= strstr(info, "AMD Radeon HD") && info[strlen(info) - 1] == 'G';

                DAVA::Logger::Info("using adapter  \"%s\"  vendor= %04X  device= %04X", info, desc.VendorId, desc.DeviceId);
            }
        }
    }

#if defined(_DEBUG)
    if (SdkLayersAvailable())
    {
        ThrowIfFailed(
        device.As(&m_d3Debug));
    }
#endif

    ThrowIfFailed(
    context.As(&m_d3UserAnnotation));

    // Store pointers to the Direct3D 11.1 API device and immediate context.
    ThrowIfFailed(
    device.As(&m_d3dDevice));

    ThrowIfFailed(
    context.As(&m_d3dContext));
}

bool CreateWindowSizeDependentResources()
{
    const UINT backBuffersCount = 3;

    if (_DX11_InitParam.FrameCommandExecutionSync)
        _DX11_InitParam.FrameCommandExecutionSync->Lock();

    // Prevent zero size DirectX content from being created.
    m_backbufferSize.Width = std::max(m_backbufferSize.Width, 1.f);
    m_backbufferSize.Height = std::max(m_backbufferSize.Height, 1.f);

    // The width and height of the swap chain must be based on the window's
    // natively-oriented width and height. If the window is not in the native
    // orientation, the dimensions must be reversed.
    DXGI_MODE_ROTATION displayRotation = ComputeDisplayRotation();

    bool swapDimensions = displayRotation == DXGI_MODE_ROTATION_ROTATE90 || displayRotation == DXGI_MODE_ROTATION_ROTATE270;
    m_d3dRenderTargetSize.Width = swapDimensions ? m_backbufferSize.Height : m_backbufferSize.Width;
    m_d3dRenderTargetSize.Height = swapDimensions ? m_backbufferSize.Width : m_backbufferSize.Height;

    uint32 swapchainBufferWidth = lround(m_d3dRenderTargetSize.Width);
    uint32 swapchainBufferHeight = lround(m_d3dRenderTargetSize.Height) + (DAVA::UWPWorkaround::enableSurfaceSizeWorkaround ? 1 : 0);

    if (m_swapChain != nullptr)
    {
        ID3D11RenderTargetView* view[] = { nullptr };
        m_d3dContext->OMSetRenderTargets(1, view, nullptr);
        m_d3dRenderTargetView.Reset();
        m_swapChainBuffer.Reset();

        m_d3dDepthStencilBuffer.Reset();
        m_d3dDepthStencilView.Reset();

        m_d3dContext->ClearState();
        m_d3dContext->Flush();

        // If the swap chain already exists, resize it.
        HRESULT hr = m_swapChain->ResizeBuffers(backBuffersCount, swapchainBufferWidth, swapchainBufferHeight, DXGI_FORMAT_B8G8R8A8_UNORM, 0);

        CHECK_HR(hr);

        if (hr == DXGI_ERROR_DEVICE_REMOVED || hr == DXGI_ERROR_DEVICE_RESET)
        {
            // If the device was removed for any reason, a new device and swap chain will need to be created.
            HandleDeviceLost();

            // Everything is set up now. Do not continue execution of this method. HandleDeviceLost will reenter this method
            // and correctly set up the new device.
            return false;
        }
        else
        {
            ThrowIfFailed(hr);
        }
    }
    else
    {
        // Otherwise, create a new one using the same adapter as the existing Direct3D device.
        DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
        swapChainDesc.Width = swapchainBufferWidth; // Match the size of the window.
        swapChainDesc.Height = swapchainBufferHeight;
        swapChainDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM; // This is the most common swap chain format.
        swapChainDesc.Stereo = false;
        swapChainDesc.SampleDesc.Count = 1; // Don't use multi-sampling.
        swapChainDesc.SampleDesc.Quality = 0;
        swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        swapChainDesc.BufferCount = backBuffersCount; // Use triple-buffering to minimize latency.
        swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL; // All Windows Store apps must use this SwapEffect.
        swapChainDesc.Flags = 0;
        swapChainDesc.Scaling = DXGI_SCALING_STRETCH;
        //swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_IGNORE;

        // This sequence obtains the DXGI factory that was used to create the Direct3D device above.
        ComPtr<IDXGIDevice1> dxgiDevice;
        ThrowIfFailed(m_d3dDevice.As(&dxgiDevice));

        ComPtr<IDXGIAdapter> dxgiAdapter;
        ThrowIfFailed(dxgiDevice->GetAdapter(&dxgiAdapter));

        ComPtr<IDXGIFactory2> dxgiFactory;
        ThrowIfFailed(dxgiAdapter->GetParent(IID_PPV_ARGS(&dxgiFactory)));

        // When using XAML interop, the swap chain must be created for composition.
        try
        {
            ThrowIfFailed(dxgiFactory->CreateSwapChainForComposition(m_d3dDevice.Get(), &swapChainDesc, nullptr, &m_swapChain));
        }
        catch (...)
        {
            m_dxgiAdapter.Reset();
            m_d3dContext.Reset();
            m_d3Debug.Reset();
            m_d3UserAnnotation.Reset();
            m_d3dDevice.Reset();

            if (_DX11_InitParam.FrameCommandExecutionSync)
                _DX11_InitParam.FrameCommandExecutionSync->Unlock();

            if (DAVA::UWPWorkaround::enableSurfaceSizeWorkaround)
            {
                DAVA::Logger::Error("DX11: failed to create swapchain even with workaround. Terminating application.");
            }
            else
            {
                DAVA::Logger::Error("DX11: failed to create swapchain, attempting with workaround...");
                DAVA::UWPWorkaround::enableSurfaceSizeWorkaround = true;
            }
            return false;
        }

        DVASSERT(m_swapChain != nullptr);

        // Associate swap chain with SwapChainPanel
        // UI changes will need to be dispatched back to the UI thread
        using Windows::UI::Core::CoreDispatcherPriority;
        using Windows::UI::Core::DispatchedHandler;
        m_swapChainPanel->Dispatcher->RunAsync(CoreDispatcherPriority::High, ref new DispatchedHandler([]() {
                                                   // Get backing native interface for SwapChainPanel
                                                   ComPtr<ISwapChainPanelNative> panelNative;
                                                   ThrowIfFailed(reinterpret_cast<IUnknown*>(m_swapChainPanel)->QueryInterface(IID_PPV_ARGS(&panelNative)));
                                                   ThrowIfFailed(panelNative->SetSwapChain(m_swapChain.Get()));
                                               },
                                                                                                       Platform::CallbackContext::Any));

        // Ensure that DXGI does not queue more than one frame at a time. This both reduces latency and
        // ensures that the application will only render after each VSync, minimizing power consumption.
        ThrowIfFailed(dxgiDevice->SetMaximumFrameLatency(1));
    }

// Set the proper orientation for the swap chain, and generate 2D and
// 3D matrix transformations for rendering to the rotated swap chain.
// Note the rotation angle for the 2D and 3D transforms are different.
// This is due to the difference in coordinate spaces.  Additionally,
// the 3D matrix is specified explicitly to avoid rounding errors.
#if 0
    switch (displayRotation)
    {
        case DXGI_MODE_ROTATION_IDENTITY:
            m_orientationTransform3D = ScreenRotation::Rotation0;
            break;

        case DXGI_MODE_ROTATION_ROTATE90:
            m_orientationTransform3D = ScreenRotation::Rotation270;
            break;

        case DXGI_MODE_ROTATION_ROTATE180:
            m_orientationTransform3D = ScreenRotation::Rotation180;
            break;

        case DXGI_MODE_ROTATION_ROTATE270:
            m_orientationTransform3D = ScreenRotation::Rotation90;
            break;

        default:
            throw ref new Platform::FailureException();
    }

    ThrowIfFailed(
        m_swapChain->SetRotation(displayRotation)
    );
#endif

    ComPtr<IDXGISwapChain2> spSwapChain2;
    ThrowIfFailed(m_swapChain.As<IDXGISwapChain2>(&spSwapChain2));

    DXGI_MATRIX_3X2_F inverseScale = { 0 };

    inverseScale._11 = 1.0f / m_backbufferScale.Width;
    inverseScale._22 = 1.0f / m_backbufferScale.Height;

    ThrowIfFailed(spSwapChain2->SetMatrixTransform(&inverseScale));

    // Create a render target view of the swap chain back buffer.
    ThrowIfFailed(
    m_swapChain->GetBuffer(0, IID_PPV_ARGS(&m_swapChainBuffer)));

    ThrowIfFailed(
    m_d3dDevice->CreateRenderTargetView(
    m_swapChainBuffer.Get(),
    nullptr,
    &m_d3dRenderTargetView));

    // Create a depth stencil view for use with 3D rendering if needed.
    CD3D11_TEXTURE2D_DESC depthStencilDesc(
    DXGI_FORMAT_D24_UNORM_S8_UINT,
    swapchainBufferWidth,
    swapchainBufferHeight,
    1, // This depth stencil view has only one texture.
    1, // Use a single mipmap level.
    D3D11_BIND_DEPTH_STENCIL);

    ThrowIfFailed(
    m_d3dDevice->CreateTexture2D(
    &depthStencilDesc,
    nullptr,
    &m_d3dDepthStencilBuffer));

    CD3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc(D3D11_DSV_DIMENSION_TEXTURE2D);
    ThrowIfFailed(
    m_d3dDevice->CreateDepthStencilView(
    m_d3dDepthStencilBuffer.Get(),
    &depthStencilViewDesc,
    &m_d3dDepthStencilView));

    // Set the 3D rendering viewport to target the entire window.
    m_screenViewport = CD3D11_VIEWPORT(
    0.0f,
    0.0f,
    m_d3dRenderTargetSize.Width,
    m_d3dRenderTargetSize.Height);

    m_d3dContext->RSSetViewports(1, &m_screenViewport);

    if (_DX11_InitParam.FrameCommandExecutionSync)
        _DX11_InitParam.FrameCommandExecutionSync->Unlock();

    return true;
}

// This method is called when the XAML control is created (or re-created).
void SetSwapChainPanel(Windows::UI::Xaml::Controls::SwapChainPanel ^ panel)
{
    m_swapChainPanel = panel;
}

// This method is called in the event handler for the SizeChanged event.
void SetBackBufferSize(const Windows::Foundation::Size& backbufferSize, const Windows::Foundation::Size& backbufferScale)
{
    m_backbufferSize = backbufferSize;
    m_backbufferScale = backbufferScale;
}

// This method is called in the event handler for the OrientationChanged event.
void SetCurrentOrientation(Windows::Graphics::Display::DisplayOrientations currentOrientation)
{
    if (m_currentOrientation != currentOrientation)
    {
        m_currentOrientation = currentOrientation;
        CreateWindowSizeDependentResources();
    }
}

// This method is called in the event handler for the DisplayContentsInvalidated event.
void ValidateDevice()
{
    // The D3D Device is no longer valid if the default adapter changed since the device
    // was created or if the device has been removed.

    // First, get the information for the default adapter from when the device was created.

    ComPtr<IDXGIDevice1> dxgiDevice;
    ThrowIfFailed(m_d3dDevice.As(&dxgiDevice));

    ComPtr<IDXGIAdapter> deviceAdapter;
    ThrowIfFailed(dxgiDevice->GetAdapter(&deviceAdapter));

    ComPtr<IDXGIFactory2> deviceFactory;
    ThrowIfFailed(deviceAdapter->GetParent(IID_PPV_ARGS(&deviceFactory)));

    ComPtr<IDXGIAdapter1> previousDefaultAdapter;
    ThrowIfFailed(deviceFactory->EnumAdapters1(0, &previousDefaultAdapter));

    DXGI_ADAPTER_DESC previousDesc;
    ThrowIfFailed(previousDefaultAdapter->GetDesc(&previousDesc));

    // Next, get the information for the current default adapter.

    ComPtr<IDXGIFactory2> currentFactory;
    ThrowIfFailed(CreateDXGIFactory1(IID_PPV_ARGS(&currentFactory)));

    ComPtr<IDXGIAdapter1> currentDefaultAdapter;
    ThrowIfFailed(currentFactory->EnumAdapters1(0, &currentDefaultAdapter));

    DXGI_ADAPTER_DESC currentDesc;
    ThrowIfFailed(currentDefaultAdapter->GetDesc(&currentDesc));

    // If the adapter LUIDs don't match, or if the device reports that it has been removed,
    // a new D3D device must be created.

    if (previousDesc.AdapterLuid.LowPart != currentDesc.AdapterLuid.LowPart ||
        previousDesc.AdapterLuid.HighPart != currentDesc.AdapterLuid.HighPart ||
        FAILED(m_d3dDevice->GetDeviceRemovedReason()))
    {
        // Release references to resources related to the old device.
        dxgiDevice = nullptr;
        deviceAdapter = nullptr;
        deviceFactory = nullptr;
        previousDefaultAdapter = nullptr;

        // Create a new device and swap chain.
        HandleDeviceLost();
    }
}

// Recreate all device resources and set them back to the current state.
void HandleDeviceLost()
{
    m_swapChain = nullptr;
    CreateDeviceResources();
    CreateWindowSizeDependentResources();
}

// Call this method when the app suspends. It provides a hint to the driver that the app
// is entering an idle state and that temporary buffers can be reclaimed for use by other apps.
void Trim()
{
#if 0
    ComPtr<IDXGIDevice3> dxgiDevice;
    m_d3dDevice.As(&dxgiDevice);

    dxgiDevice->Trim();
#endif
}

// Present the contents of the swap chain to the screen.
void Present()
{
#if 0
    // The first argument instructs DXGI to block until VSync, putting the application
    // to sleep until the next VSync. This ensures we don't waste any cycles rendering
    // frames that will never be displayed to the screen.
    HRESULT hr = m_swapChain->Present(1, 0);

    // Discard the contents of the render target.
    // This is a valid operation only when the existing contents will be entirely
    // overwritten. If dirty or scroll rects are used, this call should be removed.
    m_d3dContext->DiscardView(m_d3dRenderTargetView.Get());

    // Discard the contents of the depth stencil.
    m_d3dContext->DiscardView(m_d3dDepthStencilView.Get());

    // If the device was removed either by a disconnection or a driver upgrade, we 
    // must recreate all device resources.
    if (hr == DXGI_ERROR_DEVICE_REMOVED || hr == DXGI_ERROR_DEVICE_RESET)
    {
        HandleDeviceLost();
    }
    else
    {
        ThrowIfFailed(hr);
    }
#endif
}

void init_device_and_swapchain_uap(void* panel)
{
    using ::Windows::UI::Xaml::Controls::SwapChainPanel;
    Windows::Foundation::Size backBufferSize(static_cast<float>(_DX11_InitParam.width), static_cast<float>(_DX11_InitParam.height));
    Windows::Foundation::Size backBufferScale(_DX11_InitParam.scaleX, _DX11_InitParam.scaleY);
    SetBackBufferSize(backBufferSize, backBufferScale);
    SetSwapChainPanel(reinterpret_cast<SwapChainPanel ^>(panel));

    CreateDeviceResources();
    if (!CreateWindowSizeDependentResources() && DAVA::UWPWorkaround::enableSurfaceSizeWorkaround)
    {
        CreateDeviceResources();
        if (!CreateWindowSizeDependentResources())
        {
            abort();
        }
    }

    _D3D11_Device = m_d3dDevice.Get();
    _D3D11_ImmediateContext = m_d3dContext.Get();
    _D3D11_FeatureLevel = m_d3dFeatureLevel;
    _D3D11_Debug = m_d3Debug.Get();
    _D3D11_UserAnnotation = m_d3UserAnnotation.Get();
    _D3D11_SwapChain = m_swapChain.Get();
    _D3D11_SwapChainBuffer = m_swapChainBuffer.Get();
    _D3D11_RenderTargetView = m_d3dRenderTargetView.Get();
    _D3D11_DepthStencilBuffer = m_d3dDepthStencilBuffer.Get();
    _D3D11_DepthStencilView = m_d3dDepthStencilView.Get();
}

void resize_swapchain(int32 width, int32 height, float32 scaleX, float32 scaleY)
{
    SetBackBufferSize(Windows::Foundation::Size(static_cast<float32>(width), static_cast<float32>(height)),
                      Windows::Foundation::Size(scaleX, scaleY));

    rhi::CommandBufferDX11::DiscardAll();
    rhi::ConstBufferDX11::InvalidateAll();
    CreateWindowSizeDependentResources();

    _D3D11_SwapChain = m_swapChain.Get();
    _D3D11_SwapChainBuffer = m_swapChainBuffer.Get();
    _D3D11_RenderTargetView = m_d3dRenderTargetView.Get();
    _D3D11_DepthStencilBuffer = m_d3dDepthStencilBuffer.Get();
    _D3D11_DepthStencilView = m_d3dDepthStencilView.Get();
}

void get_device_description(char* dst)
{
    if (m_dxgiAdapter)
    {
        DXGI_ADAPTER_DESC desc = { 0 };
        if (SUCCEEDED(m_dxgiAdapter->GetDesc(&desc)))
        {
            ::WideCharToMultiByte(CP_ACP, WC_NO_BEST_FIT_CHARS, desc.Description, -1, dst, 128, NULL, NULL);
        }
    }
}

#endif