<CONFIG>
uniform sampler2D albedo = 0;
uniform sampler2D decal = 1;
uniform sampler2D lightmap = 1;
uniform sampler2D decalmask = 1;
uniform sampler2D alphamask = 1;
uniform sampler2D vegetationmap = 2
uniform sampler2D normalmap = 2;
uniform sampler2D detail = 3;
uniform sampler2D cubemap = 3;
uniform sampler2D heightmap = 4;
uniform sampler2D densitymap = 5;
uniform sampler2D decaltexture = 6;
uniform samplerCube atmospheremap = 7;

uniform float normalScale = 1.0;
uniform float inGlossiness = 0.5;
uniform float inSpecularity = 1.0;
uniform vec3 metalFresnelReflectance = vec3(0.5, 0.5, 0.5);
uniform float dielectricFresnelReflectance = 0.5;
<FRAGMENT_SHADER>

//#define MATERIAL_TEXTURE
//#define PIXEL_LIT
//#define NORMALIZED_BLINN_PHONG
//#define FAST_METAL
//#define FAST_NORMALIZATION
//#define REFLECTION

#extension GL_ARB_shader_texture_lod : enable

#if defined(FRAMEBUFFER_FETCH)
#extension GL_EXT_shader_framebuffer_fetch : require
#endif

#ifdef GL_ES
// define default precision for float, vec, mat.
precision highp float;
#else
#define lowp
#define highp
#define mediump
#endif

#if !defined(VIEW_AMBIENT) && !defined(VIEW_DIFFUSE) && !defined(VIEW_SPECULAR) && !defined(VIEW_ALBEDO)
#define VIEW_AMBIENT
#define VIEW_DIFFUSE
#define VIEW_SPECULAR
#define VIEW_ALBEDO
#endif

const float _PI = 3.141592654;

// DECLARATIONS
#if defined(MATERIAL_TEXTURE)
uniform sampler2D albedo;
#if defined(TEXTURE0_ANIMATION_SHIFT)
varying highp vec2 varTexCoord0;
#else
varying mediump vec2 varTexCoord0;
#endif
#elif defined(MATERIAL_SKYBOX)
uniform samplerCube cubemap;
varying mediump vec3 varTexCoord0;
#endif

#if defined(REFLECTION)
uniform samplerCube cubemap;
#if defined(VERTEX_LIT)
varying mediump vec3 reflectionDirectionInWorldSpace;
#elif defined(PIXEL_LIT)
uniform mat3 worldInvTransposeMatrix;
varying mediump vec3 cameraToPointInTangentSpace;
varying mediump mat3 tbnToWorldMatrix;
#endif
#endif

#if defined(MATERIAL_DECAL)
uniform sampler2D decal;
#endif

#if defined(ALPHA_MASK)
uniform sampler2D alphamask;
#endif

#if defined(MATERIAL_DETAIL) || defined(MATERIAL_GRASS_BLEND)
uniform sampler2D detail;
#endif

#if defined(MATERIAL_LIGHTMAP)
uniform sampler2D lightmap;
#endif

#if defined(MATERIAL_DECAL) || defined(MATERIAL_LIGHTMAP) || defined(FRAME_BLEND) || defined(ALPHA_MASK)
varying highp vec2 varTexCoord1;
#endif

#if defined(MATERIAL_DETAIL)
varying mediump vec2 varDetailTexCoord;
#endif

#if defined(PIXEL_LIT)
uniform sampler2D normalmap;
uniform float materialSpecularShininess;
uniform float lightIntensity0;
uniform float inSpecularity;
uniform float physicalFresnelReflectance;
uniform vec3 metalFresnelReflectance;
uniform float normalScale;
#endif

#if defined(TILED_DECAL_MASK)
uniform sampler2D decalmask;
uniform sampler2D decaltexture;
uniform lowp vec4 decalTileColor;
varying vec2 varDecalTileTexCoord;
#endif

#if defined(VERTEX_LIT) || defined(PIXEL_LIT) || defined(VERTEX_FOG)
uniform vec3 lightAmbientColor0;
uniform vec3 lightColor0;
uniform float inGlossiness;
#endif

