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

    #include "_dx9.h"

    #pragma warning( disable: 7 193 271 304 791 )
    #include <d3d9.h>
    #pragma warning( default: 7 193 271 304 791 )
    #include <stdio.h>

    #include "../rhi_Public.h"


//==============================================================================

namespace rhi
{

IDirect3D9*             _D3D9           = nullptr;
IDirect3DDevice9*       _D3D9_Device    = nullptr;
unsigned                _D3D9_Adapter   = 0;
IDirect3DSurface9*      _D3D9_BackBuf   = nullptr;
IDirect3DSurface9*      _D3D9_DepthBuf  = nullptr;

InitParam               _DX9_InitParam;
D3DPRESENT_PARAMETERS   _DX9_PresentParam;

void(*_End_Frame)() = nullptr;

}


//==============================================================================
//
//  publics:

const char*
D3D9ErrorText( HRESULT hr )
{
    switch( hr )
    {
        case D3D_OK :
            return "No error occurred";

        case D3DOK_NOAUTOGEN :
            return "This is a success code. However, the autogeneration of mipmaps is not supported for this format. This means that resource creation will succeed but the mipmap levels will not be automatically generated";

        case D3DERR_CONFLICTINGRENDERSTATE :
            return "The currently set render states cannot be used together";

        case D3DERR_CONFLICTINGTEXTUREFILTER :
            return "The current texture filters cannot be used together";

        case D3DERR_CONFLICTINGTEXTUREPALETTE :
            return "The current textures cannot be used simultaneously";

        case D3DERR_DEVICELOST :
            return "The device has been lost but cannot be reset at this time. Therefore, rendering is not possible";

        case D3DERR_DEVICENOTRESET :
            return "The device has been lost but can be reset at this time";

        case D3DERR_DRIVERINTERNALERROR :
            return "Internal driver error. Applications should destroy and recreate the device when receiving this error. For hints on debugging this error, see Driver Internal Errors";

        case D3DERR_DRIVERINVALIDCALL :
            return "Not used";

        case D3DERR_INVALIDCALL :
            return "The method call is invalid. For example, a method's parameter may not be a valid pointer";

        case D3DERR_INVALIDDEVICE :
            return "The requested device type is not valid";

        case D3DERR_MOREDATA :
            return "There is more data available than the specified buffer size can hold";

        case D3DERR_NOTAVAILABLE :
            return "This device does not support the queried technique";

        case D3DERR_NOTFOUND :
            return "The requested item was not found";

        case D3DERR_OUTOFVIDEOMEMORY :
            return "Direct3D does not have enough display memory to perform the operation";

        case D3DERR_TOOMANYOPERATIONS :
            return "The application is requesting more texture-filtering operations than the device supports";

        case D3DERR_UNSUPPORTEDALPHAARG :
            return "The device does not support a specified texture-blending argument for the alpha channel";

        case D3DERR_UNSUPPORTEDALPHAOPERATION :
            return "The device does not support a specified texture-blending operation for the alpha channel";

        case D3DERR_UNSUPPORTEDCOLORARG :
            return "The device does not support a specified texture-blending argument for color values";

        case D3DERR_UNSUPPORTEDCOLOROPERATION :
            return "The device does not support a specified texture-blending operation for color values";

        case D3DERR_UNSUPPORTEDFACTORVALUE :
            return "The device does not support the specified texture factor value. Not used; provided only to support older drivers";

        case D3DERR_UNSUPPORTEDTEXTUREFILTER :
            return "The device does not support the specified texture filter";

        case D3DERR_WASSTILLDRAWING :
            return "The previous blit operation that is transferring information to or from this surface is incomplete";

        case D3DERR_WRONGTEXTUREFORMAT :
            return "The pixel format of the texture surface is not valid";

        case E_FAIL :
            return "An undetermined error occurred inside the Direct3D subsystem";

        case E_INVALIDARG :
            return "An invalid parameter was passed to the returning function";

//        case E_INVALIDCALL :
//            return "The method call is invalid. For example, a method's parameter may have an invalid value";

        case E_NOINTERFACE :
            return "No object interface is available";

        case E_NOTIMPL :
            return "Not implemented";

        case E_OUTOFMEMORY :
            return "Direct3D could not allocate sufficient memory to complete the call";

    }

    static char text[1024];

    _snprintf( text, sizeof(text),"unknown D3D9 error (%08X)\n", (unsigned)hr );
    return text;
}

namespace rhi
{

//------------------------------------------------------------------------------

D3DFORMAT
DX9_TextureFormat( TextureFormat format )
{
    switch( format )
    {
        case TEXTURE_FORMAT_R8G8B8A8        : return D3DFMT_A8R8G8B8;
        case TEXTURE_FORMAT_R8G8B8X8        : return D3DFMT_X8R8G8B8;

        case TEXTURE_FORMAT_R5G5B5A1        : return D3DFMT_A1R5G5B5;
        case TEXTURE_FORMAT_R5G6B5          : return D3DFMT_R5G6B5;

        case TEXTURE_FORMAT_R4G4B4A4        : return D3DFMT_A4R4G4B4;

        case TEXTURE_FORMAT_A16R16G16B16    : return D3DFMT_A16B16G16R16F;
        case TEXTURE_FORMAT_A32R32G32B32    : return D3DFMT_A32B32G32R32F;

        case TEXTURE_FORMAT_R8              : return D3DFMT_A8;
        case TEXTURE_FORMAT_R16             : return D3DFMT_R16F;

        case TEXTURE_FORMAT_DXT1            : return D3DFMT_DXT1;
        case TEXTURE_FORMAT_DXT3            : return D3DFMT_DXT3;
        case TEXTURE_FORMAT_DXT5            : return D3DFMT_DXT5;

        case TEXTURE_FORMAT_PVRTC2_4BPP_RGB :
        case TEXTURE_FORMAT_PVRTC2_4BPP_RGBA :
        case TEXTURE_FORMAT_PVRTC2_2BPP_RGB :
        case TEXTURE_FORMAT_PVRTC2_2BPP_RGBA :
            return D3DFMT_UNKNOWN;

        case TEXTURE_FORMAT_ATC_RGB :
        case TEXTURE_FORMAT_ATC_RGBA_EXPLICIT :
        case TEXTURE_FORMAT_ATC_RGBA_INTERPOLATED :
            return D3DFMT_UNKNOWN;

        case TEXTURE_FORMAT_ETC1 :
        case TEXTURE_FORMAT_ETC2_R8G8B8 :
        case TEXTURE_FORMAT_ETC2_R8G8B8A8 :
        case TEXTURE_FORMAT_ETC2_R8G8B8A1 :
            return D3DFMT_UNKNOWN;

        case TEXTURE_FORMAT_EAC_R11_UNSIGNED :
        case TEXTURE_FORMAT_EAC_R11_SIGNED :
        case TEXTURE_FORMAT_EAC_R11G11_UNSIGNED :
        case TEXTURE_FORMAT_EAC_R11G11_SIGNED :
            return D3DFMT_UNKNOWN;

        case TEXTURE_FORMAT_D16             : return D3DFMT_D16;
        case TEXTURE_FORMAT_D24S8           : return D3DFMT_D24S8;
    }

    return D3DFMT_UNKNOWN;
}



} // namespace rhi
