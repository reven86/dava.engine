<CONFIG>
albedo = 0
normalmap = 1
dynamicReflection = 2
dynamicRefraction = 3
<FRAGMENT_SHADER>

#ifdef GL_ES
// define default precision for float, vec, mat.
precision highp float;
#else
#define lowp
#define highp
#define mediump
#endif

varying mediump vec2 varTexCoord0;
varying mediump vec2 varTexCoord1;

varying mediump float eyeDist;

varying mediump vec3 cameraToPointInTangentSpace;
varying mediump mat3 tbnToWorldMatrix;

varying vec3 varLightVec;


uniform lowp vec3 reflectionTintColor;
uniform lowp vec3 refractionTintColor;
uniform float fresnelBias;
uniform float fresnelPow;
uniform float eta;
uniform float aproxDepth;
uniform float aproxReflectionScale;
uniform float cubmapOffset;

uniform float reflectionDistortion;
uniform float refractionDistortion;

uniform mediump vec2 rcpScreenSize;
uniform sampler2D dynamicReflection;
uniform sampler2D dynamicRefraction;

uniform mat4 worldViewProjMatrix;


uniform sampler2D normalmap;
uniform float materialSpecularShininess;
uniform vec3 materialLightSpecularColor;    // engine pass premultiplied material * light specular color

#if defined(VERTEX_FOG)
uniform vec3 fogColor;
varying float varFogFactor;
#endif


float FresnelShlick(float NdotL, float fresnelBias, float fresnelPow)
{	  
	return fresnelBias + (1.0 - fresnelBias) * pow(1.0 - NdotL, fresnelPow);
}


void main()
{

    vec3 normal1 = 2.0 * texture2D (normalmap, varTexCoord0).rgb - 1.0;
    vec3 normal2 = 2.0 * texture2D (normalmap, varTexCoord1).rgb - 1.0;
    vec3 normal = normalize (normal1 + normal2);

	normal = vec3(0.0, 0.0, 1.0);
    
    vec3 cameraToPointInTangentSpaceNorm = normalize(cameraToPointInTangentSpace);
    // compute diffuse lighting
    float lambertFactor = max (dot (-cameraToPointInTangentSpaceNorm, normal), 0.0);
    // compute specular
	vec3 halfVec = (normalize(varLightVec)-cameraToPointInTangentSpaceNorm);
    float shininess = pow (max (dot (normalize(halfVec), normal), 0.0), materialSpecularShininess);
    
	float fresnel = FresnelShlick(lambertFactor, fresnelBias, fresnelPow);



	
#if defined(SCREEN_SPACE_WATER)
	lowp vec2 waveOffset = normal.xy/max(1.0, eyeDist);
	mediump vec2 screenPos = gl_FragCoord.xy*rcpScreenSize;
	lowp vec3 reflectionColor = texture2D(dynamicReflection, screenPos+waveOffset*reflectionDistortion).rgb; //vec3(reflectedDirection.x, reflectedDirection.y, reflectedDirection.z));	
	screenPos.y=1.0-screenPos.y;
	lowp vec3 refractionColor = texture2D(dynamicRefraction, screenPos+waveOffset*refractionDistortion).rgb; //vec3(reflectedDirection.x, reflectedDirection.y, reflectedDirection.z));	
#else	
	/**/
	//reflected vector is mixed with camera vector 
	//we need to swap z as reflected texture is already in reflected camera space
	mediump vec3 reflectionVectorInTangentSpace = mix(-cameraToPointInTangentSpace, reflect(normalize(cameraToPointInTangentSpace), normal)*vec3(1.0,1.0,-1.0), aproxReflectionScale); 		
	mediump vec4 reflectionVectorInNDCSpace = (worldViewProjMatrix * vec4((tbnToWorldMatrix * reflectionVectorInTangentSpace), 0.0));	
	lowp vec3 reflectionColor = texture2D(dynamicReflection, reflectionVectorInNDCSpace.xy/reflectionVectorInNDCSpace.w*0.5 + vec2(0.5, 0.5)).rgb;
	
	mediump vec3 refractionVectorInTangentSpace = refract(normalize(cameraToPointInTangentSpace), normal, eta)*aproxDepth-cameraToPointInTangentSpace;
	mediump vec4 refractionVectorInNDCSpace = (worldViewProjMatrix * vec4((tbnToWorldMatrix * refractionVectorInTangentSpace), 0.0));	
	lowp vec3 refractionColor = texture2D(dynamicRefraction, refractionVectorInNDCSpace.xy/refractionVectorInNDCSpace.w*vec2(0.5, -0.5)+vec2(0.5, 0.5)).rgb;
#endif	    
  
    //gl_FragColor = vec4(vec3(lambertFactor), 1.0);	
    gl_FragColor = vec4(mix(refractionColor*refractionTintColor, reflectionColor*reflectionTintColor, fresnel) + shininess * materialLightSpecularColor, 1.0);	
    
#if defined(VERTEX_FOG)
    gl_FragColor.rgb = mix(fogColor, gl_FragColor.rgb, varFogFactor);
#endif
}