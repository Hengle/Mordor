/*
	Created 	Elinor Townsend 2012
*/

/*	
	Name		Volcano
	Brief		Definition of Volcano Class inherited from State
*/

#include <vector>

#include "States\Volcano.hpp"
#include "Scene\Scene.hpp"
#include "Geometry\Terrain.hpp"
#include "Shaders\TerrainShader.hpp"
#include "Geometry\SkySphere.hpp"
#include "Shaders\SkyMapShader.hpp"
#include "Geometry\ScreenspaceQuad.hpp"
#include "Shaders\HeatHazeShader.hpp"
#include "Utilities\SimplexNoise.hpp"
#include "ParticleSystem\ParticleSystem.hpp"
#include "ParticleSystem\Particle.hpp"

/*
	Name		Volcano::Volcano
	Syntax		Volcano()
	Brief		Volcano constructor initialises member variables
*/
Volcano::Volcano()
: d3dDevice_(0), moveX_(0), moveZ_(0), yaw_(0), pitch_(0), terrain_(0), skySphere_(0), 
  screenQuad_(0), terrainShader_(0), skyMapShader_(0), heatHazeShader_(0), time_(0), hazeScroll_(0), 
  MOVESPEED(100), ROTATESPEED(50), fogColour_(0.5f, 0.5f, 0.6f), cameraRotation_(0.0f, 0.0f, 0.0f),
  ashRV_(0), fireRV_(0), smokeRV_(0), ash_(0), useHeatHaze_(true), particlesInitialised_(false),
  currentCamera_(CAMERA_ONE), paused_(false)
{
}

/*
	Name		Volcano::~Volcano
	Syntax		~Volcano()
	Brief		Volcano destructor
*/
Volcano::~Volcano()
{
}

/*
	Name		Volcano::getNextState
	Syntax		Volcano::getNextState()
	Return		State* - A pointer to a next state type object
	Brief		Creates a new object of the next state type
*/
State* Volcano::getNextState()
{
    return new Volcano;
}

/*
	Name		Volcano::initialise
	Syntax		Volcano::initialise()
	Return		bool - Returns true once initialised
	Brief		Initialises the state
*/
bool Volcano::initialise()
{
	cameraOne_ = Camera::getCamera(CAMERA_AUTO);
	cameraTwo_ = Camera::getCamera(CAMERA_FREE);

	// Initialise using automated camera
	camera_ = cameraOne_;
	camera_->initialise();

	input_ = new DirectInput;

	d3dDevice_ = Scene::instance()->getDevice();
	if (!d3dDevice_)
	{
		MessageBox(0, "Retrieving device - Failed",
			"Error", MB_OK);
		return false;
	}

	terrain_ = new Terrain();
	terrain_->initialise(d3dDevice_, 500);
	terrain_->setPos(-250, -50, 25);
	terrain_->setTheta(0, 0, 0);

	// Create and initialise terrain shader
	terrainShader_ = new TerrainShader;
	terrainShader_->initialise();

	moveX_ = 0.0f;
	moveY_ = 0.0f;
	moveZ_ = 0.0f;
	yaw_ = 0.0f;
	pitch_ = 0.0f;
	roll_ = 0.0f;

	// Create sky sphere and scale it so that it does not clip with the camera
	skySphere_ = new SkySphere;
	skySphere_->initialise(d3dDevice_);
	skySphere_->setScale(50.0f, 50.0f, 50.0f);
	skySphere_->setTrans();

	// Create and initialise sky map shader
	skyMapShader_ = new SkyMapShader;
	skyMapShader_->initialise();

	parallelLight_.ambientColour = D3DXCOLOR(0.0f, 0.0f, 0.0f, 1.0f);
	parallelLight_.diffuseColour = D3DXCOLOR(1.0f, 0.8f, 0.7f, 1.0f);
	parallelLight_.specularColour = D3DXCOLOR(0.0f, 0.0f, 0.0f, 1.0f);
	parallelLight_.diffuseDirection = D3DXVECTOR3(2.0f, -0.5f, 1.5f);
	parallelLight_.ambientIntensity = 1.0f;
	parallelLight_.diffuseIntensity = 0.2f;
	parallelLight_.specularIntensity = 1.0f;
	parallelLight_.shininess = 2000;

	heatHazeMap_.initialise(d3dDevice_);

	screenQuad_ = new ScreenspaceQuad;
	screenQuad_->initialise(d3dDevice_);

	heatHazeShader_ = new HeatHazeShader;
	heatHazeShader_->initialise();

	perturbance_ = SimplexNoise::getPeturbationTex();

	initialiseParticles();

	return true;
}

