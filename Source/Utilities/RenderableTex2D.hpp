/*
	Created 	Elinor Townsend 2012
*/

/*	
	Name		Renderable Texture 2D
	Brief		Declaration of Renderable Texture 2D Class
*/

#ifndef RENDERABLETEX2D_H
#define RENDERABLETEX2D_H

#include <d3dx10.h>

class RenderableTex2D
{
public:
	RenderableTex2D();
	~RenderableTex2D();

	void initialise(ID3D10Device* device);
	void deinitialise();

	ID3D10ShaderResourceView* getTextureSRV() const { return shaderResourceView_; };

	void setRenderTarget();
	

private:
	void clearRenderTarget();
	
	int width_;
	int height_;
	ID3D10Device* d3dDevice_;
	ID3D10Texture2D* renderTargetTexture_;
	ID3D10RenderTargetView* renderTargetView_;
	ID3D10ShaderResourceView* shaderResourceView_;
	ID3D10Texture2D* depthStencilBuffer_;
	ID3D10DepthStencilView* depthStencilView_;
	D3D10_VIEWPORT viewport_;
};

#endif // RENDERABLETEX2D_H