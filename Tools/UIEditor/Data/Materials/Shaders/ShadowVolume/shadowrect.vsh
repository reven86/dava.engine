attribute vec4 inPosition;

uniform mat4 projMatrix;
uniform mat4 worldViewProjMatrix;

void main()
{
	gl_Position = projMatrix * inPosition;
}
