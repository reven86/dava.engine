
#define NUM_TEXTURES 1
#define NUM_TEX_COORDS 1

#ifdef GL_ES
// define default precision for float, vec, mat.
precision highp float;
#else
#define lowp
#define highp
#define mediump
#endif

uniform sampler2D texture[NUM_TEXTURES];
varying mediump vec2 varTexCoord[NUM_TEX_COORDS];

#if defined(PIXEL_LIT)
uniform sampler2D normalMapTexture;
uniform float materialSpecularShininess;
uniform float lightIntensity0; 
#endif

#if defined(VERTEX_LIT) || defined(PIXEL_LIT)
uniform vec3 materialLightAmbientColor;     // engine pass premultiplied material * light ambient color
uniform vec3 materialLightDiffuseColor;     // engine pass premultiplied material * light diffuse color
uniform vec3 materialLightSpecularColor;    // engine pass premultiplied material * light specular color
#endif

#if defined(VERTEX_LIT)
varying lowp float varDiffuseColor;
varying lowp float varSpecularColor;
#endif 

#if defined(PIXEL_LIT)
varying vec3 varLightVec;
varying vec3 varHalfVec;
varying vec3 varEyeVec;
varying float varPerPixelAttenuation;
#endif

#if defined(VERTEX_FOG)
uniform vec3 fogColor;
varying float varFogFactor;
#endif

#if defined(SETUP_LIGHTMAP)
varying lowp float varLightmapSize;
#endif

#if defined(VERTEX_COLOR)
varying lowp vec4 varVertexColor;
#endif

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
#ifdef HAS_NORMAL
	// lookup normal from normal map, move from [0, 1] to  [-1, 1] range, normalize
    vec3 normal = normalize(2.0 * IN_NORMAL_TEXTURE.rgb - 1.0);//2.0 * texture2D (normalMapTexture, varTexCoord0).rgb - 1.0;

    // compute diffuse lighting
    float finalAtt = lightIntensity0 / (varPerPixelAttenuation * varPerPixelAttenuation);
    float lambertFactor = max (dot (varLightVec, normal), 0.0) * finalAtt;
    
    // compute ambient
    vec4 color = IN_EMISSIVE + IN_DIFFUSE * lambertFactor;
    
	#ifdef HAS_SPECULAR && HAS_SPECULAR_SHININESS
    float shininess = pow (max (dot (varHalfVec, normal), 0.0), IN_SPECULAR_SHININESS) * finalAtt;
    color += IN_SPECULAR * shininess;
	#endif

#endif
//defined(VERTEX_LIT)  // No normal use vertex lighting
	vec3 color = IN_EMISSIVE;// + IN_DIFFUSE + IN_SPECULAR;
//#endif
	
	gl_FragColor =
#ifdef COLOR_VEC4
	color.rgba;
#else
	vec4(color.rgb, 1.0);
#endif
}
