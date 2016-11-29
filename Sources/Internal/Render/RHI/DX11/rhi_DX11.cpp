#include "rhi_DX11.h"
#include "rhi_DX11.h"
#include "../Common/RenderLoop.h"
#include "../Common/FrameLoop.h"
#include "../Common/dbg_StatSet.h"
#include "Platform/SystemTimer.h"

#if defined(__DAVAENGINE_WIN_UAP__)
#include "Platform/DeviceInfo.h"
#endif

#define RHI_DX11_ASSERT_ON_ERROR 1

extern "C" {
__declspec(dllexport) DWORD NvOptimusEnablement = 0x00000001;
__declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
}

namespace rhi
{
static Dispatch DispatchDX11 = {};
static ResetParam resetParams;
static DAVA::Mutex resetParamsSync;
static DAVA::Mutex pendingSecondaryCmdListSync;
static DAVA::Vector<ID3D11CommandList*> pendingSecondaryCmdLists;

ID3D11Device* _D3D11_Device = nullptr;
IDXGISwapChain* _D3D11_SwapChain = nullptr;
ID3D11Texture2D* _D3D11_SwapChainBuffer = nullptr;
ID3D11RenderTargetView* _D3D11_RenderTargetView = nullptr;
ID3D11Texture2D* _D3D11_DepthStencilBuffer = nullptr;
ID3D11DepthStencilView* _D3D11_DepthStencilView = nullptr;
D3D_FEATURE_LEVEL _D3D11_FeatureLevel = D3D_FEATURE_LEVEL_9_1;
ID3D11DeviceContext* _D3D11_ImmediateContext = nullptr;
ID3D11DeviceContext* _D3D11_SecondaryContext = nullptr;
DAVA::Mutex _D3D11_SecondaryContextSync;
ID3D11Debug* _D3D11_Debug = nullptr;
ID3DUserDefinedAnnotation* _D3D11_UserAnnotation = nullptr;
InitParam _DX11_InitParam;
DWORD _DX11_RenderThreadId = 0;
bool _DX11_UseHardwareCommandBuffers = false;

const UINT DX11_BackBuffersCount = 3; // TODO : make shared variable across DX11 code
const DXGI_FORMAT DX11_BackBufferFormat = DXGI_FORMAT_B8G8R8A8_UNORM;

namespace uap
{
void InitDeviceAndSwapChain(void* panel);
void ResizeSwapChain(int32 width, int32 height, float32 scaleX, float32 scaleY);
void GetDeviceDescription(char* dst);
}

/*
 * Helper functions
 */
static bool _IsValidIntelCardDX11(uint32 vendor_id, uint32 device_id)
{
    return ((vendor_id == 0x8086) && // Intel Architecture
            // These guys are prehistoric :)
            (device_id == 0x2572) || // 865G
            (device_id == 0x3582) || // 855GM
            (device_id == 0x2562) || // 845G
            (device_id == 0x3577) || // 830M
            // These are from 2005 and later
            (device_id == 0x2A02) || // GM965 Device 0
            (device_id == 0x2A03) || // GM965 Device 1
            (device_id == 0x29A2) || // G965 Device 0
            (device_id == 0x29A3) || // G965 Device 1
            (device_id == 0x27A2) || // 945GM Device 0
            (device_id == 0x27A6) || // 945GM Device 1
            (device_id == 0x2772) || // 945G Device 0
            (device_id == 0x2776) || // 945G Device 1
            (device_id == 0x2592) || // 915GM Device 0
            (device_id == 0x2792) || // 915GM Device 1
            (device_id == 0x2582) || // 915G Device 0
            (device_id == 0x2782) // 915G Device 1
            );
}

void DX11_CreateSizeDependentResources(UINT w, UINT h)
{
    HRESULT hr = _D3D11_SwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)(&_D3D11_SwapChainBuffer));
    DX11_ProcessCallResult(hr, "_D3D11_SwapChain->GetBuffer", __FILE__, __LINE__);

    D3D11_TEXTURE2D_DESC ds_desc = {};
    ds_desc.Width = w;
    ds_desc.Height = h;
    ds_desc.MipLevels = 1;
    ds_desc.ArraySize = 1;
    ds_desc.Format = (_D3D11_FeatureLevel == D3D_FEATURE_LEVEL_11_0) ? DXGI_FORMAT_D32_FLOAT : DXGI_FORMAT_D24_UNORM_S8_UINT;
    ds_desc.SampleDesc.Count = 1;
    ds_desc.Usage = D3D11_USAGE_DEFAULT;
    ds_desc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
    DX11DeviceCommand(DX11Command::CREATE_TEXTURE_2D, &ds_desc, 0, &_D3D11_DepthStencilBuffer);
    DX11DeviceCommand(DX11Command::CREATE_RENDER_TARGET_VIEW, _D3D11_SwapChainBuffer, 0, &_D3D11_RenderTargetView);
    DX11DeviceCommand(DX11Command::CREATE_DEPTH_STENCIL_VIEW, _D3D11_DepthStencilBuffer, 0, &_D3D11_DepthStencilView);
}

