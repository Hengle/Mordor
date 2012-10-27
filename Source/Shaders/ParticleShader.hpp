/*
	Created 	Elinor Townsend 2012
*/

/*	
	Name		Particle Shader
	Brief		Definition of Particle Shader Class inherited from Shader
*/

#ifndef _PARTICLE_SHADER_H
#define _PARTICLE_SHADER_H

#include "Shaders/Shader.hpp"

struct LightSimple;
enum Particle;

class ParticleShader : public Shader
{
public:
	bool initialise();
    bool initialise(Particle particle);
	void deinitialise();

	void setup(	float sceneTime, float timeStep, D3DXVECTOR4* cameraPos,  D3DXVECTOR4* emitPos, 
				D3DXVECTOR4* emitDir, ID3D10ShaderResourceView* texArrayRV, 
				ID3D10ShaderResourceView* heightMap);

	void setStreamOutTech(D3D10_TECHNIQUE_DESC* techDesc);
	void setDrawTech(D3D10_TECHNIQUE_DESC* techDesc);

	void applyStreamOutPass(UINT pass);
	void applyDrawPass(UINT pass);

	ID3D10InputLayout *  getLayout() const { return vertexLayout_; };
	
private:
	bool createEffectFile(Particle particle);
	bool buildVertexLayout();

	ID3D10EffectTechnique* streamOutTech_;
	ID3D10EffectTechnique* drawTech_;
	ID3D10EffectMatrixVariable* viewProjVar_;
	ID3D10EffectScalarVariable* sceneTimeVar_;
	ID3D10EffectScalarVariable* timeStepVar_;
	ID3D10EffectVectorVariable* cameraPosVar_;
	ID3D10EffectVectorVariable* emitPosVar_;
	ID3D10EffectVectorVariable* emitDirVar_;
	ID3D10EffectShaderResourceVariable* texArrayVar_;
	ID3D10EffectShaderResourceVariable* randomTexVar_;
	ID3D10EffectShaderResourceVariable* heightMapVar_;
};

#endif // _PARTICLE_SHADER_H