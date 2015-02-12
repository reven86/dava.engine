#ifdef GL_ES
// define default precision for float, vec, mat.
precision highp float;
#else
#define lowp
#define highp
#define mediump
#endif


#define MAX_JOINTS 32

// INPUT ATTRIBUTES
attribute vec4 inPosition;

#if defined(VERTEX_LIT) || defined(PIXEL_LIT) || defined(MATERIAL_GRASS_TRANSFORM)
attribute vec3 inNormal;
#endif 

#if defined(MATERIAL_SKYBOX)
attribute vec3 inTexCoord0;
#else
attribute vec2 inTexCoord0;
#endif

#if defined(MATERIAL_DECAL) || defined(MATERIAL_LIGHTMAP) || defined(FRAME_BLEND) || defined(ALPHA_MASK)
attribute vec2 inTexCoord1;
#endif

#if defined(VERTEX_COLOR)
attribute vec4 inColor;
#endif

#if defined(VERTEX_LIT)
#endif

#if defined(PIXEL_LIT) || defined(MATERIAL_GRASS_TRANSFORM)
attribute vec3 inTangent;
attribute vec3 inBinormal;
#endif

#if defined (SKINNING)
attribute vec4 inJointIndex;
attribute vec4 inJointWeight;
#endif


#if defined(SPEED_TREE_LEAF)
attribute vec3 inPivot;
#if defined(WIND_ANIMATION)
attribute vec2 inAngleSinCos;
#endif
#endif

#if defined(WIND_ANIMATION)
attribute float inFlexibility;
#endif

#if defined(FRAME_BLEND)
attribute float inTime;
#endif

// UNIFORMS
uniform mat4 worldViewProjMatrix;

#if defined(VERTEX_LIT) || defined(PIXEL_LIT) || defined(VERTEX_FOG) || defined(SPEED_TREE_LEAF) || defined(SPHERICAL_LIT)
uniform mat4 worldViewMatrix;
#endif

#if defined(VERTEX_LIT) || defined(PIXEL_LIT) || (defined(VERTEX_FOG) && defined(FOG_ATMOSPHERE))
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

#if defined (SKINNING)
    uniform vec4 jointPositions[MAX_JOINTS]; // (x, y, z, scale)
    uniform vec4 jointQuaternions[MAX_JOINTS];    
#endif

#if defined(VERTEX_FOG)
    uniform lowp vec3 fogColor;
    uniform lowp float fogLimit;
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

#if defined(MATERIAL_LIGHTMAP)
uniform mediump vec2 uvOffset;
uniform mediump vec2 uvScale;
#endif

#if defined(WIND_ANIMATION)
uniform mediump vec2 trunkOscillationParams;
#endif

#if defined(SPEED_TREE_LEAF)
uniform vec3 worldViewTranslate;
uniform vec3 worldScale;
uniform mat4 projMatrix;
uniform float cutDistance;

	#if !defined(SPHERICAL_LIT) //legacy for old tree lighting
		uniform lowp vec3 treeLeafColorMul;
		uniform lowp float treeLeafOcclusionOffset;
		uniform lowp float treeLeafOcclusionMul;
	#endif
	
	#if defined(WIND_ANIMATION)
		uniform mediump vec2 leafOscillationParams; //x: A*sin(T); y: A*cos(T);
	#endif
	
	#if defined(SPHERICAL_LIT)
		uniform mediump float speedTreeLightSmoothing;
	#endif
#endif

#if defined(SPHERICAL_LIT)
uniform vec3 worldViewObjectCenter;
uniform mat4 invViewMatrix;
uniform vec3 boundingBoxSize;

	#if defined(SPHERICAL_HARMONICS_9)
		uniform vec3 sphericalHarmonics[9];
	#elif defined(SPHERICAL_HARMONICS_4)
		uniform vec3 sphericalHarmonics[4];
	#else
		uniform vec3 sphericalHarmonics[1];
	#endif
	
#endif

#if defined(TILED_DECAL_MASK)
uniform vec2 decalTileCoordScale;
#endif

#if defined(MATERIAL_DETAIL)
uniform vec2 detailTileCoordScale;
#endif