static void ResizeSwapchain()
{
    DAVA::LockGuard<DAVA::Mutex> lock(resetParamsSync);

#if defined(__DAVAENGINE_WIN_UAP__)
    uap::ResizeSwapChain(resetParams.width, resetParams.height, resetParams.scaleX, resetParams.scaleY);
#else
    DAVA::SafeRelease(_D3D11_RenderTargetView);
    DAVA::SafeRelease(_D3D11_DepthStencilBuffer);
    DAVA::SafeRelease(_D3D11_DepthStencilView);
    DAVA::SafeRelease(_D3D11_SwapChainBuffer);
    HRESULT hr = _D3D11_SwapChain->ResizeBuffers(DX11_BackBuffersCount, resetParams.width, resetParams.height, DX11_BackBufferFormat, 0);
    DX11_ProcessCallResult(hr, "_D3D11_SwapChain->ResizeBuffers", __FILE__, __LINE__);
    DX11_CreateSizeDependentResources(resetParams.width, resetParams.height);
#endif
}

void dx11_InitCaps()
{
    MutableDeviceCaps::Get().is32BitIndicesSupported = true;
    MutableDeviceCaps::Get().isFramebufferFetchSupported = true;
    MutableDeviceCaps::Get().isVertexTextureUnitsSupported = (_D3D11_FeatureLevel >= D3D_FEATURE_LEVEL_10_0);
    MutableDeviceCaps::Get().isUpperLeftRTOrigin = true;
    MutableDeviceCaps::Get().isZeroBaseClipRange = true;
    MutableDeviceCaps::Get().isCenterPixelMapping = false;
    MutableDeviceCaps::Get().isInstancingSupported = (_D3D11_FeatureLevel >= D3D_FEATURE_LEVEL_9_2);
    MutableDeviceCaps::Get().isPerfQuerySupported = (_D3D11_FeatureLevel >= D3D_FEATURE_LEVEL_9_2);
    MutableDeviceCaps::Get().maxAnisotropy = D3D11_REQ_MAXANISOTROPY;

#if defined(__DAVAENGINE_WIN_UAP__)
    if (DAVA::DeviceInfo::GetPlatform() == DAVA::DeviceInfo::ePlatform::PLATFORM_PHONE_WIN_UAP)
    {
        // explicitly disable multisampling support on win phones
        MutableDeviceCaps::Get().maxSamples = 1;
    }
    else
    #endif
    {
        MutableDeviceCaps::Get().maxSamples = DX11_GetMaxSupportedMultisampleCount(_D3D11_Device);
    }

    //Some drivers returns untrue DX feature-level, so check it manually
    if (MutableDeviceCaps::Get().isPerfQuerySupported)
    {
        ID3D11Query* freqQuery = nullptr;
        D3D11_QUERY_DESC desc = { D3D11_QUERY_TIMESTAMP_DISJOINT };
        _D3D11_Device->CreateQuery(&desc, &freqQuery);
        MutableDeviceCaps::Get().isPerfQuerySupported = (freqQuery != nullptr);
        DAVA::SafeRelease(freqQuery);
    }

    D3D11_FEATURE_DATA_THREADING threadingData = {};
    _D3D11_Device->CheckFeatureSupport(D3D11_FEATURE_THREADING, &threadingData, sizeof(threadingData));
    _DX11_UseHardwareCommandBuffers = threadingData.DriverCommandLists && threadingData.DriverConcurrentCreates;
    DAVA::Logger::Info("DX11: hardware command buffers enabled: %d", static_cast<int32>(_DX11_UseHardwareCommandBuffers));

    if (_DX11_UseHardwareCommandBuffers)
        CommandBufferDX11::BindHardwareCommandBufferDispatch(&DispatchDX11);
    else
        CommandBufferDX11::BindSoftwareCommandBufferDispatch(&DispatchDX11);

    SetDispatchTable(DispatchDX11);
}

void ExecDX11(DX11Command* command, uint32 cmdCount, bool force_immediate)
{
    CommonImpl::ImmediateCommand cmd;
    cmd.cmdData = command;
    cmd.cmdCount = cmdCount;
    cmd.forceImmediate = force_immediate;
    RenderLoop::IssueImmediateCommand(&cmd);
}

bool ExecDX11DeviceCommand(DX11Command cmd, const char* cmdName, const char* fileName, DAVA::uint32 line)
{
    DVASSERT(cmd.func >= DX11Command::DEVICE_FIRST_COMMAND);
    DVASSERT(cmd.func < DX11Command::DEVICE_LAST_COMMAND);

    if (_DX11_UseHardwareCommandBuffers || (GetCurrentThreadId() == _DX11_RenderThreadId))
    {
        // running on render thread
        // immediate execution, device validation will occur inside
        ExecDX11(&cmd, 1, true);
    }
    else
    {
        // call occured from secondary (non-render thread)
        // validate device before sending commands to execution
        ValidateDX11Device(cmdName);
        ExecDX11(&cmd, 1, false);
    }

    DX11_ProcessCallResult(cmd.retval, cmdName, fileName, line);
    return SUCCEEDED(cmd.retval);
}

void ValidateDX11Device(const char* call)
{
    if (_D3D11_Device == nullptr)
    {
        DAVA::Logger::Error("DX11 Device is not ready, %s and further calls will be blocked.", call);
        for (;;)
        {
            Sleep(1);
        }
    }
}