#if defined(VERTEX_LIT)
varying lowp float varDiffuseColor;

#if defined(BLINN_PHONG)
varying lowp float varSpecularColor;
#elif defined(NORMALIZED_BLINN_PHONG)
varying lowp vec3 varSpecularColor;
varying lowp float varNdotH;
#endif
#endif

#if defined(PIXEL_LIT)
varying vec3 varToLightVec;

    #if defined(FAST_NORMALIZATION)
    varying vec3 varHalfVec;
    #endif

varying vec3 varToCameraVec;
uniform vec4 lightPosition0;
varying float varPerPixelAttenuation;
#endif

#if defined(VERTEX_FOG)
varying lowp float varFogAmoung;
varying lowp vec3 varFogColor;
#endif

#if defined(SETUP_LIGHTMAP)
varying lowp float varLightmapSize;
#endif

#if defined(VERTEX_COLOR) || defined(SPHERICAL_LIT)
varying lowp vec4 varVertexColor;
#endif

#if defined(FRAME_BLEND)
varying lowp float varTime;
#endif

#if defined(FLATCOLOR)
uniform lowp vec4 flatColor;
#endif

float FresnelShlick(float NdotL, float Cspec)
{
    float expf = 5.0;
	return Cspec + (1.0 - Cspec) * pow(1.0 - NdotL, expf);
}

vec3 FresnelShlickVec3(float NdotL, vec3 Cspec)
{
    float expf = 5.0;
	return Cspec + (1.0 - Cspec) * (pow(1.0 - NdotL, expf));
}

#if defined(MATERIAL_GRASS_TRANSFORM)

#if defined(VEGETATION_DRAW_LOD_COLOR)
uniform vec3 lodColor;
#endif


#if defined(MATERIAL_GRASS_BLEND)
varying vec2 varTexCoord2;
#endif

#if defined(MATERIAL_GRASS_OPAQUE) || defined(MATERIAL_GRASS_BLEND)
varying lowp vec3 varVegetationColor;
#endif

#endif

