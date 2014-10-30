<CONFIG>
uniform sampler2D tileTexture0 = 2;
uniform sampler2D tileMask = 1;
uniform sampler2D colorTexture = 0;
uniform sampler2D fullTiledTexture = 3;
uniform sampler2D specularMap = 6;
uniform samplerCube atmospheremap = 7;
<FRAGMENT_SHADER>
#ifdef GL_ES
precision highp float;
#else
#define lowp
#define highp
#define mediump
#endif

#ifdef SPECULAR
uniform sampler2D specularMap;
uniform float inGlossiness;

varying vec3 varSpecularColor;
varying float varNdotH;
#endif

#ifdef TILEMASK
uniform lowp vec3 tileColor0;
uniform lowp vec3 tileColor1;
uniform lowp vec3 tileColor2;
uniform lowp vec3 tileColor3;

uniform sampler2D tileTexture0;
uniform sampler2D tileMask;
uniform sampler2D colorTexture;

varying mediump vec2 varTexCoord0;
#else
uniform sampler2D fullTiledTexture;
#endif

varying mediump vec2 varTexCoordOrig;

#ifdef EDITOR_CURSOR
varying vec2 varTexCoordCursor;
uniform sampler2D cursorTexture;
#endif

#if defined(VERTEX_FOG)
varying lowp float varFogAmoung;
varying lowp vec3 varFogColor;
#endif

void main()
{
#ifdef TILEMASK
    lowp vec4 color0 = texture2D(tileTexture0, varTexCoord0).rgba;
    lowp vec4 mask = texture2D(tileMask, varTexCoordOrig);
    lowp vec4 lightMask = texture2D(colorTexture, varTexCoordOrig);

    lowp vec3 color = (color0.r*mask.r*tileColor0 + color0.g*mask.g*tileColor1 + color0.b*mask.b*tileColor2 + color0.a*mask.a*tileColor3) * lightMask.rgb * 2.0;
#else
	lowp vec3 color = texture2D(fullTiledTexture, varTexCoordOrig).rgb;
#endif
    
#ifdef EDITOR_CURSOR
	vec4 colorCursor = texture2D(cursorTexture, varTexCoordCursor);
	color *= 1.0-colorCursor.a;
	color += colorCursor.rgb*colorCursor.a;
#endif
	
#ifdef SPECULAR
	float glossiness = pow(5000.0, inGlossiness * lightMask.a);
    float specularNorm = (glossiness + 2.0) / 8.0;
    color += varSpecularColor * pow(varNdotH, glossiness) * specularNorm;
#endif
    //color = vec3(1.0);

#if defined(VERTEX_FOG)
    gl_FragColor = vec4(mix(color, varFogColor, varFogAmoung), 1.0);
#else
    gl_FragColor = vec4(color, 1.0);
#endif
    //gl_FragColor = vec4(varSpecularColor, 1.0);
}
