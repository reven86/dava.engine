attribute vec4 inPosition;
attribute vec2 inTexCoord;

uniform mat4 worldViewProjMatrix;
uniform vec4 flatColor;
varying vec4 varColor;
varying vec2 varTexCoord;

void main()
{
	gl_Position = worldViewProjMatrix * inPosition;
	varColor = flatColor * flatColor.a;
	varTexCoord = inTexCoord;
}
