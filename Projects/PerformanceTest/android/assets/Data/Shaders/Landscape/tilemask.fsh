#ifdef GL_ES
precision highp float;
#else
#define lowp
#define highp
#define mediump
#endif

uniform sampler2D tileTexture0;
uniform sampler2D tileTexture1;
uniform sampler2D tileTexture2;
uniform sampler2D tileTexture3;
uniform sampler2D tileMask;
uniform sampler2D colorTexture;

varying mediump vec2 varTexCoordOrig;
varying mediump vec2 varTexCoord0;
varying mediump vec2 varTexCoord1;
varying mediump vec2 varTexCoord2;
varying mediump vec2 varTexCoord3;

#ifdef EDITOR_CURSOR
varying vec2 varTexCoordCursor;
uniform sampler2D cursorTexture;
#endif

#if defined(VERTEX_FOG)
uniform vec3 fogColor;
varying float varFogFactor;
#endif

void main()
{
    lowp vec3 color0 = texture2D(tileTexture0, varTexCoord0).rgb;
    lowp vec3 color1 = texture2D(tileTexture1, varTexCoord1).rgb;
    lowp vec3 color2 = texture2D(tileTexture2, varTexCoord2).rgb;
    lowp vec3 color3 = texture2D(tileTexture3, varTexCoord3).rgb;
    
    lowp vec4 mask = texture2D(tileMask, varTexCoordOrig);
    lowp vec4 lightMask = texture2D(colorTexture, varTexCoordOrig);

    lowp vec3 color = (mask.r * color0 + mask.g * color1 + mask.b * color2 + mask.a * color3) * lightMask.rgb;
    
#ifdef EDITOR_CURSOR
	vec4 colorCursor = texture2D(cursorTexture, varTexCoordCursor);
	color *= 1.0-colorCursor.a;
	color += colorCursor.rgb*colorCursor.a;
#endif

#if defined(VERTEX_FOG)
    gl_FragColor = vec4(mix(fogColor, color, varFogFactor), 1.0);
#else
    gl_FragColor = vec4(color, 1.0);
#endif
}