/*
	Name		Volcano::deinitialise
	Syntax		Volcano::deinitialise()
	Return		bool - Returns true once deinitialised
	Brief		deinitialises the state
*/
bool Volcano::deinitialise()
{
	// Clean up cameras
	delete cameraOne_;
	delete cameraTwo_;
	camera_ = 0;

	// Delete input
	delete input_;

	// Delete geometry objects
	delete terrain_;
	delete skySphere_;
	delete screenQuad_;
	
	// Clear shaders
	delete terrainShader_;
	delete skyMapShader_;	
	delete heatHazeShader_;
	
	// Clean up particle systems
	int i;
	delete ash_;
	for (i = 0; i < NUM_FIRE_SYSTEMS; ++i)
		delete fire_[i];
	for (i = 0; i < NUM_SMOKE_SYSTEMS; ++i)
		delete smoke_[i];

	return true;
}

/*
	Name		Volcano::update
	Syntax		Volcano::update(float dt)
	Param		float dt - Time since last frame
	Return		bool - True if the state is to be changed
	Brief		Updates the state
*/
bool Volcano::update(float dt)
{
	int i;
	
	time_ += dt;

	input_->update();

	// Pause
	if (input_->isKeyPressed(DIK_P)) 
		togglePause();

	if (!paused_)
	{
		// Reset
		if (input_->isKeyPressed(DIK_R))
		{
			camera_->reset();
			camera_->setZoomFactor(1.0f);
			terrain_->reset();
			if (particlesInitialised_)
			{
				ash_->reset();
			
				for (i = 0; i < NUM_FIRE_SYSTEMS; ++i)
					fire_[i]->reset();
				for (i = 0; i < NUM_SMOKE_SYSTEMS; ++i)
					smoke_[i]->reset();

				particlesInitialised_ = false;
			}
		}
	
		// Autocomplete generation
		if (input_->isKeyPressed(DIK_C))
			terrain_->autoComplete();

		// Change camera
		if (input_->isKeyPressed(DIK_V))
		{
			camera_->reset();
			camera_->setZoomFactor(1.0f);
			changeCamera();
		}

		// Toggle heat haze on/off
		if (input_->isKeyPressed(DIK_H))
		{
			toggleHeatHaze();
		}	

		// Zoom camera
		camera_->zoom(input_->getMouseZ());

		// Camera two can be rotated and positioned by the user
		if (currentCamera_ == CAMERA_TWO)
		{
			// Rotate camera
			if (input_->isKeyDown(DIK_LEFT))	yaw_   = -ROTATESPEED*dt;
			if (input_->isKeyDown(DIK_RIGHT))	yaw_   = ROTATESPEED*dt;
			if (input_->isKeyDown(DIK_UP))		pitch_ = ROTATESPEED*dt;
			if (input_->isKeyDown(DIK_DOWN))	pitch_ = -ROTATESPEED*dt;

			if (input_->getMouseLeftDown())
			{
				if (input_->getMouseX() < 0) yaw_	= -ROTATESPEED*dt;
				if (input_->getMouseX() > 0) yaw_	= ROTATESPEED*dt;
				if (input_->getMouseY() > 0) pitch_	= ROTATESPEED*dt;
				if (input_->getMouseY() < 0) pitch_	= -ROTATESPEED*dt;
			}
			
			camera_->rotate(yaw_, pitch_, roll_);

			yaw_ = 0.0f;
			pitch_ = 0.0f;
			roll_ = 0.0f;

			// Move the camera
			if (input_->isKeyDown(DIK_D)) moveX_ = MOVESPEED*dt;
			if (input_->isKeyDown(DIK_A)) moveX_ = -MOVESPEED*dt;
			if (input_->isKeyDown(DIK_Z)) moveY_ = MOVESPEED*dt;
			if (input_->isKeyDown(DIK_X)) moveY_ = -MOVESPEED*dt;
			if (input_->isKeyDown(DIK_W)) moveZ_ = MOVESPEED*dt;
			if (input_->isKeyDown(DIK_S)) moveZ_ = -MOVESPEED*dt;

			if (moveX_ || moveY_ || moveZ_)
			{
				cameraRotation_ = camera_->getRotation();

				// Create the rotation matrix.
				D3DXMATRIX rotation;
				D3DXMatrixRotationYawPitchRoll(&rotation, cameraRotation_.y, cameraRotation_.x, cameraRotation_.z);

				// Transform the default target and up vectors by the rotation matrix.
				D3DXVECTOR3 move = D3DXVECTOR3(moveX_, moveY_, moveZ_);
				D3DXVec3TransformCoord(&move, &move, &rotation);

				camera_->move(move.x, move.y, move.z);
				moveX_ = 0.0f;
				moveY_ = 0.0f;
				moveZ_ = 0.0f;
			}
		}

		// Update camera - updates the view matrix
		camera_->setDeltaTime(dt);
		camera_->update();

		// Position the sky sphere centred on the camera
		skySphere_->setPos(camera_->getPosition());
		skySphere_->setTrans();

		// Update the terrain - terrain generates over time
		terrain_->update(dt);

		// Update particle effects
		if (terrain_->isComplete())
		{
			if (!particlesInitialised_)
			{
				// Ash
				ash_->setEmitPos(terrain_->getAshEmitter());
				// Fire
				for (i = 0; i < NUM_FIRE_SYSTEMS; ++i)
					fire_[i]->setEmitPos(terrain_->getFireEmitter(i));
				// Smoke
				for (i = 0; i < NUM_SMOKE_SYSTEMS; ++i)
					smoke_[i]->setEmitPos(terrain_->getFireEmitter(i));
				// Initialised
				particlesInitialised_ = true;
			}	
			else
			{
				// Ash
				ash_->update(dt, Scene::instance()->getTimer()->getGameTime());
				// Fire
				for (i = 0; i < NUM_FIRE_SYSTEMS; ++i)
					fire_[i]->update(dt, Scene::instance()->getTimer()->getGameTime());
				// Smoke
				for (i = 0; i < NUM_SMOKE_SYSTEMS; ++i)
					smoke_[i]->update(dt, Scene::instance()->getTimer()->getGameTime());
			}
		}

		if (Scene::instance()->isResizing())
		{
			screenQuad_->updateVertices();
			Scene::instance()->setResizing(false);
			camera_->setZoomFactor(1.0f);
		}
	}

    return false;
}

