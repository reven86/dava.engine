#ifdef GL_ES
// define default precision for float, vec, mat.
precision highp float;
#else
#define lowp
#define highp
#define mediump
#endif


attribute vec4 inPosition;
attribute vec2 inTexCoord0;

uniform mat4 worldViewProjMatrix;
//uniform lowp vec4 flatColor;
//varying lowp vec4 varColor;
varying mediump vec2 varTexCoord;

#if defined(VERTEX_FOG)
uniform mat4 worldViewMatrix;
uniform float fogDensity;
#endif

#if defined(VERTEX_FOG)
varying float varFogFactor;
#endif


void main()
{
	gl_Position = worldViewProjMatrix * inPosition;
//	varColor = flatColor * flatColor.a;
	varTexCoord = inTexCoord0;

#if defined(VERTEX_FOG)
    const float LOG2 = 1.442695;
    vec3 eyeCoordsPosition = vec3(worldViewMatrix * inPosition);
    float fogFragCoord = length(eyeCoordsPosition);
    varFogFactor = exp2( -fogDensity * fogDensity * fogFragCoord * fogFragCoord *  LOG2);
    varFogFactor = clamp(varFogFactor, 0.0, 1.0);
#endif



}
