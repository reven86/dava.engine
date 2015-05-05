VPROG_IN_BEGIN
    VPROG_IN_POSITION
    VPROG_IN_TEXCOORD
VPROG_IN_END

VPROG_OUT_BEGIN
    VPROG_OUT_POSITION
    VPROG_OUT_TEXCOORD0(uv,2)
VPROG_OUT_END

property float4x4 worldViewProjMatrix : unique,dynamic : ui_hidden=yes ;

VPROG_BEGIN
    float3 in_pos      = VP_IN_POSITION;
    float2 in_texcoord = VP_IN_TEXCOORD;
    VP_OUT(uv) =  in_texcoord;
#if defined(SKYOBJECT)
	float4x4 mwpWOtranslate = float4x4(worldViewProjMatrix[0], worldViewProjMatrix[1], worldViewProjMatrix[2], float4(0.0, 0.0, 0.0, 1.0));
	float4 vecPos = mul(float4(in_pos.x,in_pos.y,in_pos.z,1.0), mwpWOtranslate);
	VP_OUT_POSITION = float4(vecPos.xy, vecPos.w - 0.0001, vecPos.w);
#else
    VP_OUT_POSITION   = mul( float4(in_pos.x,in_pos.y,in_pos.z,1.0), worldViewProjMatrix );
#endif
VPROG_END;