// OUTPUT ATTRIBUTES
#if defined(MATERIAL_SKYBOX)
varying vec3 varTexCoord0;
#else
varying vec2 varTexCoord0;
#endif

#if defined(MATERIAL_DECAL) || defined(MATERIAL_LIGHTMAP) || defined(FRAME_BLEND) || defined(ALPHA_MASK)
varying vec2 varTexCoord1;
#endif

#if defined(MATERIAL_DETAIL)
varying mediump vec2 varDetailTexCoord;
#endif

#if defined(TILED_DECAL_MASK)
varying vec2 varDecalTileTexCoord;
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

#if defined(SETUP_LIGHTMAP)
uniform float lightmapSize;
varying lowp float varLightmapSize;
#endif

#if defined(VERTEX_COLOR) || defined(SPHERICAL_LIT)
varying lowp vec4 varVertexColor;
#endif

#if defined(FRAME_BLEND)
varying lowp float varTime;
#endif

#if defined(TEXTURE0_SHIFT_ENABLED)
uniform mediump vec2 texture0Shift;
#endif 

uniform vec3 cameraPosition;
uniform mat4 worldMatrix;

#if defined(REFLECTION) // works now only with VERTEX_LIT
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

#if defined(MATERIAL_GRASS_TRANSFORM)
uniform vec3 tilePos;
uniform vec3 worldSize;
uniform vec2 lodSwitchScale;
uniform vec3 billboardDirection;

uniform sampler2D heightmap;
uniform sampler2D vegetationmap;
uniform sampler2D densitymap;

uniform vec2 heightmapScale;

#if defined(MATERIAL_GRASS_TRANSFORM_WAVE)

uniform float vegWaveOffset[8]; //2 floats (xy) per layer

#endif

#if defined(MATERIAL_GRASS_OPAQUE) || defined(MATERIAL_GRASS_BLEND)

varying lowp vec3 varVegetationColor;

#endif

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

