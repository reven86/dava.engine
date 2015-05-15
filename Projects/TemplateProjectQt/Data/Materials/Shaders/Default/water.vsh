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


#if defined(VERTEX_LIT) || defined(PIXEL_LIT) || (defined(VERTEX_FOG) && defined(FOG_GLOW))
uniform vec4 lightPosition0;
#endif

#if defined(PIXEL_LIT)
uniform mat3 worldViewInvTransposeMatrix;
uniform float lightIntensity0; 
uniform float materialSpecularShininess;
#endif

#if defined(DEBUG_NORMAL_ROTATION)
uniform float normalRotation;
#endif

#if !defined (TANGENT_SPACE_WATER_REFLECTIONS)
varying vec3 eyeDist;
#endif

#if defined(PIXEL_LIT)
varying vec3 varLightVec;
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
    uniform lowp vec3 fogColor;
    uniform float fogLimit;
    #if defined(FOG_LINEAR)
        uniform float fogStart;
        uniform float fogEnd;
    #else
        uniform float fogDensity;
    #endif
    #if defined(FOG_ATMOSPHERE)
        uniform float fogAtmosphereDistance;
        #if defined(FOG_ATMOSPHERE_MAP)
            uniform samplerCube atmospheremap;
        #else
            uniform lowp vec3 fogAtmosphereColorSun;
            uniform lowp vec3 fogAtmosphereColorSky;
            uniform float fogAtmosphereScattering;
        #endif
    #endif
    #if defined(FOG_HALFSPACE)
        uniform float fogHalfspaceHeight;
        uniform float fogHalfspaceFalloff;
        uniform float fogHalfspaceDensity;
        uniform float fogHalfspaceLimit;
    #endif
    varying lowp float varFogAmoung;
    varying lowp vec3 varFogColor;
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

#if defined(VERTEX_LIT) || defined(PIXEL_LIT) || (defined(VERTEX_FOG) && defined(FOG_GLOW))
	vec3 eyeCoordsPosition = vec3(worldViewMatrix * inPosition);
    vec3 toLightDir = lightPosition0.xyz - eyeCoordsPosition * lightPosition0.w;
#endif

#if defined(PIXEL_LIT)
	vec3 n = normalize (worldViewInvTransposeMatrix * inNormal);
	vec3 t = normalize (worldViewInvTransposeMatrix * inTangent);
	vec3 b = cross (n, t);

	#if !defined (TANGENT_SPACE_WATER_REFLECTIONS)
		eyeDist = eyeCoordsPosition;
	#endif
    
	// transform light and half angle vectors by tangent basis
	vec3 v;
	v.x = dot (toLightDir, t);
	v.y = dot (toLightDir, b);
	v.z = dot (toLightDir, n);
	varLightVec = v;       

    v.x = dot (eyeCoordsPosition, t);
	v.y = dot (eyeCoordsPosition, b);
	v.z = dot (eyeCoordsPosition, n);
	cameraToPointInTangentSpace = v;
    
    vec3 binormTS = cross(inNormal, inTangent);
    tbnToWorldMatrix = mat3(inTangent, binormTS, inNormal);
#endif
	
#if defined(PIXEL_LIT)||defined(MATERIAL_DECAL)	
	varTexCoord0 = inTexCoord0 * normal0Scale + normal0ShiftPerSecond * globalTime;
    #if defined(SEPARATE_NORMALMAPS)
        #if defined(DEBUG_NORMAL_ROTATION)
            float rota = radians(normalRotation);        
            float cr = cos(rota);
            float sr = sin(rota);
            vec2 rotatedTC = vec2(inTexCoord0.x * cr + inTexCoord0.y * sr, inTexCoord0.x * sr - inTexCoord0.y * cr);
            varTexCoord1 = rotatedTC * normal1Scale + normal1ShiftPerSecond * globalTime;
        #else            
            varTexCoord1 = inTexCoord0 * normal1Scale + normal1ShiftPerSecond * globalTime;
        #endif
    #else
        varTexCoord1 = vec2(inTexCoord0.x+inTexCoord0.y, inTexCoord0.x-inTexCoord0.y) * normal1Scale + normal1ShiftPerSecond * globalTime;
    #endif
    
