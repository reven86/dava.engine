#include "common.slh"
#include "blending.slh"

fragment_in
{
#if defined(ENCODE_DISTANCE)

    float3  directionFromPoint  : TEXCOORD0;

#elif defined(DECODE_DISTANCE)

    float3  directionFromPoint  : TEXCOORD0;

#elif defined(REPROJECTION)

    float4  reprojectedCoords   : TEXCOORD0;
    float   distanceToOrigin    : TEXCOORD1;
    float4  viewportCoords      : TEXCOORD2;
#endif
};

fragment_out
{
    float4  color   : SV_TARGET0;
};

float4 decodeVector = float4(1.0, 255.0, 65025.0, 16581375.0);
float MAGIC_TRESHOLD_2 = 1.0 / 255.0;
float MAGIC_TRESHOLD_1 = 1.0 + 1.0 / 255.0;

float4 EncodeFloat(float v)
{
    return float4(v, v * v, 0.0, 0.0);
}

float DecodeFloat(float4 encoded)
{
    return encoded.x;
}

float ComputeOcclusion(float4 sampledDistance, float actualDistance)
{
    float Ex_2 = sampledDistance.x * sampledDistance.x;
    float E_x2 = sampledDistance.y;
    float variance = E_x2 - Ex_2;
    float mD = sampledDistance.x - actualDistance;
    float p = variance / (variance + mD * mD);
    return max(p, float(actualDistance <= sampledDistance.x)); 
}

#if defined(DECODE_DISTANCE)

uniform samplerCUBE cubemap;

[material][a] property float4 flatColor;
[material][a] property float3 transformedNormal;
[material][a] property float3 pointProperties;

#elif defined(REPROJECTION)

uniform sampler2D fixedFrame;
uniform sampler2D fixedFrameDistances;
uniform sampler2D currentFrame;

[material][a] property float2 viewportSize;
[material][a] property float currentFrameCompleteness;

#endif


fragment_out
fp_main( fragment_in input )
{
    fragment_out    output;

#if defined(PRERENDER)

    output.color = float4(0.0, 0.0, 0.0, 0.0);

#elif defined(ENCODE_DISTANCE)

    output.color = EncodeFloat(length(input.directionFromPoint));

#elif defined(DECODE_DISTANCE)

    float4 sampledDistance = texCUBE(cubemap, input.directionFromPoint);
    float actualDistance = length(input.directionFromPoint);
    float occluded = ComputeOcclusion(sampledDistance, actualDistance);
    
    float3 nDir = normalize(input.directionFromPoint);
    float DdotN = dot(nDir, transformedNormal);
    float angleLimit = float(clamp(DdotN, pointProperties.x, pointProperties.y) == DdotN);
    float distanceLimit = step(actualDistance, pointProperties.z);
    output.color = flatColor * (distanceLimit * occluded * angleLimit);

#elif defined(REPROJECTION)

    float2 vpCoords = 0.5 + 0.5 * input.viewportCoords.xy / input.viewportCoords.w;
    float2 vp = viewportSize * vpCoords;

    float cx = float(int(vp.x) / 4);
    float cy = float(int(vp.y) / 4);
    float4 checkboard = float4(0.25 * fmod(cx + fmod(cy, 2.0), 2.0));

    float3 reprojectedUVW = 0.5 + 0.5 * input.reprojectedCoords.xyz / input.reprojectedCoords.w;
    float4 sampledColor = tex2D(fixedFrame, reprojectedUVW.xy);
    float4 currentColor = lerp(checkboard, tex2D(currentFrame, vpCoords), currentFrameCompleteness);

    float actualDistance = input.distanceToOrigin;
    float sampledDistance = DecodeFloat(tex2D(fixedFrameDistances, reprojectedUVW.xy));
    float visibleInProjection = 1.0 - float(abs(actualDistance / sampledDistance - 1.0) > MAGIC_TRESHOLD_2);

    float3 rpClamped = clamp(reprojectedUVW, float3(0.0), float3(1.0));
    float insideProjection = float((rpClamped.x == reprojectedUVW.x) && (rpClamped.y == reprojectedUVW.y) && (rpClamped.z == reprojectedUVW.z));

    output.color = lerp(currentColor, sampledColor, insideProjection * visibleInProjection);

#else
#   error Undefined
#endif

    return output;
}
