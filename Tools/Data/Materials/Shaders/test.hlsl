#include "dx9-def.h"
//#include "gl-def.h"

VPROG_IN_BEGIN
    VPROG_IN_POSITION
    VPROG_IN_NORMAL
    VPROG_IN_TEXCOORD
VPROG_IN_END

VPROG_OUT_BEGIN
    VPROG_OUT_POSITION
    VPROG_OUT_TEXCOORD0(uv,2)
    VPROG_OUT_TEXCOORD1(color,4)
VPROG_OUT_END

DECL_VPROG_BUFFER(0,16)
DECL_VPROG_BUFFER(1,16)

VPROG_BEGIN

    float3 in_pos      = VP_IN_POSITION;
    float3 in_normal   = VP_IN_NORMAL;
    float2 in_texcoord = VP_IN_TEXCOORD;
    float4x4 ViewProjection = float4x4( VP_Buffer0[0], VP_Buffer0[1], VP_Buffer0[2], VP_Buffer0[3] );
    float4x4 World = float4x4( VP_Buffer1[0], VP_Buffer1[1], VP_Buffer1[2], VP_Buffer1[3] );
//        "    float3x3 World3 = float3x3( (float3)(float4(VP_Buffer1[0])), (float3)(float4(VP_Buffer1[1])), (float3)(float4(VP_Buffer1[2])) );
//        "    float3x3 World3 = float3x3( float3(VP_Buffer1[0]), float3(VP_Buffer1[1]), float3(VP_Buffer1[2]) );
    float3x3 World3 = VP_BUF_FLOAT3X3(1,0);
    float4 wpos = mul( float4(in_pos.x,in_pos.y,in_pos.z,1.0), World );
    float i   = dot( float3(0,0,-1), normalize(mul(float3(in_normal),World3)) );
    VP_OUT_POSITION   = mul( wpos, ViewProjection );
    VP_OUT(uv)        = in_texcoord;
    VP_OUT(color)     = float4(i,i,i,1.0);

VPROG_END
