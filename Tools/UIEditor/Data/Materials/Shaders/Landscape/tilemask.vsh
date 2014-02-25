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
#ifndef DETAILMASK
uniform mediump vec2 texture1Tiling;
uniform mediump vec2 texture2Tiling;
uniform mediump vec2 texture3Tiling;
#endif

varying mediump vec2 varTexCoordOrig;
varying mediump vec2 varTexCoord0;

#ifndef DETAILMASK
varying mediump vec2 varTexCoord1;
varying mediump vec2 varTexCoord2;
varying mediump vec2 varTexCoord3;
#endif

#if defined(VERTEX_FOG) || defined(SPECULAR_LAND)
uniform mat4 worldViewMatrix;
#endif

#if defined(VERTEX_FOG)
uniform float fogDensity;
#endif

#if defined(VERTEX_FOG)
varying float varFogFactor;
#endif

#ifdef EDITOR_CURSOR
varying vec2 varTexCoordCursor;
#endif

#ifdef SPECULAR_LAND
attribute vec3 inNormal;

uniform vec3 lightPosition0;
uniform float lightIntensity0;
uniform mat3 normalMatrix;
uniform float materialSpecularShininess;

varying float varSpecularColor;
#endif

void main()
{
	gl_Position = worldViewProjMatrix * inPosition;

	varTexCoordOrig = inTexCoord0;

	varTexCoord0 = inTexCoord0 * texture0Tiling;
#ifndef DETAILMASK
	varTexCoord1 = inTexCoord0 * texture1Tiling;
	varTexCoord2 = inTexCoord0 * texture2Tiling;
	varTexCoord3 = inTexCoord0 * texture3Tiling;
#endif
	
#if defined(SPECULAR_LAND) || defined(VERTEX_FOG)
	vec3 eyeCoordsPosition = vec3(worldViewMatrix * inPosition);
#endif
	
#if defined(SPECULAR_LAND)
    vec3 normal = normalize(normalMatrix * inNormal); // normal in eye coordinates
    vec3 lightDir = normalize(lightPosition0 - eyeCoordsPosition);
    
    // Blinn-phong reflection
    vec3 E = normalize(-eyeCoordsPosition);
    vec3 H = normalize(lightDir + E);
    float nDotHV = max(0.0, dot(normal, H));
    
    varSpecularColor = pow(nDotHV, materialSpecularShininess);
#endif
    
#if defined(VERTEX_FOG)
    const float LOG2 = 1.442695;
    float fogFragCoord = length(eyeCoordsPosition);
    varFogFactor = exp2( -fogDensity * fogDensity * fogFragCoord * fogFragCoord *  LOG2);
    varFogFactor = clamp(varFogFactor, 0.0, 1.0);
#endif
	
#ifdef EDITOR_CURSOR
	varTexCoordCursor = inTexCoord0;
#endif
}
