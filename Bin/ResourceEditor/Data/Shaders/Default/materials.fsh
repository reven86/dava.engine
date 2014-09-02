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

#if defined(MATERIAL_DECAL) || defined(MATERIAL_DETAIL) 
uniform sampler2D texture1;
varying mediump vec2 varTexCoord1;
#endif 

#if defined(VERTEX_LIT)
varying lowp vec4 varDiffuseColor;
#endif 

void main()
{
    // FETCH PHASE
    lowp vec4 textureColor0 = texture2D(texture0, varTexCoord0);
    
#if defined(MATERIAL_DECAL) || defined(MATERIAL_DETAIL)
    lowp vec4 textureColor1 = texture2D(texture1, varTexCoord1);
#endif

    // DRAW PHASE
#if defined(VERTEX_LIT)
    gl_FragColor = varDiffuseColor * textureColor0;
#else
    #if defined(MATERIAL_TEXTURE)
    	gl_FragColor = textureColor0;
    #elif defined(MATERIAL_DECAL)
    	gl_FragColor = textureColor0 * textureColor1;
    #elif defined(MATERIAL_DETAIL)
        gl_FragColor = textureColor0 * textureColor1 * 2.0;
    #endif 
#endif
}
