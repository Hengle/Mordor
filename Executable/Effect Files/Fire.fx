//=============================================================================
// Fire.fx by Elinor Townsend
//
// Fire sparks particle system.  Particles are emitted directly in world space.
//=============================================================================


//***********************************************
// GLOBALS                                      *
//***********************************************

cbuffer cbPerFrame
{
	float4 cameraPos;
	float4 emitPos;
	float4 emitDir;
	
	float sceneTime;
	float timeStep;
	float4x4 viewProj; 
};

cbuffer cbFixed
{
	// Net constant acceleration used to accerlate the particles.
	float3 accelW = {0.0f, -12.8f, 0.0f};
};
 
// Array of textures for texturing the particles.
Texture2DArray texArray;

// Random texture used to generate random numbers in shaders.
Texture1D randomTex;

// Heightmap for the terrain
Texture2D terrainHeightMap;
 
SamplerState TriLinearSample
{
	Filter = MIN_MAG_MIP_LINEAR;
	AddressU = WRAP;
	AddressV = WRAP;
};

DepthStencilState DisableDepth
{
    DepthEnable = FALSE;
    DepthWriteMask = ZERO;
};

DepthStencilState NoDepthWrites
{
    DepthEnable = TRUE;
    DepthWriteMask = ZERO;
};


//***********************************************
// HELPER FUNCTIONS                             *
//***********************************************
float3 RandUnitVec3(float offset)
{
	// Use game time plus offset to sample random texture.
	float u = (sceneTime + offset);
	
	// coordinates in [-1,1]
	float3 v = randomTex.SampleLevel(TriLinearSample, u, 0);
	
	// project onto unit sphere
	return normalize(v);
}

float3 RandVec3(float offset)
{
	// Use game time plus offset to sample random texture.
	float u = (sceneTime + offset);
	
	// coordinates in [-1,1]
	float3 v = randomTex.SampleLevel(TriLinearSample, u, 0);
	
	return v;
}
 
//***********************************************
// STREAM-OUT TECH                              *
//***********************************************

#define PT_EMITTER 0
#define PT_FLARE 1
 
struct Particle
{
	float3 posW		: POSITION;
	float3 velW		: VELOCITY;
	float2 sizeW    : SIZE;
	float age       : AGE;
	uint type       : TYPE;
};
  
Particle StreamOutVS(Particle vIn)
{
	return vIn;
}

// The stream-out GS is just responsible for emitting 
// new particles and destroying old particles.  The logic
// programed here will generally vary from particle system
// to particle system, as the destroy/spawn rules will be 
// different.
[maxvertexcount(6)]
void StreamOutGS(point Particle gIn[1], 
                 inout PointStream<Particle> ptStream)
{	
	gIn[0].age += timeStep;
	
	if( gIn[0].type == PT_EMITTER )
	{	
		// time to emit a new particle?
		if( gIn[0].age > 0.25f )
		{
			for(int i = 0; i < 5; ++i)
			{
				// Give the fire sparks a random initial velocity
				float3 velRandom = RandUnitVec3(i);
				velRandom.x *= 7.5f;
				velRandom.z *= 7.5f;
				velRandom.y += 40.0f;

				Particle p;
				p.posW	= emitPos.xyz;
				p.posW.y += 10;
				p.velW	= velRandom;
				p.sizeW = float2(1.0f, 1.0f);
				p.age	= 0.0f;
				p.type  = PT_FLARE;
			
				ptStream.Append(p);
			}
			
			// reset the time to emit
			gIn[0].age = 0.0f;
		}
		
		// always keep emitters
		ptStream.Append(gIn[0]);
	}
	else
	{
		// Update position using the constant acceleration equation
		gIn[0].posW += 0.5f * timeStep * timeStep * accelW + timeStep * gIn[0].velW;
		gIn[0].velW += timeStep * accelW;

		// Specify conditions to keep particle; this may vary from system to system.
		if (gIn[0].age <= 5.0f)
			ptStream.Append(gIn[0]);
	}		
}

GeometryShader gsStreamOut = ConstructGSWithSO( 
	CompileShader( gs_4_0, StreamOutGS() ), 
	"POSITION.xyz; VELOCITY.xyz; SIZE.xy; AGE.x; TYPE.x" );
	
technique10 StreamOutTech
{
    pass P0
    {
        SetVertexShader( CompileShader( vs_4_0, StreamOutVS() ) );
        SetGeometryShader( gsStreamOut );
        
        // disable pixel shader for stream-out only
        SetPixelShader(NULL);
        
        // we must also disable the depth buffer for stream-out only
        SetDepthStencilState( DisableDepth, 0 );
    }
}

//***********************************************
// DRAW TECH                                    *
//***********************************************

struct VS_OUT
{
	float3 posW  : POSITION;
	float3 velW	 : VELOCITY;
	uint   type  : TYPE;
};

VS_OUT DrawVS(Particle vIn)
{
	VS_OUT vOut;

	vOut.posW = vIn.posW;
	vOut.velW = vIn.velW;
	vOut.type  = vIn.type;
	
	return vOut;
}

struct GS_OUT
{
	float4 posH  : SV_Position;
	float2 texC  : TEXCOORD;
};

// The draw GS just expands points into lines.
[maxvertexcount(2)]
void DrawGS(point VS_OUT gIn[1], 
            inout LineStream<GS_OUT> lineStream)
{	
	// do not draw emitter particles.
	if( gIn[0].type != PT_EMITTER )
	{
		// Slant line in velocity direction.
		float3 p0 = gIn[0].posW;
		float3 p1 = gIn[0].posW + 0.15f * gIn[0].velW;
		
		GS_OUT v0;
		v0.posH = mul(float4(p0, 1.0f), viewProj);
		v0.texC = float2(0.0f, 0.0f);
		lineStream.Append(v0);
		
		GS_OUT v1;
		v1.posH = mul(float4(p1, 1.0f), viewProj);
		v1.texC = float2(1.0f, 1.0f);
		lineStream.Append(v1);
	}
}

float4 DrawPS(GS_OUT pIn) : SV_TARGET
{
	return texArray.Sample(TriLinearSample, float3(pIn.texC, 0));
}

technique10 DrawTech
{
    pass P0
    {
        SetVertexShader(   CompileShader( vs_4_0, DrawVS() ) );
        SetGeometryShader( CompileShader( gs_4_0, DrawGS() ) );
        SetPixelShader(    CompileShader( ps_4_0, DrawPS() ) );
        
        SetDepthStencilState( NoDepthWrites, 0 );
    }
}