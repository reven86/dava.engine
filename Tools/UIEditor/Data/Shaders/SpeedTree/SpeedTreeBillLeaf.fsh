#ifdef GL_ES
// define default precision for float, vec, mat.
precision lowp float;
#else
#define lowp
#define highp
#define mediump
#endif

uniform sampler2D texture0;
varying mediump vec2 varTexCoord0;

void main()
{
    lowp vec4 textureColor0 = texture2D(texture0, varTexCoord0);

    if(textureColor0.a < 0.5) discard;

	gl_FragColor = textureColor0;
}