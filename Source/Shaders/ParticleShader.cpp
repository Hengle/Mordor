/*
	Created 	Elinor Townsend 2012
*/

/*	
	Name		Particle Shader
	Brief		Definition of Particle Shader Class inherited from Shader
*/

#include "Shaders\ParticleShader.hpp"
#include "Scene\Scene.hpp"
#include "ParticleSystem\Particle.hpp"
#include "Graphics\Vertex.hpp"
#include "Utilities\SimplexNoise.hpp"

/*
	Name		ParticleShader::initialise
	Syntax		ParticleShader::initialise()
	Return		bool - Returns true once initialised
	Brief		Initialises the shader
*/
bool ParticleShader::initialise()
{
	d3dDevice_ = Scene::instance()->getDevice();
	if (!d3dDevice_)
	{
		MessageBox(0, "Retrieving device - Failed", "Error", MB_OK);
		return false;
	}

	if (!createEffectFile(PARTICLE_FIRE))
		return false;

	streamOutTech_  = fx_->GetTechniqueByName("StreamOutTech");	
	drawTech_       = fx_->GetTechniqueByName("DrawTech");	
	viewProjVar_	= fx_->GetVariableByName("viewProj")->AsMatrix();
	sceneTimeVar_	= fx_->GetVariableByName("sceneTime")->AsScalar();
	timeStepVar_	= fx_->GetVariableByName("timeStep")->AsScalar();
	cameraPosVar_	= fx_->GetVariableByName("cameraPos")->AsVector();
	emitPosVar_		= fx_->GetVariableByName("emitPos")->AsVector();
	emitDirVar_		= fx_->GetVariableByName("emitDir")->AsVector();
	texArrayVar_	= fx_->GetVariableByName("texArray")->AsShaderResource();
	randomTexVar_	= fx_->GetVariableByName("randomTex")->AsShaderResource();
	heightMapVar_	= fx_->GetVariableByName("terrainHeightMap")->AsShaderResource();

	randomTexVar_->SetResource(SimplexNoise::createRandomTexture());

	if (!buildVertexLayout())
		return false;

	return true;
}

/*
	Name		ParticleShader::initialise
	Syntax		ParticleShader::initialise(Particle particle)
	Param		Particle particle - The particle effect file to use
	Return		bool - Returns true once initialised
	Brief		Initialises the shader
*/
bool ParticleShader::initialise(Particle particle)
{
	d3dDevice_ = Scene::instance()->getDevice();
	if (!d3dDevice_)
	{
		MessageBox(0, "Retrieving device - Failed", "Error", MB_OK);
		return false;
	}

	if (!createEffectFile(particle))
		return false;

	streamOutTech_  = fx_->GetTechniqueByName("StreamOutTech");	
	drawTech_       = fx_->GetTechniqueByName("DrawTech");	
	viewProjVar_	= fx_->GetVariableByName("viewProj")->AsMatrix();
	sceneTimeVar_	= fx_->GetVariableByName("sceneTime")->AsScalar();
	timeStepVar_	= fx_->GetVariableByName("timeStep")->AsScalar();
	cameraPosVar_	= fx_->GetVariableByName("cameraPos")->AsVector();
	emitPosVar_		= fx_->GetVariableByName("emitPos")->AsVector();
	emitDirVar_		= fx_->GetVariableByName("emitDir")->AsVector();
	texArrayVar_	= fx_->GetVariableByName("texArray")->AsShaderResource();
	randomTexVar_	= fx_->GetVariableByName("randomTex")->AsShaderResource();
	heightMapVar_	= fx_->GetVariableByName("terrainHeightMap")->AsShaderResource();

	randomTexVar_->SetResource(SimplexNoise::createRandomTexture());

	if (!buildVertexLayout())
		return false;

	return true;
}

/*
	Name		ParticleShader::deinitialise
	Syntax		ParticleShader::deinitialise()
	Brief		Deinitialises the shader
*/
void ParticleShader::deinitialise()
{
}

/*
	Name		ParticleShader::setup
	Syntax		ParticleShader::setup(float sceneTime, float timeStep, 
										D3DXVECTOR4* cameraPos, 
										D3DXVECTOR4* emitPos, 
										D3DXVECTOR4* emitDir,
										ID3D10ShaderResourceView* texArrayRV)
	Param		float sceneTime - The current scene time
	Param		float timeStep - The change in time between frames
	Param		D3DXVECTOR4* cameraPos - The camera position
	Param		D3DXVECTOR4* emitPos - The particle system emitter position
	Param		D3DXVECTOR4* emitDir - The particle system emitter direction
	Param		ID3D10ShaderResourceView* texArrayRV - The resources used by 
				the particles in the system
	Brief		Sets the variable values in the shader
*/
void ParticleShader::setup(float sceneTime, float timeStep, 
								 D3DXVECTOR4* cameraPos, D3DXVECTOR4* emitPos, 
								 D3DXVECTOR4* emitDir, 
								 ID3D10ShaderResourceView* texArrayRV, ID3D10ShaderResourceView* heightMap)
{
	D3DXMATRIX view = Scene::instance()->getView();
	D3DXMATRIX projection = Scene::instance()->getProjection();

	viewProjVar_->SetMatrix((float*)&(view * projection));
	sceneTimeVar_->SetFloat(sceneTime);
	timeStepVar_->SetFloat(timeStep);
	cameraPosVar_->SetFloatVector((float*)cameraPos);
	emitPosVar_->SetFloatVector((float*)emitPos);
	emitDirVar_->SetFloatVector((float*)emitDir);
	texArrayVar_->SetResource(texArrayRV);
	heightMapVar_->SetResource(heightMap);
}

