/*
	Created 	Elinor Townsend 2012
*/

/*	
	Name		Scene
	Brief		DirectX 3D Scene - maintains the Direct3D device and the FSM 
*/

#include "Scene\Scene.hpp"
#include "States\Volcano.hpp"
#include "Global\Global.hpp"

Scene* Scene::instance_ = 0;

/*
	Name		Scene::instance
	Syntax		Scene::instance()
	Brief		Create a single instance of Scene
*/
Scene * Scene::instance()
{
  if (!instance_)
      instance_ = new Scene();

  return instance_;
}

/*
	Name		Scene::Scene
	Syntax		Scene()
	Brief		Scene constructor initialises member variables
*/
Scene::Scene()
: d3dDevice_(0), swapChain_(0), renderTargetView_(0), depthStencilBuffer_(0),
  depthStencilView_(0), depthDisabledStencilState_(0), depthStencilState_(0), 
  width_(SCREENWIDTH), height_(SCREENHEIGHT), paused_(false), minimised_(false), 
  maximised_(false), resizing_(false), initialised_(false), currentState_(0), 
  screenNear_(SCREEN_NEAR), screenDepth_(SCREEN_DEPTH)
{
	aspect_ = (float)width_/height_;
}

/*
	Name		Scene::~Scene
	Syntax		~Scene()
	Brief		Scene destructor
*/
Scene::~Scene()
{

}

/*
	Name		Scene::initialise
	Syntax		Scene::initialise()
	Brief		Creates the Direct3D device and the DXGI swap chain 
	Details	The swap chain is responsible for getting the data from the D3D device and representing it on the screen
*/
void Scene::initialise()
{
	// DXGI_SWAP_CHAIN_DESC is used to describe the swap chain
	DXGI_SWAP_CHAIN_DESC swapChainDesc;
	ZeroMemory(&swapChainDesc, sizeof(swapChainDesc));

	// Description of the display mode
	swapChainDesc.BufferDesc.Width = width_;
	swapChainDesc.BufferDesc.Height = height_;
	swapChainDesc.BufferDesc.RefreshRate.Numerator = 60;
	swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
	swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

	// Multi-sampling parameters (count 1, quality 0 disables multi-sampling)
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.SampleDesc.Quality = 0;
	
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.BufferCount = 1;

	swapChainDesc.OutputWindow = ghWnd;
	swapChainDesc.Windowed = true;

	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	swapChainDesc.Flags = 0;

	// Create the device and swap chain
	HRESULT hr = D3D10CreateDeviceAndSwapChain(NULL, D3D10_DRIVER_TYPE_HARDWARE, NULL, D3D10_CREATE_DEVICE_DEBUG, 
									D3D10_SDK_VERSION, &swapChainDesc, &swapChain_, &d3dDevice_ );
	if(FAILED(hr))
	{
		MessageBox(0, "Creating device and swap chain - Failed",
			"Error", MB_OK);
	}
	timer_.reset();

	initialised_ = true;

	onResize();

	// Set initial state for the scene
	currentState_ = new Volcano;
	currentState_->initialise();
}

/*
	Name		Scene::resetOMTargetsAndViewport
	Syntax		Scene::resetOMTargetsAndViewport()
	Brief		Resets the OM targets and viewport
*/
void Scene::resetOMTargetsAndViewport()
{
	d3dDevice_->OMSetRenderTargets(1, &renderTargetView_, depthStencilView_);

	D3D10_VIEWPORT vp;
	vp.TopLeftX = 0;
	vp.TopLeftY = 0;
	vp.Width    = width_;
	vp.Height   = height_;
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;

	d3dDevice_->RSSetViewports(1, &vp);

	startFrame();
}

