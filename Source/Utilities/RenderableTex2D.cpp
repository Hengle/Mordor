/*
	Created 	Elinor Townsend 2012
*/

/*	
	Name		Renderable Tex 2D
	Brief		Definition of Renderable Tex 2D Class methods
*/

#include "Utilities\RenderableTex2D.hpp"
#include "Scene\Scene.hpp"

/*
	Name		RenderableTex2D::RenderableTex2D
	Syntax		RenderableTex2D()
	Brief		RenderableTex2D constructor initialises member variables
*/
RenderableTex2D::RenderableTex2D()
: width_(0), height_(0),d3dDevice_(0), renderTargetTexture_(0), renderTargetView_(0), 
  shaderResourceView_(0), depthStencilBuffer_(0), depthStencilView_(0)
{
	ZeroMemory(&viewport_, sizeof(D3D10_VIEWPORT));
}

/*
	Name		RenderableTex2D::~RenderableTex2D
	Syntax		~RenderableTex2D()
	Brief		RenderableTex2D destructor
*/
RenderableTex2D::~RenderableTex2D()
{

}

/*
	Name		RenderableTex2D::initialise
	Syntax		RenderableTex2D::initialise(ID3D10Device* device)
	Brief		Initialises the renderable 2D texture, creating a textures and views
				for render target, depth and stencil
				
*/
void RenderableTex2D::initialise(ID3D10Device* device)
{
	width_  = Scene::instance()->getWidth();
	height_ = Scene::instance()->getHeight();
	d3dDevice_ = device;

	// Set up viewport
	viewport_.TopLeftX = 0;
	viewport_.TopLeftY = 0;
	viewport_.Width    = width_;
	viewport_.Height   = height_;
	viewport_.MinDepth = 0.0f;
	viewport_.MaxDepth = 1.0f;

	D3D10_TEXTURE2D_DESC textureDesc;
	
	// Initialize the render target texture description.
	ZeroMemory(&textureDesc, sizeof(textureDesc));

	// Setup the render target texture description.
	textureDesc.Width = width_;
	textureDesc.Height = height_;
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 1;
	textureDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.Usage = D3D10_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D10_BIND_RENDER_TARGET | D3D10_BIND_SHADER_RESOURCE;
	textureDesc.CPUAccessFlags = 0;
    textureDesc.MiscFlags = 0;

	// Create the render target texture.
	HRESULT hr = d3dDevice_->CreateTexture2D(&textureDesc, NULL, &renderTargetTexture_);
	if (FAILED(hr))
	{
		MessageBoxA(0, "Creating render 2d tex desc - Failed", "Error", MB_OK);
		return;
	}

	D3D10_RENDER_TARGET_VIEW_DESC renderTargetViewDesc;

	// Setup the description of the render target view.
	renderTargetViewDesc.Format = textureDesc.Format;
	renderTargetViewDesc.ViewDimension = D3D10_RTV_DIMENSION_TEXTURE2D;
	renderTargetViewDesc.Texture2D.MipSlice = 0;

	// Create the render target view.
	hr = d3dDevice_->CreateRenderTargetView(renderTargetTexture_, &renderTargetViewDesc, &renderTargetView_);
	if (FAILED(hr))
	{
		MessageBoxA(0, "Creating render2d target view desc - Failed", "Error", MB_OK);
		return;
	}

	D3D10_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc;

	// Setup the description of the shader resource view.
	shaderResourceViewDesc.Format = textureDesc.Format;
	shaderResourceViewDesc.ViewDimension = D3D10_SRV_DIMENSION_TEXTURE2D;
	shaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
	shaderResourceViewDesc.Texture2D.MipLevels = 1;

	// Create the shader resource view.
	hr = d3dDevice_->CreateShaderResourceView(renderTargetTexture_, &shaderResourceViewDesc, &shaderResourceView_);
	if (FAILED(hr))
	{
		MessageBoxA(0, "Creating render2d shader RV desc - Failed", "Error", MB_OK);
		return;
	}

	D3D10_TEXTURE2D_DESC depthBufferDesc;

	// Initialize the description of the depth buffer.
	ZeroMemory(&depthBufferDesc, sizeof(depthBufferDesc));

	// Set up the description of the depth buffer.
	depthBufferDesc.Width = width_;
	depthBufferDesc.Height = height_;
	depthBufferDesc.MipLevels = 1;
	depthBufferDesc.ArraySize = 1;
	depthBufferDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthBufferDesc.SampleDesc.Count = 1;
	depthBufferDesc.SampleDesc.Quality = 0;
	depthBufferDesc.Usage = D3D10_USAGE_DEFAULT;
	depthBufferDesc.BindFlags = D3D10_BIND_DEPTH_STENCIL;
	depthBufferDesc.CPUAccessFlags = 0;
	depthBufferDesc.MiscFlags = 0;

	// Create the texture for the depth buffer using the filled out description.
	hr = d3dDevice_->CreateTexture2D(&depthBufferDesc, NULL, &depthStencilBuffer_);
	if (FAILED(hr))
	{
		MessageBoxA(0, "Creating render2d depth buffer desc - Failed", "Error", MB_OK);
		return;
	}

	D3D10_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc;

	// Initialize the depth stencil view description.
	ZeroMemory(&depthStencilViewDesc, sizeof(depthStencilViewDesc));

	// Set up the depth stencil view description.
	depthStencilViewDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthStencilViewDesc.ViewDimension = D3D10_DSV_DIMENSION_TEXTURE2D;
	depthStencilViewDesc.Texture2D.MipSlice = 0;

	// Create the depth stencil view.
	hr = d3dDevice_->CreateDepthStencilView(depthStencilBuffer_, &depthStencilViewDesc, &depthStencilView_);
	if (FAILED(hr))
	{
		MessageBoxA(0, "Creating render2d depth stencil view desc - Failed", "Error", MB_OK);
		return;
	}
}

