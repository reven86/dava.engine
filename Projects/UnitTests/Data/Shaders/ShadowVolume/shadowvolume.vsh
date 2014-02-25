attribute vec4 inPosition;
attribute vec3 inNormal;

uniform mat4 modelViewProjectionMatrix;
uniform mat4 modelViewMatrix;
uniform mat4 projectionMatrix;
uniform mat3 normalMatrix;

uniform vec3 lightPosition0;

varying vec3 clr;

void main()
{
	vec3 normal = normalMatrix * inNormal.xyz;
	vec4 PosView = modelViewMatrix * inPosition;
	
	vec3 LightVecView = PosView.xyz - lightPosition0;

	if(dot(normal, -LightVecView) < 0.0)
	{
		if(PosView.z < lightPosition0.z)
		{
			PosView.xyz -= LightVecView * (5000.0 - 10.0 + PosView.z) / LightVecView.z;
		}
		else
		{
			PosView = vec4(LightVecView, 0.0);
		}
		gl_Position = projectionMatrix * PosView;
	}
	else
	{
		gl_Position = modelViewProjectionMatrix * inPosition;
	}

	//gl_Position = modelViewProjectionMatrix * inPosition;
	clr = inNormal;
}