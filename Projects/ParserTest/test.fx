
struct
VP_Input
{
    float3  pos : POSITION;
};

struct
VP_Output
{
    float4  pos : SV_POSITION;
    float2  uv : TTT;
};


[global][dynamic] property float Bla;

uniform float4x4 World;
uniform float4x4 ViewProjection;

static float4x4 WorldAlias = World;

float3 
p_offset( float3 p )
{
    float3  p2 = float3(p*p,p*p,p*p);

    return p2 + float3(0.001,0.002,0.003);
}


VP_Output
vp_main( VP_Input vp_in )
{
    VP_Output   vp_out;

    float4  wpos = mul( World, float4(vp_in.pos.x,vp_in.pos.y,vp_in.pos.z,1.0) );
    
    vp_out.uv = wpos.zw;
    vp_out.pos = mul( wpos, ViewProjection );
    
//    {
        float3 pp = vp_out.pos.xyz + p_offset( wpos.xyz );
        
        vp_out.pos.xyz = pp;
//    }
    
    return vp_out;
}
