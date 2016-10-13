#include "rhi_DX9.h"
    #include "../Common/rhi_Impl.h"
    
    #include "Debug/DVAssert.h"
    #include "Logger/Logger.h"
    #include "Core/Core.h"
using DAVA::Logger;

    #include "_dx9.h"
    #include "../rhi_Type.h"
    #include "../Common/dbg_StatSet.h"

    #include <vector>


#define E_MINSPEC (-3) // Error code for gfx-card that doesn't meet min.spec

namespace rhi
{
//==============================================================================

Dispatch DispatchDX9 = {};

//==============================================================================

struct
DisplayMode
{
    unsigned width;
    unsigned height;
    //    Texture::Format format;
    unsigned refresh_rate;
};

std::vector<DisplayMode> _DisplayMode;

//------------------------------------------------------------------------------

static Api
dx9_HostApi()
{
    return RHI_DX9;
}

//------------------------------------------------------------------------------

static bool
dx9_TextureFormatSupported(TextureFormat format, ProgType progType)
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
    case TEXTURE_FORMAT_R32F:
    case TEXTURE_FORMAT_RGBA32F:
        supported = true;
        break;
    }

    if (progType == PROG_VERTEX)
    {
        const char* found = strstr(DeviceCaps().deviceDescription, "GeForce");
        if (found && strlen(found) >= strlen("GeForce XXX0")) //filter GeForce 6 and 7 series
        {
            if ((found[8] == '6' || found[8] == '7') && found[11] == '0')
            {
                supported = (format == TEXTURE_FORMAT_R32F || format == TEXTURE_FORMAT_RGBA32F);
            }
        }
    }

    return supported;
}

//------------------------------------------------------------------------------

static bool
_IsValidIntelCardDX9(unsigned vendor_id, unsigned device_id)
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

//------------------------------------------------------------------------------

static void
dx9_Uninitialize()
{
    QueryBufferDX9::ReleaseQueryPool();
    PerfQueryDX9::ReleasePerfQueryPool();
    UninitializeRenderThreadDX9();
}

//------------------------------------------------------------------------------

static void
dx9_Reset(const ResetParam& param)
{
    UINT interval = (param.vsyncEnabled) ? D3DPRESENT_INTERVAL_ONE : D3DPRESENT_INTERVAL_IMMEDIATE;

    if (param.width != _DX9_PresentParam.BackBufferWidth
        || param.height != _DX9_PresentParam.BackBufferHeight
        || param.fullScreen != !_DX9_PresentParam.Windowed
        || interval != _DX9_PresentParam.PresentationInterval
        )
    {
        _DX9_PresentParam.BackBufferWidth = param.width;
        _DX9_PresentParam.BackBufferHeight = param.height;
        _DX9_PresentParam.Windowed = !param.fullScreen;
        _DX9_PresentParam.PresentationInterval = interval;

        ScheduleDeviceReset();
    }
}

//------------------------------------------------------------------------------

static bool
dx9_NeedRestoreResources()
{
    uint32 pendingTextures = TextureDX9::NeedRestoreCount();
    uint32 pendingVertexBuffers = VertexBufferDX9::NeedRestoreCount();
    uint32 pendingIndexBuffers = IndexBufferDX9::NeedRestoreCount();

    bool needRestore = (pendingTextures || pendingVertexBuffers || pendingIndexBuffers);
    if (needRestore)
    {
        Logger::Debug("NeedRestore %d TEX, %d VB, %d IB", pendingTextures, pendingVertexBuffers, pendingIndexBuffers);
    }
    return needRestore;
}

static void dx9_SynchronizeCPUGPU(uint64* cpuTimestamp, uint64* gpuTimestamp)
{
    DX9Command cmd = { DX9Command::SYNC_CPU_GPU, { uint64(cpuTimestamp), uint64(gpuTimestamp) } };
    ExecDX9(&cmd, 1, false);
}

//------------------------------------------------------------------------------

void DX9CheckMultisampleSupport()
{
    const _D3DFORMAT formatsToCheck[] = { D3DFMT_A8R8G8B8, D3DFMT_D24S8 };
    const D3DMULTISAMPLE_TYPE samplesToCheck[] = { D3DMULTISAMPLE_2_SAMPLES, D3DMULTISAMPLE_4_SAMPLES, D3DMULTISAMPLE_8_SAMPLES };

    uint32 sampleCount = 2;

    for (uint32 s = 0; (s < countof(samplesToCheck)) && (sampleCount <= 8); ++s, sampleCount *= 2)
    {
        DWORD qualityLevels = 0;
        for (uint32 f = 0; f < countof(formatsToCheck); ++f)
        {
            HRESULT hr = _D3D9->CheckDeviceMultiSampleType(_D3D9_Adapter, D3DDEVTYPE_HAL, formatsToCheck[f], TRUE, samplesToCheck[s], &qualityLevels);
            if (FAILED(hr))
            {
                break;
            }
        }

        if (qualityLevels == 0)
        {
            DAVA::Logger::Info("DX9 max multisample samples: %u", sampleCount);
            break;
        }
    }

    MutableDeviceCaps::Get().maxSamples = sampleCount / 2;
}

