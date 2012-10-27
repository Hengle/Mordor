/*
	Created 	Elinor Townsend 2012
*/

/*	
	Name		Terrain Shader
	Brief		Definition of Terrain Shader Class inherited from Shader
*/

#ifndef TERRAINSHADER_H
#define TERRAINSHADER_H

#include "Shaders/Shader.hpp"

struct Light;

class TerrainShader : public Shader
{
public:
    bool initialise();
	void render(D3DXVECTOR3* cameraPos, Light* light, D3DXVECTOR3* fogColour, int indices);
	void deinitialise();

private:
	ID3D10EffectMatrixVariable* worldVar_;
	ID3D10EffectShaderResourceVariable* permTableVar_;
	ID3D10EffectShaderResourceVariable* simplexVar_;
	ID3D10EffectVectorVariable* cameraPosVar_;
	ID3D10EffectVectorVariable* fogColourVar_;
	ID3D10EffectVariable* lightVar_;
};

#endif