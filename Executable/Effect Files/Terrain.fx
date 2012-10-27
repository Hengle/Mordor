//==========================================================================================
// 
// Terrain.fx by Elinor Townsend
// Based on tex.fx and the terrain.fx examples by Frank Luna (C) 2008 All Rights Reserved.
//
//==========================================================================================
 
#include "SimplexNoise.fx"
#include "Light.fx"

#define ROCK 0
#define LAVA 1

cbuffer cbPerObject
{
	float4x4 World;
	float4x4 Wvp;
};

cbuffer cbPerFrame
{	
	float3 cameraPos;
	float3 fogColour;
};

Light parallelLight;

cbuffer cbFog
{
	float fogStart = 300.0f;
	float fogRange = 300.0f;
};

cbuffer cbHeatHaze
{
	float hazeStart = 150.0f;
	float hazeRange = 500.0f;	
};

SamplerState TriLinearSample
{
	Filter = MIN_MAG_MIP_LINEAR;
	AddressU = Wrap;
	AddressV = Wrap;
};

RasterizerState rsWireframe 
{ 
	FillMode = WireFrame; 
	CullMode = None;
};

cbuffer TextureColours
{
	float4 rock		= {0.25f,  0.2f, 0.25f,  1.0f};
	float4 lava		= {1.0f,  0.2f,  0.0f,  1.0f};
};

struct VS_IN
{
	float3 posL		: POSITION;
	float3 normalL	: NORMAL;
	float2 texC		: TEXCOORD;
	uint   type     : TYPE;
};

struct VS_OUT
{
	float4 posH			: SV_POSITION;
    float3 normalW		: NORMAL;
	float2 texC			: TEXCOORD;
	float  fogLerp		: FOG;
	float hazeLerp		: HAZE;
	uint   type			: TYPE;
	float3 cameraView	: VIEW;
};
 
VS_OUT VS(VS_IN vIn)
{
	VS_OUT vOut;
	
	// Transform to world space space
	vOut.normalW = mul(vIn.normalL, World);

	// Transform to homogeneous clip space
	vOut.posH = mul(float4(vIn.posL, 1.0f), Wvp);

	vOut.texC = vIn.texC;

	// Fog
	float3 posW	= mul(vIn.posL, World);
	float d = distance(posW, cameraPos);
	vOut.fogLerp = saturate((d - fogStart) / fogRange);

	// Heat haze
	vOut.hazeLerp = saturate((d - hazeStart) / hazeRange);

	[branch]
	if (vIn.type == LAVA)
	{
		vOut.hazeLerp = saturate(vOut.hazeLerp + 0.5f);
	}
	
	vOut.type = vIn.type;

	

	// Find the camera view vector
	vOut.cameraView = normalize(cameraPos - posW);
	
	return vOut;
}

float4 PS(VS_OUT pIn) : SV_Target
{
	float4 colour = rock * turbulence(float3(pIn.texC.x, pIn.texC.y, 0)*2, 10);
	float4 hot = lava * ridgedMultifractal(float3(pIn.texC.x, pIn.texC.y, 0)*25, 5);

	[branch]
	if (pIn.type == LAVA)
	{
		colour = hot;
	}

	float3 normal = normalize(pIn.normalW);

	// Blend the fog color and the shade * diffuse component
	float3 foggedColour = lerp(colour + ParallelLight(parallelLight, normal, pIn.cameraView), fogColour, pIn.fogLerp);
	
	return float4(foggedColour, pIn.hazeLerp);
}

technique10 TexTech
{
    pass P0
    {
//      SetRasterizerState( rsWireframe );
		SetVertexShader( CompileShader( vs_4_0, VS() ) );
        SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_4_0, PS() ) );
    }
}
