/*
	Created 	Elinor Townsend 2012
*/

/*	
	Name		Sky Map Shader
	Brief		Definition of Sky Map Shader Class inherited from Shader
*/

#include "Shaders\SkyMapShader.hpp"
#include "Scene\Scene.hpp"
#include "Utilities\SimplexNoise.hpp"

/*
	Name		SkyMapShader::initialise
	Syntax		SkyMapShader::initialise()
	Return		bool - Returns true once initialised
	Brief		Initialises the shader
*/
bool SkyMapShader::initialise()
{
	d3dDevice_ = Scene::instance()->getDevice();
	if (!d3dDevice_)
	{
		MessageBox(0, "Retrieving device - Failed", "Error", MB_OK);
		return false;
	}

	// Build FX
	ID3D10Blob* compilationErrors = 0;
	HRESULT hr = 0;

	hr = D3DX10CreateEffectFromFile("Effect Files/SkyMap.fx", 0, 0, "fx_4_0", 
									D3D10_SHADER_ENABLE_STRICTNESS, 0, d3dDevice_, 
									0, 0, &fx_, &compilationErrors, 0);
	if (FAILED(hr))
	{
		MessageBoxA(0, "Creating effect from file - Failed", "Error", MB_OK);
		return false;
	}

	wvpVar_			= fx_->GetVariableByName("wvp")->AsMatrix();
	technique_		= fx_->GetTechniqueByName("SkyTech");
	timeVar_		= fx_->GetVariableByName("time")->AsScalar();
	permTableVar_	= fx_->GetVariableByName("PermTexture")->AsShaderResource();
	simplexVar_		= fx_->GetVariableByName("SimplexTexture")->AsShaderResource();

	permTableVar_->SetResource(SimplexNoise::createPermTableTexture());
	simplexVar_->SetResource(SimplexNoise::createSimplexTexture());

	// Build vertex layout
	D3D10_INPUT_ELEMENT_DESC layout[] =
	{
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,  D3D10_INPUT_PER_VERTEX_DATA, 0},
		{"NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D10_INPUT_PER_VERTEX_DATA, 0},
		{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    0, 24, D3D10_INPUT_PER_VERTEX_DATA, 0},
	};

	// Create the input layout
    D3D10_PASS_DESC passDesc;
    technique_->GetPassByIndex(0)->GetDesc(&passDesc);
    hr = d3dDevice_->CreateInputLayout(layout, 3, passDesc.pIAInputSignature, passDesc.IAInputSignatureSize, &vertexLayout_);

	if (FAILED(hr))
	{
		MessageBoxA(0, "Creating input layout - Failed", "Error", MB_OK);
		return false;
	}
	
	return true;
}

/*
	Name		SkyMapShader::deinitialise
	Syntax		SkyMapShader::deinitialise()
	Brief		Deinitialises the shader
*/
void SkyMapShader::deinitialise()
{
}

/*
	Name		SkyMapShader::render
	Syntax		SkyMapShader::render(float time, int indices)
	Param		float time - The time the program has been running
	Param		int indices - The number of indices to render
	Brief		Renders using the shader
*/
void SkyMapShader::render(float time, int indices)
{
	wvpVar_->SetMatrix((float*)&Scene::instance()->getWVP());

	timeVar_->SetFloat(time);

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


