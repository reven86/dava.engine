attribute vec4 inPosition;
attribute vec2 inTexCoord0;

uniform mat4 worldViewProjMatrix;
uniform vec3 cameraPosition;
uniform vec2 texture0Tiling;
uniform vec2 texture1Tiling;

varying vec4 varColor;
varying vec2 varTexCoordOrig;
varying vec2 varTexCoord0;
varying vec2 varTexCoord1;

void main()
{
	gl_Position = worldViewProjMatrix * inPosition;
	varTexCoordOrig = inTexCoord0;
	varTexCoord0 = inTexCoord0 * texture0Tiling;
	varTexCoord1 = inTexCoord0 * texture1Tiling;
}