//------------------------------------------------------------------------------

struct AdapterInfo
{
    UINT index;
    D3DCAPS9 caps;
    D3DADAPTER_IDENTIFIER9 info;
};

void dx9_EnumerateAdapters(DAVA::Vector<AdapterInfo>& adapters)
{
    for (UINT i = 0, e = _D3D9->GetAdapterCount(); i < e; ++i)
    {
        AdapterInfo adapter = { i };
        HRESULT hr = _D3D9->GetAdapterIdentifier(i, 0, &adapter.info);
        if (SUCCEEDED(hr))
        {
            hr = _D3D9->GetDeviceCaps(i, D3DDEVTYPE_HAL, &adapter.caps);
            if (SUCCEEDED(hr))
            {
                DAVA::Logger::Info("DX9 DEVICE: %s", adapter.info.Description);
                adapters.push_back(adapter);
            }
        }
    }
}

bool dx9_SelectAdapter(DAVA::Vector<AdapterInfo>& adapters, DWORD& vertex_processing, UINT& indexInVector)
{
    // sort adapters by preferred vendor id
    std::sort(adapters.begin(), adapters.end(), [](const AdapterInfo& l, const AdapterInfo& r) {
        static const UINT preferredVendorIds[] =
        {
          0x10DE, // nVIDIA
          0x1002, // ATI
          0x8086, // Intel
        };
        UINT leftId = UINT(-1);
        UINT rightId = UINT(-1);
        for (UINT i = 0; i < countof(preferredVendorIds); ++i)
        {
            if (preferredVendorIds[i] == l.info.VendorId)
                leftId = i;
            if (preferredVendorIds[i] == r.info.VendorId)
                rightId = i;
        }
        return leftId < rightId;
    });

    // now go through sorted (by preferred vendor) adapters
    // and find one with hadrware vertex processing
    indexInVector = 0;
    vertex_processing = E_FAIL;
    for (const AdapterInfo& adapter : adapters)
    {
        if (adapter.caps.DevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT)
        {
            vertex_processing = D3DCREATE_HARDWARE_VERTEXPROCESSING;
            _D3D9_Adapter = adapter.index;
            return true;
        }
        else if (_IsValidIntelCardDX9(adapter.info.VendorId, adapter.info.DeviceId))
        {
            vertex_processing = D3DCREATE_SOFTWARE_VERTEXPROCESSING;
            _D3D9_Adapter = adapter.index;
        }
        else
        {
            Logger::Error("GPU %s does not meet minimum specs: Intel(R) 845G or Hardware T&L chip required", adapter.info.Description);
        }
        ++indexInVector;
    }
    return (vertex_processing != E_FAIL);
}

void _InitDX9()
{
    _D3D9 = Direct3DCreate9(D3D_SDK_VERSION);

    if (_D3D9 == nullptr)
    {
        Logger::Error("Failed to create Direct3D object");
        return;
    }

    DAVA::Vector<AdapterInfo> adapters;
    dx9_EnumerateAdapters(adapters);

    UINT index = 0;
    DWORD vertex_processing = E_FAIL;
    if (dx9_SelectAdapter(adapters, vertex_processing, index))
    {
        const AdapterInfo& adapter = adapters.at(index);

        // CRAP: hardcoded params
        HWND wnd = (HWND)_DX9_InitParam.window;

        _DX9_PresentParam.hDeviceWindow = wnd;
        _DX9_PresentParam.Windowed = TRUE;
        _DX9_PresentParam.BackBufferFormat = D3DFMT_UNKNOWN;
        _DX9_PresentParam.BackBufferWidth = _DX9_InitParam.width;
        _DX9_PresentParam.BackBufferHeight = _DX9_InitParam.height;
        _DX9_PresentParam.SwapEffect = D3DSWAPEFFECT_DISCARD;
        _DX9_PresentParam.BackBufferCount = _DX9_InitParam.vsyncEnabled ? 2 : 1;
        _DX9_PresentParam.EnableAutoDepthStencil = TRUE;
        _DX9_PresentParam.AutoDepthStencilFormat = D3DFMT_D24S8;
        _DX9_PresentParam.PresentationInterval = (_DX9_InitParam.vsyncEnabled) ? D3DPRESENT_INTERVAL_ONE : D3DPRESENT_INTERVAL_IMMEDIATE;

        // TODO: check z-buf formats and create most suitable

        HRESULT hr = _D3D9->CreateDevice(_D3D9_Adapter, D3DDEVTYPE_HAL, wnd, vertex_processing, &_DX9_PresentParam, &_D3D9_Device);
        if (SUCCEEDED(hr))
        {
            Memcpy(MutableDeviceCaps::Get().deviceDescription, adapter.info.Description,
                   DAVA::Min(countof(MutableDeviceCaps::Get().deviceDescription), strlen(adapter.info.Description) + 1));

            Logger::Info("Adapter [%u]: %s (%s), driver: %u.%u.%u.%u", _D3D9_Adapter, adapter.info.Description, adapter.info.DeviceName,
                         HIWORD(adapter.info.DriverVersion.HighPart), LOWORD(adapter.info.DriverVersion.HighPart),
                         HIWORD(adapter.info.DriverVersion.LowPart), LOWORD(adapter.info.DriverVersion.LowPart));
        }
        else
        {
            Logger::Error("Failed to create device: %s", D3D9ErrorText(hr));
        }

        if (adapter.caps.RasterCaps & D3DPRASTERCAPS_ANISOTROPY)
        {
            MutableDeviceCaps::Get().maxAnisotropy = adapter.caps.MaxAnisotropy;
        }
    }
    else
    {
        Logger::Error("Failed to select adapter for D3D9");
    }
}

