/*
	Created 	Elinor Townsend 2012
*/

/*	
	Name		Heat Haze Shader
	Brief		Definition of Heat Haze Shader Class inherited from Shader
*/

#ifndef HEATHAZESHADER_H
#define HEATHAZESHADER_H

#include "Shaders/Shader.hpp"

class HeatHazeShader : public Shader
{
public:
    bool initialise();
	void render(float time, int indices);
	void setSceneSRV(ID3D10ShaderResourceView* sceneSRV);
	void setPerturbanceSRV(ID3D10ShaderResourceView* perturbanceSRV);
	void deinitialise();

private:
	ID3D10EffectMatrixVariable* orthoVar_;
	ID3D10EffectShaderResourceVariable* sceneVar_;
	ID3D10EffectShaderResourceVariable* perturbanceVar_;
	ID3D10EffectScalarVariable* timeVar_;
};

#endif // HEATHAZESHADER_H