#ifdef GL_ES
precision highp float;
#else
#define lowp
#define highp
#define mediump
#endif

attribute vec4 inPosition;
attribute vec2 inTexCoord0;

uniform mat4 worldViewProjMatrix;
uniform mediump vec2 texture0Tiling;

varying mediump vec2 varTexCoordOrig;
varying mediump vec2 varTexCoord0;

#if defined(VERTEX_FOG) || defined(VERTEX_LIT) || defined(PIXEL_LIT)
uniform mat4 worldViewMatrix;
#endif

#if defined(VERTEX_FOG)
    uniform float fogLimit;
    #if !defined(FOG_LINEAR)
    uniform float fogDensity;
    #else
    uniform float fogStart;
    uniform float fogEnd;
    #endif
#endif

#if defined(VERTEX_FOG)
varying float varFogFactor;
#endif

#ifdef EDITOR_CURSOR
varying vec2 varTexCoordCursor;
#endif

#if defined(VERTEX_LIT) || defined(PIXEL_LIT)
uniform mat3 worldViewInvTransposeMatrix;
attribute vec3 inNormal;
attribute vec3 inTangent;

uniform vec4 lightPosition0;
uniform vec3 lightAmbientColor0;
uniform vec3 lightColor0;
uniform mat3 normalMatrix;

uniform float inSpecularity;
uniform float physicalFresnelReflectance;
uniform vec3 metalFresnelReflectance;

varying vec3 varSpecularColor;
varying float varNdotH;

const float _PI = 3.141592654;

float FresnelShlick(float NdotL, float Cspec)
{
	float fresnel_exponent = 5.0;
	return Cspec + (1.0 - Cspec) * pow(1.0 - NdotL, fresnel_exponent);
}

vec3 FresnelShlickVec3(float NdotL, vec3 Cspec)
{
	float fresnel_exponent = 5.0;
	return Cspec + (1.0 - Cspec) * (pow(1.0 - NdotL, fresnel_exponent));
}
#endif

#if defined(PIXEL_LIT)
varying vec3 varLightPosition;
varying vec3 varToLightVec;
varying vec3 varHalfVec;
varying vec3 varToCameraVec;
varying float varPerPixelAttenuation;
#endif

#if defined(LANDSCAPE_INSTANCING)
//uniform sampler2D heightmapTexture;
//uniform vec4 chunkBounds;
//uniform vec2 landscapeBoundingBoxZMinHeight;
#endif

void main()
{
#if defined(LANDSCAPE_INSTANCING)
    //vec2 position = vec2(chunkBounds.x + chunkBounds.z * inPosition.x, chunkBounds.y + chunkBounds.z * inPosition.y);
    //float height = 0.0; //texture2D(heightmapTexture, position).a;
    //gl_Position = worldViewProjMatrix * vec4(position.xy, landscapeBoundingBoxZMinHeight.x + height * landscapeBoundingBoxZMinHeight.y, 1.0);
    gl_Position = worldViewProjMatrix * (inPosition * vec4(20.0));//vec4(position.xy, 0.0, 1.0);

#else
	gl_Position = worldViewProjMatrix * inPosition;
#endif
    
	varTexCoordOrig = inTexCoord0;

	varTexCoord0 = inTexCoord0 * texture0Tiling;
	
#if defined(SPECULAR) || defined(VERTEX_FOG)
	vec3 eyeCoordsPosition = vec3(worldViewMatrix * inPosition);
#endif
	
#if defined(VERTEX_LIT)
    vec3 normal = normalize(worldViewInvTransposeMatrix * inNormal); // normal in eye coordinates
    vec3 toLightDir = lightPosition0.xyz - eyeCoordsPosition * lightPosition0.w;
    toLightDir = normalize(toLightDir);
    
    vec3 toCameraNormalized = normalize(-eyeCoordsPosition);
    vec3 H = normalize(toLightDir + toCameraNormalized);
    
    float NdotL = max (dot (normal, toLightDir), 0.0);
    float NdotH = max (dot (normal, H), 0.0);
    float LdotH = max (dot (toLightDir, H), 0.0);
    float NdotV = max (dot (normal, toCameraNormalized), 0.0);
    
    //vec3 fresnelIn = FresnelShlickVec3(NdotL, metalFresnelReflectance);
    vec3 fresnelOut = FresnelShlickVec3(NdotV, metalFresnelReflectance);
    float specularity = inSpecularity;
    
	//varDiffuseColor = NdotL / _PI;
    
    float Dbp = NdotL;
    float Geo = 1.0 / LdotH * LdotH;
    
    varSpecularColor = Dbp * Geo * fresnelOut * specularity * lightColor0;
    varNdotH = NdotH;
#endif

#if defined(PIXEL_LIT)
	vec3 n = normalize (worldViewInvTransposeMatrix * inNormal);
	vec3 t = normalize (worldViewInvTransposeMatrix * inTangent);
	vec3 b = -cross(n, t); //normalize (worldViewInvTransposeMatrix * inBinormal);
    
    vec3 toLightDir = lightPosition0.xyz - eyeCoordsPosition * lightPosition0.w;
#if defined(DISTANCE_ATTENUATION)
    varPerPixelAttenuation = length(toLightDir);
#endif
    //lightDir = normalize(lightDir);
    
	// transform light and half angle vectors by tangent basis
	vec3 v;
	v.x = dot (toLightDir, t);
	v.y = dot (toLightDir, b);
	v.z = dot (toLightDir, n);
    
#if !defined(FAST_NORMALIZATION)
	varToLightVec = v;
#else
    varToLightVec = normalize(v);
#endif
    
    vec3 toCameraDir = -eyeCoordsPosition;
    
    v.x = dot (toCameraDir, t);
	v.y = dot (toCameraDir, b);
	v.z = dot (toCameraDir, n);
#if !defined(FAST_NORMALIZATION)
	varToCameraVec = v;
#else
    varToCameraVec = normalize(v);
#endif
    
    /* Normalize the halfVector to pass it to the fragment shader */
	// No need to divide by two, the result is normalized anyway.
	// vec3 halfVector = normalize((E + lightDir) / 2.0);
#if defined(FAST_NORMALIZATION)
	vec3 halfVector = normalize(normalize(toCameraDir) + normalize(toLightDir));
	v.x = dot (halfVector, t);
	v.y = dot (halfVector, b);
	v.z = dot (halfVector, n);
    
	// No need to normalize, t,b,n and halfVector are normal vectors.
	varHalfVec = v;
#endif
    
#endif

    
#if defined(VERTEX_FOG)
    float fogFragCoord = length(eyeCoordsPosition);
    #if !defined(FOG_LINEAR)
        const float LOG2 = 1.442695;
        varFogFactor = exp2( -fogDensity * fogDensity * fogFragCoord * fogFragCoord *  LOG2);
        varFogFactor = clamp(varFogFactor, 1.0 - fogLimit, 1.0);
    #else
        varFogFactor = 1.0 - clamp((fogFragCoord - fogStart) / (fogEnd - fogStart), 0.0, fogLimit);
    #endif
#endif
	
#ifdef EDITOR_CURSOR
	varTexCoordCursor = inTexCoord0;
#endif
}
