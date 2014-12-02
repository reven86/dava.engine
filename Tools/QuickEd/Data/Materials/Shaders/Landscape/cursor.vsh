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
uniform vec2 position;
uniform float scale;

varying lowp vec2 varTexCoord0;

void main()
{
	gl_Position = worldViewProjMatrix * inPosition;

	varTexCoord0 = inTexCoord0*scale-position*scale;
}
