/*
	Created 	Elinor Townsend 2012
*/

/*	
	Name		Sky Sphere
	Brief		Definition of Sky Sphere Class used for creating a sphere
*/

#include "Geometry/SkySphere.hpp"
#include "Graphics/Vertex.hpp"

#define VERTICES_NO 18
#define FACES_NO 28

/*
	Name		SkySphere::SkySphere
	Syntax		SkySphere()
	Brief		SkySphere constructor
*/
SkySphere::SkySphere()
: verticesNo_(0), facesNo_(0), d3dDevice_(0), vertexBuffer_(0), indexBuffer_(0), 
  scale_(1,1,1), pos_(0,0,0)
{

}

/*
	Name		SkySphere::~SkySphere
	Syntax		~SkySphere()
	Brief		SkySphere destructor
*/
SkySphere::~SkySphere()
{
	if (vertexBuffer_)
		vertexBuffer_->Release();
	if (indexBuffer_)
		indexBuffer_->Release();
}

/*
	Name		SkySphere::initialise
	Syntax		SkySphere::initialise(ID3D10Device* device)
	Param		ID3D10Device* device - pointer to the D3D device
	Brief		Creates the sky sphere's vertices and indices
*/
void SkySphere::initialise(ID3D10Device* device)
{
	d3dDevice_ = device;
 
	verticesNo_ = VERTICES_NO;
	facesNo_    = FACES_NO;

	int width = 8;	
	int height = 4;	
	float theta, phi;
	int i, j, t, ntri, nvec;
	
	nvec = (height - 2) * width + 2;		
	ntri = (height - 2) * (width - 1) * 2;
	
	// Create vertex buffer
    Vertex vertices[VERTICES_NO];
	t = 0;
	for (j = 1; j < height - 1; ++j)
	{
		for (i = 0; i < width; ++i)		
		{			
			theta = float(j) / (height - 1) * (float)D3DX_PI;			
			phi   = float(i) / (width - 1)  * (float)(D3DX_PI * 2);	
			vertices[t++].pos = D3DXVECTOR3(sinf(theta) * cosf(phi), cosf(theta), -sinf(theta) * sinf(phi));
		}		
	}	
	vertices[t++].pos = D3DXVECTOR3(0.0f, 1.0f, 0.0f);		
	vertices[t++].pos = D3DXVECTOR3(0.0f, -1.0f, 0.0f);
	

    D3D10_BUFFER_DESC vbd;
    vbd.Usage = D3D10_USAGE_IMMUTABLE;
    vbd.ByteWidth = sizeof(Vertex) * verticesNo_;
    vbd.BindFlags = D3D10_BIND_VERTEX_BUFFER;
    vbd.CPUAccessFlags = 0;
    vbd.MiscFlags = 0;
    D3D10_SUBRESOURCE_DATA vinitData;
    vinitData.pSysMem = vertices;
    HRESULT hr = d3dDevice_->CreateBuffer(&vbd, &vinitData, &vertexBuffer_);
	if (FAILED(hr))
	{
		MessageBox(0, "Creating sky sphere vertex buffer - Failed", "Error", MB_OK);
	}

	// Create the index buffer

	DWORD indices[FACES_NO * 3]; 
	t = 0;
	for (j = 0; j < height - 3; ++j)
	{
		for (i = 0; i < width - 1; ++i)			
		{				
			indices[t++] = (j  )	* width + i  ;				
			indices[t++] = (j + 1)	* width + i + 1;				
			indices[t++] = (j  )	* width + i + 1;				
			indices[t++] = (j  )	* width + i  ;				
			indices[t++] = (j + 1)	* width + i  ;				
			indices[t++] = (j + 1)	* width + i + 1;			
		}			
	}	
	for (i = 0; i < width - 1; ++i)			
	{				
		indices[t++] = (height - 2) * width;				
		indices[t++] = i;				
		indices[t++] = i + 1;				
		indices[t++] = (height - 2) * width + 1;				
		indices[t++] = (height - 3) * width + i + 1;				
		indices[t++] = (height - 3) * width + i;			
	}	
	

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
		MessageBox(0, "Creating sky sphere index buffer - Failed", "Error", MB_OK);
	}

	D3DXMatrixIdentity(&world_);
}

/*
	Name		SkySphere::render
	Syntax		SkySphere::render()
	Brief		Renders the sky sphere
*/
void SkySphere::render()
{
	UINT stride = sizeof(Vertex);
    UINT offset = 0;
    d3dDevice_->IASetVertexBuffers(0, 1, &vertexBuffer_, &stride, &offset);
	d3dDevice_->IASetIndexBuffer(indexBuffer_, DXGI_FORMAT_R32_UINT, 0);
	d3dDevice_->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

/*
	Name		SkySphere::setTrans
	Syntax		SkySphere::setTrans()
	Brief		Applies the sky sphere's translation to its world matrix
*/
void SkySphere::setTrans()
{
	D3DXMATRIX m;
	D3DXMatrixIdentity(&world_);
	D3DXMatrixScaling(&m, scale_.x, scale_.y, scale_.z);
	world_ *= m;
	D3DXMatrixTranslation(&m, pos_.x, pos_.y, pos_.z);
	world_ *= m;
}

/*
	Name		SkySphere::setPos
	Syntax		SkySphere::setPos(D3DXVECTOR3 pos)
	Param		D3DXVECTOR3 pos - Position vector to move sky sphere to
	Brief		Sets the sky sphere's position
*/
void SkySphere::setPos(D3DXVECTOR3 pos)
{
	pos_ = pos;
}

/*
	Name		SkySphere::setScale
	Syntax		SkySphere::setScale(float x, float y, float z)
	Param		float x - Value to set scale along the x axis to
	Param		float y - Value to set scale along the y axis to
	Param		float z - Value to set scale along the z axis to
	Brief		Set the sky sphere's scale
*/
void SkySphere::setScale(float x, float y, float z)
{
	scale_.x = x;
	scale_.y = y;
	scale_.z = z;
}