<CONFIG>
uniform sampler2D normalmap = 0
uniform sampler2D normalmap1 = 3
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


uniform sampler2D normalmap;
#if defined(SEPARATE_NORMALMAPS)
uniform sampler2D normalmap1; 
#endif

uniform mat3 worldInvTransposeMatrix;
varying mediump vec3 cameraToPointInTangentSpace;
varying mediump mat3 tbnToWorldMatrix;

#if defined (DEBUG_Z_NORMAL_SCALE)
uniform lowp float normal0_z_scale;
uniform lowp float normal1_z_scale;
#endif

#if defined(SPECULAR) && defined (REAL_REFLECTION)
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

#if defined(SPECULAR) && defined (REAL_REFLECTION)
uniform lowp vec3 lightColor0;   
uniform float inGlossiness;
uniform float inSpecularity;
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
    lowp vec3 normal0 = texture2D (normalmap, varTexCoord0).rgb;
    #if defined(SEPARATE_NORMALMAPS)
        lowp vec3 normal1 = texture2D (normalmap1, varTexCoord1).rgb;    
    #else
        lowp vec3 normal1 = texture2D (normalmap, varTexCoord1).rgb;
    #endif
    
    #if defined (DEBUG_Z_NORMAL_SCALE)
        normal0 = normal0 * 2.0 - 1.0;
        normal1 = normal1 * 2.0 - 1.0;
        normal0.xy *= normal0_z_scale;
        normal1.xy *= normal1_z_scale;
        lowp vec3 normal = normalize (normal0 + normal1);
    #else    
        lowp vec3 normal = normalize (normal0 + normal1 - 1.0); //same as * 2 -2
    #endif
	
#if defined (DEBUG_UNITY_Z_NORMAL)    
	normal = vec3(0.0,0.0,1.0);
#endif
	
    //compute shininess and fresnel
    lowp vec3 cameraToPointInTangentSpaceNorm = normalize(cameraToPointInTangentSpace);    
    lowp float lambertFactor = max (dot (-cameraToPointInTangentSpaceNorm, normal), 0.0);
    lowp float fresnel = FresnelShlick(lambertFactor, fresnelBias, fresnelPow);
#if defined (SPECULAR) && defined (REAL_REFLECTION)
    lowp vec3 halfVec = normalize(normalize(varLightVec)-cameraToPointInTangentSpaceNorm);    
    
    float glossPower = pow(5000.0, inGlossiness); //textureColor0.a;
    float specularNorm = (glossPower + 2.0) / 8.0;
    float specularNormalized = specularNorm * pow(max (dot (halfVec, normal), 0.0), glossPower) * inSpecularity;
    float specular = specularNormalized  * fresnel;
    lowp vec3 resSpecularColor = lightColor0*specular;
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
        resColor+=resSpecularColor*reflectionColor;        
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
        gl_FragColor = vec4(resColor, 1.0);

    #endif
#endif
#endif
    
#if defined(VERTEX_FOG)
	gl_FragColor.rgb = mix(gl_FragColor.rgb, varFogColor, varFogAmoung);
#endif
}