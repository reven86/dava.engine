
fragment_in
FP_Input
{
    float2  uv : SV_TEXCOORD0;
};

fragment_out
FP_Output
{
    float4  color : SV_COLOR;
};

[unique] property float4    Tint;

//Texture2D    Albedo : register(t0);
//SamplerState Albedo_Sampler : register(s0);
uniform sampler2D Albedo;
uniform sampler2D Detail;


FP_Output
main( FP_Input input )
{
    FP_Output   output;

//    output.color  = Albedo.Sample( Albedo_Sampler, input.uv );
    output.color  = tex2D( Albedo, input.uv );
    output.color += tex2D( Detail, input.uv ) * Tint;



    return output;
}