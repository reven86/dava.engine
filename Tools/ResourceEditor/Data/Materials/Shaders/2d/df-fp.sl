#include "blending.slh"

fragment_in
{
    float2  uv      : TEXCOORD0;
    half4   color   : COLOR0;
};

fragment_out
{
    float4  color   : SV_TARGET0;
};

sampler2D tex;

[statik][instance] property float smoothing;


fragment_out
fp_main( fragment_in input )
{
    fragment_out    output;

    min10float4 in_color = input.color;
    float2      in_uv    = input.uv;

    min10float distance = FP_A8(tex2D( tex, in_uv ));
    min10float alpha = smoothstep(0.5 - smoothing, 0.5 + smoothing, distance);
    alpha = min(alpha, in_color.a);

    output.color = float4(in_color.rgb, alpha);

    return output;
}
