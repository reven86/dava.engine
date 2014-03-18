<CONFIG>
albedo = 0
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
uniform float exposure;
uniform float brightMax;

varying vec2 varTexCoord0;

void main()
{
	vec4 textureColor0 = texture2D(albedo, varTexCoord0);

	float YD = exposure * (exposure/brightMax + 1.0) / (exposure + 1.0);
#ifdef POSTEFFECT_DARKEN	
	float Y = dot(vec4(0.30, 0.59, 0.11, 0.0), textureColor0);
	gl_FragColor = textureColor0*Y*YD;
#else
	gl_FragColor = textureColor0*YD;
#endif //POSTEFFECT_DARKEN
}
