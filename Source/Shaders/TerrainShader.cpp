/*
	Created 	Elinor Townsend 2012
*/

/*	
	Name		Terrain Shader
	Brief		Definition of Terrain Shader Class inherited from Shader
*/

#include "Shaders\TerrainShader.hpp"
#include "Scene\Scene.hpp"
#include "Utilities\SimplexNoise.hpp"
#include "Graphics\Light.hpp"

/*
	Name		TerrainShader::initialise
	Syntax		TerrainShader::initialise()
	Return		bool - Returns true once initialised
	Brief		Initialises the shader
*/
bool TerrainShader::initialise()
{
	d3dDevice_ = Scene::instance()->getDevice();
	if (!d3dDevice_)
	{
		MessageBox(0, "Retrieving device - Failed",
			"Error", MB_OK);
		return false;
	}

	// Build FX
	ID3D10Blob* compilationErrors = 0;
	HRESULT hr = 0;
	
	hr = D3DX10CreateEffectFromFile("Effect Files/Terrain.fx", 0, 0, 
		"fx_4_0", D3D10_SHADER_ENABLE_STRICTNESS, 0, d3dDevice_, 0, 0, &fx_, &compilationErrors, 0);
	if (FAILED(hr))
	{
		if ( compilationErrors )
		{
			MessageBoxA(0, "Creating effect from file - Failed",
			"Error", MB_OK);
			MessageBoxA(0, (char*)compilationErrors->GetBufferPointer(), 0, 0);
			compilationErrors->Release();
			return false;
		}
	} 

	technique_		= fx_->GetTechniqueByName("TexTech");
	
	wvpVar_			= fx_->GetVariableByName("Wvp")->AsMatrix();
	worldVar_		= fx_->GetVariableByName("World")->AsMatrix();
	permTableVar_	= fx_->GetVariableByName("PermTexture")->AsShaderResource();
	simplexVar_		= fx_->GetVariableByName("SimplexTexture")->AsShaderResource();
	cameraPosVar_	= fx_->GetVariableByName("cameraPos")->AsVector();
	fogColourVar_	= fx_->GetVariableByName("fogColour")->AsVector();
	lightVar_		= fx_->GetVariableByName("parallelLight");

	permTableVar_->SetResource(SimplexNoise::getPermTable());
	simplexVar_->SetResource(SimplexNoise::getSimplexTex());

	// Build vertex layout
	D3D10_INPUT_ELEMENT_DESC layout[] =
	{
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,	D3D10_INPUT_PER_VERTEX_DATA, 0},
		{"NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D10_INPUT_PER_VERTEX_DATA, 0},
		{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    0, 24, D3D10_INPUT_PER_VERTEX_DATA, 0},
		{"TYPE",     0, DXGI_FORMAT_R32_UINT,        0, 32, D3D10_INPUT_PER_VERTEX_DATA, 0},
	};

	// Create the input layout
    D3D10_PASS_DESC PassDesc;
    technique_->GetPassByIndex(0)->GetDesc(&PassDesc);
    hr = d3dDevice_->CreateInputLayout(layout, 4, PassDesc.pIAInputSignature, PassDesc.IAInputSignatureSize, &vertexLayout_);

	if (FAILED(hr))
	{
		MessageBoxA(0, "Creating input layout - Failed", "Error", MB_OK);
		return false;
	}

	return true;
}

/*
	Name		TerrainShader::deinitialise
	Syntax		TerrainShader::deinitialise()
	Brief		Deinitialises the shader
*/
void TerrainShader::deinitialise()
{
}

/*
	Name		TerrainShader::render
	Syntax		TerrainShader::render(D3DXVECTOR3* cameraPos, Light* light, D3DXVECTOR3* fogColour, int indices)
	Param		D3DXVECTOR3* cameraPos - The position of the camera in the scene
	Param		Light* light - Light data
	Param		D3DXVECTOR3* fogColour - The colour of the fog effect to apply to the terrain
	Param		int indices - The number of indices to render
	Brief		Prepares the shader for rendering
*/
void TerrainShader::render(D3DXVECTOR3* cameraPos, Light* light, D3DXVECTOR3* fogColour, int indices)
{
	// Set constants
	wvpVar_->SetMatrix((float*)&Scene::instance()->getWVP());
	worldVar_->SetMatrix((float*)&Scene::instance()->getWorld());
	cameraPosVar_->SetFloatVector((float*)cameraPos);
	fogColourVar_->SetFloatVector((float*)fogColour);
	lightVar_->SetRawValue(&light->ambientColour, 0, sizeof(D3DXCOLOR));
	lightVar_->SetRawValue(&light->diffuseColour, 16, sizeof(D3DXCOLOR));
	lightVar_->SetRawValue(&light->specularColour, 32, sizeof(D3DXCOLOR));
	lightVar_->SetRawValue(&light->diffuseDirection, 48, sizeof(D3DXVECTOR3));
	lightVar_->SetRawValue(&light->ambientIntensity, 60, sizeof(float));
	lightVar_->SetRawValue(&light->diffuseIntensity, 64, sizeof(float));
	lightVar_->SetRawValue(&light->specularIntensity, 68, sizeof(float));
	lightVar_->SetRawValue(&light->shininess, 72, sizeof(float));

    D3D10_TECHNIQUE_DESC techniqueDesc;
	unsigned int i;
	
	// Set the input layout.
	Scene::instance()->getDevice()->IASetInputLayout(vertexLayout_);

	// Get the description structure of the technique from inside the shader so it can be used for rendering.
    technique_->GetDesc(&techniqueDesc);

    // Go through each pass in the technique (should be just one currently) and render the triangles.
	for(i=0; i<techniqueDesc.Passes; ++i)
    {
        technique_->GetPassByIndex(i)->Apply(0);
        Scene::instance()->getDevice()->DrawIndexed(indices, 0, 0);
    }

}