/*
	Name		RenderableTex2D::deinitialise
	Syntax		RenderableTex2D::deinitialise()
	Brief		Deinitialises the renderable 2D texture				
*/
void RenderableTex2D::deinitialise()
{
	if(depthStencilView_)
	{
		depthStencilView_->Release();
		depthStencilView_ = 0;
	}

	if(depthStencilBuffer_)
	{
		depthStencilBuffer_->Release();
		depthStencilBuffer_ = 0;
	}

	if(shaderResourceView_)
	{
		shaderResourceView_->Release();
		shaderResourceView_ = 0;
	}

	if(renderTargetView_)
	{
		renderTargetView_->Release();
		renderTargetView_ = 0;
	}

	if(renderTargetTexture_)
	{
		renderTargetTexture_->Release();
		renderTargetTexture_ = 0;
	}
}

/*
	Name		RenderableTex2D::setRenderTarget
	Syntax		RenderableTex2D::setRenderTarget()
	Brief		Sets the render target and depth/stencil views to the device and sets the viewport				
*/
void RenderableTex2D::setRenderTarget()
{
	// Bind the render target view and depth stencil buffer to the output render pipeline.
	d3dDevice_->OMSetRenderTargets(1, &renderTargetView_, depthStencilView_);
	
	// Set the viewport.
    d3dDevice_->RSSetViewports(1, &viewport_);

	clearRenderTarget();
}

/*
	Name		RenderableTex2D::clearRenderTarget
	Syntax		RenderableTex2D::clearRenderTarget()
	Brief		Clears the render target to black				
*/
void RenderableTex2D::clearRenderTarget()
{
	d3dDevice_->ClearRenderTargetView(renderTargetView_, D3DXVECTOR4(0, 0, 0, 0)); // Clear to black
	d3dDevice_->ClearDepthStencilView(depthStencilView_, D3D10_CLEAR_DEPTH|D3D10_CLEAR_STENCIL, 1.0f, 0); // Set depth to 1 (furthest away)
}