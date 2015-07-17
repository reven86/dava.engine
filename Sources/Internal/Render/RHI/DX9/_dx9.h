#pragma once

    #include "../rhi_Type.h"

    #if !defined(WIN32_LEAN_AND_MEAN)
        #define WIN32_LEAN_AND_MEAN
    #endif    
    #include <windows.h>

#if defined(__DAVAENGINE_WIN_UAP__) //RHI_COMPLETE
typedef struct _POINTFLOAT {
	FLOAT   x;
	FLOAT   y;
} POINTFLOAT, *PPOINTFLOAT;

typedef struct _GLYPHMETRICSFLOAT {
    FLOAT       gmfBlackBoxX;
    FLOAT       gmfBlackBoxY;
    POINTFLOAT  gmfptGlyphOrigin;
    FLOAT       gmfCellIncX;
    FLOAT       gmfCellIncY;
} GLYPHMETRICSFLOAT, *PGLYPHMETRICSFLOAT, FAR *LPGLYPHMETRICSFLOAT;
#endif
	
    #pragma warning( disable: 7 9 193 271 304 791 )
    #include <d3d9.h>



const char* D3D9ErrorText( HRESULT hr );

namespace rhi
{
struct InitParam;


D3DFORMAT           DX9_TextureFormat( TextureFormat format );

void                InitializeRenderThreadDX9();
void                UninitializeRenderThreadDX9();


void                AcquireDevice();
void                ReleaseDevice();


extern IDirect3D9*          _D3D9;
extern IDirect3DDevice9*    _D3D9_Device;
extern unsigned             _D3D9_Adapter;
extern IDirect3DSurface9*   _D3D9_BackBuf;

extern InitParam            _DX9_InitParam;

} // namespace rhi

