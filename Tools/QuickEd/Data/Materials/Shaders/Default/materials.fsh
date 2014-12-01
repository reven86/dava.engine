<CONFIG>
albedo = 0
cubemap = 2
decal = 1
detail = 1
lightmap = 1
normalmap = 1
<FRAGMENT_SHADER>

#ifdef GL_ES
// define default precision for float, vec, mat.
precision highp float;
#else
#define lowp
#define highp
#define mediump
#endif

//#define MATERIAL_TEXTURE
//#define VERTEX_COLOR
//#define ALPHABLEND
//#define FLATCOLOR
//#define VERTEX_FOG

// DECLARATIONS
#if defined(MATERIAL_TEXTURE)
uniform sampler2D albedo;
varying mediump vec2 varTexCoord0;
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

#if defined(MATERIAL_DETAIL)
uniform sampler2D detail;
#endif

#if defined(MATERIAL_LIGHTMAP) || defined(MATERIAL_VIEW_LIGHTMAP_ONLY)
uniform sampler2D lightmap; //[1]:ONCE
#endif

#if defined(MATERIAL_DECAL) || defined(MATERIAL_DETAIL) || defined(MATERIAL_LIGHTMAP) || defined(MATERIAL_VIEW_LIGHTMAP_ONLY) || defined(FRAME_BLEND)
varying highp vec2 varTexCoord1;
#endif

#if defined(PIXEL_LIT)
uniform sampler2D normalmap; // [1]:ONCE
uniform float materialSpecularShininess;
uniform float lightIntensity0; 
#endif

#if defined(VERTEX_LIT) || defined(PIXEL_LIT)
uniform vec3 materialLightAmbientColor;     // engine pass premultiplied material * light ambient color
uniform vec3 materialLightDiffuseColor;     // engine pass premultiplied material * light diffuse color
uniform vec3 materialLightSpecularColor;    // engine pass premultiplied material * light specular color
#endif

#if defined(VERTEX_LIT)
varying lowp float varDiffuseColor;
varying lowp float varSpecularColor;
#endif 

#if defined(PIXEL_LIT)
varying vec3 varLightVec;
varying vec3 varHalfVec;
varying vec3 varEyeVec;
varying float varPerPixelAttenuation;
#endif

#if defined(VERTEX_FOG)
uniform vec3 fogColor;
varying float varFogFactor;
#endif

#if defined(SPEED_TREE_LEAF)
uniform lowp vec3 treeLeafColorMul;
uniform lowp float treeLeafOcclusionOffset;
uniform lowp float treeLeafOcclusionMul;
#endif

#if defined(SETUP_LIGHTMAP)
varying lowp float varLightmapSize;
#endif

#if defined(VERTEX_COLOR)
varying lowp vec4 varVertexColor;
#endif

#if defined(FRAME_BLEND)
varying lowp float varTime;
#endif

#if defined(FLATCOLOR)
uniform lowp vec4 flatColor;
#endif

void main()
{
    // FETCH PHASE
#if defined(MATERIAL_TEXTURE)
	
#if defined(PIXEL_LIT) || defined(ALPHATEST) || defined(ALPHABLEND) || defined(VERTEX_LIT)
    lowp vec4 textureColor0 = texture2D(albedo, varTexCoord0);
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
#endif
 
#if defined(MATERIAL_DECAL)
        lowp vec3 textureColor1 = texture2D(decal, varTexCoord1).rgb;
#endif

#if defined(MATERIAL_DETAIL)
        lowp vec3 textureColor1 = texture2D(detail, varTexCoord1).rgb;
#endif
        
#if defined(MATERIAL_LIGHTMAP) || defined(MATERIAL_VIEW_LIGHTMAP_ONLY)
        lowp vec3 textureColor1 = texture2D(lightmap, varTexCoord1).rgb;
#endif
        
#if defined(MATERIAL_DECAL) || defined(MATERIAL_DETAIL) || defined(MATERIAL_LIGHTMAP) || defined(MATERIAL_VIEW_LIGHTMAP_ONLY)
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
    vec3 color = materialLightAmbientColor + varDiffuseColor * materialLightDiffuseColor
                                           + (varSpecularColor * textureColor0.a) * materialLightSpecularColor;
    color *= textureColor0.rgb;
#elif defined(PIXEL_LIT)
        // lookup normal from normal map, move from [0, 1] to  [-1, 1] range, normalize
    vec3 normal = 2.0 * texture2D (normalmap, varTexCoord0).rgb - 1.0;
    normal = normalize (normal);

    float attenuation = lightIntensity0;
#if defined(DISTANCE_ATTENUATION)
    attenuation /= (varPerPixelAttenuation * varPerPixelAttenuation);
#endif
    
    // compute diffuse lighting
    float lambertFactor = max (dot (varLightVec, normal), 0.0);
    // compute ambient
    vec3 color = materialLightAmbientColor + materialLightDiffuseColor * lambertFactor * attenuation;

#if defined(SPECULAR)
    if (lambertFactor > 0.0)
    {
        float shininess = pow (max (dot (varHalfVec, normal), 0.0), materialSpecularShininess);
        color += materialLightSpecularColor * (shininess * textureColor0.a * attenuation);
    }
    color *= textureColor0.rgb;
#endif

#elif defined(MATERIAL_VIEW_LIGHTMAP_ONLY)        
    vec3 color = textureColor1.rgb;
#elif defined(MATERIAL_VIEW_TEXTURE_ONLY)
    vec3 color = textureColor0.rgb;
#elif defined(MATERIAL_DECAL) || defined(MATERIAL_LIGHTMAP) || defined(MATERIAL_DETAIL)
    vec3 color = textureColor0.rgb * textureColor1.rgb * 2.0;
#elif defined(MATERIAL_TEXTURE)
    vec3 color = textureColor0.rgb;
#elif defined(MATERIAL_SKYBOX)
        vec4 color = textureColor0;
#else
        vec3 color = vec3(1.0);
#endif

#if defined(ALPHABLEND) && defined(MATERIAL_TEXTURE)
        gl_FragColor = vec4(color, textureColor0.a);
#elif defined(MATERIAL_SKYBOX)
        gl_FragColor = color;
#else
    gl_FragColor = vec4(color, 1.0);
#endif

#if defined(SPEED_TREE_LEAF)
        gl_FragColor *= vec4(varVertexColor.rgb * treeLeafColorMul * treeLeafOcclusionMul + vec3(treeLeafOcclusionOffset), varVertexColor.a);
#elif defined(VERTEX_COLOR)
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
    mediump vec3 reflectionVectorInTangentSpace = reflect(cameraToPointInTangentSpace, normal);
    mediump vec3 reflectionVectorInWorldSpace = worldInvTransposeMatrix * (tbnToWorldMatrix * reflectionVectorInTangentSpace);
    lowp vec4 reflectionColor = textureCube(cubemap, reflectionVectorInWorldSpace); //vec3(reflectedDirection.x, reflectedDirection.y, reflectedDirection.z));
    gl_FragColor = reflectionColor;
#endif
#endif
    
    
#if defined(VERTEX_FOG)
    gl_FragColor.rgb = mix(fogColor, gl_FragColor.rgb, varFogFactor);
#endif
}