/*
	Created 	Elinor Townsend 2012
*/

/*	
	Name		Heat Haze Shader
	Brief		Definition of Heat Haze Shader Class inherited from Shader
*/

#include "Shaders\HeatHazeShader.hpp"
#include "Scene\Scene.hpp"

/*
	Name		HeatHazeShader::initialise
	Syntax		HeatHazeShader::initialise()
	Return		bool - Returns true once initialised
	Brief		Initialises the shader
*/
bool HeatHazeShader::initialise()
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
	
	hr = D3DX10CreateEffectFromFile("Effect Files/HeatHaze.fx", 0, 0, 
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

	technique_		= fx_->GetTechniqueByName("HeatHazeTech");
	orthoVar_		= fx_->GetVariableByName("Orthogonal")->AsMatrix();
	sceneVar_		= fx_->GetVariableByName("screenTexture")->AsShaderResource();
	perturbanceVar_ = fx_->GetVariableByName("perturbanceTexture")->AsShaderResource();
	timeVar_		= fx_->GetVariableByName("time")->AsScalar();

	// Build vertex layout
	D3D10_INPUT_ELEMENT_DESC layout[] =
	{
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,	D3D10_INPUT_PER_VERTEX_DATA, 0},
		{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    0, 12, D3D10_INPUT_PER_VERTEX_DATA, 0},
	};

	// Create the input layout
    D3D10_PASS_DESC PassDesc;
    technique_->GetPassByIndex(0)->GetDesc(&PassDesc);
    hr = d3dDevice_->CreateInputLayout(layout, 2, PassDesc.pIAInputSignature, PassDesc.IAInputSignatureSize, &vertexLayout_);

	if (FAILED(hr))
	{
		MessageBoxA(0, "Creating input layout - Failed", "Error", MB_OK);
		return false;
	}

	return true;
}

/*
	Name		HeatHazeShader::deinitialise
	Syntax		HeatHazeShader::deinitialise()
	Brief		Deinitialises the shader
*/
void HeatHazeShader::deinitialise()
{
}

/*
	Name		HeatHazeShader::render
	Syntax		HeatHazeShader::render(float time,  int indices)
	Param		float time - The time the program has been running
	Param		int indices - The number of indices to render
	Brief		Renders using the shader
*/
void HeatHazeShader::render(float time, int indices)
{
	orthoVar_->SetMatrix((float*)&Scene::instance()->getOrthogonal());
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

/*
	Name		HeatHazeShader::setSceneSRV
	Syntax		HeatHazeShader::setSceneSRV(ID3D10ShaderResourceView* sceneSRV)
	Param		ID3D10ShaderResourceView* sceneSRV - Scene texture resource
	Brief		Sets the shader resource view of the scene texture
*/
void HeatHazeShader::setSceneSRV(ID3D10ShaderResourceView* sceneSRV)
{
	sceneVar_->SetResource(sceneSRV);
}

/*
	Name		HeatHazeShader::setPerturbanceSRV
	Syntax		HeatHazeShader::setPerturbanceSRV(ID3D10ShaderResourceView* perturbanceSRV)
	Param		ID3D10ShaderResourceView* perturbanceSRV - Perturbance texture resource
	Brief		Sets the shader resource view of the perturbance texture
*/
void HeatHazeShader::setPerturbanceSRV(ID3D10ShaderResourceView* perturbanceSRV)
{
	perturbanceVar_->SetResource(perturbanceSRV);
}