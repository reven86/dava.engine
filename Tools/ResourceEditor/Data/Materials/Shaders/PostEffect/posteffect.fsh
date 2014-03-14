<CONFIG>
albedo = 0
cubemap = 2
decal = 1
detail = 1
lightmap = 1
normalmap = 1
<FRAGMENT_SHADER>


#ifdef GL_ES
// define default precision for float, vec, mat.
precision highp float;
#else
#define lowp
#define highp
#define mediump
#endif

uniform sampler2D albedo;

varying vec2 varTexCoord0;

const float exposure = 1.5;
const float brightMax = 2.0;

void main()
{
	vec4 textureColor0 = texture2D(albedo, varTexCoord0);

	float Y = dot(vec4(0.30, 0.59, 0.11, 0.0), textureColor0);
	float YD = exposure * (exposure/brightMax + 1.0) / (exposure + 1.0);

	gl_FragColor = textureColor0*Y*YD;
}
