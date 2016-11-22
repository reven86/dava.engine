#define VPROG_IN_BEGIN          
#define VPROG_IN_POSITION attribute vec3 attr_position;
#define VPROG_IN_NORMAL attribute vec3 attr_normal;
#define VPROG_IN_TEXCOORD attribute vec2 attr_texcoord;
#define VPROG_IN_COLOR attribute vec4 attr_color;
#define VPROG_IN_END            

#define VPROG_OUT_BEGIN         
#define VPROG_OUT_POSITION      
#define VPROG_OUT_TEXCOORD0(name, size) varying vec##size var_##name;
#define VPROG_OUT_TEXCOORD1(name, size) varying vec##size var_##name;
#define VPROG_OUT_COLOR0(name, size) varying vec##size var_##name;
#define VPROG_OUT_END           

#define DECL_VPROG_BUFFER(idx, sz) uniform vec4 VP_Buffer##idx[sz];
#define VP_BUF_FLOAT3X3(buf, reg) float3x3(float3(VP_Buffer##buf[reg + 0]), float3(VP_Buffer##buf[reg + 1]), float3(VP_Buffer##buf[reg + 2]));

#define VPROG_BEGIN             void main() {
#define VPROG_END }

#define VP_IN_POSITION attr_position
#define VP_IN_NORMAL attr_normal
#define VP_IN_TEXCOORD attr_texcoord
#define VP_IN_COLOR attr_color

#define VP_OUT_POSITION gl_Position
#define VP_OUT(name) var_##name


#define FPROG_IN_BEGIN          
#define FPROG_IN_TEXCOORD0(name, size) varying vec##size var_##name;
#define FPROG_IN_TEXCOORD1(name, size) varying vec##size var_##name;
#define FPROG_IN_COLOR0(name, size) varying vec##size var_##name;
#define FPROG_IN_END            

#define FPROG_OUT_BEGIN         
#define FPROG_OUT_COLOR         
#define FPROG_OUT_END           
           
#define DECL_SAMPLER2D(unit) uniform sampler2D Texture##unit;
       
#define FP_TEXTURE2D(unit, uv) texture2D(Texture##unit, uv);
#define FP_IN(name) var_##name

#define FP_OUT_COLOR gl_FragColor

#define DECL_FPROG_BUFFER(idx, sz) uniform vec4 FP_Buffer##idx[sz];

#define FPROG_BEGIN             void main() {
#define FPROG_END }






#define float2 vec2
#define float3 vec3
#define float4 vec4
#define float4x4 mat4
#define float3x3 mat3

vec4 mul(vec4 v, mat4 m)
{
    return m * v;
}
vec4 mul(mat4 m, vec4 v)
{
    return v * m;
}
vec3 mul(vec3 v, mat3 m)
{
    return m * v;
}

float4 tex2D(sampler2D s, vec2 t)
{
    return texture2D(s, t);
}