/*
	Name		Volcano::render
	Syntax		Volcano::render()
	Brief		Updates the state
*/
void Volcano::render()
{
	// Reset the depth stencil state and blend state 
	Scene::instance()->turnZBufferOn();
	float blendFactor[] = {0.0f, 0.0f, 0.0f, 0.0f};
	d3dDevice_->OMSetBlendState(0, blendFactor, 0xffffffff);

	if (terrain_->isComplete() && particlesInitialised_)
	{
		if (useHeatHaze_)
			heatHazeMap_.setRenderTarget();
	}	

	// Set world and wvp transformation matrices
	Scene::instance()->setWorld(terrain_->getWorld());
	Scene::instance()->setWVP();

	// Render the terrain
	terrain_->render();
	terrainShader_->render(&camera_->getPosition(), &parallelLight_, &fogColour_, terrain_->getNumIndices());

	// Set world and wvp transformation matrices
	Scene::instance()->setWorld(skySphere_->getWorld());
	Scene::instance()->setWVP();

	// Render sky map
	skySphere_->render();
	skyMapShader_->render(time_, skySphere_->getNumIndices());

	if (terrain_->isComplete() && particlesInitialised_)
	{
		if (!paused_)
		{
			// Render fire particles
			for (int i = 0; i < NUM_FIRE_SYSTEMS; ++i)
			{
				fire_[i]->setCameraPos(camera_->getPosition());
				fire_[i]->setHeightMapRV(terrain_->getHeightMap());
				fire_[i]->render();
			}

			// Render smoke particles
			for (int i = 0; i < NUM_SMOKE_SYSTEMS; ++i)
			{
				smoke_[i]->setCameraPos(camera_->getPosition());
				smoke_[i]->setHeightMapRV(terrain_->getHeightMap());
				smoke_[i]->render();
			}
		}

		if (useHeatHaze_)
		{
			Scene::instance()->resetOMTargetsAndViewport();

			Scene::instance()->turnZBufferOff();

			// Set resources for heat haze shader
			heatHazeShader_->setPerturbanceSRV(perturbance_);
			heatHazeShader_->setSceneSRV(heatHazeMap_.getTextureSRV());

			// Render scene using heat haze post-processing effect
			screenQuad_->render();
			heatHazeShader_->render(hazeScroll_, screenQuad_->getNumIndices());
			hazeScroll_ += 0.005f;
			if (hazeScroll_ == 1.0f)
				hazeScroll_ = -1.0f;

			Scene::instance()->turnZBufferOn();
		}

		if (!paused_)
		{
			// Render ash particles
			ash_->setCameraPos(camera_->getPosition());
			ash_->setHeightMapRV(terrain_->getHeightMap());
			ash_->render();
		}
	}
}

