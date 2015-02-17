#ifdef GL_ES
precision highp float;
#endif

uniform sampler2D texture0;
uniform sampler2D texture1;
uniform sampler2D texture2;

varying vec2 varTexCoord0;

void main()
{
	vec4 colorOld = texture2D(texture0, varTexCoord0);
	vec4 colorNew = texture2D(texture1, varTexCoord0);
	vec4 stencil  = texture2D(texture2, varTexCoord0);

    vec3 outColor3 = mix(colorOld.rgb, colorNew.rgb, stencil.rgb);

	gl_FragColor = vec4(outColor3, colorOld.a);
}