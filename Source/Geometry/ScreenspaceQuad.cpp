/*
	Created 	Elinor Townsend 2012
*/

/*	
	Name		Screenspace Quad
	Brief		Definition of Screenspace Quad Class used for creating a quad in screenspace
*/

#include "Geometry\ScreenspaceQuad.hpp"
#include "Graphics\Vertex.hpp"
#include "Scene\Scene.hpp"

#define VERTICES_NO 4
#define FACES_NO 2

/*
	Name		ScreenspaceQuad::ScreenspaceQuad
	Syntax		ScreenspaceQuad()
	Brief		ScreenspaceQuad constructor
*/
ScreenspaceQuad::ScreenspaceQuad()
: verticesNo_(0), facesNo_(0), d3dDevice_(0), vertexBuffer_(0), indexBuffer_(0), initialised_(false)
{

}

/*
	Name		ScreenspaceQuad::~ScreenspaceQuad
	Syntax		~ScreenspaceQuad()
	Brief		ScreenspaceQuad destructor
*/
ScreenspaceQuad::~ScreenspaceQuad()
{
	if (vertexBuffer_)
		vertexBuffer_->Release();
	if (indexBuffer_)
		indexBuffer_->Release();
}

/*
	Name		ScreenspaceQuad::initialise
	Syntax		ScreenspaceQuad::initialise(ID3D10Device* device)
	Param		ID3D10Device* device - pointer to the D3D device
	Brief		Creates the quad's vertices and indices
*/
void ScreenspaceQuad::initialise(ID3D10Device* device)
{
	d3dDevice_ = device;
 
	verticesNo_ = VERTICES_NO;
	facesNo_    = FACES_NO;

	// Create vertex buffer
    SimpleVertex vertices[VERTICES_NO];

	int halfWidth = Scene::instance()->getWidth()/2;
	int halfHeight = Scene::instance()->getHeight()/2;

	float left = -halfWidth;
	float right = halfWidth;
	float top = halfHeight;
	float bottom = -halfHeight;

	// First triangle.
	vertices[0].pos = D3DXVECTOR3(left, top, 50.0f);  // Top left.
	vertices[0].texC = D3DXVECTOR2(0.0f, 0.0f);

	vertices[1].pos = D3DXVECTOR3(right, top, 50.0f);  // Top right.
	vertices[1].texC = D3DXVECTOR2(1.0f, 0.0f);
	
	vertices[2].pos = D3DXVECTOR3(right, bottom, 0.0f);  // Bottom right.
	vertices[2].texC = D3DXVECTOR2(1.0f, 1.0f);

	vertices[3].pos = D3DXVECTOR3(left, bottom, 0.0f);  // Bottom left.
	vertices[3].texC = D3DXVECTOR2(0.0f, 1.0f);



    D3D10_BUFFER_DESC vbd;
    vbd.Usage = D3D10_USAGE_DYNAMIC;
    vbd.ByteWidth = sizeof(SimpleVertex) * verticesNo_;
    vbd.BindFlags = D3D10_BIND_VERTEX_BUFFER;
    vbd.CPUAccessFlags = D3D10_CPU_ACCESS_WRITE;
    vbd.MiscFlags = 0;
    D3D10_SUBRESOURCE_DATA vinitData;
    vinitData.pSysMem = vertices;
    HRESULT hr = d3dDevice_->CreateBuffer(&vbd, &vinitData, &vertexBuffer_);
	if (FAILED(hr))
	{
		MessageBox(0, "Creating screenspace quad vertex buffer - Failed", "Error", MB_OK);
	}

	// Create the index buffer

	DWORD indices[FACES_NO * 3]; 
	
	indices[0] = 0;
	indices[1] = 1;
	indices[2] = 3;
	indices[3] = 1;
	indices[4] = 2;
	indices[5] = 3;
	

	D3D10_BUFFER_DESC ibd;
    ibd.Usage = D3D10_USAGE_IMMUTABLE;
    ibd.ByteWidth = sizeof(DWORD) * facesNo_ * 3;
    ibd.BindFlags = D3D10_BIND_INDEX_BUFFER;
    ibd.CPUAccessFlags = 0;
    ibd.MiscFlags = 0;
    D3D10_SUBRESOURCE_DATA iinitData;
    iinitData.pSysMem = indices;
    hr = d3dDevice_->CreateBuffer(&ibd, &iinitData, &indexBuffer_);
	if (FAILED(hr))
	{
		MessageBox(0, "Creating screenspace quad buffer - Failed", "Error", MB_OK);
	}

	initialised_ = true;
}

/*
	Name		ScreenspaceQuad::render
	Syntax		ScreenspaceQuad::render()
	Brief		Renders the sky sphere
*/
void ScreenspaceQuad::render()
{
	UINT stride = sizeof(SimpleVertex);
    UINT offset = 0;
    d3dDevice_->IASetVertexBuffers(0, 1, &vertexBuffer_, &stride, &offset);
	d3dDevice_->IASetIndexBuffer(indexBuffer_, DXGI_FORMAT_R32_UINT, 0);
	d3dDevice_->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

/*
	Name		ScreenspaceQuad::updateVertices
	Syntax		ScreenspaceQuad::updateVertices()
	Brief		Updates the vertices to the current window dimensions
*/
void ScreenspaceQuad::updateVertices()
{
	if (initialised_)
	{
		// Create vertex buffer
		SimpleVertex vertices[VERTICES_NO];

		int halfWidth = Scene::instance()->getWidth()/2;
		int halfHeight = Scene::instance()->getHeight()/2;

		float left = -halfWidth;
		float right = halfWidth;
		float top = halfHeight;
		float bottom = -halfHeight;

		// First triangle.
		vertices[0].pos = D3DXVECTOR3(left, top, 50.0f);  // Top left.
		vertices[0].texC = D3DXVECTOR2(0.0f, 0.0f);

		vertices[1].pos = D3DXVECTOR3(right, top, 50.0f);  // Top right.
		vertices[1].texC = D3DXVECTOR2(1.0f, 0.0f);
	
		vertices[2].pos = D3DXVECTOR3(right, bottom, 0.0f);  // Bottom right.
		vertices[2].texC = D3DXVECTOR2(1.0f, 1.0f);

		vertices[3].pos = D3DXVECTOR3(left, bottom, 0.0f);  // Bottom left.
		vertices[3].texC = D3DXVECTOR2(0.0f, 1.0f);

		// Initialize the vertex buffer pointer
		void* verts = 0;

		// Lock the vertex buffer
		HRESULT hr = vertexBuffer_->Map(D3D10_MAP_WRITE_DISCARD, 0, (void**)&verts);
		if(FAILED(hr))
		{
			MessageBox(0, "Updating quad vertices - Failed", "Error", MB_OK);
		}

		// Copy the data into the vertex buffer
		memcpy(verts, (void*)vertices, (sizeof(SimpleVertex) * verticesNo_));

		// Unlock the vertex buffer
		vertexBuffer_->Unmap();
	}
}