/*
	Name		ParticleShader::setStreamOutTech
	Syntax		ParticleShader::setStreamOutTech(D3D10_TECHNIQUE_DESC* techDesc)
	Param		D3D10_TECHNIQUE_DESC* techDesc - The technique description for 
				the particle shader technique
	Brief		Sets the stream out technique description
*/
void ParticleShader::setStreamOutTech(D3D10_TECHNIQUE_DESC* techDesc)
{
	streamOutTech_->GetDesc(techDesc);
}

/*
	Name		ParticleShader::setDrawTech
	Syntax		ParticleShader::setDrawTech(D3D10_TECHNIQUE_DESC* techDesc)
	Param		D3D10_TECHNIQUE_DESC* techDesc - The technique description for 
				the particle shader technique
	Brief		Sets the draw technique description
*/
void ParticleShader::setDrawTech(D3D10_TECHNIQUE_DESC* techDesc)
{
	drawTech_->GetDesc(techDesc);
}

/*
	Name		ParticleShader::applyStreamOutPass
	Syntax		ParticleShader::applyStreamOutPass(UINT pass)
	Param		UINT pass - Pass number
	Brief		Sets the state contained in the pass of the stream out technique 
				to the device
*/
void ParticleShader::applyStreamOutPass(UINT pass)
{
	streamOutTech_->GetPassByIndex(pass)->Apply(0);
}

/*
	Name		ParticleShader::applyDrawPass
	Syntax		ParticleShader::applyDrawPass(UINT pass)
	Param		UINT pass - Pass number
	Brief		Sets the state contained in the pass of the draw technique to 
				the device
*/
void ParticleShader::applyDrawPass(UINT pass)
{
	drawTech_->GetPassByIndex(pass)->Apply(0);
}

/*
	Name		ParticleShader::createEffectFile
	Syntax		ParticleShader::createEffectFile(Particle particle)
	Param		Particle particle - The particle type
	Return		bool - True once the effect file has been created
	Brief		Creates the effect file based on the particle type passed
*/
bool ParticleShader::createEffectFile(Particle particle)
{
	ID3D10Blob* compilationErrors = 0;
	HRESULT hr = 0;
	switch (particle)
	{
	case PARTICLE_FIRE:
			hr = D3DX10CreateEffectFromFile("Effect Files/Fire.fx", 0, 0, 
											"fx_4_0", 
											D3D10_SHADER_ENABLE_STRICTNESS, 0, 
											d3dDevice_, 0, 0, &fx_, 
											&compilationErrors, 0);
			break;

	case PARTICLE_SMOKE:
			hr = D3DX10CreateEffectFromFile("Effect Files/Smoke.fx", 0, 0, 
											"fx_4_0", 
											D3D10_SHADER_ENABLE_STRICTNESS, 0, 
											d3dDevice_, 0, 0, &fx_, 
											&compilationErrors, 0);
			break;

	case PARTICLE_ASH:
			hr = D3DX10CreateEffectFromFile("Effect Files/Ash.fx", 0, 0, 
											"fx_4_0", 
											D3D10_SHADER_ENABLE_STRICTNESS, 0, 
											d3dDevice_, 0, 0, &fx_, 
											&compilationErrors, 0);
			break;

	default:
			return false;
	}

	if (FAILED(hr))
	{
		if (compilationErrors)
		{
			MessageBoxA(0, "Creating effect from file - Failed", "Error", MB_OK);
			compilationErrors->Release();
			return false;
		}
	} 
	return true;
}

/*
	Name		ParticleShader::buildVertexLayout
	Syntax		ParticleShader::buildVertexLayout()
	Return		bool - True once vertex layout has been built
	Brief		Initialises the effect file and variables for the shader
*/
bool ParticleShader::buildVertexLayout()
{
	// Build vertex layout
	D3D10_INPUT_ELEMENT_DESC layout[] =
	{
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,  0, 
							D3D10_INPUT_PER_VERTEX_DATA, 0},
		{"VELOCITY", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, 
							D3D10_INPUT_PER_VERTEX_DATA, 0},
		{"SIZE",     0, DXGI_FORMAT_R32G32_FLOAT,    0, 24, 
							D3D10_INPUT_PER_VERTEX_DATA, 0},
		{"AGE",      0, DXGI_FORMAT_R32_FLOAT,       0, 32, 
							D3D10_INPUT_PER_VERTEX_DATA, 0},
		{"TYPE",     0, DXGI_FORMAT_R32_UINT,        0, 36, 
							D3D10_INPUT_PER_VERTEX_DATA, 0},
	};

	// Create the input layout
	D3D10_PASS_DESC passDesc;
	streamOutTech_->GetPassByIndex(0)->GetDesc(&passDesc);
    HRESULT hr = d3dDevice_->CreateInputLayout(layout, 5, 
											   passDesc.pIAInputSignature,
											   passDesc.IAInputSignatureSize, 
											   &vertexLayout_);
	if (FAILED(hr))
	{
		MessageBoxA(0, "Creating ps vertex layout - Failed", "Error", MB_OK);
		return false;
	} 

	return true;
}
