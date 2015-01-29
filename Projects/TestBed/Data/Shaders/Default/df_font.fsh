#ifdef GL_ES
// define default precision for float, vec, mat.
precision highp float;
#else
#define lowp
#define highp
#define mediump
#endif

uniform sampler2D texture0;
uniform float smoothing;
uniform mediump vec4 color;

varying mediump vec2 v_texCoord;

void main() {
    float distance = texture2D(texture0, v_texCoord).a;
    float alpha = smoothstep(0.5 - smoothing, 0.5 + smoothing, distance);
    alpha = min(alpha, color.a);
    gl_FragColor = vec4(color.rgb, alpha);
}