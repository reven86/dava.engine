#pragma once

#include "Base/Platform.h"
#include "Base/BaseObject.h"
#include "../Common/rhi_RingBuffer.h"
#include "../Common/rhi_Pool.h"
#include "../Common/rhi_Private.h"
#include "../Common/rhi_BackendImpl.h"
#include "../rhi_Type.h"
#include "../rhi_Public.h"

#if defined(__DAVAENGINE_WIN_UAP__)
    #include <DXGI1_3.h>
#else
    #define _WIN32_WINNT 0x0601
#endif

#include <d3d11_1.h>
#include <dxgi.h>

namespace rhi
{
struct DX11Command
{
    enum Func : uint32_t
    {
        NOP,

        MAP,
        UNMAP,
        UPDATE_SUBRESOURCE,
        COPY_RESOURCE,
        SYNC_CPU_GPU,

        /*
        * Device commands (invokes _D3D11_Device method)
        */
        QUERY_INTERFACE = 0x1000,
        CREATE_DEFERRED_CONTEXT,

        CREATE_BLEND_STATE,
        CREATE_SAMPLER_STATE,
        CREATE_RASTERIZER_STATE,
        CREATE_DEPTH_STENCIL_STATE,

        CREATE_VERTEX_SHADER,
        CREATE_PIXEL_SHADER,
        CREATE_INPUT_LAYOUT,

        CREATE_QUERY,
        CREATE_BUFFER,

        CREATE_TEXTURE_2D,
        CREATE_RENDER_TARGET_VIEW,
        CREATE_DEPTH_STENCIL_VIEW,
        CREATE_SHADER_RESOURCE_VIEW,

        // service values for range checking
        DEVICE_LAST_COMMAND,
        DEVICE_FIRST_COMMAND = QUERY_INTERFACE
    };

    struct Arguments
    {
        DAVA::uint64 arg[12];
    };

    Func func = Func::NOP;
    HRESULT retval = S_OK;
    Arguments arguments;

    template <class... args>
    DX11Command(Func f, args&&... a)
        : func(f)
        , arguments({ DAVA::uint64(a)... })
    {
    }
};

const char* DX11_GetErrorText(HRESULT hr);
DXGI_FORMAT DX11_TextureFormat(TextureFormat format);
D3D11_COMPARISON_FUNC DX11_CmpFunc(CmpFunc func);
D3D11_STENCIL_OP DX11_StencilOp(StencilOperation op);
D3D11_TEXTURE_ADDRESS_MODE DX11_TextureAddrMode(TextureAddrMode mode);
D3D11_FILTER DX11_TextureFilter(TextureFilter min_filter, TextureFilter mag_filter, TextureMipFilter mip_filter, DAVA::uint32 anisotropy);
D3D11_BLEND DX11_BlendOp(BlendOp op);
}