/*
	Name		Scene::onResize
	Syntax		Scene::onResize()
	Brief		
	Details	Retrieves the back buffer from the swap chain as a texture, creates a render target 
				using the texture, and passes it to D3D device as the active render target
*/
void Scene::onResize()
{
	if(initialised_)
	{

		setResizing(true);

		// Release the old views
		if(renderTargetView_)
			renderTargetView_->Release();

		if(depthStencilView_)
			depthStencilView_->Release();

		if(depthStencilBuffer_)
			depthStencilBuffer_->Release();

		// Resize the swap chain
		swapChain_->ResizeBuffers(1, width_, height_, DXGI_FORMAT_R8G8B8A8_UNORM, 0);

		// Set up the back buffer of the swap chain to be used as a render target by the D3D device
		ID3D10Texture2D* backBuffer;

		swapChain_->GetBuffer(0, __uuidof(ID3D10Texture2D), reinterpret_cast<void**>(&backBuffer));
		d3dDevice_->CreateRenderTargetView(backBuffer, 0, &renderTargetView_);
		
		backBuffer->Release();


		// Create the depth/stencil buffer.
		D3D10_TEXTURE2D_DESC depthBufferDesc;
		
		depthBufferDesc.Width = width_;
		depthBufferDesc.Height = height_;
		depthBufferDesc.MipLevels = 1;
		depthBufferDesc.ArraySize = 1;
		depthBufferDesc.Format= DXGI_FORMAT_D24_UNORM_S8_UINT;

		// Multi-sampling must match the swap chain values
		depthBufferDesc.SampleDesc.Count = 1; 
		depthBufferDesc.SampleDesc.Quality = 0;

		depthBufferDesc.Usage = D3D10_USAGE_DEFAULT;
		depthBufferDesc.BindFlags = D3D10_BIND_DEPTH_STENCIL;
		depthBufferDesc.CPUAccessFlags = 0; 
		depthBufferDesc.MiscFlags = 0;

		d3dDevice_->CreateTexture2D(&depthBufferDesc, 0, &depthStencilBuffer_);


		// Create the depth/stencil state.
		D3D10_DEPTH_STENCIL_DESC depthStencilDesc;

		// Initialize the description of the stencil state.
		ZeroMemory(&depthStencilDesc, sizeof(depthStencilDesc));

		// Set up the description of the stencil state.
		depthStencilDesc.DepthEnable = true;
		depthStencilDesc.DepthWriteMask = D3D10_DEPTH_WRITE_MASK_ALL;
		depthStencilDesc.DepthFunc = D3D10_COMPARISON_LESS;

		depthStencilDesc.StencilEnable = true;
		depthStencilDesc.StencilReadMask = 0xFF;
		depthStencilDesc.StencilWriteMask = 0xFF;

		// Stencil operations if pixel is front-facing.
		depthStencilDesc.FrontFace.StencilFailOp = D3D10_STENCIL_OP_KEEP;
		depthStencilDesc.FrontFace.StencilDepthFailOp = D3D10_STENCIL_OP_INCR;
		depthStencilDesc.FrontFace.StencilPassOp = D3D10_STENCIL_OP_KEEP;
		depthStencilDesc.FrontFace.StencilFunc = D3D10_COMPARISON_ALWAYS;

		// Stencil operations if pixel is back-facing.
		depthStencilDesc.BackFace.StencilFailOp = D3D10_STENCIL_OP_KEEP;
		depthStencilDesc.BackFace.StencilDepthFailOp = D3D10_STENCIL_OP_DECR;
		depthStencilDesc.BackFace.StencilPassOp = D3D10_STENCIL_OP_KEEP;
		depthStencilDesc.BackFace.StencilFunc = D3D10_COMPARISON_ALWAYS;

		// Create the depth stencil state.
		d3dDevice_->CreateDepthStencilState(&depthStencilDesc, &depthStencilState_);

		// Set the depth stencil state on the D3D device.
		d3dDevice_->OMSetDepthStencilState(depthStencilState_, 1);


		// Create depth disabled depth/stencil state.
		D3D10_DEPTH_STENCIL_DESC depthDisabledStencilDesc;

		// Clear the second depth stencil state before setting the parameters.
		ZeroMemory(&depthDisabledStencilDesc, sizeof(depthDisabledStencilDesc));

		// Now create a second depth stencil state which turns off the Z buffer for 2D rendering.  The only difference is 
		// that DepthEnable is set to false, all other parameters are the same as the other depth stencil state.
		depthDisabledStencilDesc.DepthEnable = false;
		depthDisabledStencilDesc.DepthWriteMask = D3D10_DEPTH_WRITE_MASK_ALL;
		depthDisabledStencilDesc.DepthFunc = D3D10_COMPARISON_LESS;
		depthDisabledStencilDesc.StencilEnable = true;
		depthDisabledStencilDesc.StencilReadMask = 0xFF;
		depthDisabledStencilDesc.StencilWriteMask = 0xFF;
		depthDisabledStencilDesc.FrontFace.StencilFailOp = D3D10_STENCIL_OP_KEEP;
		depthDisabledStencilDesc.FrontFace.StencilDepthFailOp = D3D10_STENCIL_OP_INCR;
		depthDisabledStencilDesc.FrontFace.StencilPassOp = D3D10_STENCIL_OP_KEEP;
		depthDisabledStencilDesc.FrontFace.StencilFunc = D3D10_COMPARISON_ALWAYS;
		depthDisabledStencilDesc.BackFace.StencilFailOp = D3D10_STENCIL_OP_KEEP;
		depthDisabledStencilDesc.BackFace.StencilDepthFailOp = D3D10_STENCIL_OP_DECR;
		depthDisabledStencilDesc.BackFace.StencilPassOp = D3D10_STENCIL_OP_KEEP;
		depthDisabledStencilDesc.BackFace.StencilFunc = D3D10_COMPARISON_ALWAYS;

		// Create the state using the device.
		d3dDevice_->CreateDepthStencilState(&depthDisabledStencilDesc, &depthDisabledStencilState_);

		// Set the viewport transform.
		D3D10_VIEWPORT viewport = {0, 0, width_, height_, 0, 1};

		// Create the viewport.
		d3dDevice_->RSSetViewports(1, &viewport);

	
		// Create the projection matrix for 3D rendering.
		aspect_ = (float)width_/height_;
		D3DXMatrixPerspectiveFovLH(&projection_, (float)D3DX_PI * 0.25f, aspect_, screenNear_, screenDepth_);

		// Initialize the world matrix to the identity matrix.
		D3DXMatrixIdentity(&world_);

		// Create an orthogonal projection matrix for 2D rendering.
		D3DXMatrixOrthoLH(&orthogonal_, (float)width_, (float)height_, screenNear_, screenDepth_);


		// Create the depth/stencil view.
		D3D10_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc;

		// Initialize the depth stencil view.
		ZeroMemory(&depthStencilViewDesc, sizeof(depthStencilViewDesc));

		// Set up the depth stencil view description.
		depthStencilViewDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		depthStencilViewDesc.ViewDimension = D3D10_DSV_DIMENSION_TEXTURE2D;
		depthStencilViewDesc.Texture2D.MipSlice = 0;

		// Create the depth stencil view.
		d3dDevice_->CreateDepthStencilView(depthStencilBuffer_, 0, &depthStencilView_);

		// Bind the render target view and depth stencil buffer to the output render pipeline.
		d3dDevice_->OMSetRenderTargets(1, &renderTargetView_, depthStencilView_);
	}
}

