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
	if(gl_FragColor.a < 0.8)
	{
		gl_FragDepth = 1.0;
	}
	else
	{
		gl_FragDepth = 0.0;
	}
}