/*
	Name		Volcano::initialiseParticles
	Syntax		Volcano::initialiseParticles()
	Brief		Initialises the particle systems
*/
void Volcano::initialiseParticles()
{
	initialiseAsh();
	initialiseFire();
	initialiseSmoke();
}

/*
	Name		Volcano::initialiseAsh
	Syntax		Volcano::initialiseAsh()
	Brief		Initialises the ash particle system
*/
void Volcano::initialiseAsh()
{
	HRESULT hr = 0;

	std::vector<std::string> ash;
	ash.push_back("Assets/ash.dds");

	// Load the texture elements individually from file.  These textures
	// won't be used by the GPU (0 bind flags), they are just used to 
	// load the image data from file.  We use the STAGING usage so the
	// CPU can read the resource.
	UINT arraySize = (UINT)ash.size();

	std::vector<ID3D10Texture2D*> srcTex(arraySize, 0);
	for (UINT i = 0; i < arraySize; ++i)
	{
		D3DX10_IMAGE_LOAD_INFO loadInfo;

        loadInfo.Width  = D3DX10_FROM_FILE;
        loadInfo.Height = D3DX10_FROM_FILE;
        loadInfo.Depth  = D3DX10_FROM_FILE;
        loadInfo.FirstMipLevel = 0;
        loadInfo.MipLevels = D3DX10_FROM_FILE;
        loadInfo.Usage = D3D10_USAGE_STAGING;
        loadInfo.BindFlags = 0;
        loadInfo.CpuAccessFlags = D3D10_CPU_ACCESS_WRITE | D3D10_CPU_ACCESS_READ;
        loadInfo.MiscFlags = 0;
        loadInfo.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        loadInfo.Filter = D3DX10_FILTER_NONE;
        loadInfo.MipFilter = D3DX10_FILTER_NONE;
		loadInfo.pSrcInfo  = 0;

        hr = D3DX10CreateTextureFromFile(d3dDevice_, ash[i].c_str(), 
			&loadInfo, 0, (ID3D10Resource**)&srcTex[i], 0);

		if (FAILED(hr))
		{
			MessageBox(0, "Create ash texture - Failed", "Error", MB_OK);
			return;
		}
	}

	// Create the texture array.  Each element in the texture 
	// array has the same format/dimensions.
	D3D10_TEXTURE2D_DESC texElementDesc;
	srcTex[0]->GetDesc(&texElementDesc);

	D3D10_TEXTURE2D_DESC texArrayDesc;
	texArrayDesc.Width              = texElementDesc.Width;
	texArrayDesc.Height             = texElementDesc.Height;
	texArrayDesc.MipLevels          = texElementDesc.MipLevels;
	texArrayDesc.ArraySize          = arraySize;
	texArrayDesc.Format             = DXGI_FORMAT_R8G8B8A8_UNORM;
	texArrayDesc.SampleDesc.Count   = 1;
	texArrayDesc.SampleDesc.Quality = 0;
	texArrayDesc.Usage              = D3D10_USAGE_DEFAULT;
	texArrayDesc.BindFlags          = D3D10_BIND_SHADER_RESOURCE;
	texArrayDesc.CPUAccessFlags     = 0;
	texArrayDesc.MiscFlags          = 0;

	ID3D10Texture2D* texArray = 0;
	hr = d3dDevice_->CreateTexture2D( &texArrayDesc, 0, &texArray);
	if (FAILED(hr))
	{
		MessageBox(0, "Create ash array - Failed", "Error", MB_OK);
		return;
	}

	// Copy individual texture elements into texture array.

	// for each texture element...
	for (UINT i = 0; i < arraySize; ++i)
	{
		// for each mipmap level...
		for (UINT j = 0; j < texElementDesc.MipLevels; ++j)
		{
			D3D10_MAPPED_TEXTURE2D mappedTex2D;
			srcTex[i]->Map(j, D3D10_MAP_READ, 0, &mappedTex2D);
                    
            d3dDevice_->UpdateSubresource(texArray, 
				D3D10CalcSubresource(j, i, texElementDesc.MipLevels),
                0, mappedTex2D.pData, mappedTex2D.RowPitch, 0);

            srcTex[i]->Unmap(j);
		}
	}	

	// Create a resource view to the texture array.
	D3D10_SHADER_RESOURCE_VIEW_DESC viewDesc;
	viewDesc.Format = texArrayDesc.Format;
	viewDesc.ViewDimension = D3D10_SRV_DIMENSION_TEXTURE2DARRAY;
	viewDesc.Texture2DArray.MostDetailedMip = 0;
	viewDesc.Texture2DArray.MipLevels = texArrayDesc.MipLevels;
	viewDesc.Texture2DArray.FirstArraySlice = 0;
	viewDesc.Texture2DArray.ArraySize = arraySize;

	hr = d3dDevice_->CreateShaderResourceView(texArray, &viewDesc, &ashRV_);
	if (FAILED(hr))
	{
		MessageBox(0, "Create ash RV - Failed", "Error", MB_OK);
		return;
	}

	// Cleanup - we only need the resource view.
	texArray->Release();

	for (UINT i = 0; i < arraySize; ++i)
	{
		srcTex[i]->Release(); 
	}

	ash_ = new ParticleSystem(PARTICLE_ASH);
	ash_->initialise(d3dDevice_, ashRV_, 1000);
}

