#ifdef GL_ES
precision lowp float;
#endif

uniform vec4 shadowColor;

void main()
{
	gl_FragColor = shadowColor;
}
