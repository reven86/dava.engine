#include "common.slh"

vertex_in
{
    float3  pos     : POSITION;
    float3  normal  : NORMAL;

    #if SKINNING
    float   index   : BLENDINDICES;
    #endif
};

vertex_out
{
    float4  pos     : SV_POSITION;        
};

[auto][instance] property float4x4 worldViewMatrix;
[auto][instance] property float4x4 projMatrix;
[auto][instance] property float4x4 worldViewInvTransposeMatrix;

[auto][instance] property float4 lightPosition0;

#if SKINNING
[auto][jpos] property float4 jointPositions[MAX_JOINTS] : "bigarray"; // (x, y, z, scale)
[auto][jrot] property float4 jointQuaternions[MAX_JOINTS] : "bigarray";
#endif

inline float3 JointTransformTangent( float3 inVec, float4 jointQuaternion )
{
    float3 t = 2.0 * cross( jointQuaternion.xyz, inVec );
    return inVec + jointQuaternion.w * t + cross(jointQuaternion.xyz, t); 
}

vertex_out vp_main( vertex_in input )
{
    vertex_out  output;

    float3 in_pos      = input.pos.xyz;
    float3 in_normal   = input.normal;

    float3x3 normalMatrix = float3x3(worldViewInvTransposeMatrix[0].xyz, 
                                     worldViewInvTransposeMatrix[1].xyz, 
                                     worldViewInvTransposeMatrix[2].xyz);

#if SKINNING
    // compute final state - for now just effected by 1 bone - later blend everything here
    int     index                    = int(input.index);
    float4  weightedVertexPosition   = jointPositions[index];
    float4  weightedVertexQuaternion = jointQuaternions[index];
    float3 tmpVec = 2.0 * cross(weightedVertexQuaternion.xyz, in_pos.xyz);
    float4 position = float4(weightedVertexPosition.xyz + (in_pos.xyz + weightedVertexQuaternion.w * tmpVec + cross(weightedVertexQuaternion.xyz, tmpVec))*weightedVertexPosition.w, 1.0);
    float3 normal = normalize( mul( JointTransformTangent(in_normal, weightedVertexQuaternion), normalMatrix ) );
#else
    float4 position = float4(in_pos.x, in_pos.y, in_pos.z, 1.0);
    float3 normal = mul( in_normal, normalMatrix );
#endif

    float4 posView = mul( position, worldViewMatrix );
    float3 lightVecView = normalize(posView.xyz * lightPosition0.w - lightPosition0.xyz);        

    float4 posProj = mul(posView, projMatrix);
    
    if (dot(normal, lightVecView) > 0.0)    
    {
    
        float g = 1.45; //guardband extrusion limit
        float depthExtrusion = 200.0; //depth extrusion limit - for now just const, but it can be possibly computed on cpu as distance to landscape
                
        //compute projected space vector
        float4 lightProj = mul(float4(lightVecView, 0.0), projMatrix);    
        
        //compute extrusion value to hit guardband 
        float2 sl = sign(lightProj.xy);
        float2 evRes = (posProj.w*g*sl - posProj.xy)/(lightProj.xy - lightProj.w*g*sl);
        
        //negative component means that ray will not hit selected plane (as frustum is not a cube!)
        //equal to per-compound if (evRes < 0.0) evRes = depthExtrusion;            
        evRes = lerp(evRes, float2(depthExtrusion, depthExtrusion), step(evRes, float2(0.0, 0.0)));
        
        //select nearest hit between planes
        float ev = min(evRes.x, evRes.y);                        
            
        //extrude to corresponding value
        output.pos = posProj+lightProj*ev;        
    }       
    else
    {                
        output.pos = posProj;                
    }    
    
    


    return output; 
};
