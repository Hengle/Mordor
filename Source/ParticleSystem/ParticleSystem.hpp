/*
	Created 	Elinor Townsend 2012
				Based on PSystem by Frank Luna (C) 2008 All Rights Reserved.
*/

/*	
	Name		ParticleSystem
	Brief		Definition of ParticleSystem Class used to handle the creation, 
				updating and rendering of a particle system
*/

#ifndef _PARTICLESYSTEM_H
#define _PARTICLESYSTEM_H

#include <d3dx10.h>

class ParticleShader;
enum Particle;

class ParticleSystem
{
public:
	ParticleSystem(Particle particle);
	~ParticleSystem();

	float getAge()const { return age_; }; // Time elapsed since the system was reset

	void setCameraPos(const D3DXVECTOR3& cameraPos);
	void setEmitPos(const D3DXVECTOR3& emitPos);
	void setEmitDir(const D3DXVECTOR3& emitDir);

	void initialise(ID3D10Device* device, ID3D10ShaderResourceView* texArrayRV, 
					UINT maxParticles);
	void setHeightMapRV(ID3D10ShaderResourceView* heightMapRV);
	void reset();
	void update(float dt, float sceneTime);
	
	void render();

private:
	void buildVertexBuffer();

	ParticleSystem(const ParticleSystem& rhs);
	ParticleSystem& operator = (const ParticleSystem& rhs);

	Particle particle_;
 
	UINT maxParticles_;
	bool firstRun_;

	float sceneTime_;
	float timeStep_;
	float age_;

	D3DXVECTOR4 cameraPos_;
	D3DXVECTOR4 emitPos_;
	D3DXVECTOR4 emitDir_;

	ID3D10Device* d3dDevice_;
	ID3D10Buffer* initVertexBuffer_;	
	ID3D10Buffer* renderVertexBuffer_;
	ID3D10Buffer* streamOutVertexBuffer_;
 
	ID3D10ShaderResourceView* texArrayRV_;
	ID3D10ShaderResourceView* heightMapRV_;

	ParticleShader* particleShader_;
};

#endif // _PARTICLESYSTEM_H