#include "common.slh"

vertex_in
{
    float4  position : POSITION;
    float3  normal   : NORMAL;
    #if SKINNING
    float   index    : BLENDINDICES;
    #endif
};

vertex_out
{
    float4  position : SV_POSITION;
};

[auto][a] property float4x4 worldViewProjMatrix;
[auto][a] property float4x4 worldViewMatrix;
[auto][a] property float4x4 projMatrix;
[auto][a] property float4x4 worldViewInvTransposeMatrix;



#if SKINNING
[auto][jpos] property float4 jointPositions[MAX_JOINTS] : "bigarray" ; // (x, y, z, scale)
[auto][jrot] property float4 jointQuaternions[MAX_JOINTS] : "bigarray" ;
#endif

[material][a] property float silhouetteScale = 1.0;
[material][a] property float silhouetteExponent = 0;


vertex_out vp_main( vertex_in input )
{
    vertex_out  output;

#if SKINNING
    // compute final state - for now just effected by 1 bone - later blend everything here
    int     index                    = int(input.index);
    float4  weightedVertexPosition   = jointPositions[index];
    float4  weightedVertexQuaternion = jointQuaternions[index];


    float3 tmpVec = 2.0 * cross(weightedVertexQuaternion.xyz, input.position.xyz);
    float4 position = float4(weightedVertexPosition.xyz + (input.position.xyz + weightedVertexQuaternion.w * tmpVec + cross(weightedVertexQuaternion.xyz, tmpVec))*weightedVertexPosition.w, 1.0);

    tmpVec = 2.0 * cross(weightedVertexQuaternion.xyz, input.normal.xyz);    
    float3 normal = input.normal + weightedVertexQuaternion.w * tmpVec + cross(weightedVertexQuaternion.xyz, tmpVec);    
#else
    float4 position = float4(input.position.xyz, 1.0);
    float3 normal = input.normal;
#endif

    normal = normalize(mul(float4(normal, 0.0), worldViewInvTransposeMatrix).xyz);
    float4 PosView = mul(position, worldViewMatrix);

    float distanceScale = length(PosView.xyz) / 100.0;

    PosView.xyz += normal * pow(silhouetteScale * distanceScale, silhouetteExponent);
    output.position = mul(PosView, projMatrix);
    
    return output;
}
