#include <dxgiformat.h>
#include "_dx11.h"
#include <stdio.h>

#include "../rhi_Public.h"
#include "../Common/rhi_Utils.h"

//==============================================================================

namespace rhi
{
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

const char* DX11_GetErrorText(HRESULT hr)
{
    switch (hr)
    {
    case S_OK:
        return "S_OK: No error occurred";

    case D3D11_ERROR_FILE_NOT_FOUND:
        return "D3D11_ERROR_FILE_NOT_FOUND: The file was not found";

    case D3D11_ERROR_TOO_MANY_UNIQUE_STATE_OBJECTS:
        return "D3D11_ERROR_TOO_MANY_UNIQUE_STATE_OBJECTS: There are too many unique instances of a particular type of state object";

    case D3D11_ERROR_TOO_MANY_UNIQUE_VIEW_OBJECTS:
        return "D3D11_ERROR_TOO_MANY_UNIQUE_VIEW_OBJECTS: There are too many unique instance of a particular type of view object";

    case D3D11_ERROR_DEFERRED_CONTEXT_MAP_WITHOUT_INITIAL_DISCARD:
        return "D3D11_ERROR_DEFERRED_CONTEXT_MAP_WITHOUT_INITIAL_DISCARD: The first call to ID3D11DeviceContext::Map after either ID3D11Device::CreateDeferredContext or ID3D11DeviceContext::FinishCommandList per Resource was not D3D11_MAP_WRITE_DISCARD";

    case E_FAIL:
        return "E_FAIL: Attempted to create a device with the debug layer enabled and the layer is not installed.";

    case E_INVALIDARG:
        return "E_INVALIDARG: An invalid parameter was passed to the returning function.";

    case E_OUTOFMEMORY:
        return "E_OUTOFMEMORY: Direct3D could not allocate sufficient memory to complete the call.";

    case S_FALSE:
        return "S_FALSE: Alternate success value, indicating a successful but nonstandard completion (the precise meaning depends on context).";

    case DXGI_ERROR_DEVICE_HUNG:
        return "DXGI_ERROR_DEVICE_HUNG: The application's device failed due to badly formed commands sent by the application. This is an design-time issue that should be investigated and fixed.";

    case DXGI_ERROR_DEVICE_REMOVED:
        return "DXGI_ERROR_DEVICE_REMOVED: The video card has been physically removed from the system, or a driver upgrade for the video card has occurred.";

    case DXGI_ERROR_DEVICE_RESET:
        return "DXGI_ERROR_DEVICE_RESET: The device failed due to a badly formed command.";

    case DXGI_ERROR_DRIVER_INTERNAL_ERROR:
        return "DXGI_ERROR_DRIVER_INTERNAL_ERROR: The driver encountered a problem and was put into the device removed state.";

    case DXGI_ERROR_FRAME_STATISTICS_DISJOINT:
        return "DXGI_ERROR_FRAME_STATISTICS_DISJOINT: An event (for example, a power cycle) interrupted the gathering of presentation statistics.";

    case DXGI_ERROR_GRAPHICS_VIDPN_SOURCE_IN_USE:
        return "DXGI_ERROR_GRAPHICS_VIDPN_SOURCE_IN_USE: The application attempted to acquire exclusive ownership of an output, but failed because some other application (or device within the application) already acquired ownership.";

    case DXGI_ERROR_INVALID_CALL:
        return "DXGI_ERROR_INVALID_CALL: The application provided invalid parameter data; this must be debugged and fixed before the application is released.";

    case DXGI_ERROR_MORE_DATA:
        return "DXGI_ERROR_MORE_DATA: The buffer supplied by the application is not big enough to hold the requested data.";

    case DXGI_ERROR_NONEXCLUSIVE:
        return "DXGI_ERROR_NONEXCLUSIVE: A global counter resource is in use, and the Direct3D device can't currently use the counter resource.";

    case DXGI_ERROR_NOT_CURRENTLY_AVAILABLE:
        return "DXGI_ERROR_NOT_CURRENTLY_AVAILABLE: The resource or request is not currently available, but it might become available later.";

    case DXGI_ERROR_NOT_FOUND:
        return "DXGI_ERROR_NOT_FOUND: When calling IDXGIObject::GetPrivateData, the GUID passed in is not recognized as one previously passed to IDXGIObject::SetPrivateData or IDXGIObject::SetPrivateDataInterface. When calling IDXGIFactory::EnumAdapters or IDXGIAdapter::EnumOutputs, the enumerated ordinal is out of range.";

    case DXGI_ERROR_REMOTE_CLIENT_DISCONNECTED:
        return "DXGI_ERROR_REMOTE_CLIENT_DISCONNECTED";

    case DXGI_ERROR_REMOTE_OUTOFMEMORY:
        return "DXGI_ERROR_REMOTE_OUTOFMEMORY";

    case DXGI_ERROR_WAS_STILL_DRAWING:
        return "DXGI_ERROR_WAS_STILL_DRAWING: The GPU was busy at the moment when a call was made to perform an operation, and did not execute or schedule the operation.";

    case DXGI_ERROR_UNSUPPORTED:
        return "DXGI_ERROR_UNSUPPORTED: The requested functionality is not supported by the device or the driver.";

    case DXGI_ERROR_ACCESS_LOST:
        return "DXGI_ERROR_ACCESS_LOST: The desktop duplication interface is invalid. The desktop duplication interface typically becomes invalid when a different type of image is displayed on the desktop.";

    case DXGI_ERROR_WAIT_TIMEOUT:
        return "DXGI_ERROR_WAIT_TIMEOUT: The time-out interval elapsed before the next desktop frame was available.";

    case DXGI_ERROR_SESSION_DISCONNECTED:
        return "DXGI_ERROR_SESSION_DISCONNECTED: The Remote Desktop Services session is currently disconnected.";

    case DXGI_ERROR_RESTRICT_TO_OUTPUT_STALE:
        return "DXGI_ERROR_RESTRICT_TO_OUTPUT_STALE: The DXGI output (monitor) to which the swap chain content was restricted is now disconnected or changed.";

    case DXGI_ERROR_CANNOT_PROTECT_CONTENT:
        return "DXGI_ERROR_CANNOT_PROTECT_CONTENT: DXGI can't provide content protection on the swap chain. This error is typically caused by an older driver, or when you use a swap chain that is incompatible with content protection.";

    case DXGI_ERROR_ACCESS_DENIED:
        return "DXGI_ERROR_ACCESS_DENIED: You tried to use a resource to which you did not have the required access privileges. This error is most typically caused when you write to a shared resource with read-only access.";

    case DXGI_ERROR_NAME_ALREADY_EXISTS:
        return "DXGI_ERROR_NAME_ALREADY_EXISTS: The supplied name of a resource in a call to IDXGIResource1::CreateSharedHandle is already associated with some other resource.";

    case DXGI_ERROR_SDK_COMPONENT_MISSING:
        return "DXGI_ERROR_SDK_COMPONENT_MISSING: The operation depends on an SDK component that is missing or mismatched.";
    }

    static char text[1024] = {};
    _snprintf(text, sizeof(text), "unknown D3D9 error (%08X)\n", (unsigned)hr);
    return text;
}

DXGI_FORMAT DX11_TextureFormat(TextureFormat format)
{
    switch (format)
    {
    case TEXTURE_FORMAT_R8G8B8A8:
        return DXGI_FORMAT_B8G8R8A8_UNORM;
    case TEXTURE_FORMAT_R8G8B8X8:
        return DXGI_FORMAT_B8G8R8X8_UNORM;

    case TEXTURE_FORMAT_R5G5B5A1:
        return DXGI_FORMAT_B5G5R5A1_UNORM;
    case TEXTURE_FORMAT_R5G6B5:
        return DXGI_FORMAT_B5G6R5_UNORM;

    case TEXTURE_FORMAT_R4G4B4A4:
        return DXGI_FORMAT_B4G4R4A4_UNORM;

    case TEXTURE_FORMAT_A16R16G16B16:
        return DXGI_FORMAT_R16G16B16A16_FLOAT;
    case TEXTURE_FORMAT_A32R32G32B32:
        return DXGI_FORMAT_R32G32B32A32_FLOAT;

    case TEXTURE_FORMAT_R8:
        return DXGI_FORMAT_R8_UNORM;
    case TEXTURE_FORMAT_R16:
        return DXGI_FORMAT_R16_UNORM;

    case TEXTURE_FORMAT_DXT1:
        return DXGI_FORMAT_BC1_UNORM;
    case TEXTURE_FORMAT_DXT3:
        return DXGI_FORMAT_BC2_UNORM;
    case TEXTURE_FORMAT_DXT5:
        return DXGI_FORMAT_BC3_UNORM;

    case TEXTURE_FORMAT_PVRTC2_4BPP_RGB:
    case TEXTURE_FORMAT_PVRTC2_4BPP_RGBA:
    case TEXTURE_FORMAT_PVRTC2_2BPP_RGB:
    case TEXTURE_FORMAT_PVRTC2_2BPP_RGBA:
        return DXGI_FORMAT_UNKNOWN;

    case TEXTURE_FORMAT_ATC_RGB:
    case TEXTURE_FORMAT_ATC_RGBA_EXPLICIT:
    case TEXTURE_FORMAT_ATC_RGBA_INTERPOLATED:
        return DXGI_FORMAT_UNKNOWN;

    case TEXTURE_FORMAT_ETC1:
    case TEXTURE_FORMAT_ETC2_R8G8B8:
    case TEXTURE_FORMAT_ETC2_R8G8B8A8:
    case TEXTURE_FORMAT_ETC2_R8G8B8A1:
        return DXGI_FORMAT_UNKNOWN;

    case TEXTURE_FORMAT_EAC_R11_UNSIGNED:
    case TEXTURE_FORMAT_EAC_R11_SIGNED:
    case TEXTURE_FORMAT_EAC_R11G11_UNSIGNED:
    case TEXTURE_FORMAT_EAC_R11G11_SIGNED:
        return DXGI_FORMAT_UNKNOWN;

    case TEXTURE_FORMAT_D16:
        return DXGI_FORMAT_D16_UNORM;
    case TEXTURE_FORMAT_D24S8:
        return DXGI_FORMAT_D24_UNORM_S8_UINT;

    case TEXTURE_FORMAT_R16F:
        return DXGI_FORMAT_R16_FLOAT;
    case TEXTURE_FORMAT_RG16F:
        return DXGI_FORMAT_R16G16_FLOAT;
    case TEXTURE_FORMAT_RGBA16F:
        return DXGI_FORMAT_R16G16B16A16_FLOAT;

    case TEXTURE_FORMAT_R32F:
        return DXGI_FORMAT_R32_FLOAT;
    case TEXTURE_FORMAT_RG32F:
        return DXGI_FORMAT_R32G32_FLOAT;
    case TEXTURE_FORMAT_RGBA32F:
        return DXGI_FORMAT_R32G32B32A32_FLOAT;

    default:
        DVASSERT_MSG(0, "Invalid TextureFormat provided");
    }

    return DXGI_FORMAT_UNKNOWN;
}

D3D11_COMPARISON_FUNC DX11_CmpFunc(CmpFunc func)
{
    switch (func)
    {
    case CMP_NEVER:
        return D3D11_COMPARISON_NEVER;
    case CMP_LESS:
        return D3D11_COMPARISON_LESS;
    case CMP_EQUAL:
        return D3D11_COMPARISON_EQUAL;
    case CMP_LESSEQUAL:
        return D3D11_COMPARISON_LESS_EQUAL;
    case CMP_GREATER:
        return D3D11_COMPARISON_GREATER;
    case CMP_NOTEQUAL:
        return D3D11_COMPARISON_NOT_EQUAL;
    case CMP_GREATEREQUAL:
        return D3D11_COMPARISON_GREATER_EQUAL;
    case CMP_ALWAYS:
        return D3D11_COMPARISON_ALWAYS;
    default:
        DVASSERT_MSG(0, "Invalid CmpFunc provided");
    }

    return D3D11_COMPARISON_ALWAYS;
}

D3D11_STENCIL_OP DX11_StencilOp(StencilOperation op)
{
    switch (op)
    {
    case STENCILOP_KEEP:
        return D3D11_STENCIL_OP_KEEP;
    case STENCILOP_ZERO:
        return D3D11_STENCIL_OP_ZERO;
    case STENCILOP_REPLACE:
        return D3D11_STENCIL_OP_REPLACE;
    case STENCILOP_INVERT:
        return D3D11_STENCIL_OP_INVERT;
    case STENCILOP_INCREMENT_CLAMP:
        return D3D11_STENCIL_OP_INCR_SAT;
    case STENCILOP_DECREMENT_CLAMP:
        return D3D11_STENCIL_OP_DECR_SAT;
    case STENCILOP_INCREMENT_WRAP:
        return D3D11_STENCIL_OP_INCR;
    case STENCILOP_DECREMENT_WRAP:
        return D3D11_STENCIL_OP_DECR;
    default:
        DVASSERT_MSG(0, "Invalid StencilOperation provided");
    }
    return D3D11_STENCIL_OP_KEEP;
}

D3D11_FILTER DX11_TextureFilter(TextureFilter min_filter, TextureFilter mag_filter, TextureMipFilter mip_filter, DAVA::uint32 anisotropy)
{
    switch (mip_filter)
    {
    case TEXMIPFILTER_NONE:
    case TEXMIPFILTER_NEAREST:
    {
        if (min_filter == TEXFILTER_NEAREST && mag_filter == TEXFILTER_NEAREST)
            return D3D11_FILTER_MIN_MAG_MIP_POINT;
        else if (min_filter == TEXFILTER_NEAREST && mag_filter == TEXFILTER_LINEAR)
            return D3D11_FILTER_MIN_POINT_MAG_LINEAR_MIP_POINT;
        else if (min_filter == TEXFILTER_LINEAR && mag_filter == TEXFILTER_NEAREST)
            return D3D11_FILTER_MIN_LINEAR_MAG_MIP_POINT;
        else if (min_filter == TEXFILTER_LINEAR && mag_filter == TEXFILTER_LINEAR)
            return D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT;
    }
    break;

    case TEXMIPFILTER_LINEAR:
    {
        if (anisotropy > 1)
            return D3D11_FILTER_ANISOTROPIC;
        else if (min_filter == TEXFILTER_NEAREST && mag_filter == TEXFILTER_NEAREST)
            return D3D11_FILTER_MIN_MAG_MIP_LINEAR;
        else if (min_filter == TEXFILTER_NEAREST && mag_filter == TEXFILTER_LINEAR)
            return D3D11_FILTER_MIN_POINT_MAG_MIP_LINEAR;
        else if (min_filter == TEXFILTER_LINEAR && mag_filter == TEXFILTER_NEAREST)
            return D3D11_FILTER_MIN_LINEAR_MAG_POINT_MIP_LINEAR;
        else if (min_filter == TEXFILTER_LINEAR && mag_filter == TEXFILTER_LINEAR)
            return D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    }
    break;

    default:
        DVASSERT_MSG(0, "Invalid TextureMipFilter provided");
    }

    return D3D11_FILTER_MIN_MAG_MIP_POINT;
}

D3D11_TEXTURE_ADDRESS_MODE DX11_TextureAddrMode(TextureAddrMode mode)
{
    switch (mode)
    {
    case TEXADDR_WRAP:
        return D3D11_TEXTURE_ADDRESS_WRAP;
    case TEXADDR_CLAMP:
        return D3D11_TEXTURE_ADDRESS_CLAMP;
    case TEXADDR_MIRROR:
        return D3D11_TEXTURE_ADDRESS_MIRROR;
    default:
        DVASSERT_MSG(0, "Invalid TextureAddrMode provided");
    }

    return D3D11_TEXTURE_ADDRESS_WRAP;
}

D3D11_BLEND DX11_BlendOp(BlendOp op)
{
    switch (op)
    {
    case BLENDOP_ZERO:
        return D3D11_BLEND_ZERO;
    case BLENDOP_ONE:
        return D3D11_BLEND_ONE;
    case BLENDOP_SRC_ALPHA:
        return D3D11_BLEND_SRC_ALPHA;
    case BLENDOP_INV_SRC_ALPHA:
        return D3D11_BLEND_INV_SRC_ALPHA;
    case BLENDOP_SRC_COLOR:
        return D3D11_BLEND_SRC_COLOR;
    case BLENDOP_DST_COLOR:
        return D3D11_BLEND_DEST_COLOR;
    default:
        DVASSERT_MSG(0, "Invalid BlendOp provided");
    }

    return D3D11_BLEND_ONE;
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
        DAVA::Logger::Error("D3D11Error at %s: %d\n%s\nCondition: %s", fileName, line, DX11_GetErrorText(hr), call);
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

} // namespace rhi
