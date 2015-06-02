attribute vec4 inPosition;
attribute vec3 inNormal;

uniform mat4 worldViewProjMatrix;
uniform mat4 worldViewMatrix;
uniform mat4 projMatrix;
uniform mat3 worldViewInvTransposeMatrix;

uniform vec4 lightPosition0;

void main()
{
	vec3 normal = worldViewInvTransposeMatrix * inNormal.xyz;
	vec4 posView = worldViewMatrix * inPosition;
	
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
		gl_Position = worldViewProjMatrix * inPosition;
	}

}