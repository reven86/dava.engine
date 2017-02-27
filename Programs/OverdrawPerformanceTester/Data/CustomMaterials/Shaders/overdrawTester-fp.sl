#include "blending.slh"

fragment_in
{
    float2 texcoord0 : TEXCOORD0;
};

fragment_out
{
    float4  color : SV_TARGET0;
};

#if DEPENDENT_READ_TEST
    uniform sampler2D t1;
    uniform sampler2D t2;
    uniform sampler2D t3;
#else
    #if SAMPLE_COUNT == 1
        uniform sampler2D t1;
    #elif SAMPLE_COUNT == 2
        uniform sampler2D t1;
        uniform sampler2D t2;
    #elif SAMPLE_COUNT == 3
        uniform sampler2D t1;
        uniform sampler2D t2;
        uniform sampler2D t3;
    #elif SAMPLE_COUNT == 4
        uniform sampler2D t1;
        uniform sampler2D t2;
        uniform sampler2D t3;
        uniform sampler2D t4;
    #endif
#endif

fragment_out fp_main( fragment_in input )
{
    fragment_out output;
    output.color = float4(input.texcoord0 * 0.5f, 0.0f, 0.3f);
#if ALPHA8

    #if DEPENDENT_READ_TEST
        float4 uv = FP_A8(tex2D(t1, input.texcoord0)).xxxx * 0.05f;
        float4 c1 = FP_A8(tex2D(t2, uv.xy)).xxxx;
        float4 c2 = FP_A8(tex2D(t3, uv.zw)).xxxx;
        output.color = lerp(c1, c2, 0.5f);
    #else
        #if SAMPLE_COUNT == 1
            output.color += FP_A8(tex2D(t1, input.texcoord0)).xxxx;
        #elif SAMPLE_COUNT == 2
            output.color += FP_A8(tex2D(t1, input.texcoord0)).xxxx;
            output.color += FP_A8(tex2D(t2, input.texcoord0)).xxxx;
        #elif SAMPLE_COUNT == 3
            output.color += FP_A8(tex2D(t1, input.texcoord0)).xxxx;
            output.color += FP_A8(tex2D(t2, input.texcoord0)).xxxx;
            output.color += FP_A8(tex2D(t3, input.texcoord0)).xxxx;
        #elif SAMPLE_COUNT == 4
            output.color += FP_A8(tex2D(t1, input.texcoord0)).xxxx;
            output.color += FP_A8(tex2D(t2, input.texcoord0)).xxxx;
            output.color += FP_A8(tex2D(t3, input.texcoord0)).xxxx;
            output.color += FP_A8(tex2D(t4, input.texcoord0)).xxxx;
        #endif
    #endif

#else // ALPHA8

    #if DEPENDENT_READ_TEST
        float4 uv = tex2D(t1, input.texcoord0) * 0.05f;
        float4 c1 = tex2D(t2, uv.xy);
        float4 c2 = tex2D(t3, uv.zw);
        output.color = lerp(c1, c2, 0.5f);
    #else
        #if SAMPLE_COUNT == 1
            output.color += tex2D(t1, input.texcoord0);
        #elif SAMPLE_COUNT == 2
            output.color += tex2D(t1, input.texcoord0);
            output.color += tex2D(t2, input.texcoord0);
        #elif SAMPLE_COUNT == 3
            output.color += tex2D(t1, input.texcoord0);
            output.color += tex2D(t2, input.texcoord0);
            output.color += tex2D(t3, input.texcoord0);
        #elif SAMPLE_COUNT == 4
            output.color += tex2D(t1, input.texcoord0);
            output.color += tex2D(t2, input.texcoord0);
            output.color += tex2D(t3, input.texcoord0);
            output.color += tex2D(t4, input.texcoord0);
        #endif
    #endif

#endif //ALPHA8 else
    return output;
}