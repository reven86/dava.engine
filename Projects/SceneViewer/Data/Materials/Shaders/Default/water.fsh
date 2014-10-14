<CONFIG>
uniform sampler2D normalmap = 0
uniform sampler2D albedo = 0
uniform samplerCube cubemap = 1
uniform sampler2D decal = 2
uniform sampler2D dynamicReflection = 1
uniform sampler2D dynamicRefraction = 2
uniform samplerCube atmospheremap = 7;
<FRAGMENT_SHADER>

#ifdef GL_ES
// define default precision for float, vec, mat.
precision highp float;
#else
#define lowp
#define highp
#define mediump
#endif


uniform samplerCube cubemap;
uniform lowp vec3 reflectionTintColor;

#if defined(VERTEX_LIT)
varying mediump vec3 reflectionDirectionInWorldSpace;

#if defined(MATERIAL_DECAL)
uniform sampler2D decal;
uniform sampler2D albedo;
uniform lowp vec3 decalTintColor;
uniform lowp vec3 reflectanceColor;
varying highp vec2 varTexCoord0;
varying highp vec2 varTexCoord1;
varying highp vec2 varTexCoordDecal;

#endif

#elif defined(PIXEL_LIT)

uniform sampler2D normalmap; // [1]:ONCE

uniform mat3 worldInvTransposeMatrix;
varying mediump vec3 cameraToPointInTangentSpace;
varying mediump mat3 tbnToWorldMatrix;

#if defined(SPECULAR)
varying vec3 varLightVec;
#endif

varying highp vec2 varTexCoord0;
varying highp vec2 varTexCoord1;

uniform lowp vec3 refractionTintColor;

uniform lowp float fresnelBias;
uniform mediump float fresnelPow;

#if defined CONST_REFRACTION_COLOR
uniform lowp vec3 refractionConstColor;
#else
uniform lowp float eta;
#endif

#if defined(SPECULAR)
uniform mediump float materialSpecularShininess;
uniform lowp vec3 materialLightSpecularColor;    // engine pass premultiplied material * light specular color
#endif

#if defined (REAL_REFLECTION)
uniform sampler2D dynamicReflection;
uniform sampler2D dynamicRefraction;

#if defined (TANGENT_SPACE_WATER_REFLECTIONS)
uniform float aproxDepth;
uniform float aproxReflectionScale;

uniform mat4 worldViewProjMatrix;
#else
varying mediump vec3 eyeDist;

uniform mediump vec2 rcpScreenSize;
uniform mediump vec2 screenOffset;

uniform float reflectionDistortion;
uniform float refractionDistortion;

uniform highp float distortionFallSquareDist;

#endif

#endif

#endif

#if defined(VERTEX_FOG)
varying lowp float varFogAmoung;
varying lowp vec3 varFogColor;
#endif


float FresnelShlick(float NdotL, float fresnelBias, float fresnelPow)
{	  
    return fresnelBias + (1.0 - fresnelBias) * pow(1.0 - NdotL, fresnelPow);
}


