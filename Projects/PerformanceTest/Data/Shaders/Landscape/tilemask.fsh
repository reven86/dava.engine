#ifdef GL_ES
precision highp float;
#else
#define lowp
#define highp
#define mediump
#endif

#ifdef DETAILMASK
uniform lowp vec3 tileColor0;
uniform lowp vec3 tileColor1;
uniform lowp vec3 tileColor2;
uniform lowp vec3 tileColor3;
#endif

uniform sampler2D tileTexture0;
#ifndef DETAILMASK
uniform sampler2D tileTexture1;
uniform sampler2D tileTexture2;
uniform sampler2D tileTexture3;
#endif
uniform sampler2D tileMask;
uniform sampler2D colorTexture;

varying mediump vec2 varTexCoordOrig;
varying mediump vec2 varTexCoord0;
#ifndef DETAILMASK
varying mediump vec2 varTexCoord1;
varying mediump vec2 varTexCoord2;
varying mediump vec2 varTexCoord3;
#endif

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
#ifndef DETAILMASK
    lowp vec3 color0 = texture2D(tileTexture0, varTexCoord0).rgb;
    lowp vec3 color1 = texture2D(tileTexture1, varTexCoord1).rgb;
    lowp vec3 color2 = texture2D(tileTexture2, varTexCoord2).rgb;
    lowp vec3 color3 = texture2D(tileTexture3, varTexCoord3).rgb;
#else
    lowp vec4 color0 = texture2D(tileTexture0, varTexCoord0).rgba;
#endif

    lowp vec4 mask = texture2D(tileMask, varTexCoordOrig);
    lowp vec4 lightMask = texture2D(colorTexture, varTexCoordOrig);

#ifndef DETAILMASK
    lowp vec3 color = (mask.r * color0 + mask.g * color1 + mask.b * color2 + mask.a * color3) * lightMask.rgb * 2.0;
#else
    lowp vec3 color = (color0.r*mask.r*tileColor0 + color0.g*mask.g*tileColor1 + color0.b*mask.b*tileColor2 + color0.a*mask.a*tileColor3) * lightMask.rgb * 2.0;
#endif
    
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
