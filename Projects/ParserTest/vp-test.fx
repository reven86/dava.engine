
struct
VP_Input
{
    [vertex]   float3  pos     : SV_POSITION;
    [vertex]   float3  normal  : SV_NORMAL;
    [vertex]   float2  uv      : SV_TEXCOORD0;
    [instance] float2  uv2     : SV_TEXCOORD1;
};

struct
VP_Output
{
    float4  pos : SV_POSITION;
    float2  uv  : SV_TEXCOORD0;
};

[global] property float4x4  World;
[global] property float4x4  ViewProjection;
[unique] property float4    Prop_F4;
[unique] property float3    Prop_F3;
[unique] property float2    Prop_F2;
[unique] property float     Prop_F1;

/*
cbuffer test_t : register(b3) { float4 test[4]; }; 

static float4   _f4 = test[0];
static float3   _f3 = test[1].xyz;
static float2   _f2 = test[1].xy;
static float    _f1 = test[1].w;
static float    _mm = float4x4( test[0], test[1], test[2], test[3] );
uniform float4x4   _World;
*/

VP_Output
main( VP_Input input )
{
    VP_Output   output;
    float4      pos = mul( float4(input.pos,1.0), World );

    output.pos = mul( pos, ViewProjection );
    output.uv  = input.uv;



    return output;
}