void main()
{

#if defined(VERTEX_LIT)
    lowp vec3 reflectionColor = textureCube(cubemap, reflectionDirectionInWorldSpace).rgb; //vec3(reflectedDirection.x, reflectedDirection.y, reflectedDirection.z));
	#if defined(MATERIAL_DECAL)
		lowp vec3 textureColorDecal = texture2D(decal, varTexCoordDecal).rgb;
		lowp vec3 textureColor0 = texture2D(albedo, varTexCoord0).rgb;
		lowp vec3 textureColor1 = texture2D(albedo, varTexCoord1).rgb;
		//gl_FragColor = vec4((textureColor0 *textureColorDecal* decalTintColor * 2.0 + reflectionColor * reflectanceColor) * textureColor1 * 2.0, 1.0);
		//gl_FragColor = vec4(((textureColor0 + textureColor1)* decalTintColor + reflectionColor * reflectanceColor) * textureColorDecal * 2.0, 1.0);
		//gl_FragColor = vec4((textureColorDecal* decalTintColor * reflectionColor * reflectanceColor) * (textureColor0 + textureColor1) * 2.0, 1.0);
		gl_FragColor = vec4(((textureColor0 * textureColor1) * 3.0 * decalTintColor * textureColorDecal + reflectionColor * reflectanceColor) , 1.0);
	#else
		gl_FragColor = vec4(reflectionColor * reflectionTintColor, 1.0);
	#endif
#elif defined(PIXEL_LIT)
  
    //compute normal
    lowp vec3 normal1 = texture2D (normalmap, varTexCoord0).rgb;
    lowp vec3 normal2 = texture2D (normalmap, varTexCoord1).rgb;
    lowp vec3 normal = normalize (normal1 + normal2 - 1.0); //same as * 2 -2
	
#if defined (DEBUG_UNITY_Z_NORMAL)    
	normal = vec3(0.0,0.0,1.0);
#endif
	
    //compute shininess and fresnel
    lowp vec3 cameraToPointInTangentSpaceNorm = normalize(cameraToPointInTangentSpace);    
    lowp float lambertFactor = max (dot (-cameraToPointInTangentSpaceNorm, normal), 0.0);
    lowp float fresnel = FresnelShlick(lambertFactor, fresnelBias, fresnelPow);
#if defined (SPECULAR)
    lowp vec3 halfVec = normalize(normalize(varLightVec)-cameraToPointInTangentSpaceNorm);
    lowp vec3 resSpecularColor = pow (max (dot (halfVec, normal), 0.0), materialSpecularShininess) * materialLightSpecularColor;
#endif

    //compute reflection
#if defined (REAL_REFLECTION)
    #if defined(TANGENT_SPACE_WATER_REFLECTIONS)        
        //reflected vector is mixed with camera vector 
        //we need to swap z as reflected texture is already in reflected camera space
        mediump vec3 reflectionVectorInTangentSpace = mix(-cameraToPointInTangentSpace, reflect(cameraToPointInTangentSpaceNorm, normal)*vec3(1.0,1.0,-1.0), aproxReflectionScale); 		
        mediump vec4 reflectionVectorInNDCSpace = (worldViewProjMatrix * vec4((tbnToWorldMatrix * reflectionVectorInTangentSpace), 0.0));	
        lowp vec3 reflectionColor = texture2D(dynamicReflection, reflectionVectorInNDCSpace.xy/reflectionVectorInNDCSpace.w*0.5 + vec2(0.5, 0.5)).rgb;
		
        mediump vec3 refractionVectorInTangentSpace = refract(cameraToPointInTangentSpaceNorm, normal, eta)*aproxDepth-cameraToPointInTangentSpace;
        mediump vec4 refractionVectorInNDCSpace = (worldViewProjMatrix * vec4((tbnToWorldMatrix * refractionVectorInTangentSpace), 0.0));	
        lowp vec3 refractionColor = texture2D(dynamicRefraction, refractionVectorInNDCSpace.xy/refractionVectorInNDCSpace.w*vec2(0.5, -0.5)+vec2(0.5, 0.5)).rgb;
    #else	
		//mediump vec2 waveOffset = normal.xy/max(10.0, length(eyeDist));
		mediump vec2 waveOffset = normal.xy*max(0.1, 1.0-dot(eyeDist, eyeDist)*distortionFallSquareDist);
        mediump vec2 screenPos = (gl_FragCoord.xy-screenOffset)*rcpScreenSize;
        lowp vec3 reflectionColor = texture2D(dynamicReflection, screenPos+waveOffset*reflectionDistortion).rgb;
        screenPos.y=1.0-screenPos.y;
        lowp vec3 refractionColor = texture2D(dynamicRefraction, screenPos+waveOffset*refractionDistortion).rgb;        
    #endif	    
    lowp vec3 resColor = mix(refractionColor*refractionTintColor, reflectionColor*reflectionTintColor, fresnel);
    #if defined (SPECULAR)
        resColor+=resSpecularColor;
    #endif
    gl_FragColor = vec4(resColor, 1.0);
	//gl_FragColor = vec4(vec3(normalize(eyeDist)/100.0), 1.0);
#else
    lowp vec3 reflectionVectorInTangentSpace = reflect(cameraToPointInTangentSpaceNorm, normal);
	reflectionVectorInTangentSpace.z = abs(reflectionVectorInTangentSpace.z); //prevent reflection through surface
    lowp vec3 reflectionVectorInWorldSpace = (tbnToWorldMatrix * reflectionVectorInTangentSpace);    
    lowp vec3 reflectionColor = textureCube(cubemap, reflectionVectorInWorldSpace).rgb * reflectionTintColor;
    
    #if defined (FRESNEL_TO_ALPHA)	
		lowp vec3 resColor = reflectionColor;
		#if defined (SPECULAR)
			resColor+=resSpecularColor;
		#endif
		gl_FragColor = vec4(resColor, fresnel);
    
    #else	
		#if defined CONST_REFRACTION_COLOR
			lowp vec3 refractionColor = refractionConstColor; 
		#else
			lowp vec3 refractedVectorInTangentSpace = refract(cameraToPointInTangentSpaceNorm, normal, eta);
			lowp vec3 refractedVectorInWorldSpace = (tbnToWorldMatrix * refractedVectorInTangentSpace);
			lowp vec3 refractionColor = textureCube(cubemap, refractedVectorInWorldSpace).rgb*refractionTintColor; 
        #endif   		
        lowp vec3 resColor = mix(refractionColor, reflectionColor, fresnel);
        #if defined (SPECULAR)
            resColor+=resSpecularColor;
        #endif
        gl_FragColor = vec4(resColor, 1.0);

    #endif
#endif
#endif
    
#if defined(VERTEX_FOG)
	gl_FragColor.rgb = mix(gl_FragColor.rgb, varFogColor, varFogAmoung);
#endif
}