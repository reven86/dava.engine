#ifdef GL_ES
// define default precision for float, vec, mat.
precision highp float;
#else
#define lowp
#define highp
#define mediump
#endif

attribute vec4 inPosition;
attribute vec3 inNormal;

uniform mat4 worldViewProjMatrix;
uniform mat4 worldViewMatrix;
uniform mat4 projMatrix;
uniform mat3 worldViewInvTransposeMatrix;

uniform mediump float silhouetteScale;
uniform mediump float silhouetteExponent;

void main()
{
	vec3 normal = normalize(worldViewInvTransposeMatrix * inNormal.xyz);
	vec4 PosView = worldViewMatrix * inPosition;

	mediump float distanceScale = length(PosView.xyz) / 100.0;

	PosView.xyz += normal * pow(silhouetteScale * distanceScale, silhouetteExponent);
	gl_Position = projMatrix * PosView;
}