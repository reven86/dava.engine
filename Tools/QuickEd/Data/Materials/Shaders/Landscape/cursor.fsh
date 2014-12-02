#ifdef GL_ES
precision highp float;
#else
#define lowp
#define highp
#define mediump
#endif

uniform sampler2D texture0;

varying lowp vec2 varTexCoord0;

void main()
{
	gl_FragColor = texture2D(texture0, varTexCoord0);
}
