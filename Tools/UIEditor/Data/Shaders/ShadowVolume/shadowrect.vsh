attribute vec4 inPosition;

uniform mat4 projectionMatrix;
uniform mat4 modelViewProjectionMatrix;

void main()
{
	gl_Position = projectionMatrix * inPosition;
}