//------------------------------------------------------------------------------

void dx9_Initialize(const InitParam& param)
{
    _DX9_InitParam = param;
    InitializeRenderThreadDX9((param.threadedRenderEnabled) ? param.threadedRenderFrameCount : 0);

    VertexBufferDX9::SetupDispatch(&DispatchDX9);
    IndexBufferDX9::SetupDispatch(&DispatchDX9);
    QueryBufferDX9::SetupDispatch(&DispatchDX9);
    PerfQueryDX9::SetupDispatch(&DispatchDX9);
    TextureDX9::SetupDispatch(&DispatchDX9);
    PipelineStateDX9::SetupDispatch(&DispatchDX9);
    ConstBufferDX9::SetupDispatch(&DispatchDX9);
    DepthStencilStateDX9::SetupDispatch(&DispatchDX9);
    SamplerStateDX9::SetupDispatch(&DispatchDX9);
    RenderPassDX9::SetupDispatch(&DispatchDX9);
    CommandBufferDX9::SetupDispatch(&DispatchDX9);

    DispatchDX9.impl_Uninitialize = &dx9_Uninitialize;
    DispatchDX9.impl_Reset = &dx9_Reset;
    DispatchDX9.impl_HostApi = &dx9_HostApi;
    DispatchDX9.impl_NeedRestoreResources = &dx9_NeedRestoreResources;
    DispatchDX9.impl_TextureFormatSupported = &dx9_TextureFormatSupported;
    DispatchDX9.impl_SyncCPUGPU = &dx9_SynchronizeCPUGPU;

    SetDispatchTable(DispatchDX9);

    if (param.maxVertexBufferCount)
        VertexBufferDX9::Init(param.maxVertexBufferCount);
    if (param.maxIndexBufferCount)
        IndexBufferDX9::Init(param.maxIndexBufferCount);
    if (param.maxConstBufferCount)
        ConstBufferDX9::Init(param.maxConstBufferCount);
    if (param.maxTextureCount)
        TextureDX9::Init(param.maxTextureCount);

    uint32 ringBufferSize = 4 * 1024 * 1024;
    if (param.shaderConstRingBufferSize)
        ringBufferSize = param.shaderConstRingBufferSize;
    ConstBufferDX9::InitializeRingBuffer(ringBufferSize);

    stat_DIP = StatSet::AddStat("rhi'dip", "dip");
    stat_DP = StatSet::AddStat("rhi'dp", "dp");
    stat_SET_PS = StatSet::AddStat("rhi'set-ps", "set-ps");
    stat_SET_TEX = StatSet::AddStat("rhi'set-tex", "set-tex");
    stat_SET_CB = StatSet::AddStat("rhi'set-cb", "set-cb");

    MutableDeviceCaps::Get().is32BitIndicesSupported = true;
    MutableDeviceCaps::Get().isFramebufferFetchSupported = true;
    MutableDeviceCaps::Get().isVertexTextureUnitsSupported = true;
    MutableDeviceCaps::Get().isInstancingSupported = true;
    MutableDeviceCaps::Get().isUpperLeftRTOrigin = true;
    MutableDeviceCaps::Get().isZeroBaseClipRange = true;
    MutableDeviceCaps::Get().isCenterPixelMapping = true;
    MutableDeviceCaps::Get().isPerfQuerySupported = true;
    DX9CheckMultisampleSupport();
}

//==============================================================================
} // namespace rhi
