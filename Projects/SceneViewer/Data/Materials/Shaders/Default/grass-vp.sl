#include "common.slh"

vertex_in
{
    float4  position    : POSITION;
    float2  uv0         : TEXCOORD0;
    float3  uv1         : TEXCOORD1;
    float3  uv2         : TEXCOORD2;
};

vertex_out
{
    float4  position    : SV_POSITION;
    float2  texCoord    : TEXCOORD0;
    half3   vegetationColor :COLOR0;
    #if VERTEX_FOG
    float4  varFog      : TEXCOORD5;
    #endif
};

uniform sampler2D heightmap;
uniform sampler2D vegetationmap;

#if VERTEX_FOG
[dynamic][a] property float4x4 worldViewMatrix;
#endif
#if VERTEX_FOG && FOG_ATMOSPHERE
[dynamic][a] property float4x4 worldViewInvTransposeMatrix;
[dynamic][a] property float4 lightPosition0;
#if DISTANCE_ATTENUATION
[statik][a] property float lightIntensity0; 
#endif
#endif

#if VERTEX_FOG
[dynamic][a] property float3 cameraPosition;
[dynamic][instance] property float4x4 worldMatrix;
#endif

#include "vp-fog-props.slh"

[dynamic][instance] property float4x4 worldViewProjMatrix;
[dynamic][a] property float heightmapTextureSize;
    
[statik][a] property float3 tilePos;
[statik][a] property float3 worldSize;
[statik][a] property float2 lodSwitchScale;
[statik][a] property float4 vegWaveOffsetx;
[statik][a] property float4 vegWaveOffsety;
//8 floats: xxxxyyyy (xy per layer)

vertex_out
vp_main( vertex_in input )
{
    vertex_out  output;

    float3 inPosition = input.position.xyz;
    float2 inTexCoord0 = input.uv0;
    float3 inTexCoord1 = input.uv1;
    float3 inTexCoord2 = input.uv2;
    
    output.texCoord = inTexCoord0;
    
    //inTexCoord1.y - cluster type (0...3)
    //inTexCoord1.z - cluster's reference density (0...15)

    float3 clusterCenter = float3(inTexCoord2.x + tilePos.x, inTexCoord2.y + tilePos.y, inTexCoord2.z);
    
    float2 uv = 0.5 - clusterCenter.xy / worldSize.xy;
    float2 uvColor = float2(1.0 - uv.x, uv.y);
    float2 uvHeight = float2(uvColor.x, 1.0 - uv.y) + 0.5 / heightmapTextureSize;
            
    float4 heightVec = tex2Dlod(heightmap, uvHeight, 0.0);
    float height = dot(heightVec, float4(0.00022888532845, 0.00366216525521, 0.05859464408331, 0.93751430533303)) * worldSize.z;

    float3 pos = float3(inPosition.x + tilePos.x, inPosition.y + tilePos.y, inPosition.z);
    pos.z += height;
    clusterCenter.z += height;

    float clusterScale = tilePos.z;
    if(int(inTexCoord1.x) == int(lodSwitchScale.x))
    {
        clusterScale *= lodSwitchScale.y;
    }

    float4 vegetationMask = tex2Dlod( vegetationmap, uvColor, 0.0 );
    
    output.vegetationColor = half3(vegetationMask.rgb);
    
    //wave transform
    int waveIndex = int(inTexCoord1.y);
    
    pos.x += inTexCoord1.z * vegWaveOffsetx[waveIndex];
    pos.y += inTexCoord1.z * vegWaveOffsety[waveIndex];
    
    pos = lerp(clusterCenter, pos, vegetationMask.a * clusterScale);
    output.position = mul( float4(pos, 1.0), worldViewProjMatrix );

#if VERTEX_FOG
    
    float3 eyeCoordsPosition = mul( float4( pos, 1.0 ), worldViewMatrix ).xyz;
    #define FOG_view_position eyeCoordsPosition
    
#if FOG_ATMOSPHERE
    float3 tolight_dir = lightPosition0.xyz - eyeCoordsPosition * lightPosition0.w;
    #define FOG_to_light_dir tolight_dir
#endif
    
#if FOG_HALFSPACE || FOG_ATMOSPHERE_MAP
    float3 world_position = mul( float4( pos, 1.0 ), worldMatrix ).xyz;
    #define FOG_world_position world_position
#endif

    #define FOG_eye_position cameraPosition

    #include "vp-fog-math.slh" // in{ float3 FOG_view_position, float3 FOG_eye_position, float3 FOG_to_light_dir, float3 FOG_world_position }; out{ float4 FOG_result };
    
    output.varFog = FOG_result;
    
#endif

    return output;
}
