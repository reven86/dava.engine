attribute vec4 inPosition;
attribute vec3 inNormal;

uniform mat4 worldViewProjMatrix;
uniform mat4 worldViewMatrix;
uniform mat4 projMatrix;
uniform mat3 worldViewInvTransposeMatrix;

uniform vec3 lightPosition0;

void main()
{
	vec3 normal = worldViewInvTransposeMatrix * inNormal.xyz;
	vec4 PosView = worldViewMatrix * inPosition;
	
	vec3 LightVecView = PosView.xyz - lightPosition0;

	if(dot(normal, -LightVecView) < 0.0)
	{
		if(PosView.z < lightPosition0.z)
		{
			PosView.xyz -= LightVecView * (1000.0 - 10.0 + PosView.z) / LightVecView.z;
		}
		else
		{
			PosView = vec4(LightVecView, 0.0);
		}
		gl_Position = projMatrix * PosView;
	}
	else
	{
		gl_Position = worldViewProjMatrix * inPosition;
	}

}