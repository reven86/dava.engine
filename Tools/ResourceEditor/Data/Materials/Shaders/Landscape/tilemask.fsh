<CONFIG>
uniform sampler2D tileTexture0 = 2;
uniform sampler2D tileMask = 1;
uniform sampler2D colorTexture = 0;
uniform sampler2D normalmap01 = 3;
uniform sampler2D normalmap23 = 4;
uniform vec4 inGlossiness = vec4(0.5, 0.5, 0.5, 0.5);
uniform vec4 inSpecularity = vec4(1.0, 1.0, 1.0, 1.0);
uniform vec3 metalFresnelReflectance = vec3(0.5, 0.5, 0.5);
uniform float dielectricFresnelReflectance = 0.5;
uniform sampler2D fullTiledTexture = 5;
uniform sampler2D specularMap = 6;
uniform samplerCube atmospheremap = 7;
uniform float normalDiffuseIntensity = 0.3;

<FRAGMENT_SHADER>
#ifdef GL_ES
precision highp float;
#else
#define lowp
#define highp
#define mediump
#endif

const float _PI = 3.141592654;

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



#if defined(VERTEX_LIT) || defined(PIXEL_LIT)
uniform vec3 lightColor0;
uniform vec4 inGlossiness;

varying vec3 varSpecularColor;
varying float varNdotH;
#endif

#if defined(PIXEL_LIT)
uniform sampler2D normalmap01;
uniform sampler2D normalmap23;

uniform float normalDiffuseIntensity;

uniform float materialSpecularShininess;
uniform float lightIntensity0;
uniform vec4 inSpecularity;
uniform float physicalFresnelReflectance;
uniform vec3 metalFresnelReflectance;
#endif

#if defined(PIXEL_LIT)
varying vec3 varToLightVec;

    #if defined(FAST_NORMALIZATION)
    varying vec3 varHalfVec;
    #endif

varying vec3 varToCameraVec;
uniform vec4 lightPosition0;
varying float varPerPixelAttenuation;

float FresnelShlick(float NdotL, float Cspec)
{
    float expf = 5.0;
	return Cspec + (1.0 - Cspec) * pow(1.0 - NdotL, expf);
}

vec3 FresnelShlickVec3(float NdotL, vec3 Cspec)
{
    float expf = 5.0;
	return Cspec + (1.0 - Cspec) * (pow(1.0 - NdotL, expf));
}
#endif

//#define ENABLE_GAMMA_CORRECTION

#if defined(ENABLE_GAMMA_CORRECTION)

    #if defined(TRUE_GAMMA_CORRECTION)
        #define SRGB_TO_LINEAR(x) pow(x.rgb, vec3(2.2))
        #define LINEAR_TO_SRGB(x) pow(x.rgb, vec3(1.0 / 2.2))
    #else
        #define SRGB_TO_LINEAR(x) pow(x.rgb * x.rgb)
        #define LINEAR_TO_SRGB(x) pow(sqrt(x.rgb))
    #endif

#else
#define SRGB_TO_LINEAR(x) x
#define LINEAR_TO_SRGB(x) x
#endif


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
    color = SRGB_TO_LINEAR(color);
    
#if defined(PIXEL_LIT)
    float tileSpecularity = dot(mask, inSpecularity);
    float tileGlossiness = dot(mask, inGlossiness);
#endif
    
#else
	lowp vec3 color = texture2D(fullTiledTexture, varTexCoordOrig).rgb;
#endif
    
#ifdef EDITOR_CURSOR
	vec4 colorCursor = texture2D(cursorTexture, varTexCoordCursor);
	color *= 1.0-colorCursor.a;
	color += colorCursor.rgb*colorCursor.a;
#endif
	

#if defined(VERTEX_LIT) && defined(SPECULAR)
	float glossiness = pow(5000.0, tileGlossiness * lightMask.a);
    float specularNorm = (glossiness + 2.0) / 8.0;
    color += varSpecularColor * pow(varNdotH, glossiness) * specularNorm;
#endif
    
#if defined(PIXEL_LIT)
    vec4 normal01 = 2.0 * texture2D (normalmap01, varTexCoord0).rgba - 1.0;
    vec4 normal23 = 2.0 * texture2D (normalmap23, varTexCoord0).rgba - 1.0;
    vec3 normal;
    normal.rg = (normal01.rg * mask.r) + (normal01.ba * mask.g) + (normal23.rg * mask.b) + (normal23.ba * mask.a);
   	normal.z = sqrt(1.0 - (normal.x * normal.x + normal.y * normal.y));

    
    float attenuation = lightIntensity0;
