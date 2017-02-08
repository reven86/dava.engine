#include "blending.slh"

fragment_in
{
    float2 texcoord0 : TEXCOORD0;
};

fragment_out
{
    float4  color : SV_TARGET0;
};

uniform sampler2D particlesRT;
uniform sampler2D heatMap;

fragment_out fp_main( fragment_in input )
{
    fragment_out output;
    float4 color = tex2D(particlesRT, input.texcoord0);

    output.color = tex2D(heatMap, float2(color.r, 0));
    // output.color = float4(0.5);
    return output;
}
