#include "common.slh"
#include "blending.slh"


////////////////////////////////////////////////////////////////////////////////
// fprog-input

fragment_in
{    
    #if MATERIAL_TEXTURE
        
        #if TEXTURE0_ANIMATION_SHIFT
        float2 varTexCoord0 : TEXCOORD0;
        #else
        float2 varTexCoord0 : TEXCOORD0;
        #endif

    #elif MATERIAL_SKYBOX
        
        float3 varTexCoord0 : TEXCOORD0;

    #endif

    #if MATERIAL_DECAL || ( MATERIAL_LIGHTMAP  && VIEW_DIFFUSE ) || FRAME_BLEND || ALPHA_MASK
        float2 varTexCoord1 : TEXCOORD1;
    #endif

    #if TILED_DECAL_MASK
        float2 varDecalTileTexCoord : TEXCOORD2;
    #endif
    #if MATERIAL_DETAIL
        half2 varDetailTexCoord : TEXCOORD2;
    #endif

    #if VERTEX_LIT

        [lowp] half varDiffuseColor : COLOR0;

        #if BLINN_PHONG
            [lowp] half varSpecularColor : TEXCOORD4;
        #elif NORMALIZED_BLINN_PHONG
            [lowp] half4 varSpecularColor : TEXCOORD4;
        #endif
    
    #elif PIXEL_LIT
        
        #if FAST_NORMALIZATION
        [lowp] half3 varHalfVec : COLOR0;
        #endif
        [lowp] half3 varToLightVec : COLOR1;
        float3 varToCameraVec : TEXCOORD7;
    #endif
    
    #if FLOWMAP
        float3 varFlowData : TEXCOORD4;
    #endif

    #if VERTEX_COLOR || SPHERICAL_LIT
        [lowp] half4 varVertexColor : COLOR1;
    #endif

    #if VERTEX_FOG
        [lowp] half4 varFog : TEXCOORD5;
    #endif           

    #if FRAME_BLEND
        [lowp] half varTime : TEXCOORD3;
    #endif

};



////////////////////////////////////////////////////////////////////////////////
// fprog-output

fragment_out
{
    float4 color : SV_TARGET0;
};





#if MATERIAL_TEXTURE
    uniform sampler2D albedo;
#elif MATERIAL_SKYBOX
    uniform samplerCUBE cubemap;
#endif

#if MATERIAL_DECAL
    uniform sampler2D decal;
#endif

#if ALPHA_MASK
    uniform sampler2D alphamask;
#endif

#if MATERIAL_DETAIL
    uniform sampler2D detail;
#endif

#if MATERIAL_LIGHTMAP  && VIEW_DIFFUSE
    uniform sampler2D lightmap;
#endif

#if FLOWMAP
    uniform sampler2D flowmap;
#endif

#if MATERIAL_TEXTURE && ALPHATEST && ALPHATESTVALUE
    [material][a] property float alphatestThreshold           = 0.0;
#endif

#if MATERIAL_TEXTURE && ALPHASTEPVALUE && ALPHABLEND
    [material][a] property float alphaStepValue               = 0.5;
#endif

#if PIXEL_LIT
    uniform sampler2D normalmap;
    [material][a] property float  inSpecularity               = 1.0;    
    [material][a] property float3 metalFresnelReflectance     = float3(0.5,0.5,0.5);
    [material][a] property float  normalScale                 = 1.0;
#endif


#if TILED_DECAL_MASK
    uniform sampler2D decalmask;
    uniform sampler2D decaltexture;
    [material][a] property float4 decalTileColor = float4(1.0,1.0,1.0,1.0) ;
#endif


#if (VERTEX_LIT || PIXEL_LIT ) && (!SKYOBJECT)
    [auto][a] property float3 lightAmbientColor0;
    [auto][a] property float3 lightColor0;
    #if NORMALIZED_BLINN_PHONG && VIEW_SPECULAR
        [material][a] property float inGlossiness = 0.5;
    #endif
#endif


#if PIXEL_LIT
    [auto][a] property float4 lightPosition0;