#if defined(DISTANCE_ATTENUATION)
    attenuation /= (varPerPixelAttenuation * varPerPixelAttenuation);
#endif
    
#if !defined(FAST_NORMALIZATION)
    vec3 toLightNormalized = normalize(varToLightVec);
    vec3 toCameraNormalized = normalize(varToCameraVec);
    vec3 H = toCameraNormalized + toLightNormalized;
    H = normalize(H);
    
    // compute diffuse lighting
    float NdotL = max (dot (normal, toLightNormalized), 0.0);
    float NdotH = max (dot (normal, H), 0.0);
    float LdotH = max (dot (toLightNormalized, H), 0.0);
    float NdotV = max (dot (normal, toCameraNormalized), 0.0);
#else
    // Kwasi normalization :-)
    // compute diffuse lighting
    vec3 normalizedHalf = normalize(varHalfVec);
    
    float NdotL = max (dot (normal, varToLightVec), 0.0);
    float NdotH = max (dot (normal, normalizedHalf), 0.0);
    float LdotH = max (dot (varToLightVec, normalizedHalf), 0.0);
    float NdotV = max (dot (normal, varToCameraVec), 0.0);
#endif
    
#if defined(NORMALIZED_BLINN_PHONG)
    
#if defined(DIELECTRIC)
#define ColorType float
    float fresnelOut = FresnelShlick(NdotV, dielectricFresnelReflectance);
#else
#if defined(FAST_METAL)
#define ColorType float
    float fresnelOut = FresnelShlick(NdotV, (metalFresnelReflectance.r + metalFresnelReflectance.g + metalFresnelReflectance.b) / 3.0);
#else
#define ColorType vec3
    vec3 fresnelOut = FresnelShlickVec3(NdotV, metalFresnelReflectance);
#endif
#endif
    
    float specularity = tileSpecularity * lightMask.a;
    float glossiness = tileGlossiness;
    float glossPower = pow(5000.0, glossiness); //textureColor0.a;
    
   	//float glossiness = inGlossiness * 0.999;
	//glossiness = 200.0 * glossiness / (1.0 - glossiness);
    //#define GOTANDA
#if defined(GOTANDA)
    vec3 fresnelIn = FresnelShlickVec3(NdotL, metalFresnelReflectance);
	vec3 diffuse = NdotL / _PI * (1.0 - fresnelIn * specularity);
#else
	float diffuse = NdotL / _PI;// * (1.0 - fresnelIn * specularity);
#endif
    
#if defined(GOTANDA)
	float specCutoff = 1.0 - NdotL;
	specCutoff *= specCutoff;
	specCutoff *= specCutoff;
	specCutoff *= specCutoff;
	specCutoff = 1.0 - specCutoff;
#else
    float specCutoff = NdotL;
#endif
    
#if defined(GOTANDA)
    float specularNorm = (glossPower + 2.0) * (glossPower + 4.0) / (8.0 * _PI * (pow(2.0, -glossPower / 2.0) + glossPower));
#else
    float specularNorm = (glossPower + 2.0) / 8.0;
#endif
    float specularNormalized = specularNorm * pow(NdotH, glossPower) * specCutoff * specularity;
#if defined(FAST_METAL)
	float geometricFactor = 1.0;
#else
    float geometricFactor = 1.0 / LdotH * LdotH;
#endif
    ColorType specular = specularNormalized * geometricFactor * fresnelOut;
#endif
    
    color *= 0.8;
    color += vec3(NdotL) * lightColor0 * 0.3;
    color += vec3(specular) * lightColor0 * lightMask.a;
    //color = vec3(lightMask.a);
    //color = ;
    //color = vec3(normal * 0.5 + 0.5);
#endif // PIXEL_LIT

#if defined(VERTEX_FOG)
    color = mix(color, varFogColor, varFogAmoung);
    color = LINEAR_TO_SRGB(color);
    gl_FragColor = vec4(color, 1.0);
#else
    color = LINEAR_TO_SRGB(color);
    gl_FragColor = vec4(color, 1.0);
#endif
    //gl_FragColor = vec4(1.0, 0.0, 0.0, 1.0);
}
