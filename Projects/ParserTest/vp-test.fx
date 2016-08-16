
vertex_in
VP_Input
{
    [vertex]   float3  pos     : SV_POSITION;
    [vertex]   float3  normal  : SV_NORMAL;
    [vertex]   float2  uv      : SV_TEXCOORD0;
    [instance] float2  uv2     : SV_TEXCOORD1;
};

vertex_out
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


VP_Output
main( VP_Input input )
{
    VP_Output   output;
    float4      pos = mul( float4(input.pos,1.0), World );

    output.pos = mul( pos, ViewProjection );
    output.uv  = input.uv;



    return output;
}
