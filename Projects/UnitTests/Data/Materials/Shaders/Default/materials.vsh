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

#if defined(VERTEX_LIT) || defined(PIXEL_LIT) || defined(MATERIAL_GRASS)
attribute vec3 inNormal;
#endif 

#if defined(MATERIAL_SKYBOX)
attribute vec3 inTexCoord0;
#else
attribute vec2 inTexCoord0;
#endif

#if defined(MATERIAL_DECAL) || defined(MATERIAL_DETAIL) || defined(MATERIAL_LIGHTMAP) || defined(FRAME_BLEND) || defined(MATERIAL_GRASS)
attribute vec2 inTexCoord1;
#endif

#if defined(VERTEX_COLOR)
attribute vec4 inColor;
#endif

#if defined(MATERIAL_GRASS)
attribute vec3 inBinormal;
#endif

#if defined(VERTEX_LIT)
#endif

#if defined(PIXEL_LIT) || defined(SPEED_TREE_LEAF) || defined(MATERIAL_GRASS)
attribute vec3 inTangent;
#endif

#if defined(FRAME_BLEND)
attribute float inTime;
#endif

// UNIFORMS
uniform mat4 worldViewProjMatrix;

#if defined(VERTEX_LIT) || defined(PIXEL_LIT) || defined(VERTEX_FOG) || defined(SPEED_TREE_LEAF)
uniform mat4 worldViewMatrix;
#endif

#if defined(VERTEX_LIT) || defined(PIXEL_LIT)
uniform mat3 worldViewInvTransposeMatrix;
uniform vec4 lightPosition0;
uniform float lightIntensity0; 
#endif

#if defined(VERTEX_LIT)
uniform float materialSpecularShininess;
uniform float inSpecularity;
uniform float inGlossiness;
uniform float physicalFresnelReflectance;
uniform vec3 metalFresnelReflectance;
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

#if defined(MATERIAL_LIGHTMAP)
uniform mediump vec2 uvOffset;
uniform mediump vec2 uvScale;
#endif

#if defined(SPEED_TREE_LEAF)
uniform vec3 worldViewTranslate;
uniform vec3 worldScale;
uniform mat4 projMatrix;
uniform float cutDistance;
uniform lowp vec3 treeLeafColorMul;
uniform lowp float treeLeafOcclusionOffset;
uniform lowp float treeLeafOcclusionMul;
#endif

// OUTPUT ATTRIBUTES
#if defined(MATERIAL_SKYBOX)
varying vec3 varTexCoord0;
#else
varying vec2 varTexCoord0;
#endif

#if defined(MATERIAL_DECAL) || defined(MATERIAL_DETAIL) || defined(MATERIAL_LIGHTMAP) || defined(FRAME_BLEND) || defined(MATERIAL_GRASS)
varying vec2 varTexCoord1;
#endif

#if defined(MATERIAL_GRASS)
varying vec2 varTexCoord2;
#endif

#if defined(VERTEX_LIT)
varying lowp float varDiffuseColor;

    #if defined(BLINN_PHONG)
    varying lowp float varSpecularColor;
    #elif defined(NORMALIZED_BLINN_PHONG)
    varying lowp vec3 varSpecularColor;
    varying lowp float varNdotH;
    #endif
#endif

#if defined(PIXEL_LIT)
varying vec3 varLightPosition;
varying vec3 varToLightVec;
varying vec3 varHalfVec;
varying vec3 varToCameraVec;
varying float varPerPixelAttenuation;
#endif

#if defined(VERTEX_FOG)
varying float varFogFactor;
#endif

#if defined(SETUP_LIGHTMAP)
uniform float lightmapSize;
varying lowp float varLightmapSize;
#endif

#if defined(VERTEX_COLOR)
varying lowp vec4 varVertexColor;
#endif

#if defined(FRAME_BLEND)
varying lowp float varTime;
#endif

