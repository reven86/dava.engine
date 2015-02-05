#ifdef GL_ES
// define default precision for float, vec, mat.
precision highp float;
#else
#define lowp
#define highp
#define mediump
#endif

varying lowp vec4 varColor;

void main()
{
	gl_FragColor = varColor;
}