uint32 DX11_GetMaxSupportedMultisampleCount(ID3D11Device* device)
{
    DXGI_FORMAT depthFormat = (_D3D11_FeatureLevel == D3D_FEATURE_LEVEL_11_0) ? DXGI_FORMAT_D32_FLOAT : DXGI_FORMAT_D24_UNORM_S8_UINT;
    const DXGI_FORMAT formatsToCheck[] = { DXGI_FORMAT_B8G8R8A8_UNORM, depthFormat };

    uint32 sampleCount = 2;

    for (uint32 s = 0; (sampleCount <= 8); ++s, sampleCount *= 2)
    {
        UINT numQualityLevels = 0;
        for (uint32 f = 0; f < countof(formatsToCheck); ++f)
        {
            UINT formatSupport = 0;
            HRESULT hr = device->CheckFormatSupport(formatsToCheck[f], &formatSupport);
            if (formatSupport & D3D11_FORMAT_SUPPORT_MULTISAMPLE_RENDERTARGET)
            {
                hr = device->CheckMultisampleQualityLevels(formatsToCheck[f], sampleCount, &numQualityLevels);
                if (FAILED(hr) || (numQualityLevels == 0))
                {
                    break;
                }
            }
        }
        if (numQualityLevels == 0)
        {
            DAVA::Logger::Info("DX11 max multisample samples: %u", sampleCount / 2);
            break;
        }
    }

    return sampleCount / 2;
}

bool DX11_CheckResult(HRESULT hr, const char* call, const char* fileName, const DAVA::uint32 line)
{
    if (FAILED(hr))
    {
    #if (RHI_DX11_ASSERT_ON_ERROR)
        DAVA::String error = DAVA::Format("D3D11Error at %s: %d\n%s\nCondition: %s", fileName, line, DX11_GetErrorText(hr), call);
        DVASSERT_MSG(0, error.c_str());
    #else
        DAVA::Logger::Error("D3D11Error at %s: %d\n%s\nCondition: %s", fileName, line, DX11_GetErrorText(hr), call);
    #endif
        return false;
    }

    return true;
}

void DX11_ProcessCallResult(HRESULT hr, const char* call, const char* fileName, const DAVA::uint32 line)
{
    if ((hr == DXGI_ERROR_DEVICE_REMOVED) || (hr == DXGI_ERROR_DEVICE_RESET))
    {
        const char* actualError = DX11_GetErrorText(hr);
        const char* reason = DX11_GetErrorText(_D3D11_Device->GetDeviceRemovedReason());

        DAVA::String info = DAVA::Format("DX11 Device removed/reset\n%s\nat %s [%u]:\n\n%s\n\n%s", call, fileName, line, actualError, reason);

    #if !defined(__DAVAENGINE_DEBUG__) && !defined(ENABLE_ASSERT_MESSAGE) && !defined(ENABLE_ASSERT_LOGGING) && !defined(ENABLE_ASSERT_BREAK)
        // write to log if asserts are disabled
        DAVA::Logger::Error(info.c_str());
    #else
        // assert will automatically write to log
        DVASSERT_MSG(0, info.c_str());
    #endif

        if (_DX11_InitParam.renderingErrorCallback)
        {
            _DX11_InitParam.renderingErrorCallback(RenderingError::DriverError, _DX11_InitParam.renderingErrorCallbackContext);
        }

        _D3D11_Device = nullptr;
    }
    else if (FAILED(hr))
    {
        const char* errorText = DX11_GetErrorText(hr);
        DAVA::Logger::Error("DX11 Device call %s\nat %s [%u] failed:\n%s", call, fileName, line, errorText);
    }
}

#if !defined(__DAVAENGINE_WIN_UAP__)

#define RHI_DX11__FORCE_9X_PROFILE 0

