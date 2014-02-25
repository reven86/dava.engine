#ifdef GL_ES
precision highp float;
#endif

uniform sampler2D texture0;
uniform sampler2D texture1;
uniform int colorType;
uniform float intensity;

varying vec2 varTexCoord0;

void main()
{
	vec4 colorMaskOld = texture2D(texture0, varTexCoord0);
	vec4 outColor = vec4(1.0, 0.0, 1.0, 1.0);
	
	if(0 == colorType)//r
	{
		outColor.r = colorMaskOld.r+texture2D(texture1, varTexCoord0).r*intensity;
		outColor.r = min(outColor.r, 1.0);
		float freeColors = 1.0-outColor.r;
		float usedColors = colorMaskOld.g+colorMaskOld.b+colorMaskOld.a;
		float div = usedColors/freeColors; // /0?
		outColor.gba = colorMaskOld.gba/div;
	}
	else if(1 == colorType)//g
	{
		outColor.g = colorMaskOld.g+texture2D(texture1, varTexCoord0).r*intensity;
		outColor.g = min(outColor.g, 1.0);
		float freeColors = 1.0-outColor.g;
		float usedColors = colorMaskOld.r+colorMaskOld.b+colorMaskOld.a;
		float div = usedColors/freeColors; // /0?
		outColor.rba = colorMaskOld.rba/div;
	}
	else if(2 == colorType)//b
	{
		outColor.b = colorMaskOld.b+texture2D(texture1, varTexCoord0).r*intensity;
		outColor.b = min(outColor.b, 1.0);
		float freeColors = 1.0-outColor.b;
		float usedColors = colorMaskOld.r+colorMaskOld.g+colorMaskOld.a;
		float div = usedColors/freeColors; // /0?
		outColor.rga = colorMaskOld.rga/div;
	}
	else if(3 == colorType)//a
	{
		outColor.a = colorMaskOld.a+texture2D(texture1, varTexCoord0).r*intensity;
		outColor.a = min(outColor.a, 1.0);
		float freeColors = 1.0-outColor.a;
		float usedColors = colorMaskOld.r+colorMaskOld.g+colorMaskOld.b;
		float div = usedColors/freeColors; // /0?
		outColor.rgb = colorMaskOld.rgb/div;
	}
	
	
	gl_FragColor = outColor;
}