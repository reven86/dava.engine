#ifdef GL_ES
// define default precision for float, vec, mat.
precision highp float;
#else
#define lowp
#define highp
#define mediump
#endif

attribute vec4 inPosition;

uniform mat4 worldViewProjMatrix;
uniform lowp vec4 flatColor;
varying lowp vec4 varColor;

void main()
{
	gl_Position = worldViewProjMatrix * inPosition;
	varColor = flatColor;
}
