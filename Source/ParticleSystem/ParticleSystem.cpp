/*
	Created 	Elinor Townsend 2012
				Based on PSystem by Frank Luna (C) 2008 All Rights Reserved.
*/

/*	
	Name		ParticleSystem
	Brief		Definition of ParticleSystem Class used to handle the creation, 
				updating and rendering of a particle system
*/
#include <algorithm>

#include "ParticleSystem\ParticleSystem.hpp"
#include "Scene\Scene.hpp"
#include "Graphics\Vertex.hpp"
#include "Shaders\ParticleShader.hpp"

/*
	Name		ParticleSystem::ParticleSystem
	Syntax		ParticleSystem(Particle particle)
	Param		Particle particle - The type of particle system
	Brief		ParticleSystem constructor initialises member variables
*/
ParticleSystem::ParticleSystem(Particle particle)
: d3dDevice_(0), initVertexBuffer_(0), renderVertexBuffer_(0), 
  streamOutVertexBuffer_(0), texArrayRV_(0), heightMapRV_(0)
{
	particle_ = particle;

	firstRun_ = true;
	sceneTime_ = 0.0f;
	timeStep_ = 0.0f;
	age_      = 0.0f;

	cameraPos_  = D3DXVECTOR4(0.0f, 0.0f, 0.0f, 1.0f);
	emitPos_ = D3DXVECTOR4(0.0f, 0.0f, 0.0f, 1.0f);
	emitDir_ = D3DXVECTOR4(0.0f, 1.0f, 0.0f, 0.0f);
}

/*
	Name		ParticleSystem::~ParticleSystem
	Syntax		~ParticleSystem()
	Brief		ParticleSystem destructor
*/
ParticleSystem::~ParticleSystem()
{
	delete particleShader_;

	if (initVertexBuffer_)
	{
		initVertexBuffer_->Release();
		initVertexBuffer_ = 0;
	}

	if (renderVertexBuffer_)
	{
		renderVertexBuffer_->Release();
		renderVertexBuffer_ = 0;
	}
	
	if (streamOutVertexBuffer_)
	{
		streamOutVertexBuffer_->Release();
		streamOutVertexBuffer_ = 0;
	}	
}

/*
	Name		ParticleSystem::setCameraPos
	Syntax		ParticleSystem::setCameraPos(const D3DXVECTOR3& cameraPos)
	Param		const D3DXVECTOR3& cameraPos - Position to set as camera position
	Brief		Sets the position of the camera in the world
*/
void ParticleSystem::setCameraPos(const D3DXVECTOR3& cameraPos)
{
	cameraPos_ = D3DXVECTOR4(cameraPos.x, cameraPos.y, cameraPos.z, 1.0f);
}

/*
	Name		ParticleSystem::setEmitPos
	Syntax		ParticleSystem::setEmitPos(const D3DXVECTOR3& emitPos)
	Param		const D3DXVECTOR3& emitPos - Position from which to emit
				particles for the system
	Brief		Sets the position of the particle system emitter
*/
void ParticleSystem::setEmitPos(const D3DXVECTOR3& emitPos)
{
	emitPos_ = D3DXVECTOR4(emitPos.x, emitPos.y, emitPos.z, 1.0f);
}

/*
	Name		ParticleSystem::setEmitDir
	Syntax		ParticleSystem::setEmitDir(const D3DXVECTOR3& emitDir)
	Param		const D3DXVECTOR3& emitDir - Direction in which to emit
				particles for the system
	Brief		Sets the direction of the particle system emission
*/
void ParticleSystem::setEmitDir(const D3DXVECTOR3& emitDir)
{
	emitDir_ = D3DXVECTOR4(emitDir.x, emitDir.y, emitDir.z, 0.0f);
}

/*
	Name		ParticleSystem::initialise
	Syntax		ParticleSystem::initialise(ID3D10Device* device, 
										   ID3D10ShaderResourceView* texArrayRV,
										   ID3D10ShaderResourceView* heightMapRV,
										   UINT maxParticles)
	Param		ID3D10Device* device - The D3D device
	Param		ID3D10ShaderResourceView* texArrayRV - Handle to textures used
				by the particle system
	Param		UINT maxParticles - The maximum number of particles this system
				should emit
	Brief		Initialises the particle system
*/
void ParticleSystem::initialise(ID3D10Device* device, 
								ID3D10ShaderResourceView* texArrayRV,
								UINT maxParticles)
{
	d3dDevice_ = device;

	particleShader_ = new ParticleShader;
	particleShader_->initialise(particle_);

	maxParticles_ = maxParticles;

	texArrayRV_  = texArrayRV; 
	
	buildVertexBuffer();
}

/*
	Name		ParticleSystem::setHeightMapRV
	Syntax		ParticleSystem::setHeightMapRV(ID3D10ShaderResourceView* heightMapRV)
	Brief		Sets the terrain height map used by the particle system
*/
void ParticleSystem::setHeightMapRV(ID3D10ShaderResourceView* heightMapRV)
{
	heightMapRV_ = heightMapRV;
}

