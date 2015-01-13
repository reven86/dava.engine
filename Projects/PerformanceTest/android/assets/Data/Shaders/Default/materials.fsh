#ifdef GL_ES
// define default precision for float, vec, mat.
precision highp float;
#else
#define lowp
#define highp
#define mediump
#endif

// DECLARATIONS
uniform sampler2D texture0;
varying mediump vec2 varTexCoord0;

#if defined(MATERIAL_DECAL) || defined(MATERIAL_DETAIL) || defined(MATERIAL_LIGHTMAP)
uniform sampler2D texture1;
varying mediump vec2 varTexCoord1;
#endif 

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

void main()
{
    // FETCH PHASE
#if defined(GLOSS) || defined(OPAQUE)
    lowp vec4 textureColor0 = texture2D(texture0, varTexCoord0);
#else
    lowp vec3 textureColor0 = texture2D(texture0, varTexCoord0).rgb;
#endif
#if defined(OPAQUE)
    if (textureColor0.a < 0.5)discard;
#endif
    
#if defined(MATERIAL_DECAL) || defined(MATERIAL_DETAIL) || defined(MATERIAL_LIGHTMAP)
    lowp vec3 textureColor1 = texture2D(texture1, varTexCoord1).rgb;
#if defined(SETUP_LIGHTMAP)
	vec3 lightGray = vec3(0.75, 0.75, 0.75);
	vec3 darkGray = vec3(0.25, 0.25, 0.25);
	bool isXodd;
	bool isYodd;
	if(fract(floor(varTexCoord1.x*varLightmapSize)/2.0) == 0.0)
	{
		isXodd = true;
	}
	else
	{
		isXodd = false;
	}
	if(fract(floor(varTexCoord1.y*varLightmapSize)/2.0) == 0.0)
	{
		isYodd = true;
	}
	else
	{
		isYodd = false;
	}
	
	if((isXodd && isYodd) || (!isXodd && !isYodd))
	{
		textureColor1 = lightGray;
	}
	else
	{
		textureColor1 = darkGray;
	}
#endif
#endif

    // DRAW PHASE
#if defined(VERTEX_LIT)
	vec3 color = (materialLightAmbientColor + varDiffuseColor * materialLightDiffuseColor) * textureColor0.rgb + varSpecularColor * materialLightSpecularColor;
#elif defined(PIXEL_LIT)
	// lookup normal from normal map, move from [0, 1] to  [-1, 1] range, normalize
    vec3 normal = 2.0 * texture2D (normalMapTexture, varTexCoord0).rgb - 1.0;
    normal = normalize (normal);

    
    float finalAtt = lightIntensity0 / (varPerPixelAttenuation * varPerPixelAttenuation);

    // compute diffuse lighting
    float lambertFactor = max (dot (varLightVec, normal), 0.0) * finalAtt;

    // compute ambient
    vec3 color = materialLightAmbientColor + materialLightDiffuseColor * lambertFactor;	
	color *= textureColor0.rgb;

#if defined(SPECULAR)
	//if (lambertFactor > 0.0)
	{
		// In doom3, specular value comes from a texture 
		float shininess = pow (max (dot (varHalfVec, normal), 0.0), materialSpecularShininess) * finalAtt;
		#if defined(GLOSS)
    		color += materialLightSpecularColor * (shininess * textureColor0.a);
    	#else 
		    color += materialLightSpecularColor * shininess;
    	#endif
	}
#endif
	
#elif defined(MATERIAL_TEXTURE)
    vec3 color = textureColor0.rgb;
#elif defined(MATERIAL_DECAL) || defined(MATERIAL_LIGHTMAP)
    vec3 color = textureColor0.rgb * textureColor1.rgb;
#elif defined(MATERIAL_DETAIL)
    vec3 color = textureColor0.rgb * textureColor1.rgb * 2.0;
#endif

#if defined(VERTEX_FOG)
    gl_FragColor = vec4(mix(fogColor, color, varFogFactor), 1.0);
#else
    gl_FragColor = vec4(color, 1.0);
#endif
}