vec3 JointTransformTangent(vec3 inVec, vec4 jointQuaternion)
{
    vec3 t = 2.0 * cross(jointQuaternion.xyz, inVec);
    return inVec + jointQuaternion.w * t + cross(jointQuaternion.xyz, t); 
    //return inVec; 
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

#if defined (SKINNING)
    //compute final state - for now just effected by 1 bone - later blend everything here
    int index = int(inJointIndex);
    vec4 weightedVertexPosition = jointPositions[index];
    vec4 weightedVertexQuaternion = jointQuaternions[index];
#endif

#if defined(MATERIAL_SKYBOX)
	vec4 vecPos = (worldViewProjMatrix * inPosition);
	gl_Position = vec4(vecPos.xy, vecPos.w - 0.0001, vecPos.w);
#elif defined(SKYOBJECT)
	mat4 mwpWOtranslate = mat4(worldViewProjMatrix[0], worldViewProjMatrix[1], worldViewProjMatrix[2], vec4(0.0, 0.0, 0.0, 1.0));
	vec4 vecPos = (mwpWOtranslate * inPosition);
	gl_Position = vec4(vecPos.xy, vecPos.w - 0.0001, vecPos.w);
#elif defined(SPEED_TREE_LEAF)

    vec4 eyeCoordsPosition4;
    
#if defined (CUT_LEAF)
    vec4 tangentInCameraSpace = worldViewMatrix * vec4(inPivot, 1.0);
    if (tangentInCameraSpace.z < -cutDistance)
    {
        gl_Position = worldViewProjMatrix * vec4(inPivot, inPosition.w);
    }
    else
    {
#endif

    vec3 offset = inPosition.xyz - inPivot;
    vec3 pivot = inPivot;
    
#if defined(WIND_ANIMATION)
    //inAngleSinCos:        x: cos(T0);  y: sin(T0);
    //leafOscillationParams:  x: A*sin(T); y: A*cos(T);
    vec3 windVectorFlex = vec3(trunkOscillationParams * inFlexibility, 0.0);
    pivot += windVectorFlex;
    
    vec2 SinCos = inAngleSinCos * leafOscillationParams; //vec2(A*sin(t)*cos(t0), A*cos(t)*sin(t0))
    float sinT = SinCos.x + SinCos.y;     //sin(t+t0)*A = sin*cos + cos*sin
    float cosT = 1.0 - 0.5 * sinT * sinT; //cos(t+t0)*A = 1 - 0.5*sin^2
    
    vec4 SinCosT = vec4(sinT, cosT, cosT, sinT); //temp vec for mul
    vec4 offsetXY = vec4(offset.x, offset.y, offset.x, offset.y); //temp vec for mul
    vec4 rotatedOffsetXY = offsetXY * SinCosT; //vec4(x*sin, y*cos, x*cos, y*sin)
    
    offset.x = rotatedOffsetXY.z - rotatedOffsetXY.w; //x*cos - y*sin
    offset.y = rotatedOffsetXY.x + rotatedOffsetXY.y; //x*sin + y*cos

#endif //end of (not WIND_ANIMATION and SPEED_TREE_LEAF)

	vec4 eyeCoordsPivot = worldViewMatrix * vec4(pivot, inPosition.w);
    eyeCoordsPosition4 = vec4(worldScale * offset, 0.0) + eyeCoordsPivot;
    gl_Position = projMatrix * eyeCoordsPosition4;
    
#if defined (CUT_LEAF)   
    }
#endif // not CUT_LEAF

#else // not SPEED_TREE_LEAF
    
#if defined(WIND_ANIMATION)

    vec3 windVectorFlex = vec3(trunkOscillationParams * inFlexibility, 0.0);
    gl_Position = worldViewProjMatrix * vec4(inPosition.xyz + windVectorFlex, inPosition.w);
	
#else //!defined(WIND_ANIMATION)

	#if defined(WAVE_ANIMATION)
		gl_Position = worldViewProjMatrix * Wave(globalTime, inPosition, inTexCoord0);
	#elif defined(MATERIAL_GRASS_TRANSFORM)
    
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
    
    #if defined(MATERIAL_GRASS_BILLBOARD)
        //1st method of billboards when cameraPosition is point
        vec3 toCamera = normalize(vec3(clusterCenter.xyz) - vec3(cameraPosition.xy, clusterCenter.z));
        vec3 actualDirection = normalize(cross(inNormal, toCamera));
    
    
        //2nd method of billboards when cameraDirection is vector
        //vec2 actualDirection = inNormal.z * vec2(billboardDirection);
        //
    
        vec2 planeDirection = vec2(actualDirection.x, actualDirection.y) * length(vec2(inPosition.x - inBinormal.x, inPosition.y - inBinormal.y));
        pos = clusterCenter + vec4(planeDirection.x, planeDirection.y, inPosition.z, 0.0);
    #endif
    
        highp vec2 hUVheight = vec2(clamp(1.0 - (0.5 * worldSize.x - clusterCenter.x) / worldSize.x, 0.0, 1.0),
                        clamp(1.0 - (0.5 * worldSize.y - clusterCenter.y) / worldSize.y, 0.0, 1.0));
    
        hUVheight = vec2(clamp(hUVheight.x * heightmapScale.x, 0.0, 1.0),
                   clamp(hUVheight.y * heightmapScale.y, 0.0, 1.0));
    
        highp vec4 heightVec = texture2DLod(heightmap, hUVheight, 0.0);
        float height = dot(heightVec, vec4(0.93751430533303, 0.05859464408331, 0.00366216525521, 0.00022888532845)) * worldSize.z;
    
    
        pos.z += height;
        clusterCenter.z += height;
    
        float clusterScale = tilePos.z;
        if(int(inTangent.x) == int(lodSwitchScale.x))
        {
            clusterScale *= lodSwitchScale.y;
        }
    
#if defined(MATERIAL_GRASS_BLEND)
        varTexCoord2.x = clusterLodScale;
#endif
    
        highp vec2 hUVcolor = vec2(hUVheight.x, 1.0 - hUVheight.y);
        vec4 vegetationMask = texture2DLod(vegetationmap, hUVcolor, 0.0);
    
#if defined(MATERIAL_GRASS_OPAQUE) || defined(MATERIAL_GRASS_BLEND)
        varVegetationColor = vegetationMask.rgb;
    
        /*if(int(inTangent.y) == 0)
        {
           varVegetationColor.r += 0.5;
        }
        else if(int(inTangent.y) == 1)
        {
           varVegetationColor.g += 0.5;
        }
        else if(int(inTangent.y) == 2)
        {
            varVegetationColor.rb += vec2(0.75, 0.75);
        }
        else
        {
            varVegetationColor.rgb += vec3(0.5, 0.5, 0.5);
        }*/
#endif
    
#if defined(MATERIAL_GRASS_TRANSFORM_WAVE)
    
    int clusterType = int(inTangent.y);
    int waveIndex = clusterType * 2;
    
    pos.x += inTangent.z * vegWaveOffset[waveIndex];
    pos.y += inTangent.z * vegWaveOffset[waveIndex + 1];
    
#endif

        pos = mix(clusterCenter, pos, vegetationMask.a * clusterScale);
    
        gl_Position = worldViewProjMatrix * pos;
    
    #else
        #if defined (SKINNING)
            vec3 tmpVec = 2.0 * cross(weightedVertexQuaternion.xyz, inPosition.xyz);
            vec4 skinnedPosition = vec4(weightedVertexPosition.xyz + (inPosition.xyz + weightedVertexQuaternion.w * tmpVec + cross(weightedVertexQuaternion.xyz, tmpVec))*weightedVertexPosition.w, inPosition.w);
            gl_Position = worldViewProjMatrix * skinnedPosition;
        #else
            gl_Position = worldViewProjMatrix * inPosition;
        #endif
    #endif

#endif //defined(WIND_ANIMATION)
    
#endif //end "not SPEED_TREE_LEAF

#if defined(SPEED_TREE_LEAF)
	vec3 eyeCoordsPosition = vec3(eyeCoordsPosition4);
#elif defined(VERTEX_LIT) || defined(PIXEL_LIT) || defined(VERTEX_FOG) || defined(SPHERICAL_LIT)
    #if defined(MATERIAL_GRASS_TRANSFORM)
        vec3 eyeCoordsPosition = vec3(worldViewMatrix * pos); // view direction in view space
    #else
        #if defined (SKINNING)
            vec3 eyeCoordsPosition = vec3(worldViewMatrix * skinnedPosition); // view direction in view space
        #else
            vec3 eyeCoordsPosition = vec3(worldViewMatrix *  inPosition); // view direction in view space
        #endif
    #endif
#endif

#if defined(VERTEX_LIT) || defined(PIXEL_LIT) || (defined(VERTEX_FOG) && defined(FOG_ATMOSPHERE))
    vec3 toLightDir = lightPosition0.xyz - eyeCoordsPosition * lightPosition0.w;
#endif

#if defined(VERTEX_LIT)
    vec3 normal = normalize(worldViewInvTransposeMatrix * inNormal); // normal in eye coordinates
   
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

    #if defined (SKINNING)
        vec3 n = normalize (worldViewInvTransposeMatrix * JointTransformTangent(inNormal, weightedVertexQuaternion));
        vec3 t = normalize (worldViewInvTransposeMatrix * JointTransformTangent(inTangent, weightedVertexQuaternion));
        vec3 b = normalize (worldViewInvTransposeMatrix * JointTransformTangent(inBinormal, weightedVertexQuaternion));
    #else
        vec3 n = normalize (worldViewInvTransposeMatrix * inNormal);
        vec3 t = normalize (worldViewInvTransposeMatrix * inTangent);
        vec3 b = normalize (worldViewInvTransposeMatrix * inBinormal);
    #endif
    
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
        vec3 viewDirectionInWorldSpace = viewPointInWorldSpace - cameraPosition;
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

#if defined(VERTEX_COLOR)
	varVertexColor = inColor;
#endif

#if defined(SPHERICAL_LIT)

#define A0		(0.282094)
#define A1 		(0.325734)

#define Y2_2(n) (0.273136 * (n.y * n.x))                                // (1.0 / 2.0) * sqrt(15.0 / PI) * ((n.y * n.x)) * 0.785398 / PI
#define Y2_1(n) (0.273136 * (n.y * n.z))                                // (1.0 / 2.0) * sqrt(15.0 / PI) * ((n.y * n.z)) * 0.785398 / PI
#define Y20(n)  (0.078847 * (3.0 * n.z * n.z - 1.0))  					// (1.0 / 4.0) * sqrt(5.0 / PI) * ((3.0 * n.z * n.z - 1.0)) * 0.785398 / PI
#define Y21(n)  (0.273136 * (n.z * n.x))                                // (1.0 / 2.0) * sqrt(15.0 / PI) * ((n.z * n.x)) * 0.785398 / PI
#define Y22(n)  (0.136568 * (n.x * n.x - n.y * n.y))                    // (1.0 / 4.0) * sqrt(15.0 / PI) * ((n.x * n.x - n.y * n.y)) * 0.785398 / PI

	vec3 sphericalLightFactor = A0 * sphericalHarmonics[0];
	
	#if defined(SPEED_TREE_LEAF)
		vec3 localSphericalLightFactor = sphericalLightFactor;
	#endif
	
#if !defined(CUT_LEAF)

#if defined(SPHERICAL_HARMONICS_4) || defined(SPHERICAL_HARMONICS_9)

	mat3 invViewMatrix3 = mat3(vec3(invViewMatrix[0]), vec3(invViewMatrix[1]), vec3(invViewMatrix[2]));
	vec3 normal = invViewMatrix3 * (eyeCoordsPosition - worldViewObjectCenter);
	normal /= boundingBoxSize;
	vec3 n = normalize(normal);

	mat3 shMatrix = mat3(sphericalHarmonics[1], sphericalHarmonics[2], sphericalHarmonics[3]);
	sphericalLightFactor += A1 * shMatrix * vec3(n.y, n.z, n.x);
	
	#if defined(SPEED_TREE_LEAF)
		vec3 localNormal = invViewMatrix3 * (eyeCoordsPosition - vec3(eyeCoordsPivot));
		vec3 ln = normalize(localNormal);
		localSphericalLightFactor += A1 * shMatrix * vec3(ln.y, ln.z, ln.x);
	#endif

#if defined(SPHERICAL_HARMONICS_9)
	sphericalLightFactor += Y2_2(n) * sphericalHarmonics[4];
	sphericalLightFactor += Y2_1(n) * sphericalHarmonics[5];
	sphericalLightFactor += Y20(n) * sphericalHarmonics[6];
	sphericalLightFactor += Y21(n) * sphericalHarmonics[7];
	sphericalLightFactor += Y22(n) * sphericalHarmonics[8];
#endif

	#if defined(SPEED_TREE_LEAF)
		sphericalLightFactor = mix(sphericalLightFactor, localSphericalLightFactor, speedTreeLightSmoothing);
	#endif
	
#endif

#endif

	varVertexColor = vec4(sphericalLightFactor * 2.0, 1.0);

#elif defined(SPEED_TREE_LEAF) //legacy for old tree lighting
    varVertexColor.rgb = varVertexColor.rgb * treeLeafColorMul * treeLeafOcclusionMul + vec3(treeLeafOcclusionOffset);
#endif
    
	varTexCoord0 = inTexCoord0;
	
#if defined(TEXTURE0_SHIFT_ENABLED)
	varTexCoord0 += texture0Shift;
#endif
    
#if defined(TEXTURE0_ANIMATION_SHIFT)
    varTexCoord0 += tex0ShiftPerSecond * globalTime;
#endif
	
#if defined(TILED_DECAL_MASK)
    varDecalTileTexCoord = varTexCoord0 * decalTileCoordScale;
#endif
    
#if defined(MATERIAL_DETAIL)
    varDetailTexCoord = varTexCoord0 * detailTileCoordScale;
#endif
	
#if defined(MATERIAL_DECAL) || defined(MATERIAL_LIGHTMAP) || defined(FRAME_BLEND) || defined(ALPHA_MASK)
	
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