#if defined(TEXTURE0_SHIFT_ENABLED)
uniform mediump vec2 texture0Shift;
#endif 

#if defined(REFLECTION) // works now only with VERTEX_LIT
uniform vec3 cameraPosition;
uniform mat4 worldMatrix;
uniform mat3 worldInvTransposeMatrix;
#if defined(VERTEX_LIT)
varying mediump vec3 reflectionDirectionInWorldSpace;
#elif defined(PIXEL_LIT)
varying mediump vec3 cameraToPointInTangentSpace;
varying mediump mat3 tbnToWorldMatrix;
#endif

#endif

#if defined(TEXTURE0_ANIMATION_SHIFT)
uniform float globalTime;
uniform vec2 tex0ShiftPerSecond;
#endif

#if defined(MATERIAL_GRASS)
uniform vec4 tilePos;
uniform vec3 worldSize;
uniform vec2 lodSwitchScale;

uniform float clusterScaleDensityMap[128];

uniform sampler2D detail;

uniform vec2 heightmapScale;

uniform vec3 perturbationForce;
uniform vec3 perturbationPoint;
uniform float perturbationForceDistance;
#endif

const float _PI = 3.141592654;

float FresnelShlick(float NdotL, float Cspec)
{
	float fresnel_exponent = 5.0;
	return Cspec + (1.0 - Cspec) * pow(1.0 - NdotL, fresnel_exponent);
}

vec3 FresnelShlickVec3(float NdotL, vec3 Cspec)
{
	float fresnel_exponent = 5.0;
	return Cspec + (1.0 - Cspec) * (pow(1.0 - NdotL, fresnel_exponent));
}

#if defined(WAVE_ANIMATION)
uniform float globalTime;
#endif

vec4 Wave(float time, vec4 pos, vec2 uv)
{
//	float time = globalTime;
//	vec4 pos = inPosition;
//	vec2 uv = inTexCoord0;
#if 1
    vec4 off;
    float sinOff = pos.x + pos.y + pos.z;
    float t = -time * 3.0;
    float cos1 = cos(t * 1.45 + sinOff);
    float cos2 = cos(t * 3.12 + sinOff);
    float cos3 = cos(t * 2.2 + sinOff);
    float fx= uv.x;
    float fy= uv.x * uv.y;
    
    off.y = pos.y + cos2 * fx * 0.5 - fy * 0.9;
    off.x = pos.x + cos1 * fx * 0.5;
    off.z = pos.z + cos3 * fx * 0.5;
    off.w = pos.w;
#else
    vec4 off;
    float t = -time;
    float sin2 = sin(4.0 * sqrt(uv.x + uv.x + uv.y * uv.y) + time);
    
    off.x = pos.x;// + cos1 * fx * 0.5;
    off.y = pos.y + sin2 * 0.5;// - fy * 0.9;
    off.z = pos.z;// + cos3 * fx * 0.5;
    off.w = pos.w;
#endif
    
    return off;
}

