/*
	Created 	Elinor Townsend 2012
*/

/*	
	Name		Shader
	Brief		Definition of abstract base shader class
*/

#ifndef SHADER_H
#define SHADER_H

#include <d3dx10.h>

class Shader
{
public:
	virtual bool initialise() = 0;
	virtual void deinitialise() = 0;

protected:
	ID3D10Device* d3dDevice_;

	ID3D10Effect* fx_;
	ID3D10EffectTechnique*  technique_;
	ID3D10EffectMatrixVariable* wvpVar_;
	ID3D10InputLayout* vertexLayout_;
};

#endif