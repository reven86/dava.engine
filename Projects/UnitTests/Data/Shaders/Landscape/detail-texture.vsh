attribute vec4 inPosition;
attribute vec2 inTexCoord0;

uniform mat4 modelViewProjectionMatrix;
uniform vec3 cameraPosition;
varying vec2 varTexCoord;
varying vec2 varDetailCoord;

void main()
{
	gl_Position = modelViewProjectionMatrix * inPosition;
	varTexCoord = inTexCoord0;
	varDetailCoord = inTexCoord0 * 60.0;
}