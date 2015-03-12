#define MAX_JOINTS 32

attribute vec4 inPosition;
attribute vec3 inNormal;

#if defined (SKINNING)
	attribute vec4 inJointIndex;
	attribute vec4 inJointWeight;
#endif

uniform mat4 worldViewProjMatrix;
uniform mat4 worldViewMatrix;
uniform mat4 projMatrix;
uniform mat3 worldViewInvTransposeMatrix;

uniform vec4 lightPosition0;

#if defined (SKINNING)
uniform vec4 jointPositions[MAX_JOINTS]; // (x, y, z, scale)
uniform vec4 jointQuaternions[MAX_JOINTS];    
#endif

vec3 JointTransform(vec3 inVec, vec4 jointPosition, vec4 jointQuaternion)
{
	vec3 t = 2.0 * cross(jointQuaternion.xyz, inVec);
	return jointPosition.xyz + (inVec + jointQuaternion.w * t + cross(jointQuaternion.xyz, t))*jointPosition.w; 
}

vec3 JointTransformTangent(vec3 inVec, vec4 jointQuaternion)
{
	vec3 t = 2.0 * cross(jointQuaternion.xyz, inVec);
	return inVec + jointQuaternion.w * t + cross(jointQuaternion.xyz, t); 
}

void main()
{
#if defined (SKINNING)
    int index = int(inJointIndex);
    vec4 weightedVertexPosition = jointPositions[index];
    vec4 weightedVertexQuaternion = jointQuaternions[index];
#endif

#if defined (SKINNING)
	vec3 normal = normalize (worldViewInvTransposeMatrix * JointTransformTangent(inNormal, weightedVertexQuaternion));
	vec4 position = vec4(JointTransform(inPosition.xyz, weightedVertexPosition, weightedVertexQuaternion), inPosition.w);
#else
	vec3 normal = worldViewInvTransposeMatrix * inNormal.xyz;
	vec4 position = inPosition;
#endif

	vec4 posView = worldViewMatrix * position;
	
    // convert light position + direction into light direction
	vec3 lightVecView = posView.xyz * lightPosition0.w - lightPosition0.xyz;

	if(dot(normal, -lightVecView) < 0.0)
	{
		if(posView.z * lightPosition0.w < lightPosition0.z)
		{
			posView.xyz -= lightVecView * (1000.0 - 10.0 + posView.z) / lightVecView.z;
		}
		else
		{
			posView = vec4(lightVecView, 0.0);
		}
		gl_Position = projMatrix * posView;
	}
	else
	{
		gl_Position = worldViewProjMatrix * position;
	}

}