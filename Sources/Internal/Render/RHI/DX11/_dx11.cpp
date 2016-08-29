#include <dxgiformat.h>
    #include "_dx11.h"

    #include <stdio.h>

    #include "../rhi_Public.h"

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
}

//==============================================================================
//
//  publics:

const char*
D3D11ErrorText(HRESULT hr)
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
        return "DXGI_ERROR_DEVICE_REMOVED: The video card has been physically removed from the system, or a driver upgrade for the video card has occurred. The application should destroy and recreate the device. For help debugging the problem, call ID3D10Device::GetDeviceRemovedReason.";

    case DXGI_ERROR_DEVICE_RESET:
        return "DXGI_ERROR_DEVICE_RESET: The device failed due to a badly formed command. This is a run-time issue; The application should destroy and recreate the device.";

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

    static char text[1024];

    _snprintf(text, sizeof(text), "unknown D3D9 error (%08X)\n", (unsigned)hr);
    return text;
}

namespace rhi
{
//------------------------------------------------------------------------------

DXGI_FORMAT
DX11_TextureFormat(TextureFormat format)
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
    }

    return DXGI_FORMAT_UNKNOWN;
}

uint32 DX11_CheckMultisampleSupport(ID3D11Device* device)
{
    DXGI_FORMAT depthFormat = (_D3D11_FeatureLevel == D3D_FEATURE_LEVEL_11_0) ? DXGI_FORMAT_D32_FLOAT : DXGI_FORMAT_D24_UNORM_S8_UINT;
    const DXGI_FORMAT formatsToCheck[] = { DXGI_FORMAT_B8G8R8A8_UNORM, depthFormat };

    uint32 samples = 2;

    for (uint32 s = 0; (samples <= 8); ++s, samples *= 2)
    {
        UINT numQualityLevels = 0;
        for (uint32 f = 0; f < countof(formatsToCheck); ++f)
        {
            UINT formatSupport = 0;
            HRESULT hr = device->CheckFormatSupport(formatsToCheck[f], &formatSupport);
            if (formatSupport & D3D11_FORMAT_SUPPORT_MULTISAMPLE_RENDERTARGET)
            {
                hr = device->CheckMultisampleQualityLevels(formatsToCheck[f], samples, &numQualityLevels);
                if (FAILED(hr) || (numQualityLevels == 0))
                {
                    break;
                }
            }
        }
        if (numQualityLevels == 0)
        {
            DAVA::Logger::Info("DX11 max multisample samples: %u", samples);
            break;
        }
    }

    return samples / 2;
}

} // namespace rhi