void main()
{	
#if defined(MATERIAL_SKYBOX)
	vec4 vecPos = (worldViewProjMatrix * inPosition);
	gl_Position = vec4(vecPos.xy, vecPos.w - 0.0001, vecPos.w);
#elif defined(SKYOBJECT)
	mat4 mwpWOtranslate = mat4(worldViewProjMatrix[0], worldViewProjMatrix[1], worldViewProjMatrix[2], vec4(0.0, 0.0, 0.0, 1.0));
	vec4 vecPos = (mwpWOtranslate * inPosition);
	gl_Position = vec4(vecPos.xy, vecPos.w - 0.0001, vecPos.w);
#elif defined(SPEED_TREE_LEAF)

#if defined (CUT_LEAF)
    vec3 position;
    vec4 tangentInCameraSpace = worldViewMatrix * vec4(inTangent, 1);
    if (tangentInCameraSpace.z < -cutDistance)
    {
        position = /*worldScale * vec3(0,0,0) +*/ worldViewTranslate;
    }
    else
    {
        position = worldScale * (inPosition.xyz - inTangent) + worldViewTranslate;
    }
    gl_Position = projMatrix * vec4(position, inPosition.w) + worldViewProjMatrix * vec4(inTangent, 0.0);
#else
    //mat4 mvp = worldMatrix * viewMatrix * projMatrix;
    //mat4 mvp = projMatrix * worldViewMatrix;
    //gl_Position = mvp * inPosition;
    
//    vec3 position;
//    vec3 tangentInCameraSpace = (worldScale * inTangent + worldViewTranslate);
//    float distance = length(tangentInCameraSpace);
//    if (distance > 40.0)
//    {
//        position = worldScale * ((inPosition.xyz - inTangent) * (40.0 / distance)) + worldViewTranslate;
//    }
//    else
//    {
//        position = worldScale * (inPosition.xyz - inTangent) + worldViewTranslate;
//    }
//    
//    //    vec4 position = vec4(worldScale * (inPosition.xyz - inTangent) + worldViewTranslate, inPosition.w);
//    gl_Position = projMatrix * vec4(position, inPosition.w) + worldViewProjMatrix * vec4(inTangent, 0.0);
    
	gl_Position = projMatrix * vec4(worldScale * (inPosition.xyz - inTangent) + worldViewTranslate, inPosition.w) + worldViewProjMatrix * vec4(inTangent, 0.0);
#endif

#elif defined(WAVE_ANIMATION)
	gl_Position = worldViewProjMatrix * Wave(globalTime, inPosition, inTexCoord0);
#elif defined(MATERIAL_GRASS)
        //inTangent.y - cluster type (0...3)
        //inTangent.z - cluster's reference density (0...15)
    
        //clusterScaleDensityMap[0] - density
        //clusterScaleDensityMap[1] - scale
    
        vec4 clusterCenter = vec4(inBinormal.x + tilePos.x,
                                  inBinormal.y + tilePos.y,
                                  inBinormal.z,
                                  inPosition.w);
    
        vec4 pos = vec4(inPosition.x + tilePos.x,
                        inPosition.y + tilePos.y,
                        inPosition.z,
                        inPosition.w);
    
        highp vec2 hUV = vec2(clamp(1.0 - (0.5 * worldSize.x - pos.x) / worldSize.x, 0.0, 1.0),
                        clamp(1.0 - (0.5 * worldSize.y - pos.y) / worldSize.y, 0.0, 1.0));
    
        hUV = vec2(clamp(hUV.x * heightmapScale.x, 0.0, 1.0),
                   clamp(hUV.y * heightmapScale.y, 0.0, 1.0));
    
        highp vec4 heightVec = texture2DLod(detail, hUV, 0.0);
        float height = dot(heightVec, vec4(0.93751430533303, 0.05859464408331, 0.00366216525521, 0.00022888532845)) * worldSize.z;
    
    
        pos.z += height;
        clusterCenter.z += height;
    
        int clusterType = int(inTangent.y);
        int vertexTileIndex = int(inTangent.x);
    
        float densityFactor;
    
        float clusterDensity = clusterScaleDensityMap[vertexTileIndex + clusterType];;
        float clusterScale = clusterScaleDensityMap[vertexTileIndex + 4 + clusterType];
        float clusterLodScale = 1.0;
    
        if(int(inTexCoord1.x) == int(lodSwitchScale.x))
        {
            clusterLodScale = lodSwitchScale.y;
        }
    
        vec4 lodScaledPos = pos;
        lodScaledPos.z = 0.0;
        lodScaledPos = mix(clusterCenter, lodScaledPos, clusterLodScale);
    
        pos.xy = lodScaledPos.xy;
    
        varTexCoord2.x = clusterLodScale;
    
        if(inTangent.z < clusterDensity)
        {
            densityFactor = 1.0;
        }
        else
        {
            densityFactor = 0.0;
        }
    
        pos = mix(clusterCenter, pos, clusterScale * densityFactor);
    
        //VI: don't calculate perturbation. Revise the code after oscillators etc have been integrated
        //vec3 perturbationScale = perturbationForce * clamp(1.0 - (distance(pos.xyz, perturbationPoint) / perturbationForceDistance), 0.0, 1.0);
    
        //if(pos.z > (clusterCenter.z + 0.1))
        //{
        //    pos.xy += perturbationScale.xy * normalize(pos.xy - perturbationPoint.xy);
        //}

        gl_Position = worldViewProjMatrix * pos;
        varTexCoord1 = hUV;
#else
	gl_Position = worldViewProjMatrix * inPosition;
#endif
    
#if defined(VERTEX_LIT) || defined(PIXEL_LIT) || defined(VERTEX_FOG) || defined(SPEED_TREE_LEAF)
#if defined(MATERIAL_GRASS)
    vec3 eyeCoordsPosition = vec3(worldViewMatrix * pos); // view direction in view space
#else
    vec3 eyeCoordsPosition = vec3(worldViewMatrix *  inPosition); // view direction in view space
#endif
#endif

#if defined(VERTEX_LIT)
    vec3 normal = normalize(worldViewInvTransposeMatrix * inNormal); // normal in eye coordinates
    vec3 toLightDir = lightPosition0.xyz - eyeCoordsPosition * lightPosition0.w;
    
#if defined(DISTANCE_ATTENUATION)
    float attenuation = lightIntensity0;
    float distAttenuation = length(toLightDir);
    attenuation /= (distAttenuation * distAttenuation); // use inverse distance for distance attenuation
#endif
    toLightDir = normalize(toLightDir);
    
#if defined(REFLECTION)
    vec3 viewDirectionInWorldSpace = vec3(worldMatrix * inPosition) - cameraPosition;
    vec3 normalDirectionInWorldSpace = normalize(vec3(worldInvTransposeMatrix * inNormal));
    reflectionDirectionInWorldSpace = reflect(viewDirectionInWorldSpace, normalDirectionInWorldSpace);
#endif
    
#if defined(BLINN_PHONG)
    varDiffuseColor = max(0.0, dot(normal, toLightDir));

    // Blinn-phong reflection
    vec3 toCameraDir = normalize(-eyeCoordsPosition);
    vec3 H = normalize(toLightDir + toCameraDir);
    float nDotHV = max(0.0, dot(normal, H));
    varSpecularColor = pow(nDotHV, materialSpecularShininess);
    
#elif defined(NORMALIZED_BLINN_PHONG)
    vec3 toCameraNormalized = normalize(-eyeCoordsPosition);
    vec3 H = normalize(toLightDir + toCameraNormalized);

    float NdotL = max (dot (normal, toLightDir), 0.0);
    float NdotH = max (dot (normal, H), 0.0);
    float LdotH = max (dot (toLightDir, H), 0.0);
    float NdotV = max (dot (normal, toCameraNormalized), 0.0);

    //vec3 fresnelIn = FresnelShlickVec3(NdotL, metalFresnelReflectance);
    vec3 fresnelOut = FresnelShlickVec3(NdotV, metalFresnelReflectance);
    float specularity = inSpecularity;

    float Dbp = NdotL;
    float Geo = 1.0 / LdotH * LdotH;
    
	varDiffuseColor = NdotL / _PI;
    
    varSpecularColor = Dbp * Geo * fresnelOut * specularity;
    varNdotH = NdotH;
#endif

    
#endif

#if defined(PIXEL_LIT)
	vec3 n = normalize (worldViewInvTransposeMatrix * inNormal);
	vec3 t = normalize (worldViewInvTransposeMatrix * inTangent);
	vec3 b = cross (n, t);
    
    vec3 toLightDir = lightPosition0.xyz - eyeCoordsPosition * lightPosition0.w;
#if defined(DISTANCE_ATTENUATION)
    varPerPixelAttenuation = length(toLightDir);
#endif
    //lightDir = normalize(lightDir);
    
	// transform light and half angle vectors by tangent basis
	vec3 v;
	v.x = dot (toLightDir, t);
	v.y = dot (toLightDir, b);
	v.z = dot (toLightDir, n);
    
#if !defined(FAST_NORMALIZATION)
	varToLightVec = v;
#else
    varToLightVec = normalize(v);
#endif

    vec3 toCameraDir = -eyeCoordsPosition;

    v.x = dot (toCameraDir, t);
	v.y = dot (toCameraDir, b);
	v.z = dot (toCameraDir, n);
#if !defined(FAST_NORMALIZATION)
	varToCameraVec = v;
#else
    varToCameraVec = normalize(v);
#endif
    
    /* Normalize the halfVector to pass it to the fragment shader */
	// No need to divide by two, the result is normalized anyway.
	// vec3 halfVector = normalize((E + lightDir) / 2.0);
#if defined(FAST_NORMALIZATION)
	vec3 halfVector = normalize(normalize(toCameraDir) + normalize(toLightDir));
	v.x = dot (halfVector, t);
	v.y = dot (halfVector, b);
	v.z = dot (halfVector, n);
    
	// No need to normalize, t,b,n and halfVector are normal vectors.
	varHalfVec = v;
#endif

//    varLightPosition.x = dot (lightPosition0.xyz, t);
//    varLightPosition.y = dot (lightPosition0.xyz, b);
//    varLightPosition.z = dot (lightPosition0.xyz, n);
    
#if defined(REFLECTION)
    v.x = dot (eyeCoordsPosition, t);
	v.y = dot (eyeCoordsPosition, b);
	v.z = dot (eyeCoordsPosition, n);
	cameraToPointInTangentSpace = normalize (v);
    
    vec3 binormTS = cross(inNormal, inTangent);
//    tbnToWorldMatrix = mat3(vec3(inTangent.x, binormTS.x, inNormal.x),
//                            vec3(inTangent.y, binormTS.y, inNormal.y),
//                            vec3(inTangent.z, binormTS.z, inNormal.z));
    tbnToWorldMatrix = mat3(inTangent, binormTS, inNormal);
#endif
#endif

#if defined(VERTEX_FOG)
    float fogFragCoord = length(eyeCoordsPosition);
    #if !defined(FOG_LINEAR)
        const float LOG2 = 1.442695;
        varFogFactor = exp2( -fogDensity * fogDensity * fogFragCoord * fogFragCoord *  LOG2);
        varFogFactor = clamp(varFogFactor, 1.0 - fogLimit, 1.0);
    #else
        varFogFactor = 1.0 - clamp((fogFragCoord - fogStart) / (fogEnd - fogStart), 0.0, fogLimit);
    #endif
#endif

#if defined(VERTEX_COLOR)
	varVertexColor = inColor;
#endif

	varTexCoord0 = inTexCoord0;
	
#if defined(TEXTURE0_SHIFT_ENABLED)
	varTexCoord0 += texture0Shift;
#endif
    
#if defined(TEXTURE0_ANIMATION_SHIFT)
    varTexCoord0 += tex0ShiftPerSecond * globalTime;
#endif
		
#if defined(MATERIAL_DECAL) || defined(MATERIAL_DETAIL) || defined(MATERIAL_LIGHTMAP) || defined(FRAME_BLEND)
	
	#if defined(SETUP_LIGHTMAP)
		varLightmapSize = lightmapSize;
		varTexCoord1 = inTexCoord1;
	#elif defined(MATERIAL_LIGHTMAP)
		varTexCoord1 = uvScale*inTexCoord1+uvOffset;
    #else
		varTexCoord1 = inTexCoord1;
	#endif
#endif

#if defined(FRAME_BLEND)
	varTime = inTime;
#endif


}
