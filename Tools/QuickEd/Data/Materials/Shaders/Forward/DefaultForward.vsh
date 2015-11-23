//#define VERTEX_FOG
//#define VERTEX_LIT
#define LIGHT_NUM 1
#define DISTANCE_ATTENUATION


#ifdef GL_ES
// define default precision for float, vec, mat.
precision highp float;
#else
#define lowp
#define highp
#define mediump
#endif

attribute highp vec4 inPosition;

#if defined(ATTRIBUTE_NORMAL) 
attribute mediump vec3 inNormal;
#endif

#ifdef ATTRIBUTE_TEX0
attribute mediump vec2 inTexCoord0;
#endif 

#ifdef ATTRIBUTE_TEX1
attribute mediump vec2 inTexCoord1;
#endif

#ifdef ATTRIBUTE_TEX2
attribute mediump vec2 inTexCoord2;
#endif

#ifdef ATTRIBUTE_TEX3
attribute mediump vec2 inTexCoord2;
#endif

#ifdef ATTRIBUTE_COLOR
attribute lowp vec4 inColor;
#endif

#ifdef ATTRIBUTE_TANGENT
attribute mediump vec3 inTangent;
#endif

// UNIFORMS
uniform mediump mat4 worldViewProjMatrix;

#if defined(VERTEX_LIT) || defined(PIXEL_LIT) || defined(VERTEX_FOG)
uniform mediump mat4 worldViewMatrix;
uniform mediump mat3 worldViewInvTransposeMatrix;
#endif 

#if defined(VERTEX_LIT) || defined(PIXEL_LIT)
uniform mediump vec3 lightPosition[LIGHT_NUM];
uniform mediump float lightIntensity[LIGHT_NUM];
#endif

#ifdef VERTEX_LIT
uniform mediump float materialSpecularShininess;
#endif

#if defined(VERTEX_FOG)
uniform mediump float fogDensity;
#endif

#if defined(MATERIAL_LIGHTMAP)
uniform mediump vec2 uvOffset;
uniform mediump vec2 uvScale;
#endif

#if defined(VERTEX_LIT)
varying lowp float varDiffuseColor;
varying lowp float varSpecularColor;
#endif

#if defined(PIXEL_LIT)
varying mediump vec3 varLightVec;
varying mediump vec3 varHalfVec;
varying mediump vec3 varEyeVec;
varying mediump float varPerPixelAttenuation;
#endif

#if defined(VERTEX_FOG)
varying mediump float varFogFactor;
#endif

#if defined(MATERIAL_LIGHTMAP_DEBUG)
uniform mediump float lightmapSize;
varying lowp float varLightmapSize;
#endif

#if defined(VERTEX_COLOR)
varying lowp vec4 varVertexColor;
#endif

ADDITIONAL_UNIFORMS
ADDITIONAL_VARYINGS


//#define APPLY_LIGHT_SOURCE(index,


void main()
{
	gl_Position = worldViewProjMatrix * inPosition;
#if defined(VERTEX_LIT)
    mediump vec3 eyeCoordsPosition = vec3(worldViewMatrix * inPosition);
    mediump vec3 normal = normalize(worldViewInvTransposeMatrix * inNormal); // normal in eye coordinates
    
    
    vec3 lightDir = lightPosition[0] - eyeCoordsPosition;
    float attenuation = length(lightDir);
    attenuation = lightIntensity[0] / (attenuation * attenuation); // use inverse distance for distance attenuation
    lightDir = normalize(lightDir);
	varDiffuseColor = max(0.0, dot(normal, lightDir));
	
#if defined(DISTANCE_ATTENUATION)
    varDiffuseColor *= attenuation;
#endif

    // Blinn-phong reflection
    vec3 E = normalize(-eyeCoordsPosition);
    vec3 H = normalize(lightDir + E);
    float nDotHV = max(0.0, dot(normal, H));
    
    /*
        Phong Reflection
        vec3 E = normalize(-eyeCoordsPosition);
        vec3 L = lightDir;
        vec3 R = reflect(-L, normal);
        float nDotHV = max(0.0, dot(E, R));
    */
    
    varSpecularColor = pow(nDotHV, materialSpecularShininess);
#if defined(DISTANCE_ATTENUATION)
    varSpecularColor *= attenuation;
#endif
	
#endif

#if defined(PIXEL_LIT)
	vec3 n = normalize (worldViewInvTransposeMatrix * inNormal);
	vec3 t = normalize (worldViewInvTransposeMatrix * inTangent);
	vec3 b = -cross (n, t);

    vec3 eyeCoordsPosition = vec3(worldViewMatrix *  inPosition);
    
    vec3 lightDir = lightPosition[0] - eyeCoordsPosition;
    varPerPixelAttenuation = length(lightDir);
    lightDir = normalize(lightDir);
    
	// transform light and half angle vectors by tangent basis
	vec3 v;
	v.x = dot (lightDir, t);
	v.y = dot (lightDir, b);
	v.z = dot (lightDir, n);
	varLightVec = normalize (v);

    // eyeCoordsPosition = -eyeCoordsPosition;
	// v.x = dot (eyeCoordsPosition, t);
	// v.y = dot (eyeCoordsPosition, b);
	// v.z = dot (eyeCoordsPosition, n);
	// varEyeVec = normalize (v);

    vec3 E = normalize(-eyeCoordsPosition);

	/* Normalize the halfVector to pass it to the fragment shader */

	// No need to divide by two, the result is normalized anyway.
	// vec3 halfVector = normalize((E + lightDir) / 2.0); 
	vec3 halfVector = normalize(E + lightDir);
	v.x = dot (halfVector, t);
	v.y = dot (halfVector, b);
	v.z = dot (halfVector, n);

	// No need to normalize, t,b,n and halfVector are normal vectors.
	//normalize (v);
	varHalfVec = v;
#endif

#if defined(VERTEX_FOG)
    const float LOG2 = 1.442695;
    #if defined(VERTEX_LIT) || defined(PIXEL_LIT)
        float fogFragCoord = length(eyeCoordsPosition);
    #else
        vec3 eyeCoordsPosition = vec3(worldViewMatrix * inPosition);
        float fogFragCoord = length(eyeCoordsPosition);
    #endif
    varFogFactor = exp2( -fogDensity * fogDensity * fogFragCoord * fogFragCoord *  LOG2);
    varFogFactor = clamp(varFogFactor, 0.0, 1.0);
#endif

#if defined(ATTRIBUTE_COLOR)
	varVertexColor = inColor;
#endif

    
    GRAPH_CUSTOM_VERTEX_CODE
//#if defined(ATTRIBUTE_TEX0)
//	TEX_COORD0_MODIFICATION_CODE
//	varTexCoord[0] = inTexCoord0 + 0.25 * globalTime;
//#endif
//
//#if defined(ATTRIBUTE_TEX1)
//	TEX_COORD1_MODIFICATION_CODE
//	#if defined(MATERIAL_LIGHTMAP)
//		varTexCoord[1] = uvScale * inTexCoord1 + uvOffset;
//	#else
//		varTexCoord[1] = inTexCoord1;
//	#endif
//#endif

#if defined(MATERIAL_LIGHTMAP_DEBUG)
	varLightmapSize = lightmapSize;
#endif
}
