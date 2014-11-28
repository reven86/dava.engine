#ifdef GL_ES
// define default precision for float, vec, mat.
precision highp float;
#else
#define lowp
#define highp
#define mediump
#endif

uniform sampler2D sampler2d;
uniform lowp vec4 flatColor;
varying mediump vec2 varTexCoord;

void main()
{
    lowp vec4 texColor = texture2D(sampler2d, varTexCoord);
#ifdef ALPHA_TEST_ENABLED
    if (texColor.a < 0.9)
        discard;
#endif

#ifdef IMAGE_A8
    gl_FragColor = vec4(flatColor.rgb, texColor.a * flatColor.a);
#else
    gl_FragColor = texColor * flatColor;
#endif
}
