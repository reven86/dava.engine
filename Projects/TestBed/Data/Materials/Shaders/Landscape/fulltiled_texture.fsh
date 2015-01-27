#ifdef GL_ES
// define default precision for float, vec, mat.
precision highp float;
#else
#define lowp
#define highp
#define mediump
#endif

uniform sampler2D sampler2d;
varying lowp vec4 varColor;
varying mediump vec2 varTexCoord;

#if defined(VERTEX_FOG)
uniform vec3 fogColor;
varying float varFogFactor;
#endif


void main()
{
    lowp vec3 texColor = texture2D(sampler2d, varTexCoord).rgb;


#ifdef ALPHA_TEST_ENABLED
    if (texColor.a < 0.9)
        discard;
#endif

#if defined(VERTEX_FOG)
    gl_FragColor = vec4(mix(fogColor, texColor, varFogFactor), 1.0);
#else
    gl_FragColor = vec4(texColor, 1.0);
#endif

//	gl_FragColor = texColor;
}
