#ifdef GL_ES
// define default precision for float, vec, mat.
precision highp float;
#else
#define lowp
#define highp
#define mediump
#endif

//#define TEXTURE0_SHIFT_ENABLED

// INPUT ATTRIBUTES
attribute vec4 inPosition;

#if defined(VERTEX_LIT) || defined(PIXEL_LIT)
attribute vec3 inNormal;
#endif 

#if defined(MATERIAL_SKYBOX)
attribute vec3 inTexCoord0;
#else
attribute vec2 inTexCoord0;
#endif

#if defined(MATERIAL_DECAL) || defined(MATERIAL_DETAIL) || defined(MATERIAL_LIGHTMAP)
attribute vec2 inTexCoord1;
#endif

#if defined(VERTEX_COLOR)
attribute vec4 inColor;
#endif

#if defined(VERTEX_LIT)
#endif

#if defined(PIXEL_LIT)
attribute vec3 inTangent;
#endif


// UNIFORMS
uniform mat4 modelViewProjectionMatrix;

#if defined(VERTEX_LIT) || defined(PIXEL_LIT) || defined(VERTEX_FOG)
uniform mat4 modelViewMatrix;
uniform mat3 normalMatrix;
uniform vec3 lightPosition0;
uniform float lightIntensity0; 
#endif

#if defined(VERTEX_LIT)
uniform float materialSpecularShininess;
#endif

#if defined(VERTEX_FOG)
uniform float fogDensity;
#endif

#if defined(MATERIAL_LIGHTMAP)
uniform mediump vec2 uvOffset;
uniform mediump vec2 uvScale;
#endif


// OUTPUT ATTRIBUTES
#if defined(MATERIAL_SKYBOX)
varying vec3 varTexCoord0;
#else
varying vec2 varTexCoord0;
#endif

#if defined(MATERIAL_DECAL) || defined(MATERIAL_DETAIL) || defined(MATERIAL_LIGHTMAP)
varying vec2 varTexCoord1;
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
varying float varFogFactor;
#endif

#if defined(SETUP_LIGHTMAP)
uniform float lightmapSize;
varying lowp float varLightmapSize;
#endif

#if defined(VERTEX_COLOR)
varying lowp vec4 varVertexColor;
#endif

#if defined(TEXTURE0_SHIFT_ENABLED)
uniform mediump vec2 texture0Shift;
#endif 


void main()
{
#if defined(MATERIAL_SKYBOX)
	vec4 vecPos = (modelViewProjectionMatrix * inPosition);
	gl_Position = vec4(vecPos.xy, vecPos.w - 0.0001, vecPos.w);
#else
	gl_Position = modelViewProjectionMatrix * inPosition;
#endif

#if defined(VERTEX_LIT)
    vec3 eyeCoordsPosition = vec3(modelViewMatrix * inPosition);
    vec3 normal = normalize(normalMatrix * inNormal); // normal in eye coordinates
    vec3 lightDir = lightPosition0 - eyeCoordsPosition;
    float attenuation = length(lightDir);
    attenuation = lightIntensity0 / (attenuation * attenuation); // use inverse distance for distance attenuation
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
	vec3 n = normalize (normalMatrix * inNormal);
	vec3 t = normalize (normalMatrix * inTangent);
	vec3 b = -cross (n, t);

    vec3 eyeCoordsPosition = vec3(modelViewMatrix *  inPosition);
    
    vec3 lightDir = lightPosition0 - eyeCoordsPosition;
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
        vec3 eyeCoordsPosition = vec3(modelViewMatrix * inPosition);
        float fogFragCoord = length(eyeCoordsPosition);
    #endif
    varFogFactor = exp2( -fogDensity * fogDensity * fogFragCoord * fogFragCoord *  LOG2);
    varFogFactor = clamp(varFogFactor, 0.0, 1.0);
#endif

#if defined(VERTEX_COLOR)
	varVertexColor = inColor;
#endif

	varTexCoord0 = inTexCoord0;
	
#if defined(TEXTURE0_SHIFT_ENABLED)
	varTexCoord0 += texture0Shift;
#endif
		
#if defined(MATERIAL_DECAL) || defined(MATERIAL_DETAIL) || defined(MATERIAL_LIGHTMAP)
	
	#if defined(SETUP_LIGHTMAP)
		varLightmapSize = lightmapSize;
		varTexCoord1 = inTexCoord1;
	#elif defined(MATERIAL_LIGHTMAP)
		varTexCoord1 = uvScale*inTexCoord1+uvOffset;
	#else defined(MATERIAL_DECAL)
		varTexCoord1 = inTexCoord1;
	#endif
#endif


}
