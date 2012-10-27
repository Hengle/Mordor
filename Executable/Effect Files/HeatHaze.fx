//==========================================================================================
// 
// HeatHaze.fx by Elinor Townsend
//
//==========================================================================================

cbuffer cbPerObject
{
	float4x4 Orthogonal;
	float time;
};

cbuffer cbFixed 
{
	float distortionFactor = 0.01f;
};

Texture2D screenTexture;    
Texture2D perturbanceTexture;

SamplerState NoiseSample
{
	Filter = MIN_MAG_MIP_LINEAR;
	AddressU = WRAP;
	AddressV = WRAP;
};

SamplerState SceneSample
{
	Filter = MIN_MAG_MIP_LINEAR;
	AddressU = CLAMP;
	AddressV = CLAMP;
};

struct VS_IN
{
    float3 position : POSITION;
    float2 texCoord : TEXCOORD;
};

struct VS_OUT
{
    float4 position : SV_POSITION;
    float2 texCoord : TEXCOORD;
};

VS_OUT HazeVS(VS_IN vIn)
{	
	VS_OUT vOut;
	
    // Transform to homogeneous clip space
	// Calculate the position of the vertex against the world, view, and projection matrices.
    vOut.position = mul(float4(vIn.position, 1.0f), Orthogonal);

    vOut.texCoord = vIn.texCoord;

	return vOut;
}

float4 HazePS(VS_OUT pIn) : SV_Target 
{
	float4 distortion = perturbanceTexture.Sample(NoiseSample, pIn.texCoord + cos(time * 3.14159265358979323846f));
	distortion = normalize(distortion) * distortionFactor;
	
	float4 refract = screenTexture.Sample(SceneSample, pIn.texCoord + distortion.xy * 0.5f);
	
	float4 scene = screenTexture.Sample(SceneSample, pIn.texCoord);
	
	float alpha = clamp(scene.a, 0, 1);
	
	float4 output = refract * alpha + scene * (1 - alpha);
	
	return  output;
}

technique10 HeatHazeTech
{
	pass p0
	{		
		SetVertexShader( CompileShader( vs_4_0, HazeVS() ) );
        SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_4_0, HazePS() ) );
	}
}