/*
	Created 	Elinor Townsend 2012
*/

/*	
	Name		Volcano
	Brief		Definition of Volcano Class inherited from State
*/

#ifndef VOLCANO_H
#define VOLCANO_H

#include "States\State.hpp"
#include "Graphics\Light.hpp"
#include "Utilities\RenderableTex2D.hpp"
#include "Global\Global.hpp"

class Terrain;
class SkySphere;
class ScreenspaceQuad;
class TerrainShader;
class SkyMapShader;
class HeatHazeShader;
class ParticleSystem;

enum ActiveCamera
{
	CAMERA_ONE,
	CAMERA_TWO,
};

class Volcano : public State
{
public:
	Volcano();
	~Volcano();
    virtual State* getNextState();
	virtual bool initialise();
	virtual bool deinitialise();
	virtual bool update(float dt);
	virtual void render();

private:
	void initialiseParticles();
	void initialiseAsh();
	void initialiseFire();
	void initialiseSmoke();
	void changeCamera();
	void toggleHeatHaze();
	void togglePause();

	ID3D10Device* d3dDevice_;

	float moveX_, moveY_, moveZ_, yaw_, pitch_, roll_; // Camera movement
	D3DXVECTOR3 cameraRotation_;

	Terrain* terrain_;
	SkySphere* skySphere_;
	ParticleSystem* ash_;
	ParticleSystem* fire_[NUM_FIRE_SYSTEMS];
	ParticleSystem* smoke_[NUM_SMOKE_SYSTEMS];
	ScreenspaceQuad* screenQuad_;

	TerrainShader* terrainShader_;
	SkyMapShader* skyMapShader_;
	HeatHazeShader* heatHazeShader_;

	float time_;
	float hazeScroll_;
	Light parallelLight_;
	D3DXVECTOR3 fogColour_;
	ID3D10ShaderResourceView* ashRV_;
	ID3D10ShaderResourceView* fireRV_;
	ID3D10ShaderResourceView* smokeRV_;

	RenderableTex2D heatHazeMap_;
	ID3D10ShaderResourceView* perturbance_;

	bool useHeatHaze_;
	bool particlesInitialised_;
	bool paused_;

	ActiveCamera currentCamera_;
	Camera* camera_;

	// Constants
	const int MOVESPEED;
	const float ROTATESPEED;
};

#endif // VOLCANO_H