void main()
{
    // FETCH PHASE
#if defined(MATERIAL_TEXTURE)
    
#if defined(PIXEL_LIT) || defined(ALPHATEST) || defined(ALPHABLEND) || defined(VERTEX_LIT)
    lowp vec4 textureColor0 = texture2D(albedo, varTexCoord0);
    #if defined (ALPHA_MASK)        
        textureColor0.a *= texture2D(alphamask, varTexCoord1).a;
    #endif
#else
    lowp vec3 textureColor0 = texture2D(albedo, varTexCoord0).rgb;
#endif
    
#if defined(FRAME_BLEND)
    lowp vec4 blendFrameColor = texture2D(albedo, varTexCoord1);
    textureColor0 = mix(textureColor0, blendFrameColor, varTime);
#endif
    
#elif defined(MATERIAL_SKYBOX)
    lowp vec4 textureColor0 = textureCube(cubemap, varTexCoord0);
#endif
    
#if defined(MATERIAL_TEXTURE)
    #if defined(ALPHATEST)
        float alpha = textureColor0.a;
        #if defined(VERTEX_COLOR)
            alpha *= varVertexColor.a;
        #endif
        if (alpha < 0.5)discard;
    #endif
    #if defined(ALPHATESTVALUE)
        float alpha = textureColor0.a;
        #if defined(VERTEX_COLOR)
            alpha *= varVertexColor.a;
        #endif
        if (alpha < ALPHATESTVALUE)discard;
    #endif
#endif
    
#if defined(MATERIAL_DECAL)
    lowp vec3 textureColor1 = texture2D(decal, varTexCoord1).rgb;
#endif

#if defined(MATERIAL_LIGHTMAP)
    lowp vec3 textureColor1 = texture2D(lightmap, varTexCoord1).rgb;
#endif
    
#if defined(MATERIAL_DETAIL)
    lowp vec3 detailTextureColor = texture2D(detail, varDetailTexCoord).rgb;
#endif

#if defined(MATERIAL_DECAL) || defined(MATERIAL_LIGHTMAP)
#if defined(SETUP_LIGHTMAP)
    vec3 lightGray = vec3(0.75, 0.75, 0.75);
    vec3 darkGray = vec3(0.25, 0.25, 0.25);
    
    bool isXodd;
    bool isYodd;
    if(fract(floor(varTexCoord1.x*varLightmapSize)/2.0) == 0.0)
    {
        isXodd = true;
    }
    else
    {
        isXodd = false;
    }
    if(fract(floor(varTexCoord1.y*varLightmapSize)/2.0) == 0.0)
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
#if defined(VERTEX_LIT)
    
#if defined(BLINN_PHONG)
    vec3 color = vec3(0.0);
    #if defined(VIEW_AMBIENT)
        color += lightAmbientColor0;
    #endif

    #if defined(VIEW_DIFFUSE)
        color += varDiffuseColor;
    #endif

    #if defined(VIEW_ALBEDO)
        #if defined(TILED_DECAL_MASK)
            lowp float maskSample = texture2D(decalmask, varTexCoord0).a;
            lowp vec4 tileColor = texture2D(decaltexture, varDecalTileTexCoord).rgba * decalTileColor;
            color *= textureColor0.rgb + (tileColor.rgb - textureColor0.rgb) * tileColor.a * maskSample;
        #else
            color *= textureColor0.rgb;
        #endif
    #endif

    #if defined(VIEW_SPECULAR)
        color += (varSpecularColor * textureColor0.a) * lightColor0;
    #endif
    
#elif defined(NORMALIZED_BLINN_PHONG)
   
    vec3 color = vec3(0.0);
    #if defined(VIEW_AMBIENT) && !defined(MATERIAL_LIGHTMAP)
        color += lightAmbientColor0;
    #endif
        
    #if defined(VIEW_DIFFUSE)
        #if defined(MATERIAL_LIGHTMAP)
            color = textureColor1.rgb * 2.0;
        #else
            color += varDiffuseColor * lightColor0;
        #endif
    #endif
        
    #if defined(VIEW_ALBEDO)
        #if defined(TILED_DECAL_MASK)
            lowp float maskSample = texture2D(decalmask, varTexCoord0).a;
            lowp vec4 tileColor = texture2D(decaltexture, varDecalTileTexCoord).rgba * decalTileColor;
            color *= textureColor0.rgb + (tileColor.rgb - textureColor0.rgb) * tileColor.a * maskSample;
        #else
            color *= textureColor0.rgb;
        #endif
    #endif
    
    #if defined(VIEW_SPECULAR)
        float glossiness = pow(5000.0, inGlossiness * textureColor0.a);
        float specularNorm = (glossiness + 2.0) / 8.0;
        vec3 spec = varSpecularColor * pow(varNdotH, glossiness) * specularNorm;
    
        #if defined(MATERIAL_LIGHTMAP)
            color += spec * textureColor1.rgb / 2.0 * lightColor0;
        #else
            color += spec * lightColor0;
        #endif
    #endif
    
#endif

    
#elif defined(PIXEL_LIT)
    // lookup normal from normal map, move from [0, 1] to  [-1, 1] range, normalize
    vec3 normal = 2.0 * texture2D (normalmap, varTexCoord0).rgb - 1.0;
    normal.xy *= normalScale;
    normal = normalize (normal);
   	//normal.z = sqrt(1.0 - (normal.x * normal.x + normal.y * normal.y));
    //normal = vec3(0.0, 0.0, 1.0);
    
    float attenuation = lightIntensity0;
#if defined(DISTANCE_ATTENUATION)
    attenuation /= (varPerPixelAttenuation * varPerPixelAttenuation);
#endif
    
#if !defined(FAST_NORMALIZATION)
    vec3 toLightNormalized = normalize(varToLightVec);
    vec3 toCameraNormalized = normalize(varToCameraVec);
    vec3 H = toCameraNormalized + toLightNormalized;
    H = normalize(H);

    // compute diffuse lighting
    float NdotL = max (dot (normal, toLightNormalized), 0.0);
    float NdotH = max (dot (normal, H), 0.0);
    float LdotH = max (dot (toLightNormalized, H), 0.0);
    float NdotV = max (dot (normal, toCameraNormalized), 0.0);
#else
    // Kwasi normalization :-)
    // compute diffuse lighting
    vec3 normalizedHalf = normalize(varHalfVec);
    
    float NdotL = max (dot (normal, varToLightVec), 0.0);
    float NdotH = max (dot (normal, normalizedHalf), 0.0);
    float LdotH = max (dot (varToLightVec, normalizedHalf), 0.0);
    float NdotV = max (dot (normal, varToCameraVec), 0.0);
#endif
    
#if defined(NORMALIZED_BLINN_PHONG)
    
#if defined(DIELECTRIC)
    #define ColorType float
    float fresnelOut = FresnelShlick(NdotV, dielectricFresnelReflectance);
#else
    #if defined(FAST_METAL)
        #define ColorType float
        float fresnelOut = FresnelShlick(NdotV, (metalFresnelReflectance.r + metalFresnelReflectance.g + metalFresnelReflectance.b) / 3.0);
    #else
        #define ColorType vec3
        vec3 fresnelOut = FresnelShlickVec3(NdotV, metalFresnelReflectance);
    #endif
#endif
    
    float specularity = inSpecularity;
    float glossiness = inGlossiness * textureColor0.a;
    float glossPower = pow(5000.0, glossiness); //textureColor0.a;
    
   	//float glossiness = inGlossiness * 0.999;
	//glossiness = 200.0 * glossiness / (1.0 - glossiness);
//#define GOTANDA
#if defined(GOTANDA)
    vec3 fresnelIn = FresnelShlickVec3(NdotL, metalFresnelReflectance);
	vec3 diffuse = NdotL / _PI * (1.0 - fresnelIn * specularity);
#else
	float diffuse = NdotL / _PI;// * (1.0 - fresnelIn * specularity);
#endif
    
#if defined(GOTANDA)
	float specCutoff = 1.0 - NdotL;
	specCutoff *= specCutoff;
	specCutoff *= specCutoff;
	specCutoff *= specCutoff;
	specCutoff = 1.0 - specCutoff;
#else
    float specCutoff = NdotL;
#endif
    
#if defined(GOTANDA)
    float specularNorm = (glossPower + 2.0) * (glossPower + 4.0) / (8.0 * _PI * (pow(2.0, -glossPower / 2.0) + glossPower));
#else
    float specularNorm = (glossPower + 2.0) / 8.0;
#endif
    float specularNormalized = specularNorm * pow(NdotH, glossPower) * specCutoff * specularity;
#if defined(FAST_METAL)
	float geometricFactor = 1.0;
#else
    float geometricFactor = 1.0 / LdotH * LdotH;
#endif
    ColorType specular = specularNormalized * geometricFactor * fresnelOut;
#endif
    
    vec3 color = vec3(0.0);
    
    #if defined(VIEW_AMBIENT) && !defined(MATERIAL_LIGHTMAP)
        color += lightAmbientColor0;
    #endif
    
    #if defined(VIEW_DIFFUSE)
        #if defined(MATERIAL_LIGHTMAP)
            color = textureColor1.rgb * 2.0;
        #else
            color += diffuse * lightColor0;
        #endif
    #endif
    
    #if defined(VIEW_ALBEDO)
        #if defined(TILED_DECAL_MASK)
            lowp float maskSample = texture2D(decalmask, varTexCoord0).a;
            lowp vec4 tileColor = texture2D(decaltexture, varDecalTileTexCoord).rgba * decalTileColor;
            color *= textureColor0.rgb + (tileColor.rgb - textureColor0.rgb) * tileColor.a * maskSample;
        #else
            color *= textureColor0.rgb;
        #endif
    #endif
    
    #if defined(VIEW_SPECULAR)
        #if defined(MATERIAL_LIGHTMAP)
            color += specular * textureColor1.rgb * lightColor0;
        #else
            color += specular * lightColor0;
        #endif
    #endif
#elif defined(MATERIAL_DECAL) || defined(MATERIAL_LIGHTMAP)
    vec3 color = vec3(0.0);
    #if defined(VIEW_ALBEDO)
        color = textureColor0.rgb;
	#else
		color = vec3(1.0);
    #endif
    #if defined(VIEW_DIFFUSE)
        color *= textureColor1.rgb * 2.0;
    #endif
#elif defined(MATERIAL_TEXTURE)
    vec3 color = textureColor0.rgb;
#elif defined(MATERIAL_SKYBOX)
    vec4 color = textureColor0;
#else
    vec3 color = vec3(1.0);
#endif
    
#if defined(MATERIAL_DETAIL)
	color *= detailTextureColor.rgb * 2.0;
#endif
	
#if defined(ALPHABLEND) && defined(MATERIAL_TEXTURE)
    gl_FragColor = vec4(color, textureColor0.a);
#elif defined(MATERIAL_SKYBOX)
    gl_FragColor = color;
#else
    gl_FragColor = vec4(color, 1.0);
#endif
    
#if defined(VERTEX_COLOR) || defined(SPEED_TREE_LEAF) || defined(SPHERICAL_LIT)
    gl_FragColor *= varVertexColor;
#endif
    
#if defined(FLATCOLOR)
    gl_FragColor *= flatColor;
#endif
    
    
#if defined(REFLECTION)
#if defined(VERTEX_LIT)
    lowp vec4 reflectionColor = textureCube(cubemap, reflectionDirectionInWorldSpace); //vec3(reflectedDirection.x, reflectedDirection.y, reflectedDirection.z));
    gl_FragColor = reflectionColor * 0.9;
#elif defined(PIXEL_LIT)
    //vec3 fresnelRefl = FresnelShlickVec3(NdotV, metalFresnelReflectance);

    mediump vec3 reflectionVectorInTangentSpace = reflect(cameraToPointInTangentSpace, normal);
    mediump vec3 reflectionVectorInWorldSpace = worldInvTransposeMatrix * (tbnToWorldMatrix * reflectionVectorInTangentSpace);
    lowp vec4 reflectionColor = textureCube(cubemap, reflectionVectorInWorldSpace, (1.0 - glossiness) * 7.0); //vec3(reflectedDirection.x, reflectedDirection.y, reflectedDirection.z));
    gl_FragColor.rgb += fresnelOut * reflectionColor.rgb * specularity;//* textureColor0.rgb;
    //gl_FragColor.rgb += reflectionColor.rgb * textureColor0.rgb;
#endif
#endif
    //    gl_FragColor.r += 0.5;

#if defined(MATERIAL_GRASS_TRANSFORM)
    #if defined(MATERIAL_GRASS_OPAQUE)
        gl_FragColor.rgb = gl_FragColor.rgb * varVegetationColor * 2.0;
    #endif
        
    #if defined(MATERIAL_GRASS_BLEND)
        gl_FragColor.a = gl_FragColor.a * varTexCoord2.x;
        #if defined(FRAMEBUFFER_FETCH)
            //VI: fog is taken to account here
            #if defined(VERTEX_FOG)
                gl_FragColor.rgb = mix(gl_LastFragData[0].rgb, mix(gl_FragColor.rgb * varVegetationColor * 2.0, varFogColor, varFogAmoung), gl_FragColor.a);
            #else
                gl_FragColor.rgb = mix(gl_LastFragData[0].rgb, gl_FragColor.rgb * varVegetationColor * 2.0, gl_FragColor.a);
            #endif
        #else
            gl_FragColor.rgb = gl_FragColor.rgb * varVegetationColor * 2.0;
        #endif
    #endif
    
    #if defined(MATERIAL_GRASS_OPAQUE) || defined(MATERIAL_GRASS_BLEND)
        #if defined(VEGETATION_DRAW_LOD_COLOR)
            gl_FragColor.rgb += lodColor;
        #endif
    #endif

#endif
    
#if defined(VERTEX_FOG)
    #if !defined(FRAMEBUFFER_FETCH)
        //VI: fog equation is inside of color equation for framebuffer fetch
        gl_FragColor.rgb = mix(gl_FragColor.rgb, varFogColor, varFogAmoung);
    #endif
#endif
}