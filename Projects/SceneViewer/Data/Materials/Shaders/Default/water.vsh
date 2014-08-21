#ifdef GL_ES
// define default precision for float, vec, mat.
precision highp float;
#else
#define lowp
#define highp
#define mediump
#endif



// INPUT ATTRIBUTES
attribute vec4 inPosition;
attribute vec3 inNormal;


#if defined(PIXEL_LIT)
attribute vec3 inTangent;
#endif

attribute vec2 inTexCoord0;

#if defined(MATERIAL_DECAL)
attribute vec2 inTexCoord1;
varying vec2 varTexCoordDecal;
#endif

// UNIFORMS
uniform mat4 worldViewProjMatrix;


uniform mat4 worldViewMatrix;


#if defined(PIXEL_LIT)
uniform mat3 worldViewInvTransposeMatrix;

#if defined(SPECULAR)
uniform vec4 lightPosition0;
#endif

#endif

#if !defined (TANGENT_SPACE_WATER_REFLECTIONS)
varying vec3 eyeDist;
#endif


#if defined(VERTEX_FOG)
    uniform float fogLimit;
    #if !defined(FOG_LINEAR)
    uniform float fogDensity;
    #else
    uniform float fogStart;
    uniform float fogEnd;
    #endif
#endif

#if defined(PIXEL_LIT)
#if defined(SPECULAR)
varying vec3 varLightVec;
#endif
#endif

#if defined(PIXEL_LIT)||defined(MATERIAL_DECAL)	
varying highp vec2 varTexCoord0;
varying highp vec2 varTexCoord1;
uniform mediump vec2 normal0ShiftPerSecond;
uniform mediump vec2 normal1ShiftPerSecond;
uniform mediump float normal0Scale;
uniform mediump float normal1Scale;
uniform float globalTime;
#endif




#if defined(VERTEX_FOG)
varying float varFogFactor;
#endif

uniform vec3 cameraPosition;
uniform mat4 worldMatrix;
uniform mat3 worldInvTransposeMatrix;





#if defined(VERTEX_LIT)
	varying mediump vec3 reflectionDirectionInWorldSpace;
#elif defined(PIXEL_LIT)
	varying mediump vec3 cameraToPointInTangentSpace;
	varying mediump mat3 tbnToWorldMatrix;
#endif





void main()
{
    
    gl_Position = worldViewProjMatrix * inPosition;	

#if defined(VERTEX_LIT)    
    vec3 viewDirectionInWorldSpace = vec3(worldMatrix * inPosition) - cameraPosition;
    vec3 normalDirectionInWorldSpace = normalize(vec3(worldInvTransposeMatrix * inNormal));
    reflectionDirectionInWorldSpace = reflect(viewDirectionInWorldSpace, normalDirectionInWorldSpace);
	#if defined(MATERIAL_DECAL)
		varTexCoordDecal = inTexCoord1;		
	#endif	
#endif    

#if defined(PIXEL_LIT)
	vec3 n = normalize (worldViewInvTransposeMatrix * inNormal);
	vec3 t = normalize (worldViewInvTransposeMatrix * inTangent);
	vec3 b = cross (n, t);

	
    vec3 eyeCoordsPosition = vec3(worldViewMatrix *  inPosition);
	
	#if !defined (TANGENT_SPACE_WATER_REFLECTIONS)
		eyeDist = eyeCoordsPosition;
	#endif
    
	vec3 v;
	#if defined(SPECULAR)
		vec3 lightDir = lightPosition0.xyz - eyeCoordsPosition * lightPosition0.w;    
		
		// transform light and half angle vectors by tangent basis		
		v.x = dot (lightDir, t);
		v.y = dot (lightDir, b);
		v.z = dot (lightDir, n);
		varLightVec = v;     
	#endif	

    v.x = dot (eyeCoordsPosition, t);
	v.y = dot (eyeCoordsPosition, b);
	v.z = dot (eyeCoordsPosition, n);
	cameraToPointInTangentSpace = v;
    
    vec3 binormTS = cross(inNormal, inTangent);
    tbnToWorldMatrix = mat3(inTangent, binormTS, inNormal);
#endif
	
#if defined(PIXEL_LIT)||defined(MATERIAL_DECAL)	
	varTexCoord0 = inTexCoord0 * normal0Scale + normal0ShiftPerSecond * globalTime;
    varTexCoord1 = vec2(inTexCoord0.x+inTexCoord0.y, inTexCoord0.x-inTexCoord0.y) * normal1Scale + normal1ShiftPerSecond * globalTime;
#endif

#if defined(VERTEX_FOG)
    #if defined(PIXEL_LIT)
        float fogFragCoord = length(eyeCoordsPosition);
    #else
        vec3 eyeCoordsPosition = vec3(worldViewMatrix * inPosition);
        float fogFragCoord = length(eyeCoordsPosition);
    #endif
    #if !defined(FOG_LINEAR)
        const float LOG2 = 1.442695;
        varFogFactor = exp2( -fogDensity * fogDensity * fogFragCoord * fogFragCoord *  LOG2);
        varFogFactor = clamp(varFogFactor, 1.0 - fogLimit, 1.0);
    #else
        varFogFactor = 1.0 - clamp((fogFragCoord - fogStart) / (fogEnd - fogStart), 0.0, fogLimit);
    #endif
#endif

    
}
