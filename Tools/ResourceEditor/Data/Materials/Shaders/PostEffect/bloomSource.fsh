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

varying vec2 varTexCoord0;

void main()
{
	vec4 textureColor0 = texture2D(albedo, varTexCoord0);

	gl_FragColor = textureColor0*3.0;
}
