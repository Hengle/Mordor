/*
	Created 	Elinor Townsend 2012
*/

/*	
	Name		Sky Map Shader
	Brief		Definition of Sky Map Shader Class inherited from Shader
*/

#ifndef SKYMAPSHADER_H
#define SKYMAPSHADER_H

#include "Shaders/Shader.hpp"

class SkyMapShader : public Shader
{
public:
    bool initialise();
	void render(float time, int indices);
	void deinitialise();

private:
	ID3D10EffectScalarVariable* timeVar_;
	ID3D10EffectShaderResourceVariable* permTableVar_;
	ID3D10EffectShaderResourceVariable* simplexVar_;
};

#endif