/*
	Name		Volcano::initialiseFire
	Syntax		Volcano::initialiseFire()
	Brief		Initialises the fire particle systems
*/
void Volcano::initialiseFire()
{
	HRESULT hr = 0;

	// FIRE PARTICLES
	std::vector<std::string> fire;
	fire.push_back("Assets/spark.dds");

	// Load the texture elements individually from file.  These textures
	// won't be used by the GPU (0 bind flags), they are just used to 
	// load the image data from file.  We use the STAGING usage so the
	// CPU can read the resource.
	UINT arraySize = (UINT)fire.size();

	std::vector<ID3D10Texture2D*> srcTex(arraySize, 0);
	for (UINT i = 0; i < arraySize; ++i)
	{
		D3DX10_IMAGE_LOAD_INFO loadInfo;

        loadInfo.Width  = D3DX10_FROM_FILE;
        loadInfo.Height = D3DX10_FROM_FILE;
        loadInfo.Depth  = D3DX10_FROM_FILE;
        loadInfo.FirstMipLevel = 0;
        loadInfo.MipLevels = D3DX10_FROM_FILE;
        loadInfo.Usage = D3D10_USAGE_STAGING;
        loadInfo.BindFlags = 0;
        loadInfo.CpuAccessFlags = D3D10_CPU_ACCESS_WRITE | D3D10_CPU_ACCESS_READ;
        loadInfo.MiscFlags = 0;
        loadInfo.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        loadInfo.Filter = D3DX10_FILTER_NONE;
        loadInfo.MipFilter = D3DX10_FILTER_NONE;
		loadInfo.pSrcInfo  = 0;

        hr = D3DX10CreateTextureFromFile(d3dDevice_, fire[i].c_str(), 
			&loadInfo, 0, (ID3D10Resource**)&srcTex[i], 0);

		if (FAILED(hr))
		{
			MessageBox(0, "Create fire spark texture - Failed", "Error", MB_OK);
			return;
		}
	}

	// Create the texture array.  Each element in the texture 
	// array has the same format/dimensions.
	D3D10_TEXTURE2D_DESC texElementDesc;
	srcTex[0]->GetDesc(&texElementDesc);

	D3D10_TEXTURE2D_DESC texArrayDesc;
	texArrayDesc.Width              = texElementDesc.Width;
	texArrayDesc.Height             = texElementDesc.Height;
	texArrayDesc.MipLevels          = texElementDesc.MipLevels;
	texArrayDesc.ArraySize          = arraySize;
	texArrayDesc.Format             = DXGI_FORMAT_R8G8B8A8_UNORM;
	texArrayDesc.SampleDesc.Count   = 1;
	texArrayDesc.SampleDesc.Quality = 0;
	texArrayDesc.Usage              = D3D10_USAGE_DEFAULT;
	texArrayDesc.BindFlags          = D3D10_BIND_SHADER_RESOURCE;
	texArrayDesc.CPUAccessFlags     = 0;
	texArrayDesc.MiscFlags          = 0;

	ID3D10Texture2D* texArray = 0;
	hr = d3dDevice_->CreateTexture2D( &texArrayDesc, 0, &texArray);
	if (FAILED(hr))
	{
		MessageBox(0, "Create fire array - Failed", "Error", MB_OK);
		return;
	}

	// Copy individual texture elements into texture array.

	// for each texture element...
	for (UINT i = 0; i < arraySize; ++i)
	{
		// for each mipmap level...
		for (UINT j = 0; j < texElementDesc.MipLevels; ++j)
		{
			D3D10_MAPPED_TEXTURE2D mappedTex2D;
			srcTex[i]->Map(j, D3D10_MAP_READ, 0, &mappedTex2D);
                    
            d3dDevice_->UpdateSubresource(texArray, 
				D3D10CalcSubresource(j, i, texElementDesc.MipLevels),
                0, mappedTex2D.pData, mappedTex2D.RowPitch, 0);

            srcTex[i]->Unmap(j);
		}
	}	

	// Create a resource view to the texture array.
	D3D10_SHADER_RESOURCE_VIEW_DESC viewDesc;
	viewDesc.Format = texArrayDesc.Format;
	viewDesc.ViewDimension = D3D10_SRV_DIMENSION_TEXTURE2DARRAY;
	viewDesc.Texture2DArray.MostDetailedMip = 0;
	viewDesc.Texture2DArray.MipLevels = texArrayDesc.MipLevels;
	viewDesc.Texture2DArray.FirstArraySlice = 0;
	viewDesc.Texture2DArray.ArraySize = arraySize;

	hr = d3dDevice_->CreateShaderResourceView(texArray, &viewDesc, &fireRV_);
	if (FAILED(hr))
	{
		MessageBox(0, "Create fire RV - Failed", "Error", MB_OK);
		return;
	}

	// Cleanup - we only need the resource view.
	texArray->Release();

	for (UINT i = 0; i < arraySize; ++i)
	{
		srcTex[i]->Release(); 
	}

	for (int i = 0; i < NUM_FIRE_SYSTEMS; ++i)
	{
		fire_[i] = new ParticleSystem(PARTICLE_FIRE);
		fire_[i]->initialise(d3dDevice_, fireRV_, 500);
	}
}

