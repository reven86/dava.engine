#ifdef GL_ES
// define default precision for float, vec, mat.
precision highp float;
#endif

uniform sampler2D texture0;
uniform sampler2D texture1;
uniform sampler2D textureMask;

varying vec2 varTexCoordOrig;
varying vec2 varTexCoord0;
varying vec2 varTexCoord1;
varying vec3 varCameraToVertex;


void main()
{
    vec4 color0 = texture2D(texture0, varTexCoord0);
    vec4 color1 = texture2D(texture1, varTexCoord1);
    vec4 mask = texture2D(textureMask, varTexCoordOrig);

    vec3 color = (mask.a * color0.rgb + (1.0 - mask.a) * color1.rgb);
    gl_FragColor = vec4(mask.rgb * color.rgb, 1.0);
    
    //klesch
//    vec3 color = (mask.r * color0.rgb + mask.g * color1.rgb);
//    gl_FragColor = vec4(color, 1.0); //color0 * mask.r + color1 * (1.0 - mask.r);

//  vec4 detailColor = texture2D(detailTexture, varDetailCoord);
//  vec4 color = texture2D(texture, varTexCoord);
//	gl_FragColor = color * detailColor * 2.0;
}