#endif


#if FLATCOLOR
    [material][a] property float4 flatColor = float4(0,0,0,0);
#endif

#if SETUP_LIGHTMAP && (MATERIAL_DECAL || MATERIAL_LIGHTMAP)
    [material][a] property float lightmapSize = 1.0;
#endif

#if PARTICLE_DEBUG_SHOW_ALPHA
    [material][a] property float particleAlphaThreshold = 0.2f;
    [material][a] property float4 particleDebugShowAlphaColor =  float4(0.0f, 0.0f, 1.0f, 0.4f);
#endif

inline float 
FresnelShlick( float NdotL, float Cspec )
{
    float expf = 5.0;
    return Cspec + (1.0 - Cspec) * pow(1.0 - NdotL, expf);
}

inline float3
FresnelShlickVec3( float NdotL, float3 Cspec )
{
    float expf = 5.0;
    return Cspec + (1.0 - Cspec) * (pow(1.0 - NdotL, expf));
}



////////////////////////////////////////////////////////////////////////////////
//

fragment_out fp_main( fragment_in input )
{
    fragment_out    output;

    #if VERTEX_FOG
        float   varFogAmoung = float(input.varFog.a);
        float3  varFogColor  = float3(input.varFog.rgb);
    #endif
    
    // FETCH PHASE
    #if MATERIAL_TEXTURE
    
        #if PIXEL_LIT || ALPHATEST || ALPHABLEND || VERTEX_LIT
            #if FLOWMAP
                float3 flowData = input.varFlowData;
                float2 flowDir = float2( tex2D( flowmap, input.varTexCoord0 ).xy) * 2.0 - 1.0;
                half4 flowSample1 = half4(tex2D( albedo, input.varTexCoord0 + flowDir*flowData.x));
                half4 flowSample2 = half4(tex2D( albedo, input.varTexCoord0 + flowDir*flowData.y));
                half4 textureColor0 = lerp(flowSample1, flowSample2, half(flowData.z) );
            #else
                half4 textureColor0 = half4(tex2D( albedo, input.varTexCoord0 ));
            #endif
            
            #if ALPHA_MASK 
                textureColor0.a *= FP_A8(tex2D( alphamask, input.varTexCoord1 ));
            #endif          
        #else
            #if FLOWMAP
                float3 flowData = input.varFlowData;
                float2 flowDir = float2(tex2D( flowmap, input.varTexCoord0 ).xy) * 2.0 - 1.0;
                half3 flowSample1 = half3( tex2D( albedo, input.varTexCoord0 + flowDir*flowData.x).rgb );
                half3 flowSample2 = half3( tex2D( albedo, input.varTexCoord0 + flowDir*flowData.y).rgb );
                half3 textureColor0 = lerp(flowSample1, flowSample2, half(flowData.z) );
            #else
                #if TEST_OCCLUSION
                    half4 preColor = half4(tex2D( albedo, input.varTexCoord0 ) );
                    half3 textureColor0 = half3(preColor.rgb*preColor.a);
                #else
                    half3 textureColor0 = half3(tex2D( albedo, input.varTexCoord0 ).rgb);
                #endif
                
            #endif
        #endif
        
        
        #if FRAME_BLEND
            half4 blendFrameColor = half4(tex2D( albedo, input.varTexCoord1 ));
            half varTime = input.varTime;
            textureColor0 = lerp( textureColor0, blendFrameColor, varTime );
        #endif
    
    #elif MATERIAL_SKYBOX
    
        half4 textureColor0 = half4(texCUBE( cubemap, input.varTexCoord0 ));
    
    #endif


    #if MATERIAL_TEXTURE
        #if ALPHATEST
            float alpha = textureColor0.a;
            #if VERTEX_COLOR
                alpha *= float(input.varVertexColor.a);
            #endif
            #if ALPHATESTVALUE
                if( alpha < alphatestThreshold ) discard;
            #else
                if( alpha < 0.5 ) discard;
            #endif
        #endif
        
        #if ALPHASTEPVALUE && ALPHABLEND
            textureColor0.a = half(step(alphaStepValue, float(textureColor0.a)));
        #endif
        #endif
        
    #endif

    
    #if MATERIAL_DECAL
        half3 textureColor1 = half3(tex2D( decal, input.varTexCoord1 ).rgb);
    #endif
    
    
    #if MATERIAL_LIGHTMAP  && VIEW_DIFFUSE
        half3 textureColor1 = half3(tex2D( lightmap, input.varTexCoord1 ).rgb);
    #endif
    
    
    #if MATERIAL_DETAIL
        half3 detailTextureColor = half3(tex2D( detail, input.varDetailTexCoord ).rgb);
    #endif


    #if MATERIAL_DECAL || MATERIAL_LIGHTMAP
        #if SETUP_LIGHTMAP
            half3 lightGray = float3(0.75,0.75,0.75);
            half3 darkGray = float3(0.25,0.25,0.25);
    
            bool isXodd;
            bool isYodd;            
            
            if(frac(floor(input.varTexCoord1.x*lightmapSize)/2.0) == 0.0)
            {
                isXodd = true;
            }
            else
            {
                isXodd = false;
            }
            
            if(frac(floor(input.varTexCoord1.y*lightmapSize)/2.0) == 0.0)
            {
                isYodd = true;
            }
            else
            {
                isYodd = false;
            }
    
            if((isXodd && isYodd) || (!isXodd && !isYodd))
            {
                textureColor1 = lightGray;
            }
            else
            {
                textureColor1 = darkGray;
            }
        #endif
    #endif


    // DRAW PHASE

    #if VERTEX_LIT
    
        #if BLINN_PHONG
            
            half3 color = half3(0.0,0.0,0.0);
            #if VIEW_AMBIENT
                color += half3(lightAmbientColor0);
            #endif

            #if VIEW_DIFFUSE
                color += half3(input.varDiffuseColor, input.varDiffuseColor, input.varDiffuseColor);
            #endif

            #if VIEW_ALBEDO
                #if TILED_DECAL_MASK
                    half maskSample = FP_A8(tex2D( decalmask, input.varTexCoord0 ));
                    half4 tileColor = half4(tex2D( decaltexture, input.varDecalTileTexCoord ).rgba * decalTileColor);
                    color *= textureColor0.rgb + (tileColor.rgb - textureColor0.rgb) * tileColor.a * maskSample;
                #else
                    color *= textureColor0.rgb;
                #endif
            #endif

            #if VIEW_SPECULAR
                color += half3((input.varSpecularColor * textureColor0.a) * lightColor0);
            #endif
    
        #elif NORMALIZED_BLINN_PHONG
   
            half3 color = half3(0.0,0.0,0.0);
            
            #if VIEW_AMBIENT && !MATERIAL_LIGHTMAP
                color += half3(lightAmbientColor0);
            #endif
        
            #if VIEW_DIFFUSE
                #if MATERIAL_LIGHTMAP
                    #if VIEW_ALBEDO
                        color = textureColor1.rgb * 2.0;
                    #else
                        //do not scale lightmap in view diffuse only case. artist request
                        color = textureColor1.rgb; 
                    #endif
                #else
                    color += half3(input.varDiffuseColor * lightColor0);
                #endif
            #endif
        
            #if VIEW_ALBEDO
                #if TILED_DECAL_MASK
                    half maskSample = FP_A8(tex2D( decalmask, input.varTexCoord0 ));
                    half4 tileColor = half4(tex2D( decaltexture, input.varDecalTileTexCoord ).rgba * decalTileColor);
                    color *= textureColor0.rgb + (tileColor.rgb - textureColor0.rgb) * tileColor.a * maskSample;
                #else
                    color *= textureColor0.rgb;
                #endif
            #endif
    
            #if VIEW_SPECULAR
                float glossiness = pow(5000.0, inGlossiness * textureColor0.a);
                float specularNorm = (glossiness + 2.0) / 8.0;
                float3 spec = float3(input.varSpecularColor.xyz * pow(float(input.varSpecularColor.w), glossiness) * specularNorm);
                                                     
                #if MATERIAL_LIGHTMAP
                    color += half3(spec * (textureColor1.rgb / 2.0) * lightColor0);
                #else
                    color += half3(spec * lightColor0);
                #endif
            #endif
    
        #endif


    #elif PIXEL_LIT
        
        // lookup normal from normal map, move from [0, 1] to  [-1, 1] range, normalize
        float3 normal = 2.0 * tex2D( normalmap, input.varTexCoord0 ).rgb - 1.0;
        normal.xy *= normalScale;
        normal = normalize (normal);        
    
        #if !FAST_NORMALIZATION
            float3 toLightNormalized = float3(normalize(input.varToLightVec.xyz));
            float3 toCameraNormalized = float3(normalize(input.varToCameraVec));
            float3 H = toCameraNormalized + toLightNormalized;
            H = normalize(H);

            // compute diffuse lighting
            float NdotL = max (dot (normal, toLightNormalized), 0.0);
            float NdotH = max (dot (normal, H), 0.0);
            float LdotH = max (dot (toLightNormalized, H), 0.0);
            float NdotV = max (dot (normal, toCameraNormalized), 0.0);
        #else
            // Kwasi normalization :-)
            // compute diffuse lighting
            float3 normalizedHalf = float3(normalize(input.varHalfVec.xyz));
            
            float NdotL = max (dot (normal, float3(input.varToLightVec.xyz)), 0.0);
            float NdotH = max (dot (normal, normalizedHalf), 0.0);
            float LdotH = max (dot (float3(input.varToLightVec.xyz), normalizedHalf), 0.0);
            float NdotV = max (dot (normal, input.varToCameraVec), 0.0);
        #endif
    
        #if NORMALIZED_BLINN_PHONG
    
            #if DIELECTRIC
                #define ColorType float
                float fresnelOut = FresnelShlick( NdotV, dielectricFresnelReflectance );
            #else
                #if FAST_METAL
                    #define ColorType float
                    float fresnelOut = FresnelShlick( NdotV, (metalFresnelReflectance.r + metalFresnelReflectance.g + metalFresnelReflectance.b) / 3.0 );
                #else
                    #define ColorType float3
                    float3 fresnelOut = FresnelShlickVec3( NdotV, metalFresnelReflectance );
                #endif
            #endif
            
            
            //#define GOTANDA                        
            #if GOTANDA
                float3 fresnelIn = FresnelShlickVec3(NdotL, metalFresnelReflectance);
                float3 diffuse = NdotL / _PI * (1.0 - fresnelIn * inSpecularity);
            #else
                float diffuse = NdotL / _PI;// * (1.0 - fresnelIn * inSpecularity);
            #endif
        
            #if VIEW_SPECULAR
                //float glossiness = inGlossiness * 0.999;
                //glossiness = 200.0 * glossiness / (1.0 - glossiness);                
                float glossiness = inGlossiness * textureColor0.a;
                float glossPower = pow(5000.0, glossiness); //textureColor0.a;
                       
                #if GOTANDA
                    float specCutoff = 1.0 - NdotL;
                    specCutoff *= specCutoff;
                    specCutoff *= specCutoff;
                    specCutoff *= specCutoff;
                    specCutoff = 1.0 - specCutoff;
                #else
                    float specCutoff = NdotL;
                #endif
        
                #if GOTANDA
                    float specularNorm = (glossPower + 2.0) * (glossPower + 4.0) / (8.0 * _PI * (pow(2.0, -glossPower / 2.0) + glossPower));
                #else
                    float specularNorm = (glossPower + 2.0) / 8.0;
                #endif

                float specularNormalized = specularNorm * pow(NdotH, glossPower) * specCutoff * inSpecularity;

                #if FAST_METAL
                    float geometricFactor = 1.0;
                #else
                    float geometricFactor = 1.0;// / LdotH * LdotH; //once upon a time it was written wrong, just keep it for history
                #endif

                ColorType specular = specularNormalized * geometricFactor * fresnelOut;
            #endif
        
        #endif
    
        half3 color = half3(0.0,0.0,0.0);
    
        #if VIEW_AMBIENT && !MATERIAL_LIGHTMAP
            color += half3(lightAmbientColor0);
        #endif
    
        #if VIEW_DIFFUSE
            #if MATERIAL_LIGHTMAP
                #if VIEW_ALBEDO
                    color = half3(textureColor1.rgb * 2.0);
                #else
                    //do not scale lightmap in view diffuse only case. artist request
                    color = half3(textureColor1.rgb); 
                #endif
            #else
                color += half3(diffuse * lightColor0);
            #endif
        #endif
    
        #if VIEW_ALBEDO
            #if TILED_DECAL_MASK
                half maskSample = FP_A8( tex2D( decalmask, input.varTexCoord0 ));
                half4 tileColor = half4(tex2D( decaltexture, input.varDecalTileTexCoord ).rgba * decalTileColor);
                color *= textureColor0.rgb + (tileColor.rgb - textureColor0.rgb) * tileColor.a * maskSample;
            #else
                color *= textureColor0.rgb;
            #endif
        #endif
    
        #if VIEW_SPECULAR
            #if MATERIAL_LIGHTMAP
                color += half3(specular * textureColor1.rgb * lightColor0);
            #else
                color += half3(specular * lightColor0);
            #endif
        #endif

    #else
        
        #if MATERIAL_DECAL || MATERIAL_LIGHTMAP
            
            half3 color = half3(0.0,0.0,0.0);

            #if VIEW_ALBEDO
                color = half3(textureColor0.rgb);
            #else
                color = half3(1.0,1.0,1.0);
            #endif

            #if VIEW_DIFFUSE
                #if VIEW_ALBEDO
                    color *= half3(textureColor1.rgb * 2.0);
                #else
                    //do not scale lightmap in view diffuse only case. artist request
                    color *= half3(textureColor1.rgb); 
                #endif              
            #endif

        #elif MATERIAL_TEXTURE

            half3 color = half3(textureColor0.rgb);
        
        #elif MATERIAL_SKYBOX
            
            float4 color = float4(textureColor0);
        
        #else
            
            half3 color = half3(1.0,1.0,1.0);
        
        #endif
        
        #if TILED_DECAL_MASK
            half maskSample = FP_A8(tex2D( decalmask, input.varTexCoord0 ));
            half4 tileColor = half4(tex2D( decaltexture, input.varDecalTileTexCoord ).rgba * decalTileColor);
            color.rgb += (tileColor.rgb - color.rgb) * tileColor.a * maskSample;
        #endif
        
    #endif


    #if MATERIAL_DETAIL
        color *= detailTextureColor.rgb * 2.0;
    #endif



    #if ALPHABLEND && MATERIAL_TEXTURE
        output.color = float4( float3(color.rgb), textureColor0.a );
    #elif MATERIAL_SKYBOX
        output.color = float4( color );
    #else
        output.color = float4( color.r, color.g, color.b, 1.0 );
    #endif

    
    #if VERTEX_COLOR || SPEED_TREE_OBJECT || SPHERICAL_LIT
        output.color *= float4(input.varVertexColor);
    #endif
        
    #if FLATCOLOR
        output.color *= flatColor;
    #endif




    
    
    #if VERTEX_FOG
        #if !FRAMEBUFFER_FETCH
            //VI: fog equation is inside of color equation for framebuffer fetch
            output.color.rgb = lerp( output.color.rgb, varFogColor, varFogAmoung );
        #endif
    #endif

    #if PARTICLE_DEBUG_SHOW_ALPHA
        if (output.color.a < particleAlphaThreshold)
            output.color = particleDebugShowAlphaColor;
        else
            output.color = 0.0;
    #endif
    #if PARTICLE_DEBUG_SHOW_OVERDRAW
        output.color = float4(0.01f, 0.0f, 0.0f, 1.0f);
    #endif

    return output;
}