void InitDeviceAndSwapChain()
{
    HRESULT hr;
#if RHI_DX11__FORCE_9X_PROFILE
    D3D_FEATURE_LEVEL feature[] = { D3D_FEATURE_LEVEL_9_3, D3D_FEATURE_LEVEL_9_2, D3D_FEATURE_LEVEL_9_1 };
#else
    D3D_FEATURE_LEVEL feature[] = { /*D3D_FEATURE_LEVEL_11_1, */ D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_9_1 };
#endif
    IDXGIAdapter* defAdapter = NULL;

    // enumerate adapters
    {
        IDXGIFactory* factory = NULL;
        std::vector<IDXGIAdapter*> adapter;
        const uint32 preferredVendorID[] =
        {
          0x10DE, // nVIDIA
          0x1002 // ATI
        };

        if (DX11Check(CreateDXGIFactory1(__uuidof(IDXGIFactory), (void**)&factory)))
        {
            IDXGIAdapter* a = NULL;

            for (UINT i = 0; factory->EnumAdapters(i, &a) != DXGI_ERROR_NOT_FOUND; ++i)
            {
                adapter.push_back(a);
            }
        }

        Logger::Info("detected GPUs (%u) :", adapter.size());
        for (uint32 i = 0; i != adapter.size(); ++i)
        {
            DXGI_ADAPTER_DESC desc = { 0 };

            if (DX11Check(adapter[i]->GetDesc(&desc)))
            {
                char info[128];

                ::WideCharToMultiByte(CP_ACP, WC_NO_BEST_FIT_CHARS, desc.Description, -1, info, countof(info) - 1, NULL, NULL);

                Logger::Info("  adapter[%u]  \"%s\"  vendor= %04X  device= %04X", i, info, desc.VendorId, desc.DeviceId);

                if (!defAdapter)
                {
                    for (uint32 k = 0; k != countof(preferredVendorID); ++k)
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
    }


#if 0
    flags |= D3D11_CREATE_DEVICE_DEBUG;
    flags |= D3D11_CREATE_DEVICE_PREVENT_INTERNAL_THREADING_OPTIMIZATIONS;
#endif

    DXGI_SWAP_CHAIN_DESC swapchain_desc = {};
    swapchain_desc.BufferDesc.Width = _DX11_InitParam.width;
    swapchain_desc.BufferDesc.Height = _DX11_InitParam.height;
    swapchain_desc.BufferDesc.Format = DX11_BackBufferFormat;
    swapchain_desc.BufferDesc.RefreshRate.Numerator = 0;
    swapchain_desc.BufferDesc.RefreshRate.Denominator = 0;
    swapchain_desc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_PROGRESSIVE;
    swapchain_desc.BufferDesc.Scaling = DXGI_MODE_SCALING_CENTERED;
    swapchain_desc.SampleDesc.Count = 1;
    swapchain_desc.SampleDesc.Quality = 0;
    swapchain_desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapchain_desc.BufferCount = DX11_BackBuffersCount;
    swapchain_desc.OutputWindow = (HWND)_DX11_InitParam.window;
    swapchain_desc.Windowed = (_DX11_InitParam.fullScreen) ? FALSE : TRUE;
    swapchain_desc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
    swapchain_desc.Flags = 0;

    DWORD deviceFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT | D3D11_CREATE_DEVICE_DEBUG;

    hr = D3D11CreateDeviceAndSwapChain(defAdapter, (defAdapter) ? D3D_DRIVER_TYPE_UNKNOWN : D3D_DRIVER_TYPE_HARDWARE, NULL,
                                       deviceFlags, feature, countof(feature), D3D11_SDK_VERSION, &swapchain_desc,
                                       &_D3D11_SwapChain, &_D3D11_Device, &_D3D11_FeatureLevel, &_D3D11_ImmediateContext);

    if (FAILED(hr))
    {
        // fall back to 'default' adapter
        hr = D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, deviceFlags, feature, countof(feature),
                                           D3D11_SDK_VERSION, &swapchain_desc, &_D3D11_SwapChain, &_D3D11_Device,
                                           &_D3D11_FeatureLevel, &_D3D11_ImmediateContext);
    }

    if (DX11Check(hr))
    {
        IDXGIDevice* dxgiDevice = nullptr;
        IDXGIAdapter* dxgiAdapter = nullptr;
        _GUID uuid = __uuidof(IDXGIDevice);
        if (DX11DeviceCommand(DX11Command::QUERY_INTERFACE, &uuid, (void**)(&dxgiDevice)))
        {
            if (DX11Check(dxgiDevice->GetAdapter(&dxgiAdapter)))
            {
                DXGI_ADAPTER_DESC desc = {};

                if (DX11Check(dxgiAdapter->GetDesc(&desc)))
                {
                    ::WideCharToMultiByte(CP_ACP, WC_NO_BEST_FIT_CHARS, desc.Description, -1, MutableDeviceCaps::Get().deviceDescription,
                                          countof(MutableDeviceCaps::Get().deviceDescription), NULL, NULL);
                    Logger::Info("using adapter  \"%s\"  vendor= %04X  device= %04X",
                                 MutableDeviceCaps::Get().deviceDescription, desc.VendorId, desc.DeviceId);
                }
            }
        }

        uuid = __uuidof(IDXGIDevice);
        hr = DX11DeviceCommand(DX11Command::QUERY_INTERFACE, &uuid, (void**)(&_D3D11_Debug));
        hr = _D3D11_ImmediateContext->QueryInterface(__uuidof(ID3DUserDefinedAnnotation), (void**)(&_D3D11_UserAnnotation));

        DAVA::LockGuard<DAVA::Mutex> lock(_D3D11_SecondaryContextSync);
        DX11DeviceCommand(DX11Command::CREATE_DEFERRED_CONTEXT, 0, &_D3D11_SecondaryContext);
        DX11_CreateSizeDependentResources(_DX11_InitParam.width, _DX11_InitParam.height);
    }
}
#endif

/*
 * Render pass implementation
 * It is the only one place where it being used
 * and therefore it does not make sense to move this to the separate file
 */
struct RenderPassDX11_t
{
    DAVA::Vector<Handle> commandBuffers;
    Handle perfQueryStart = InvalidHandle;
    Handle perfQueryEnd = InvalidHandle;
    int priority = 0;
};
using RenderPassPoolDX11 = ResourcePool<RenderPassDX11_t, RESOURCE_RENDER_PASS, RenderPassConfig, false>;
RHI_IMPL_POOL(RenderPassDX11_t, RESOURCE_RENDER_PASS, RenderPassConfig, false);

static Handle dx11_RenderPass_Allocate(const RenderPassConfig& passDesc, uint32 cmdBufCount, Handle* cmdBuf)
{
    DVASSERT(cmdBufCount > 0);
    DVASSERT(passDesc.IsValid());

    Handle handle = RenderPassPoolDX11::Alloc();
    RenderPassDX11_t* pass = RenderPassPoolDX11::Get(handle);
    pass->commandBuffers.resize(cmdBufCount);
    pass->priority = passDesc.priority;
    pass->perfQueryStart = passDesc.perfQueryStart;
    pass->perfQueryEnd = passDesc.perfQueryEnd;
    for (uint32 i = 0; i != cmdBufCount; ++i)
    {
        Handle cbHandle = CommandBufferDX11::Allocate(passDesc, (i == 0), (i + 1 == cmdBufCount));
        pass->commandBuffers[i] = cbHandle;
        cmdBuf[i] = cbHandle;
    }
    return handle;
}

static void dx11_RenderPass_Begin(Handle pass)
{
}

static void dx11_RenderPass_End(Handle pass)
{
}

void RenderPassDX11::SetupDispatch(Dispatch* dispatch)
{
    dispatch->impl_Renderpass_Allocate = &dx11_RenderPass_Allocate;
    dispatch->impl_Renderpass_Begin = &dx11_RenderPass_Begin;
    dispatch->impl_Renderpass_End = &dx11_RenderPass_End;
}

void RenderPassDX11::RejectCommandBuffersAndRelease(Handle handle)
{
    RenderPassDX11_t* pp = RenderPassPoolDX11::Get(handle);

    for (Handle cmdBuffer : pp->commandBuffers)
        CommandBufferDX11::SignalAndRelease(cmdBuffer);
    pp->commandBuffers.clear();

    RenderPassPoolDX11::Free(handle);
}

/*
 * Main dispatch implemenation
 */
static Api dx11_HostApi()
{
    return RHI_DX11;
}

static bool dx11_NeedRestoreResources()
{
    return false;
}

static bool dx11_TextureFormatSupported(TextureFormat format, ProgType)
{
    bool supported = false;
    switch (format)
    {
    case TEXTURE_FORMAT_R8G8B8A8:
    case TEXTURE_FORMAT_R5G5B5A1:
    case TEXTURE_FORMAT_R5G6B5:
    case TEXTURE_FORMAT_R4G4B4A4:
    case TEXTURE_FORMAT_R8:
    case TEXTURE_FORMAT_R16:
    case TEXTURE_FORMAT_DXT1:
    case TEXTURE_FORMAT_DXT3:
    case TEXTURE_FORMAT_DXT5:
        supported = true;
        break;
    }
    return supported;
}

static void dx11_Uninitialize()
{
    QueryBufferDX11::ReleaseQueryPool();
    PerfQueryDX11::ReleasePerfQueryPool();
}

static void dx11_ResetBlock()
{
    if (_DX11_UseHardwareCommandBuffers)
    {
        DAVA::LockGuard<DAVA::Mutex> lock(_D3D11_SecondaryContextSync);

        _D3D11_SecondaryContext->ClearState();
        ID3D11CommandList* commandList = nullptr;
        DX11Check(_D3D11_SecondaryContext->FinishCommandList(FALSE, &commandList));
        DAVA::SafeRelease(commandList);
        _D3D11_SecondaryContext->Release();

        DX11DeviceCommand(DX11Command::CREATE_DEFERRED_CONTEXT, 0, &_D3D11_SecondaryContext);
    }
    else
    {
        rhi::ConstBufferDX11::InvalidateAll();
    }

    ID3D11RenderTargetView* view[] = { nullptr };
    _D3D11_ImmediateContext->OMSetRenderTargets(1, view, nullptr);
    ResizeSwapchain();
}

static void dx11_Reset(const ResetParam& param)
{
    {
        DAVA::LockGuard<DAVA::Mutex> lock(resetParamsSync);
        resetParams = param;
    }
    RenderLoop::SetResetPending();
}

static void dx11_SuspendRendering()
{
#if defined(__DAVAENGINE_WIN_UAP__)
    FrameLoop::RejectFrames();

    _GUID uuid = __uuidof(IDXGIDevice3);
    IDXGIDevice3* dxgiDevice3 = nullptr;
    if (DX11DeviceCommand(DX11Command::QUERY_INTERFACE, &uuid, (void**)(&dxgiDevice3)))
    {
        _D3D11_ImmediateContext->ClearState();
        dxgiDevice3->Trim();
        dxgiDevice3->Release();
    }
#endif
}

static void dx11_SynchronizeCPUGPU(uint64* cpuTimestamp, uint64* gpuTimestamp)
{
    DX11Command cmd(DX11Command::SYNC_CPU_GPU, cpuTimestamp, gpuTimestamp);
    ExecDX11(&cmd, 1);
}

static void dx11_InitContext()
{
    _DX11_RenderThreadId = GetCurrentThreadId();

#if defined(__DAVAENGINE_WIN_UAP__)
    uap::InitDeviceAndSwapChain(_DX11_InitParam.window);
    uap::GetDeviceDescription(MutableDeviceCaps::Get().deviceDescription);
    {
        DAVA::LockGuard<DAVA::Mutex> lock(_D3D11_SecondaryContextSync);
        DX11DeviceCommand(DX11Command::CREATE_DEFERRED_CONTEXT, 0, &_D3D11_SecondaryContext);
    }
#else
    InitDeviceAndSwapChain();
#endif

    dx11_InitCaps();

    if (!_DX11_UseHardwareCommandBuffers)
    {
        ConstBufferDX11::InitializeRingBuffer(_DX11_InitParam.shaderConstRingBufferSize);
    }
}

static bool dx11_CheckSurface()
{
    return true;
}

static void dx11_ExecuteQueuedCommands(const CommonImpl::Frame& frame)
{
    DVASSERT(frame.readyToExecute);
    DVASSERT((frame.sync == InvalidHandle) || SyncObjectDX11::IsAlive(frame.sync));

    StatSet::ResetAll();

    DAVA::Vector<RenderPassDX11_t*> pass;
    pass.reserve(frame.pass.size());
    for (Handle p : frame.pass)
    {
        RenderPassDX11_t* pp = RenderPassPoolDX11::Get(p);
        pass.emplace_back(pp);
    }

    // sort from highest to lowest priorities
    std::stable_sort(pass.begin(), pass.end(), [](RenderPassDX11_t* l, RenderPassDX11_t* r) {
        return l->priority > r->priority;
    });

    if (frame.sync != InvalidHandle)
        SyncObjectDX11::SetProperties(frame.sync, frame.frameNumber, false, true);

    PerfQueryDX11::ObtainPerfQueryMeasurment(_D3D11_ImmediateContext);
    PerfQueryDX11::BeginMeasurment(_D3D11_ImmediateContext);

    if (frame.perfQueryStart != InvalidHandle)
        PerfQueryDX11::IssueTimestampQuery(frame.perfQueryStart, _D3D11_ImmediateContext);

    if (_DX11_UseHardwareCommandBuffers)
    {
        DAVA::LockGuard<DAVA::Mutex> lock(pendingSecondaryCmdListSync);
        for (ID3D11CommandList* cmdList : pendingSecondaryCmdLists)
        {
            _D3D11_ImmediateContext->ExecuteCommandList(cmdList, FALSE);
            cmdList->Release();
        }
        pendingSecondaryCmdLists.clear();
    }

    for (RenderPassDX11_t* pp : pass)
    {
        if (pp->perfQueryStart != InvalidHandle)
            PerfQueryDX11::IssueTimestampQuery(pp->perfQueryStart, _D3D11_ImmediateContext);

        for (Handle cb_h : pp->commandBuffers)
            CommandBufferDX11::ExecuteAndRelease(cb_h, frame.frameNumber);

        if (pp->perfQueryEnd != InvalidHandle)
            PerfQueryDX11::IssueTimestampQuery(pp->perfQueryEnd, _D3D11_ImmediateContext);
    }

    for (Handle p : frame.pass)
        RenderPassPoolDX11::Free(p);

    if (frame.perfQueryEnd != InvalidHandle)
        PerfQueryDX11::IssueTimestampQuery(frame.perfQueryEnd, _D3D11_ImmediateContext);

    SyncObjectDX11::CheckFrameAndSignalUsed(frame.frameNumber);
}

static bool dx11_PresentBuffer()
{
    DX11_ProcessCallResult(_D3D11_SwapChain->Present(1, 0), __FUNCTION__, __FILE__, __LINE__);
    PerfQueryDX11::EndMeasurment(_D3D11_ImmediateContext);
    return true;
}

static void dx11_ExecImmediateCommand(CommonImpl::ImmediateCommand* command)
{
    DX11Command* commandData = reinterpret_cast<DX11Command*>(command->cmdData);
    for (DX11Command *cmd = commandData, *cmdEnd = commandData + command->cmdCount; cmd != cmdEnd; ++cmd)
    {
        DAVA::uint64* arg = cmd->arguments.arg;

        Trace("exec %i\n", int(cmd->func));
        switch (cmd->func)
        {
        case DX11Command::NOP:
            break;

        case DX11Command::MAP:
            cmd->retval = _D3D11_ImmediateContext->Map((ID3D11Resource*)(arg[0]), UINT(arg[1]), D3D11_MAP(arg[2]), UINT(arg[3]), (D3D11_MAPPED_SUBRESOURCE*)(arg[4]));
            break;

        case DX11Command::UNMAP:
            _D3D11_ImmediateContext->Unmap((ID3D11Resource*)(arg[0]), UINT(arg[1]));
            break;

        case DX11Command::UPDATE_SUBRESOURCE:
            _D3D11_ImmediateContext->UpdateSubresource((ID3D11Resource*)(arg[0]), UINT(arg[1]), (const D3D11_BOX*)(arg[2]), (const void*)(arg[3]), UINT(arg[4]), UINT(arg[5]));
            break;

        case DX11Command::COPY_RESOURCE:
            _D3D11_ImmediateContext->CopyResource((ID3D11Resource*)(arg[0]), (ID3D11Resource*)(arg[1]));
            break;

        case DX11Command::SYNC_CPU_GPU:
        {
            if (DeviceCaps().isPerfQuerySupported)
            {
                ID3D11Query* tsQuery = nullptr;
                ID3D11Query* fqQuery = nullptr;

                D3D11_QUERY_DESC desc = { D3D11_QUERY_TIMESTAMP };
                DX11DeviceCommand(DX11Command::CREATE_QUERY, &desc, &tsQuery);

                desc.Query = D3D11_QUERY_TIMESTAMP_DISJOINT;
                DX11DeviceCommand(DX11Command::CREATE_QUERY, &desc, &fqQuery);

                if (tsQuery && fqQuery)
                {
                    _D3D11_ImmediateContext->Begin(fqQuery);
                    _D3D11_ImmediateContext->End(tsQuery);
                    _D3D11_ImmediateContext->End(fqQuery);

                    uint64 timestamp = 0;
                    while (S_FALSE == _D3D11_ImmediateContext->GetData(tsQuery, &timestamp, sizeof(uint64), 0))
                    {
                    };

                    if (timestamp)
                    {
                        *reinterpret_cast<uint64*>(arg[0]) = DAVA::SystemTimer::Instance()->GetAbsoluteUs();

                        D3D11_QUERY_DATA_TIMESTAMP_DISJOINT data;
                        while (S_FALSE == _D3D11_ImmediateContext->GetData(fqQuery, &data, sizeof(data), 0))
                        {
                        };

                        if (!data.Disjoint && data.Frequency)
                        {
                            *reinterpret_cast<uint64*>(arg[1]) = timestamp / (data.Frequency / 1000000); //mcs
                        }
                    }
                }
                DAVA::SafeRelease(tsQuery);
                DAVA::SafeRelease(fqQuery);
            }
        }
        break;
        case DX11Command::QUERY_INTERFACE:
        {
            ValidateDX11Device("QueryInterface");
            cmd->retval = _D3D11_Device->QueryInterface(*(const IID*)(arg[0]), (void**)(arg[1]));
            break;
        }
        case DX11Command::CREATE_DEFERRED_CONTEXT:
        {
            ValidateDX11Device("CreateDeferredContext");
            cmd->retval = _D3D11_Device->CreateDeferredContext((UINT)arg[0], (ID3D11DeviceContext**)(arg[1]));
            break;
        }
        case DX11Command::CREATE_BLEND_STATE:
        {
            ValidateDX11Device("CreateBlendState");
            cmd->retval = _D3D11_Device->CreateBlendState((const D3D11_BLEND_DESC*)(arg[0]), (ID3D11BlendState**)(arg[1]));
            break;
        }
        case DX11Command::CREATE_SAMPLER_STATE:
        {
            ValidateDX11Device("CreateSamplerState");
            cmd->retval = _D3D11_Device->CreateSamplerState((const D3D11_SAMPLER_DESC*)(arg[0]), (ID3D11SamplerState**)(arg[1]));
            break;
        }
        case DX11Command::CREATE_RASTERIZER_STATE:
        {
            ValidateDX11Device("CreateRasterizerState");
            cmd->retval = _D3D11_Device->CreateRasterizerState((const D3D11_RASTERIZER_DESC*)(arg[0]), (ID3D11RasterizerState**)(arg[1]));
            break;
        }
        case DX11Command::CREATE_DEPTH_STENCIL_STATE:
        {
            ValidateDX11Device("CreateDepthStencilState");
            cmd->retval = _D3D11_Device->CreateDepthStencilState((const D3D11_DEPTH_STENCIL_DESC*)(arg[0]), (ID3D11DepthStencilState**)(arg[1]));
            break;
        }
        case DX11Command::CREATE_VERTEX_SHADER:
        {
            ValidateDX11Device("CreateVertexShader");
            cmd->retval = _D3D11_Device->CreateVertexShader((const void*)(arg[0]), (SIZE_T)(arg[1]), (ID3D11ClassLinkage*)(arg[2]), (ID3D11VertexShader**)(arg[3]));
            break;
        }
        case DX11Command::CREATE_PIXEL_SHADER:
        {
            ValidateDX11Device("CreatePixelShader");
            cmd->retval = _D3D11_Device->CreatePixelShader((const void*)(arg[0]), (SIZE_T)(arg[1]), (ID3D11ClassLinkage*)(arg[2]), (ID3D11PixelShader**)(arg[3]));
            break;
        }
        case DX11Command::CREATE_INPUT_LAYOUT:
        {
            ValidateDX11Device("CreateInputLayout");
            cmd->retval = _D3D11_Device->CreateInputLayout((const D3D11_INPUT_ELEMENT_DESC*)(arg[0]), (UINT)(arg[1]), (const void*)(arg[2]), (SIZE_T)(arg[3]), (ID3D11InputLayout**)(arg[4]));
            break;
        }
        case DX11Command::CREATE_QUERY:
        {
            ValidateDX11Device("CreateQuery");
            cmd->retval = _D3D11_Device->CreateQuery((const D3D11_QUERY_DESC*)(arg[0]), (ID3D11Query**)(arg[1]));
            break;
        }
        case DX11Command::CREATE_BUFFER:
        {
            ValidateDX11Device("CreateBuffer");
            cmd->retval = _D3D11_Device->CreateBuffer((const D3D11_BUFFER_DESC*)(arg[0]), (const D3D11_SUBRESOURCE_DATA*)(arg[1]), (ID3D11Buffer**)(arg[2]));
            break;
        }
        case DX11Command::CREATE_TEXTURE_2D:
        {
            ValidateDX11Device("CreateTexture2D");
            cmd->retval = _D3D11_Device->CreateTexture2D((const D3D11_TEXTURE2D_DESC*)(arg[0]), (const D3D11_SUBRESOURCE_DATA*)(arg[1]), (ID3D11Texture2D**)(arg[2]));
            break;
        }
        case DX11Command::CREATE_RENDER_TARGET_VIEW:
        {
            ValidateDX11Device("CreateRenderTargetView");
            cmd->retval = _D3D11_Device->CreateRenderTargetView((ID3D11Resource*)(arg[0]), (const D3D11_RENDER_TARGET_VIEW_DESC*)(arg[1]), (ID3D11RenderTargetView**)(arg[2]));
            break;
        }
        case DX11Command::CREATE_DEPTH_STENCIL_VIEW:
        {
            ValidateDX11Device("CreateDepthStencilView");
            cmd->retval = _D3D11_Device->CreateDepthStencilView((ID3D11Resource*)(arg[0]), (const D3D11_DEPTH_STENCIL_VIEW_DESC*)(arg[1]), (ID3D11DepthStencilView**)(arg[2]));
            break;
        }
        case DX11Command::CREATE_SHADER_RESOURCE_VIEW:
        {
            ValidateDX11Device("CreateShaderResourceView");
            cmd->retval = _D3D11_Device->CreateShaderResourceView((ID3D11Resource*)(arg[0]), (const D3D11_SHADER_RESOURCE_VIEW_DESC*)(arg[1]), (ID3D11ShaderResourceView**)(arg[2]));
            break;
        }
        default:
        {
            DAVA::String message = DAVA::Format("Invalid or unsupported DX11 command: %u", cmd->func);
            DVASSERT_MSG(0, message.c_str());
        }
        }
    }
}

static void dx11_EndFrame()
{
    if (_DX11_UseHardwareCommandBuffers)
    {
        ID3D11CommandList* cmdList = nullptr;
        {
            DAVA::LockGuard<DAVA::Mutex> lock(_D3D11_SecondaryContextSync);
            DX11Check(_D3D11_SecondaryContext->FinishCommandList(TRUE, &cmdList));
        }
        {
            DAVA::LockGuard<DAVA::Mutex> lock(pendingSecondaryCmdListSync);
            pendingSecondaryCmdLists.push_back(cmdList);
        }
    }

    ConstBufferDX11::InvalidateAllInstances();
}

static void dx11_RejectFrame(const CommonImpl::Frame& frame)
{
    if (frame.sync != InvalidHandle)
        SyncObjectDX11::SetSignaledAndUsedProperties(frame.sync, true, true);

    for (Handle p : frame.pass)
        RenderPassDX11::RejectCommandBuffersAndRelease(p);
}

void dx11_Initialize(const InitParam& param)
{
    _DX11_InitParam = param;

    VertexBufferDX11::SetupDispatch(&DispatchDX11);
    IndexBufferDX11::SetupDispatch(&DispatchDX11);
    QueryBufferDX11::SetupDispatch(&DispatchDX11);
    PerfQueryDX11::SetupDispatch(&DispatchDX11);
    TextureDX11::SetupDispatch(&DispatchDX11);
    PipelineStateDX11::SetupDispatch(&DispatchDX11);
    ConstBufferDX11::SetupDispatch(&DispatchDX11);
    DepthStencilStateDX11::SetupDispatch(&DispatchDX11);
    SamplerStateDX11::SetupDispatch(&DispatchDX11);
    RenderPassDX11::SetupDispatch(&DispatchDX11);
    SyncObjectDX11::SetupDispatch(&DispatchDX11);

    DispatchDX11.impl_Uninitialize = &dx11_Uninitialize;
    DispatchDX11.impl_Reset = &dx11_Reset;
    DispatchDX11.impl_HostApi = &dx11_HostApi;
    DispatchDX11.impl_TextureFormatSupported = &dx11_TextureFormatSupported;
    DispatchDX11.impl_NeedRestoreResources = &dx11_NeedRestoreResources;
    DispatchDX11.impl_InitContext = &dx11_InitContext;
    DispatchDX11.impl_ValidateSurface = &dx11_CheckSurface;
    DispatchDX11.impl_FinishRendering = &dx11_SuspendRendering;
    DispatchDX11.impl_ResetBlock = &dx11_ResetBlock;
    DispatchDX11.impl_SyncCPUGPU = &dx11_SynchronizeCPUGPU;
    DispatchDX11.impl_ProcessImmediateCommand = &dx11_ExecImmediateCommand;
    DispatchDX11.impl_ExecuteFrame = &dx11_ExecuteQueuedCommands;
    DispatchDX11.impl_RejectFrame = &dx11_RejectFrame;
    DispatchDX11.impl_PresentBuffer = &dx11_PresentBuffer;
    DispatchDX11.impl_FinishFrame = &dx11_EndFrame;

    SetDispatchTable(DispatchDX11);

    if (param.maxVertexBufferCount)
        VertexBufferDX11::Init(param.maxVertexBufferCount);
    if (param.maxIndexBufferCount)
        IndexBufferDX11::Init(param.maxIndexBufferCount);
    if (param.maxConstBufferCount)
        ConstBufferDX11::Init(param.maxConstBufferCount);
    if (param.maxTextureCount)
        TextureDX11::Init(param.maxTextureCount);

    stat_DIP = StatSet::AddStat("rhi'dip", "dip");
    stat_DP = StatSet::AddStat("rhi'dp", "dp");
    stat_DTL = StatSet::AddStat("rhi'dtl", "dtl");
    stat_DTS = StatSet::AddStat("rhi'dts", "dts");
    stat_DLL = StatSet::AddStat("rhi'dll", "dll");
    stat_SET_PS = StatSet::AddStat("rhi'set-ps", "set-ps");
    stat_SET_SS = StatSet::AddStat("rhi'set-ss", "set-ss");
    stat_SET_TEX = StatSet::AddStat("rhi'set-tex", "set-tex");
    stat_SET_CB = StatSet::AddStat("rhi'set-cb", "set-cb");
    stat_SET_VB = StatSet::AddStat("rhi'set-vb", "set-vb");
    stat_SET_IB = StatSet::AddStat("rhi'set-ib", "set-ib");
}
}