/*
	Name		Scene::runFrame
	Syntax		Scene::runFrame()
	Return		bool - False if the scene is changing state
	Brief		Runs the current frame of the scene
*/
bool Scene::runFrame()
{
	timer_.tick();
	bool stateOver = currentState_->update(timer_.getDeltaTime());

	if(stateOver)
	{
		changeState();
	}
	else
	{
		startFrame();
		currentState_->render();
		endFrame();
	}
	if(currentState_)
		return true;
	else
	{
		return false;
	}
}

/*
	Name		Scene::deinitialise
	Syntax		Scene::deinitialise()
	Brief		Ends the scene
*/
void Scene::deinitialise()
{
	if(swapChain_)
	{
		swapChain_->SetFullscreenState(false, NULL);
	}

	if(depthDisabledStencilState_)
	{
		depthDisabledStencilState_->Release();
		depthDisabledStencilState_ = 0;
	}

	if(depthStencilView_)
	{
		depthStencilView_->Release();
		depthStencilView_ = 0;
	}

	if(depthStencilState_)
	{
		depthStencilState_->Release();
		depthStencilState_ = 0;
	}

	if(depthStencilBuffer_)
	{
		depthStencilBuffer_->Release();
		depthStencilBuffer_ = 0;
	}

	if(renderTargetView_)
	{
		renderTargetView_->Release();
		renderTargetView_ = 0;
	}

	if(swapChain_)
	{
		swapChain_->Release();
		swapChain_ = 0;
	}

	if(d3dDevice_)
	{
		d3dDevice_->Release();
		d3dDevice_ = 0;
	}
}

