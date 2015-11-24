#define LIGHT_NUM 1
#define IN_SPECULAR_SHININESS 5.0

#ifdef GL_ES
// define default precision for float, vec, mat.
precision highp float;
#else
#define lowp
#define highp
#define mediump
#endif

uniform sampler2D texture[NUM_TEXTURES];

#if defined(PIXEL_LIT)
uniform sampler2D normalMapTexture;
uniform mediump float materialSpecularShininess;
uniform mediump float lightIntensity[LIGHT_NUM];
#endif

#if defined(VERTEX_LIT) || defined(PIXEL_LIT)
uniform mediump vec3 materialLightAmbientColor;     // engine pass premultiplied material * light ambient color
uniform mediump vec3 materialLightDiffuseColor;     // engine pass premultiplied material * light diffuse color
uniform mediump vec3 materialLightSpecularColor;    // engine pass premultiplied material * light specular color
#endif

#if defined(VERTEX_LIT)
varying lowp float varDiffuseColor;
varying lowp float varSpecularColor;
#endif 

#if defined(PIXEL_LIT)
varying mediump vec3 varLightVec;
varying mediump vec3 varHalfVec;
varying mediump vec3 varEyeVec;
varying mediump float varPerPixelAttenuation;
#endif

#if defined(VERTEX_FOG)
uniform mediump vec3 fogColor;
varying mediump float varFogFactor;
#endif

#if defined(SETUP_LIGHTMAP)
varying lowp float varLightmapSize;
#endif

#if defined(VERTEX_COLOR)
varying lowp vec4 varVertexColor;
#endif

ADDITIONAL_UNIFORMS
ADDITIONAL_VARYINGS

void main()
{
    /*
        vec4 emissive;  #define HAS_EMISSIVE
        vec4 diffuse;   #define HAS_DIFFUSE
        vec4 specular;  #define HAS_SPECULAR
        vec3 normal;    #define HAS_NORMAL
    */
    
    // FETCH PHASE
    GRAPH_CUSTOM_PIXEL_CODE
    
#ifdef GRAPH_OPAQUE_ENABLED
    float opaque = GRAPH_OPAQUE;
    if (alpha < 0.5)discard;
#endif
    
    // DRAW PHASE
    vec3 color = vec3(0);
#ifdef IN_NORMAL
	// lookup normal from normal map, move from [0, 1] to  [-1, 1] range, normalize
    vec3 normal = normalize(2.0 * IN_NORMAL.rgb - 1.0);//2.0 * texture2D (normalMapTexture, varTexCoord0).rgb - 1.0;

    // compute diffuse lighting
    float finalAtt = 1.0 / (1.0 + 0.2 * varPerPixelAttenuation);
    float lambertFactor = max (dot (varLightVec, normal), 0.0) ;
    
    // compute ambient
#ifdef IN_EMISSIVE
    color += IN_EMISSIVE;
#endif
#ifdef IN_DIFFUSE
    color += IN_DIFFUSE * lambertFactor;
#endif
    
#ifdef IN_SPECULAR && IN_SPECULAR_SHININESS
    float shininess = pow (max (dot (varHalfVec, normal), 0.0), IN_SPECULAR_SHININESS) * finalAtt;
    color += IN_SPECULAR * shininess;
#endif

#endif
    
#ifdef IN_EMISSIVE
    color += IN_EMISSIVE;
#endif
#ifdef VERTEX_LIT
    #ifdef IN_DIFFUSE
        color += IN_DIFFUSE * varDiffuseColor;
    #endif
    #ifdef IN_SPECULAR
        color += IN_SPECULAR * varSpecularColor;
    #endif
#endif
    
	gl_FragColor =
#ifdef COLOR_VEC4
	color.rgba;
#else
	vec4(color.rgb, 1.0);
#endif
}
