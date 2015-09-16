/*==================================================================================
    Copyright (c) 2008, binaryzebra
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    * Neither the name of the binaryzebra nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE binaryzebra AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL binaryzebra BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/

    #include <dxgiformat.h>
    #include "_dx11.h"

    #include <stdio.h>

    #include "../rhi_Public.h"
    

//==============================================================================

namespace rhi
{
// this hack need removed, when rhi_dx thread will synchronized with rander::reset
__DAVAENGINE_WIN_UAP_INCOMPLETE_IMPLEMENTATION__MARKER__
DAVA::Mutex need_synchronized;

ID3D11Device*               _D3D11_Device               = nullptr;
IDXGISwapChain*             _D3D11_SwapChain            = nullptr;
ID3D11Texture2D*            _D3D11_SwapChainBuffer      = nullptr;
ID3D11RenderTargetView*     _D3D11_RenderTargetView     = nullptr;
ID3D11Texture2D*            _D3D11_DepthStencilBuffer   = nullptr;
ID3D11DepthStencilView*     _D3D11_DepthStencilView     = nullptr;
D3D_FEATURE_LEVEL           _D3D11_FeatureLevel         = D3D_FEATURE_LEVEL_9_1;
ID3D11DeviceContext*        _D3D11_ImmediateContext     = nullptr;
ID3D11Debug*                _D3D11_Debug                = nullptr;
ID3DUserDefinedAnnotation*  _D3D11_UserAnnotation       = nullptr;

InitParam                   _DX11_InitParam;

}


//==============================================================================
//
//  publics:

const char*
D3D11ErrorText( HRESULT hr )
{
    switch( hr )
    {
        case S_OK :
            return "No error occurred";
        
        case D3D11_ERROR_FILE_NOT_FOUND :
            return "The file was not found";

        case D3D11_ERROR_TOO_MANY_UNIQUE_STATE_OBJECTS :
            return "There are too many unique instances of a particular type of state object";

        case D3D11_ERROR_TOO_MANY_UNIQUE_VIEW_OBJECTS :
            return "There are too many unique instance of a particular type of view object";

        case D3D11_ERROR_DEFERRED_CONTEXT_MAP_WITHOUT_INITIAL_DISCARD :
            return "The first call to ID3D11DeviceContext::Map after either ID3D11Device::CreateDeferredContext or ID3D11DeviceContext::FinishCommandList per Resource was not D3D11_MAP_WRITE_DISCARD";
/*        
        case D3DERR_INVALIDCALL :
            return "The method call is invalid. For example, a method's parameter may not be a valid pointer.";
        
        case D3DERR_WASSTILLDRAWING :
            return "The previous blit operation that is transferring information to or from this surface is incomplete.";
*/        
        case E_FAIL :
            return "Attempted to create a device with the debug layer enabled and the layer is not installed.";

        case E_INVALIDARG :
            return "An invalid parameter was passed to the returning function.";

        case E_OUTOFMEMORY :
            return "Direct3D could not allocate sufficient memory to complete the call.";
        
        case S_FALSE :
            return "Alternate success value, indicating a successful but nonstandard completion (the precise meaning depends on context).";
    }

    static char text[1024];

    _snprintf( text, sizeof(text),"unknown D3D9 error (%08X)\n", (unsigned)hr );
    return text;
}

namespace rhi
{

//------------------------------------------------------------------------------

DXGI_FORMAT
DX11_TextureFormat( TextureFormat format )
{
    switch( format )
    {
        case TEXTURE_FORMAT_R8G8B8A8        : return DXGI_FORMAT_B8G8R8A8_UNORM;
        case TEXTURE_FORMAT_R8G8B8X8        : return DXGI_FORMAT_B8G8R8X8_UNORM;
//        case TEXTURE_FORMAT_R8G8B8          : return DXGI_FORMAT_R8G8B8_UNORM;

        case TEXTURE_FORMAT_R5G5B5A1        : return DXGI_FORMAT_B5G5R5A1_UNORM;
        case TEXTURE_FORMAT_R5G6B5          : return DXGI_FORMAT_B5G6R5_UNORM;

        case TEXTURE_FORMAT_R4G4B4A4        : return DXGI_FORMAT_B4G4R4A4_UNORM;

        case TEXTURE_FORMAT_A16R16G16B16    : return DXGI_FORMAT_R16G16B16A16_FLOAT;
        case TEXTURE_FORMAT_A32R32G32B32    : return DXGI_FORMAT_R32G32B32A32_FLOAT;

        case TEXTURE_FORMAT_R8              : return DXGI_FORMAT_R8_UNORM;
        case TEXTURE_FORMAT_R16             : return DXGI_FORMAT_R16_FLOAT;

        case TEXTURE_FORMAT_DXT1            : return DXGI_FORMAT_BC1_UNORM;
        case TEXTURE_FORMAT_DXT3            : return DXGI_FORMAT_BC4_UNORM;
        case TEXTURE_FORMAT_DXT5            : return DXGI_FORMAT_BC2_UNORM;

        case TEXTURE_FORMAT_PVRTC2_4BPP_RGB :
        case TEXTURE_FORMAT_PVRTC2_4BPP_RGBA :
        case TEXTURE_FORMAT_PVRTC2_2BPP_RGB :
        case TEXTURE_FORMAT_PVRTC2_2BPP_RGBA :
            return DXGI_FORMAT_UNKNOWN;

        case TEXTURE_FORMAT_ATC_RGB :
        case TEXTURE_FORMAT_ATC_RGBA_EXPLICIT :
        case TEXTURE_FORMAT_ATC_RGBA_INTERPOLATED :
            return DXGI_FORMAT_UNKNOWN;

        case TEXTURE_FORMAT_ETC1 :
        case TEXTURE_FORMAT_ETC2_R8G8B8 :
        case TEXTURE_FORMAT_ETC2_R8G8B8A8 :
        case TEXTURE_FORMAT_ETC2_R8G8B8A1 :
            return DXGI_FORMAT_UNKNOWN;

        case TEXTURE_FORMAT_EAC_R11_UNSIGNED :
        case TEXTURE_FORMAT_EAC_R11_SIGNED :
        case TEXTURE_FORMAT_EAC_R11G11_UNSIGNED :
        case TEXTURE_FORMAT_EAC_R11G11_SIGNED :
            return DXGI_FORMAT_UNKNOWN;

        case TEXTURE_FORMAT_D16             : return DXGI_FORMAT_D16_UNORM;
        case TEXTURE_FORMAT_D24S8           : return DXGI_FORMAT_D24_UNORM_S8_UINT;
    }

    return DXGI_FORMAT_UNKNOWN;
}



} // namespace rhi