/*
	Name		Scene::setWidth
	Syntax		Scene::setWidth(int width)
	Param		int width - The width of the window
	Brief		Sets the window width
*/
void Scene::setWidth(int width)
{
	width_ = width;
}

/*
	Name		Scene::setHeight
	Syntax		Scene::setHeight(int height)
	Param		int height - The height of the window
	Brief		Sets the window height
*/
void Scene::setHeight(int height)
{
	height_ = height;
}

/*
	Name		Scene::setPaused
	Syntax		Scene::setPaused(bool paused)
	Param		bool paused - Flag to indicate if the scene is paused
	Brief		Sets the window paused flag
*/
void Scene::setPaused(bool paused)
{
	paused_ = paused;
}

/*
	Name		Scene::setMinimised
	Syntax		Scene::setMinimised(bool minimised)
	Param		bool minimised - Flag to indicate if the scene is minimised
	Brief		Sets the window minimised flag
*/
void Scene::setMinimised(bool minimised)
{
	minimised_ = minimised;
}

/*
	Name		Scene::setMaximised
	Syntax		Scene::setMaximised(bool maximised)
	Param		bool maximised - Flag to indicate if the scene is maximised
	Brief		Sets the window maximised flag
*/
void Scene::setMaximised(bool maximised)
{
	maximised_ = maximised;
}

/*
	Name		Scene::setResizing
	Syntax		Scene::setResizing(bool resizing)
	Param		bool resizing - Flag to indicate if the scene is resizing
	Brief		Sets the window resizing flag
*/
void Scene::setResizing(bool resizing)
{
	resizing_ = resizing;
}

/*
	Name		Scene::setWorld
	Syntax		Scene::setWorld(D3DXMATRIX world)
	Param		bool world - World matrix
	Brief		Sets the world matrix
*/
void Scene::setWorld(D3DXMATRIX world)
{
	world_ = world;
}

/*
	Name		Scene::setView
	Syntax		Scene::setView(D3DXMATRIX view)
	Param		bool view - View matrix
	Brief		Sets the view matrix
*/
void Scene::setView(D3DXMATRIX view)
{
	view_ = view;
}

/*
	Name		Scene::setProjection
	Syntax		Scene::setProjection(D3DXMATRIX projection)
	Param		bool projection - Projection matrix
	Brief		Sets the projection matrix
*/
void Scene::setProjection(D3DXMATRIX projection)
{
	projection_ = projection;
}

/*
	Name		Scene::setWVP
	Syntax		Scene::setWVP()
	Brief		Sets the projection matrix
*/
void Scene::setWVP()
{
	wvp_ = world_ * view_ * projection_;
}

/*
	Name		Scene::start
	Syntax		Scene::start()
	Brief		Starts the frame
*/
void Scene::startFrame()
{
	d3dDevice_->ClearRenderTargetView(renderTargetView_, D3DXVECTOR4(0, 0, 0, 1)); // Clear to black
	d3dDevice_->ClearDepthStencilView(depthStencilView_, D3D10_CLEAR_DEPTH|D3D10_CLEAR_STENCIL, 1.0f, 0); // Set depth to 1 (furthest away)
}

/*
	Name		Scene::end
	Syntax		Scene::end()
	Brief		Ends the frame
*/
void Scene::endFrame()
{
	swapChain_->Present(0, 0);
}

/*
	Name		Scene::changeState
	Syntax		Scene::changeState()
	Brief		Changes the scene's state
*/
void Scene::changeState()
{
	if(currentState_)
	{
		State* state = currentState_->getNextState();
		
		currentState_->deinitialise();
        delete currentState_;

        currentState_ = state;
		currentState_->initialise();
	}
}

void Scene::turnZBufferOn()
{
	d3dDevice_->OMSetDepthStencilState(depthStencilState_, 1);
	return;
}


void Scene::turnZBufferOff()
{
	d3dDevice_->OMSetDepthStencilState(depthDisabledStencilState_, 1);
	return;
}

