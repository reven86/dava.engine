#define VPROG_IN_BEGIN          struct VP_Input {
#define VPROG_IN_POSITION float3 position : POSITION0;
#define VPROG_IN_NORMAL float3 normal : NORMAL0;
#define VPROG_IN_TEXCOORD float2 texcoord : TEXCOORD0;
#define VPROG_IN_COLOR float4 color : COLOR0;
#define VPROG_IN_END            };

#define VPROG_OUT_BEGIN         struct VP_Output {
#define VPROG_OUT_POSITION float4 position : POSITION0;
#define VPROG_OUT_TEXCOORD0(name, size) float##size name : TEXCOORD0;
#define VPROG_OUT_TEXCOORD1(name, size) float##size name : TEXCOORD1;
#define VPROG_OUT_COLOR0(name, size) float##size name : COLOR0;
#define VPROG_OUT_END           };

#define DECL_VPROG_BUFFER(idx, sz) uniform float4 VP_Buffer##idx[sz];
#define VP_BUF_FLOAT3X3(buf, reg) float3x3((float3)(float4(VP_Buffer##buf[reg + 0])), (float3)(float4(VP_Buffer##buf[reg + 1])), (float3)(float4(VP_Buffer##buf[reg + 2])));

#define VPROG_BEGIN             VP_Output vp_main(VP_Input IN) { VP_Output OUT;
#define VPROG_END               return OUT; }

#define VP_IN_POSITION IN.position
#define VP_IN_NORMAL IN.normal
#define VP_IN_TEXCOORD IN.texcoord
#define VP_IN_COLOR IN.color

#define VP_OUT_POSITION OUT.position
#define VP_OUT(name) OUT.##name


#define FPROG_IN_BEGIN          struct FP_Input {
#define FPROG_IN_TEXCOORD0(name, size) float##size name : TEXCOORD0;
#define FPROG_IN_TEXCOORD1(name, size) float##size name : TEXCOORD1;
#define FPROG_IN_COLOR0(name, size) float##size name : COLOR0;
#define FPROG_IN_END            };

#define FPROG_OUT_BEGIN         struct FP_Output {
#define FPROG_OUT_COLOR float4 color : COLOR0;
#define FPROG_OUT_END           };

#define DECL_SAMPLER2D(unit) uniform sampler2D Texture##unit;
   
#define FP_TEXTURE2D(unit, uv) tex2D(Texture##unit, uv);
#define FP_IN(name) IN.##name

#define FP_OUT_COLOR OUT.color

#define DECL_FPROG_BUFFER(idx, sz) uniform float4 FP_Buffer##idx[sz];

#define FPROG_BEGIN             FP_Output fp_main(FP_Input IN) { FP_Output OUT;
#define FPROG_END               return OUT; }