/*
	Name		ParticleSystem::reset
	Syntax		ParticleSystem::reset()
	Brief		Resets the particle system
*/
void ParticleSystem::reset()
{
	firstRun_ = true;
	age_      = 0.0f;
}

/*
	Name		ParticleSystem::update
	Syntax		ParticleSystem::update(float dt, float sceneTime)
	Param		float dt - Change in time between frames
	Param		float sceneTime - The scene time
	Brief		Updates the particle system based on time
*/
void ParticleSystem::update(float dt, float sceneTime)
{
	sceneTime_ = sceneTime;
	timeStep_ = dt;

	age_ += dt;
}

/*
	Name		ParticleSystem::render
	Syntax		ParticleSystem::render()
	Brief		Renders the particle system
*/
void ParticleSystem::render()
{
	particleShader_->setup(sceneTime_, timeStep_, &cameraPos_, &emitPos_, 
								 &emitDir_, texArrayRV_, heightMapRV_);

	// Set IA stage
	d3dDevice_->IASetInputLayout(particleShader_->getLayout());
    d3dDevice_->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_POINTLIST);

	UINT stride = sizeof(ParticleVertex);
    UINT offset = 0;

	// On the first pass, use the initialization VB.  Otherwise, use
	// the VB that contains the current particle list
	if (firstRun_)
	{
		d3dDevice_->IASetVertexBuffers(0, 1, &initVertexBuffer_, &stride, 
									   &offset);
	}
	else
	{
		d3dDevice_->IASetVertexBuffers(0, 1, &renderVertexBuffer_, &stride, 
									   &offset);
	}

	// Draw the current particle list using stream-out only to update them
	// The updated vertices are streamed-out to the target vertex buffer
	d3dDevice_->SOSetTargets(1, &streamOutVertexBuffer_, &offset);

    D3D10_TECHNIQUE_DESC techDesc;
	particleShader_->setStreamOutTech(&techDesc);

    for (UINT p = 0; p < techDesc.Passes; ++p)
    {
		particleShader_->applyStreamOutPass(p);
        
		if (firstRun_)
		{
			d3dDevice_->Draw(1, 0);
			firstRun_ = false;
		}
		else
		{
			d3dDevice_->DrawAuto();
		}
    }

	// done streaming-out - unbind the vertex buffer
	ID3D10Buffer* bufferArray[1] = { 0 };
	d3dDevice_->SOSetTargets(1, bufferArray, &offset);

	// Ping-pong the vertex buffers
	std::swap(renderVertexBuffer_, streamOutVertexBuffer_);

	// Draw the updated particle system we just streamed-out 
	d3dDevice_->IASetVertexBuffers(0, 1, &renderVertexBuffer_, &stride, 
								   &offset);

	particleShader_->setDrawTech(&techDesc);
    for (UINT p = 0; p < techDesc.Passes; ++p)
    {
      particleShader_->applyDrawPass(p);
        
		d3dDevice_->DrawAuto();
    }
}

/*
	Name		ParticleSystem::buildVertexBuffer
	Syntax		ParticleSystem::buildVertexBuffer()
	Brief		Builds the vertices for the particle system
*/
void ParticleSystem::buildVertexBuffer()
{
	// Create the buffer to kick-off the particle system
    D3D10_BUFFER_DESC vbd;
    vbd.Usage = D3D10_USAGE_DEFAULT;
    vbd.ByteWidth = sizeof(ParticleVertex) * 1;
    vbd.BindFlags = D3D10_BIND_VERTEX_BUFFER;
    vbd.CPUAccessFlags = 0;
    vbd.MiscFlags = 0;

	// The initial particle emitter has type 0 and age 0.  The rest of the 
	// particle attributes do not apply to an emitter
	ParticleVertex p;
	ZeroMemory(&p, sizeof(ParticleVertex));
	p.age  = 0.0f;
	p.type = 0; 
 
    D3D10_SUBRESOURCE_DATA vinitData;
    vinitData.pSysMem = &p;

	HRESULT hr = d3dDevice_->CreateBuffer(&vbd, &vinitData, &initVertexBuffer_);
	if (FAILED(hr))
	{
		MessageBox(0, "Creating ps initial buffer - Failed", "Error", MB_OK);
		return;
	}
	
	// Create the ping-pong buffers for stream-out and rendering
	vbd.ByteWidth = sizeof(ParticleVertex) * maxParticles_;
    vbd.BindFlags = D3D10_BIND_VERTEX_BUFFER | D3D10_BIND_STREAM_OUTPUT;

    hr = d3dDevice_->CreateBuffer(&vbd, 0, &renderVertexBuffer_);
	if (FAILED(hr))
	{
		MessageBox(0, "Creating ps render buffer - Failed", "Error", MB_OK);
		return;
	}
	hr = d3dDevice_->CreateBuffer(&vbd, 0, &streamOutVertexBuffer_);
	if (FAILED(hr))
	{
		MessageBox(0, "Creating ps streamout buffer - Failed", "Error", MB_OK);
		return;
	}
}