#endif

#if defined(VERTEX_FOG)
    float fogDistance = length(eyeCoordsPosition);
    
    // calculating fog amoung, depending on distance 
    #if !defined(FOG_LINEAR)
        varFogAmoung = 1.0 - exp(-fogDensity * fogDistance);
    #else
        varFogAmoung = (fogDistance - fogStart) / (fogEnd - fogStart);
    #endif
    
    // calculating view direction in world space, point of view in world space
    #if defined(FOG_HALFSPACE) || defined(FOG_ATMOSPHERE_MAP)
        #if defined(MATERIAL_GRASS_TRANSFORM)
            vec3 viewPointInWorldSpace = vec3(worldMatrix * pos);
        #else
            vec3 viewPointInWorldSpace = vec3(worldMatrix * inPosition);
        #endif
        #if !defined(VERTEX_LIT)
            vec3 viewDirectionInWorldSpace = viewPointInWorldSpace - cameraPosition;
        #endif
    #endif
    
    // calculating halfSpaceFog amoung
    #if defined(FOG_HALFSPACE)
        #if defined(FOG_HALFSPACE_LINEAR)
            // view http://www.terathon.com/lengyel/Lengyel-UnifiedFog.pdf
            // to get more clear understanding about this calculations
            float fogK = step(cameraPosition.z, fogHalfspaceHeight);
            float fogFdotP = viewPointInWorldSpace.z - fogHalfspaceHeight;
            float fogFdotC = cameraPosition.z - fogHalfspaceHeight;
            
            float fogC1 = fogK * (fogFdotP + fogFdotC);
            float fogC2 = (1.0 - 2.0 * fogK) * fogFdotP;
            float fogG = min(fogC2, 0.0);
            fogG = -length(viewDirectionInWorldSpace) * fogHalfspaceDensity * (fogC1 - fogG * fogG / abs(viewDirectionInWorldSpace.z));
            
            float halfSpaceFogAmoung = 1.0 - exp2(-fogG);
        #else
            float fogK = viewDirectionInWorldSpace.z / fogDistance;
            float fogB = cameraPosition.z - fogHalfspaceHeight;
            
            float halfSpaceFogAmoung = fogHalfspaceDensity * exp(-fogHalfspaceFalloff * fogB) * (1.0 - exp(-fogHalfspaceFalloff * fogK * fogDistance)) / fogK;
        #endif
        varFogAmoung = varFogAmoung + clamp(halfSpaceFogAmoung, 0.0, fogHalfspaceLimit);
    #endif

    // limit fog amoung
    varFogAmoung = clamp(varFogAmoung, 0.0, fogLimit);
    
    // calculating fog color
    #if defined(FOG_ATMOSPHERE)
        lowp vec3 atmosphereColor;
        #if defined(FOG_ATMOSPHERE_MAP)
            vec3 viewDirection = normalize(vec3(worldMatrix * inPosition) - cameraPosition);
            viewDirection.z = clamp(viewDirection.z, 0.01, 1.0);
            atmosphereColor = textureCube(atmospheremap, viewDirection).xyz;
        #else
            float atmospheteAngleFactor = dot(normalize(eyeCoordsPosition), normalize(toLightDir)) * 0.5 + 0.5;
            atmosphereColor = mix(fogAtmosphereColorSky, fogAtmosphereColorSun, pow(atmospheteAngleFactor, fogAtmosphereScattering));
        #endif
        lowp float fogAtmosphereAttenuation = clamp(fogDistance / fogAtmosphereDistance, 0.0, 1.0);
        varFogColor = mix(fogColor, atmosphereColor, fogAtmosphereAttenuation);
    #else
        varFogColor = fogColor;
    #endif
#endif
    
}
