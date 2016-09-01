#include "common.slh"

vertex_in
{
    float4  pos     : POSITION;
    float3  normal  : NORMAL;

    #if SKINNING
    float   index   : BLENDINDICES;
    #endif
};

vertex_out
{
    float4  pos     : SV_POSITION;
};

[dynamic][instance] property float4x4 worldViewProjMatrix;
[dynamic][instance] property float4x4 worldViewMatrix;
[dynamic][instance] property float4x4 projMatrix;
[dynamic][instance] property float4x4 worldViewInvTransposeMatrix;

[dynamic][instance] property float4 lightPosition0;

#if SKINNING
[dynamic][skin1][bigarray] property float4 jointPositions[MAX_JOINTS]; // (x, y, z, scale)
[dynamic][skin2][bigarray] property float4 jointQuaternions[MAX_JOINTS];
#endif

inline float3
JointTransformTangent( float3 inVec, float4 jointQuaternion )
{
    float3 t = 2.0 * cross( jointQuaternion.xyz, inVec );
    return inVec + jointQuaternion.w * t + cross(jointQuaternion.xyz, t); 
}

vertex_out
vp_main( vertex_in input )
{
    vertex_out  output;

    float3 in_pos      = input.pos.xyz;
    float3 in_normal   = input.normal;
    
#if SKINNING
    // compute final state - for now just effected by 1 bone - later blend everything here
    int     index                    = int(input.index);
    float4  weightedVertexPosition   = jointPositions[index];
    float4  weightedVertexQuaternion = jointQuaternions[index];
#endif

    float3x3 normalMatrix = float3x3(worldViewInvTransposeMatrix[0].xyz, 
                                     worldViewInvTransposeMatrix[1].xyz, 
                                     worldViewInvTransposeMatrix[2].xyz);

#if SKINNING
    float3 tmpVec = 2.0 * cross(weightedVertexQuaternion.xyz, in_pos.xyz);
    float4 position = float4(weightedVertexPosition.xyz + (in_pos.xyz + weightedVertexQuaternion.w * tmpVec + cross(weightedVertexQuaternion.xyz, tmpVec))*weightedVertexPosition.w, 1.0);
    float3 normal = normalize( mul( JointTransformTangent(in_normal, weightedVertexQuaternion), normalMatrix ) );
#else
    float4 position = float4(in_pos.x, in_pos.y, in_pos.z, 1.0);
    float3 normal = mul( in_normal, normalMatrix );
#endif
    
    float4 posView = mul( position, worldViewMatrix );
    float3 lightVecView = posView.xyz * lightPosition0.w - lightPosition0.xyz;
    
    if(dot(normal, lightVecView) >= 0.0)
    {
        if(posView.z * lightPosition0.w < lightPosition0.z)
        {
            posView.xyz -= lightVecView * (1000.0 - 10.0 + posView.z) / lightVecView.z;
        }
        else
        {
            posView = float4(lightVecView, 0.0);
        }
        output.pos = mul( posView, projMatrix );
    }
    else
    {
        output.pos = mul( position, worldViewProjMatrix );
    }

    return output; 
};
