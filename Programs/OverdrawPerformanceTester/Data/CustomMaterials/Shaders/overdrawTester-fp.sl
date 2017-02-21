#include "blending.slh"

fragment_in
{
    float2 texcoord0 : TEXCOORD0;
};

fragment_out
{
    float4  color : SV_TARGET0;
};

#if ONE_TEX || DEPENDENT_READ_TEST
    uniform sampler2D t1;
    #if TWO_TEX || DEPENDENT_READ_TEST
        uniform sampler2D t2;
        #if THREE_TEX || DEPENDENT_READ_TEST
            uniform sampler2D t3;
            #if FOUR_TEX && !DEPENDENT_READ_TEST
                uniform sampler2D t4;
            #endif
        #endif
    #endif
#endif

fragment_out fp_main( fragment_in input )
{
    fragment_out output;
    output.color = float4(input.texcoord0, 0.0f, 0.05f);

    #if DEPENDENT_READ_TEST
        float4 uv = tex2D(t1, input.texcoord0);
        float4 c1 = tex2D(t2, uv.xy);
        float4 c2 = tex2D(t3, uv.zw);
        output.color = lerp(c1, c2, 0.5f);
    #else
        #if ONE_TEX
            output.color += tex2D(t1, input.texcoord0);
            #if TWO_TEX
                output.color += tex2D(t2, input.texcoord0);
                #if THREE_TEX
                    output.color += tex2D(t3, input.texcoord0);
                    #if FOUR_TEX
                        output.color += tex2D(t4, input.texcoord0);
                    #endif
                #endif
            #endif
        #endif
    #endif
    
    return output;
}