/*
	Name		Volcano::initialiseSmoke
	Syntax		Volcano::initialiseSmoke()
	Brief		Initialises the smoke particle systems
*/
void Volcano::initialiseSmoke()
{
	HRESULT hr = 0;

	// FIRE PARTICLES
	std::vector<std::string> smoke;
	smoke.push_back("Assets/smoke.dds");

	// Load the texture elements individually from file.  These textures
	// won't be used by the GPU (0 bind flags), they are just used to 
	// load the image data from file.  We use the STAGING usage so the
	// CPU can read the resource.
	UINT arraySize = (UINT)smoke.size();

	std::vector<ID3D10Texture2D*> srcTex(arraySize, 0);
	for (UINT i = 0; i < arraySize; ++i)
	{
		D3DX10_IMAGE_LOAD_INFO loadInfo;

        loadInfo.Width  = D3DX10_FROM_FILE;
        loadInfo.Height = D3DX10_FROM_FILE;
        loadInfo.Depth  = D3DX10_FROM_FILE;
        loadInfo.FirstMipLevel = 0;
        loadInfo.MipLevels = D3DX10_FROM_FILE;
        loadInfo.Usage = D3D10_USAGE_STAGING;
        loadInfo.BindFlags = 0;
        loadInfo.CpuAccessFlags = D3D10_CPU_ACCESS_WRITE | D3D10_CPU_ACCESS_READ;
        loadInfo.MiscFlags = 0;
        loadInfo.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        loadInfo.Filter = D3DX10_FILTER_NONE;
        loadInfo.MipFilter = D3DX10_FILTER_NONE;
		loadInfo.pSrcInfo  = 0;

        hr = D3DX10CreateTextureFromFile(d3dDevice_, smoke[i].c_str(), 
			&loadInfo, 0, (ID3D10Resource**)&srcTex[i], 0);

		if (FAILED(hr))
		{
			MessageBox(0, "Create smoke texture - Failed", "Error", MB_OK);
			return;
		}
	}

	// Create the texture array.  Each element in the texture 
	// array has the same format/dimensions.
	D3D10_TEXTURE2D_DESC texElementDesc;
	srcTex[0]->GetDesc(&texElementDesc);

	D3D10_TEXTURE2D_DESC texArrayDesc;
	texArrayDesc.Width              = texElementDesc.Width;
	texArrayDesc.Height             = texElementDesc.Height;
	texArrayDesc.MipLevels          = texElementDesc.MipLevels;
	texArrayDesc.ArraySize          = arraySize;
	texArrayDesc.Format             = DXGI_FORMAT_R8G8B8A8_UNORM;
	texArrayDesc.SampleDesc.Count   = 1;
	texArrayDesc.SampleDesc.Quality = 0;
	texArrayDesc.Usage              = D3D10_USAGE_DEFAULT;
	texArrayDesc.BindFlags          = D3D10_BIND_SHADER_RESOURCE;
	texArrayDesc.CPUAccessFlags     = 0;
	texArrayDesc.MiscFlags          = 0;

	ID3D10Texture2D* texArray = 0;
	hr = d3dDevice_->CreateTexture2D( &texArrayDesc, 0, &texArray);
	if (FAILED(hr))
	{
		MessageBox(0, "Create smoke array - Failed", "Error", MB_OK);
		return;
	}

	// Copy individual texture elements into texture array.

	// for each texture element...
	for (UINT i = 0; i < arraySize; ++i)
	{
		// for each mipmap level...
		for (UINT j = 0; j < texElementDesc.MipLevels; ++j)
		{
			D3D10_MAPPED_TEXTURE2D mappedTex2D;
			srcTex[i]->Map(j, D3D10_MAP_READ, 0, &mappedTex2D);
                    
            d3dDevice_->UpdateSubresource(texArray, 
				D3D10CalcSubresource(j, i, texElementDesc.MipLevels),
                0, mappedTex2D.pData, mappedTex2D.RowPitch, 0);

            srcTex[i]->Unmap(j);
		}
	}	

	// Create a resource view to the texture array.
	D3D10_SHADER_RESOURCE_VIEW_DESC viewDesc;
	viewDesc.Format = texArrayDesc.Format;
	viewDesc.ViewDimension = D3D10_SRV_DIMENSION_TEXTURE2DARRAY;
	viewDesc.Texture2DArray.MostDetailedMip = 0;
	viewDesc.Texture2DArray.MipLevels = texArrayDesc.MipLevels;
	viewDesc.Texture2DArray.FirstArraySlice = 0;
	viewDesc.Texture2DArray.ArraySize = arraySize;

	hr = d3dDevice_->CreateShaderResourceView(texArray, &viewDesc, &smokeRV_);
	if (FAILED(hr))
	{
		MessageBox(0, "Create smoke RV - Failed", "Error", MB_OK);
		return;
	}

	// Cleanup - we only need the resource view.
	texArray->Release();

	for (UINT i = 0; i < arraySize; ++i)
	{
		srcTex[i]->Release(); 
	}

	for (int i = 0; i < NUM_SMOKE_SYSTEMS; ++i)
	{
		smoke_[i] = new ParticleSystem(PARTICLE_SMOKE);
		smoke_[i]->initialise(d3dDevice_, smokeRV_, 10000);
	}
}

/*
	Name		Volcano::changeCamera
	Syntax		Volcano::changeCamera()
	Brief		Changes the camera being used in the scene
*/
void Volcano::changeCamera()
{
	if (currentCamera_ == CAMERA_ONE)
	{
		camera_ = cameraTwo_;
		camera_->initialise();
		currentCamera_ = CAMERA_TWO;
	}
	else
	{
		camera_ = cameraOne_;
		camera_->initialise();
		currentCamera_ = CAMERA_ONE;
	}
}

/*
	Name		Volcano::toggleHeatHaze
	Syntax		Volcano::toggleHeatHaze()
	Brief		Toggles heat haze on and off
*/
void Volcano::toggleHeatHaze()
{
	if (useHeatHaze_)
	{
		useHeatHaze_ = false;
	}
	else
	{
		useHeatHaze_ = true;
	}
}

/*
	Name		Volcano::togglePause
	Syntax		Volcano::togglePause()
	Brief		Togglespause on and off
*/
void Volcano::togglePause()
{
	if (paused_)
	{
		paused_ = false;
	}
	else
	{
		paused_ = true;
	}
}
