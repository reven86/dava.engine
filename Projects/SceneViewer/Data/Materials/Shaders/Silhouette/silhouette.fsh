#ifdef GL_ES
precision lowp float;
#endif

uniform vec4 silhouetteColor;

void main()
{
	gl_FragColor = silhouetteColor;
}
