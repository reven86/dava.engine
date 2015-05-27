#pragma once

    #include "../rhi_Type.h"

    #if !defined(WIN32_LEAN_AND_MEAN)
        #define WIN32_LEAN_AND_MEAN
    #endif    
    #include <windows.h>

    #pragma warning( disable: 7 9 193 271 304 791 )
    #include <d3d9.h>



const char* D3D9ErrorText( HRESULT hr );

namespace rhi
{

extern IDirect3DDevice9*    _D3D9_Device;
extern IDirect3DSurface9*   _D3D9_BackBuf;


D3DFORMAT          DX9_TextureFormat( TextureFormat format );

} // namespace rhi

