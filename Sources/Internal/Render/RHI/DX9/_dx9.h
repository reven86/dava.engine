#pragma once

#include "../rhi_Type.h"

#if !defined(WIN32_LEAN_AND_MEAN)
    #define WIN32_LEAN_AND_MEAN
#endif    
#include <windows.h>

#pragma warning(disable : 7 9 193 271 304 791)
#include <d3d9.h>

namespace rhi
{
struct InitParam;

D3DFORMAT DX9_TextureFormat(TextureFormat format);

const char* D3D9ErrorText(HRESULT hr);
void ScheduleDeviceReset();

extern IDirect3D9* _D3D9;
extern IDirect3DDevice9* _D3D9_Device;
extern IDirect3DSurface9* _D3D9_BackBuf;
extern IDirect3DSurface9* _D3D9_DepthBuf;
extern UINT _D3D9_Adapter;

extern InitParam _DX9_InitParam;
extern D3DPRESENT_PARAMETERS _DX9_PresentParam;
extern DAVA::Mutex _DX9_ResetParamsMutex;

} // namespace rhi
