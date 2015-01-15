//==============================================================================
//
//  DX9 related stuff
//
//==============================================================================
//
//  externals:

    #include "_dx9.h"

    #pragma warning( disable: 7 193 271 304 791 )
    #include <d3d9.h>
    #pragma warning( default: 7 193 271 304 791 )
    #include <stdio.h>



//==============================================================================

namespace rhi
{
IDirect3D9*         _D3D9           = 0;
IDirect3DDevice9*   _D3D9_Device    = 0;
unsigned            _D3D9_Adapter   = 0;
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

/*
//------------------------------------------------------------------------------

void 
SetRenderMarker( const char* format, ... )
{
    va_list     arglist;
    char        buf[512];
    wchar_t     text[countof(buf)];

    va_start( arglist, format );
    _vsnprintf( buf, countof(buf), format, arglist );
    va_end( arglist );

    ::MultiByteToWideChar( CP_ACP, 0, buf, -1, text, countof(text) );
    ::D3DPERF_SetMarker( D3DCOLOR_ARGB(0xFF,0x40,0x40,0x80), text );
}


//------------------------------------------------------------------------------

void 
BeginRenderEvent( const char* format, ... )
{
    va_list     arglist;
    char        buf[512];
    wchar_t     text[countof(buf)];

    va_start( arglist, format );
    _vsnprintf( buf, countof(buf), format, arglist );
    va_end( arglist );

    ::MultiByteToWideChar( CP_ACP, 0, buf, -1, text, countof(text) );
    ::D3DPERF_BeginEvent( D3DCOLOR_ARGB(0xFF,0x40,0x80,0x40), text );
}


//------------------------------------------------------------------------------

void 
EndRenderEvent()
{
    ::D3DPERF_EndEvent();
}
*/

