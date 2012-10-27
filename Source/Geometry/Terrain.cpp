/*
	Created 	Elinor Townsend 2012
*/

/*	
	Name		Terrain
	Brief		Definition of Terrain Class
*/

#include <vector>
#include <fstream>
#include "Geometry\Terrain.hpp"
#include "Graphics\Vertex.hpp"
#include "Utilities\SimplexNoise.hpp"
#include "Scene\Scene.hpp"
#include "Global\Global.hpp"

/*
	Name		Terrain::Terrain
	Syntax		Terrain()
	Brief		Terrain constructor
*/
Terrain::Terrain() 
: currentGenStage_(GEN_FLAT),
  age_(5.0f),
  scale_(1,1,1), 
  theta_(0,0,0), 
  pos_(0,0,0), 
  verticesNo_(0), 
  facesNo_(0), 
  vertices_(0), 
  heightMap_(0), 
  indices_(0), 
  d3dDevice_(0), 
  vertexBuffer_(0), 
  indexBuffer_(0), 
  width_(0), 
  height_(0), 
  craterX_(0),
  craterZ_(0),
  craterRadius_(0),
  isComplete_(false),
  ashEmitter_(0,0,0),
  heightMapRV_(0)
{

}

/*
	Name		Terrain::~Terrain
	Syntax		~Terrain()
	Brief		Terrain denstructor
*/
Terrain::~Terrain()
{
	if (vertices_)
	{
		delete vertices_;
		vertices_ = 0;
	}
	if (heightMap_)
	{
		delete heightMap_;
		heightMap_ = 0;
	}
	if (indices_)
	{
		delete indices_;
		indices_ = 0;
	}
	if (vertexBuffer_)
	{
		vertexBuffer_->Release();
		vertexBuffer_ = 0;
	}
	if (indexBuffer_)
	{
		indexBuffer_->Release();
		indexBuffer_ = 0;
	}
}

/*
	Name		Terrain::initialise
	Syntax		Terrain::initialise(ID3D10Device* device, int gridSize)
	Param		ID3D10Device* device - Pointer to the Direct3D device
	Param		int gridSize - The number of vertices wide to set the terrain grid at
	Brief		Initialises the vertex and index buffers
*/
void Terrain::initialise(ID3D10Device* device, int gridSize)
{
	d3dDevice_ = device;

	width_ = height_ = gridSize + 1;

	if (createTerrain())
	{
		createBuffers();
	}
	setTrans();
}

/*
	Name		Terrain::render
	Syntax		Terrain::render()
	Brief		Renders the terrain
*/
void Terrain::render()
{
	// Set the type of primitive to triangle list
	d3dDevice_->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	UINT stride = sizeof(Vertex);
    UINT offset = 0;
    d3dDevice_->IASetVertexBuffers(0, 1, &vertexBuffer_, &stride, &offset);
	d3dDevice_->IASetIndexBuffer(indexBuffer_, DXGI_FORMAT_R32_UINT, 0);

	return;
}

/*
	Name		Terrain::createTerrain
	Syntax		Terrain::createTerrain()
	Return		bool - True once vertices and indices created
	Brief		Creates the vertices and indices for the terrain
*/
bool Terrain::createTerrain()
{
	UINT i = 0;
	UINT j = 0;

	// Calculate the number of vertices in the terrain mesh
	verticesNo_ = width_ * height_;

	// Initialise final heights
	if (!heightMap_)
	{
		heightMap_ = new float[verticesNo_];
		for (unsigned int v = 0; v < verticesNo_; ++v)
		{
			heightMap_[v] = 0.0f;
		}
	}

	// Three vertices for each face
	facesNo_ = (width_-1) * (height_-1) * 2;

	vertices_ = new Vertex[verticesNo_];
	if (!vertices_)
	{
		return false;
	}

	indices_ = new DWORD[facesNo_ * 3];
	if (!indices_)
	{
		return false;
	}

	// Load the vertex array with the terrain data
	float du = 1.0f / 128.0f;
	float dv = 1.0f / 128.0f;
	int index = 0;
	for (i = 0; i < height_; ++i)
	{
		for (j = 0; j < width_; ++j)
		{
			index = i * width_ + j;
			vertices_[index].pos = D3DXVECTOR3((float)i, 0.0f, (float)j);
			
			vertices_[index].normal = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
			vertices_[index].texC.x = i * du;
			vertices_[index].texC.y = j * dv;
			vertices_[index].type = ROCK;
		}
	}

	// Drop edges of terrain 
	for (i = 0; i < width_; i++)
	{
		vertices_[i].pos.y = -100.0f;
		index = (width_-1) * width_ + i;
		vertices_[index].pos.y = -100.0f;
	}

	for (j = 0; j < height_; j++)
	{
		index = j * width_;
		vertices_[index].pos.y = -100.0f;
		index = j * width_ + height_-1;
		vertices_[index].pos.y = -100.0f;
	}

	int k = 0;
	for (i = 0; i < width_ - 1; ++i)
	{
		for (j = 0; j < height_ - 1; ++j)
		{
			if (((i % 2 == 0) && (j % 2 == 0)) ||
				((i % 2 != 0) && (j % 2 != 0)))

			{
				indices_[k]   = i		* height_ + (j + 1);
				indices_[k+1] = (i + 1) * height_ + (j + 1);
				indices_[k+2] = i		* height_ + j;

				indices_[k+3] = i		* height_ + j;
				indices_[k+4] = (i + 1) * height_ + (j + 1);
				indices_[k+5] = (i + 1) * height_ + j;
			}
			else
			{
				indices_[k]   = i		* height_ + j;
				indices_[k+1] = i		* height_ + (j + 1);
				indices_[k+2] = (i + 1) * height_ + j;

				indices_[k+3] = i		* height_ + (j + 1);
				indices_[k+4] = (i + 1) * height_ + (j + 1);
				indices_[k+5] = (i + 1) * height_ + j;
			}
			k += 6; // next quad
		}
	}
	return true;
}

/*
	Name		Terrain::createBuffers
	Syntax		Terrain::createBuffers()
	Brief		Creates the vertex and index buffers 
*/
void Terrain::createBuffers()
{
	D3D10_BUFFER_DESC vbd;
	vbd.Usage = D3D10_USAGE_DYNAMIC;
	vbd.ByteWidth = sizeof(Vertex) * verticesNo_;
	vbd.BindFlags = D3D10_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = D3D10_CPU_ACCESS_WRITE;
	vbd.MiscFlags = 0;
	D3D10_SUBRESOURCE_DATA vinitData;
	vinitData.pSysMem = vertices_;
	d3dDevice_->CreateBuffer(&vbd, &vinitData, &vertexBuffer_);

	D3D10_BUFFER_DESC ibd;
	ibd.Usage = D3D10_USAGE_IMMUTABLE;
	ibd.ByteWidth = sizeof(DWORD) * facesNo_ * 3;
	ibd.BindFlags = D3D10_BIND_INDEX_BUFFER;
	ibd.CPUAccessFlags = 0;
	ibd.MiscFlags = 0;
	D3D10_SUBRESOURCE_DATA iinitData;
	iinitData.pSysMem = indices_;
	d3dDevice_->CreateBuffer(&ibd, &iinitData, &indexBuffer_);
}

/*
	Name		Terrain::calculateNormals
	Syntax		Terrain::calculateNormals()
	Param		Vertex* vertices - Array of vertices for the terrain
	Param		DWORD* indices - Array of indices for the terrain
	Brief		Calculates the normal for each vertex and applies bump mapping to these normals
				using simplex noise
*/
void Terrain::calculateNormals()
{
	// Estimate normals for interior nodes using central difference
	float invTwoDX = 1.0f / 2.0f;
	float invTwoDZ = 1.0f / 2.0f;
	float t, b, l, r;
	float noiseCoefx, noiseCoefy, noiseCoefz;
	float factor = 0.2f;
	
	for(UINT i = 2; i < width_ - 1; ++i)
	{
		for(UINT j = 2; j < height_ - 1; ++j)
		{
			t = vertices_[(i - 1) * height_ + j].pos.y;
			b = vertices_[(i + 1) * height_ + j].pos.y;
			l = vertices_[i * height_ + j - 1].pos.y;
			r = vertices_[i * height_ + j + 1].pos.y;

			D3DXVECTOR3 tanZ(0.0f, (t - b) * invTwoDZ, 1.0f);
			D3DXVECTOR3 tanX(1.0f, (r - l) * invTwoDX, 0.0f);

			D3DXVECTOR3 n;
			D3DXVec3Cross(&n, &tanZ, &tanX);
			D3DXVec3Normalize(&n, &n);

			if (isComplete_)
			{
				noiseCoefx = (float)SimplexNoise::noise(	factor * vertices_[i * height_ + j].pos.x, 
													factor * vertices_[i * height_ + j].pos.y,
													factor * vertices_[i * height_ + j].pos.z);
				noiseCoefy = (float)SimplexNoise::noise(	factor * vertices_[i * height_ + j].pos.y, 
													factor * vertices_[i * height_ + j].pos.z,
													factor * vertices_[i * height_ + j].pos.x);
				noiseCoefz = (float)SimplexNoise::noise( factor * vertices_[i * height_ + j].pos.z, 
													factor * vertices_[i * height_ + j].pos.x,
													factor * vertices_[i * height_ + j].pos.y);
				n.x += noiseCoefx; 
				n.y += noiseCoefy; 
				n.z += noiseCoefz; 
   
				D3DXVec3Normalize(&n, &n);
			}
			
			vertices_[i * height_ + j].normal = n;
		}
	}
}

/*
	Name		Terrain::update
	Syntax		Terrain::update(float deltaTime)
	Param		float deltaTime - Change in time between frames
	Brief		
*/
void Terrain::update(float deltaTime)
{
	if (age_ > 0)
		age_ -= deltaTime;

	switch (currentGenStage_)
	{
	case GEN_FLAT:
		if (age_ <= 0)
		{
			generateMountain();
			age_ = 15;
			currentGenStage_ = GEN_MOUNTAIN;
		}
		break;
	case GEN_MOUNTAIN:
		if (age_ <= 0)
		{
			generateCrater();
			age_ = 5;
			currentGenStage_ = GEN_CRATER;
		}
		else
		{
			updateVertices(deltaTime);
		}
		break;
	case GEN_CRATER:
		if (age_ <= 0)
		{
			generateNoise();
			age_ = 50;
			currentGenStage_ = GEN_NOISE;
		}
		else
		{
			updateVertices(deltaTime);
		}
		break;
	case GEN_NOISE:
		if (age_ <= 40 && age_ > 30)
		{
			generateLavaFlow();
			age_ = 30.0f;
		}
		else if (age_ <= 25 && age_ > 20)
		{
			generateLavaFlow();
			age_ = 20.0f;
		}
		else if (age_ <= 15 && age_ > 10)
		{
			generateLavaFlow();
			age_ = 10.0f;
		}
		else if (age_ <= 5 && age_ > 0)
		{
			generateLavaFlow();
			age_ = 10.0f;
			currentGenStage_ = GEN_LAVA_FLOWS;
		}
		else
		{
			if (age_ > 40)
				deltaTime /= 2.5f;
			updateVertices(deltaTime);
		}
		break;
	case GEN_LAVA_FLOWS:
		if (age_ > 0)
		{
			updateVertices(deltaTime);
		}
		else
		{
			currentGenStage_ = GEN_COMPLETE;
			isComplete_ = true;
			calculateNormals();
			createHeightMap();
			setEmitters();
		}
		break;
	case GEN_COMPLETE:
		break;
	default:
		break;
	}
}

/*
	Name		Terrain::setTrans
	Syntax		Terrain::setTrans()
	Brief		Applies the Terrain's translation to its world matrix
*/
void Terrain::setTrans()
{
	D3DXMATRIX m;
	D3DXMatrixIdentity(&world_);
	D3DXMatrixScaling(&m, scale_.x, scale_.y, scale_.z);
	world_ *= m;
	D3DXMatrixRotationYawPitchRoll(&m, theta_.y, theta_.x, theta_.z);
	world_ *= m;
	D3DXMatrixTranslation(&m, pos_.x, pos_.y, pos_.z);
	world_ *= m;
}

/*
	Name		Terrain::generateMountain
	Syntax		Terrain::generateMountain()
	Brief		Generates a mountain by creating a number of mounds stacked 
				roughly in the centre of the terrain
*/
void Terrain::generateMountain()
{
	int num = 30;
	int maxRadius = (width_/6);
	int minRadius = (width_/18);
	float maxDistance, minDistance;
	float radius;
	float moundX, moundZ;
	float angle, distance;
	float radiusSq, distanceSq, height, difference;
	int xMin, xMax, zMin, zMax;
	int i, x, z;

	for (i = 0; i < num; ++i)
	{
		// Mounds have a random radius between 1/5 and 1/9 of the terrain's width
		radius = (float)(rand() % maxRadius  + minRadius);
	
		// Each mound is generated at a random angle and distance from the centre of the terrain
		angle = (float)(rand() * (2*D3DX_PI) / RAND_MAX);	
		// Distance from centre is randomised between radius/4 and where the edge of the mound would miss the edge of the terrain
		maxDistance = width_/2 - radius*2;
		minDistance = radius/4;
		distance = (rand() * maxDistance / RAND_MAX) + minDistance;  
		
		// Set the centre of the mound
		moundX = (float)width_/2.0f + cos(angle) * distance;
		moundZ = (float)height_/2.0f + sin(angle) * distance;

		// We use the square of the radius to avoid having to use squareroot on the distance 
		radiusSq = radius * radius;
	
		// Boundaries for vertices in range of the centre of the mound
		xMin = (int)(moundX - radius - 1);
		xMax = (int)(moundX + radius + 1);
		if (xMin < 0) 
			xMin = 0;
		if (xMax >= width_) 
			xMax = width_ - 1;
	
		zMin = (int)(moundZ - radius - 1);
		zMax = (int)(moundZ + radius + 1);
		if (zMin < 0) 
			zMin = 0;
		if (zMax >= width_) 
			zMax = width_ - 1;
	
		// Calculate height for each vertex in the mound - negative value are outside of the mound's radius
		for (x = xMin; x <= xMax; ++x)
		{
			for(z = zMin; z <= zMax; ++z)
			{
				distanceSq = (moundX - x) * (moundX - x) + (moundZ - z) * (moundZ - z);
				// Use the distance from the centre to determine the height
				difference = radiusSq - distanceSq;
			
				// Ignore if negative
				if (difference > 0)
				{
					// Use the squareroot and dividing factor to create smoother terrain.
					height = (radius - sqrt(distanceSq))/4;
					// Add the height to the vertex.
					heightMap_[x + (z*height_)] += height; 
				}	
			}
		}
	}
}

/*
	Name		Terrain::generateCrater
	Syntax		Terrain::generateCrater()
	Brief		Generates a crater at the heightest point in the terrain
*/
void Terrain::generateCrater()
{
	// find the highest point on the terrain and centre the crate on it
	int index;
	float h = 0;
	for (int i = 0; i < verticesNo_; i++)
	{
		if (heightMap_[i] > h)
		{
			h = heightMap_[i];
			index = i;
		}
	}
	craterX_ = (float)(index%height_);
	craterZ_ = (float)(index/height_);
	craterRadius_ = (float)(width_/12.0f);

	ashEmitter_ = D3DXVECTOR3(craterX_, h + 150.0f, craterZ_);
	D3DXVec3TransformCoord(&ashEmitter_, &ashEmitter_, &world_);

	// We use the square of the radius to avoid having to use squareroot on the distance 
	float radiusSq = craterRadius_ * craterRadius_;
	float distanceSq;
	float height;
	
	// Boundaries for vertices in range of the centre of the mound
	int xMin = (int)(craterX_ - craterRadius_ - 1);
	int xMax = (int)(craterX_ + craterRadius_ + 1);
	if (xMin < 0) 
		xMin = 0;
	if (xMax >= width_) 
		xMax = width_ - 1;
	
	int zMin = (int)(craterZ_ - craterRadius_ - 1);
	int zMax = (int)(craterZ_ + craterRadius_ + 1);
	if (zMin < 0) 
		zMin = 0;
	if (zMax >= width_) 
		zMax = width_ - 1;

	for (int x = xMin; x <= xMax; ++x)
	{
		for(int z = zMin; z <= zMax; ++z)
		{
			distanceSq = ( craterX_ - x ) * ( craterX_ - x ) + ( craterZ_ - z ) * ( craterZ_ - z );
			// Use the distance from the centre to determine the heights
			height = radiusSq - distanceSq;
			
			// Ignore if negative
			if (height > 0)
			{
				// Subtract the squareroot of the height from the vertex to create a crater
				heightMap_[x + (z*height_)] -= sqrt(height); 

				if (height > 625)
				{
					vertices_[x + (z*height_)].type = LAVA;
					D3DXVECTOR3 lavaVertex = D3DXVECTOR3(vertices_[x + (z*height_)].pos.x, heightMap_[x + (z*height_)], vertices_[x + (z*height_)].pos.z);
					D3DXVec3TransformCoord(&lavaVertex, &lavaVertex, &world_);
					lavaVertices_.push_back(lavaVertex);
				}
			}	
		}
	}
}

/*
	Name		Terrain::generateNoise
	Syntax		Terrain::generateNoise()
	Brief		Generates noise
*/
void Terrain::generateNoise()
{
	int index;
	for (int i = 1; i < (height_-1); ++i)
	{
		for (int j = 1; j < (width_-1); ++j)
		{
			index = i * width_ + j;
			heightMap_[index] += (float)SimplexNoise::ridgedMultifractal(i/128.0f, j/128.0f, 10, 1.5f, 0.5f, 1.0f) * 35.0f;
		}
	}
}

/*
	Name		Terrain::generateLavaFlow
	Syntax		Terrain::generateLavaFlow()
	Brief		Generates a lava flow from the crater to the edge of the terrain
*/
void Terrain::generateLavaFlow()
{

	// Pick random angle and position lava flow from the edge of the crater flowing outwards at that angle
	float angle = (float)(rand() * (2*D3DX_PI) / RAND_MAX);
	float startX = craterX_ + cos(angle) * (craterRadius_ - 25.0f);
	float startZ = craterZ_ + sin(angle) * (craterRadius_ - 25.0f);
	float destinationX = craterX_ + cos(angle) * (craterRadius_ + 50.0f);
	float destinationZ = craterZ_ + sin(angle) * (craterRadius_ + 50.0f);
	D3DXVECTOR3 direction = D3DXVECTOR3(destinationX-startX, 0.0f, destinationZ-startZ);
	D3DXVec3Normalize(&direction, &direction);

	float flowX = startX;
	float flowZ = startZ;
	int acrossX, acrossZ;
	int count = 0;
	float halfWidth = 10.0f;
	float depth = 15.0f;
	float curve = 25.0f;

	bool* changed = new bool[verticesNo_];
	for (int i = 0; i < verticesNo_; i++)
	{
		changed[i] = false;
	}
	D3DXVECTOR3 acrossFlow;
	float depthChange;
	int index;
	while (flowX >= 1 && flowX < (width_-1) && flowZ >= 1 && flowZ < (height_-1))
	{
		// Find the vector perpendicular to the direction of the lava flow
		D3DXVec3Cross(&acrossFlow, &direction, &D3DXVECTOR3(0.0f, 1.0f, 0.0f));
		D3DXVec3Normalize(&acrossFlow, &acrossFlow);
		
		// Sets depths for all vectors across the width of the lava flow
		for (int j = (int)(-halfWidth); j < (int)halfWidth; j++) 
		{
			// Cosine curve used to generate depths across the lava flow - This creates a deep v-shaped curve
			depthChange = depth * (cos(D3DX_PI * (float)j/halfWidth) + 0.8f);

			acrossX = (int)(flowX - (acrossFlow.x * halfWidth) + (acrossFlow.x * j));
			acrossZ = (int)(flowZ - (acrossFlow.z * halfWidth) + (acrossFlow.z * j));

			if (acrossX >= 0 && acrossX < width_ && acrossZ >= 0 && acrossZ < height_)
			{
				index = acrossX + acrossZ * height_;
				if (!changed[index])
				{
					heightMap_[index] -= depthChange;
					if (j > (-halfWidth+5) && j < (halfWidth-5))
					{
						vertices_[index].type = LAVA;
						D3DXVECTOR3 lavaVertex = D3DXVECTOR3(vertices_[index].pos.x, heightMap_[index], vertices_[index].pos.z);
						D3DXVec3TransformCoord(&lavaVertex, &lavaVertex, &world_);
						lavaVertices_.push_back(lavaVertex);
					}
					changed[index] = true;
				}
			}
		}

		// Follow a sine wave pattern along the direction vector
		flowZ += direction.z/10 + sin(count/(curve*10))/20;
		flowX += direction.x/10 + sin(count/(curve*10))/20;
		count++;
	}

	delete changed;
}

/*
	Name		Terrain::updateVertices
	Syntax		Terrain::updateVertices(float deltaTime)
	Param		float deltaTime - Time between frames
	Brief		Updates the vertices, interpolating towards final heights
*/
void Terrain::updateVertices(float deltaTime)
{
	int index;
	float diff = 0.0f;
	bool changed = false;
	deltaTime /= 10.0f;
	for (int i = 1; i < (height_-1); ++i)
	{
		for (int j = 1; j < (width_-1); ++j)
		{
			index = i * width_ + j;
			diff = heightMap_[index] - vertices_[index].pos.y;
			if (diff)
			{
				vertices_[index].pos.y += diff * deltaTime;
				changed = true;
			}
		}
	}
	
	if (changed)
	{
		calculateNormals();

		// Initialize the vertex buffer pointer
		void* verts = 0;

		// Lock the vertex buffer
		HRESULT hr = vertexBuffer_->Map(D3D10_MAP_WRITE_DISCARD, 0, (void**)&verts);
		if(FAILED(hr))
		{
			MessageBox(0, "Updating terrain vertices - Failed", "Error", MB_OK);
		}

		// Copy the data into the vertex buffer
		memcpy(verts, (void*)vertices_, (sizeof(Vertex) * verticesNo_));

		// Unlock the vertex buffer
		vertexBuffer_->Unmap();
	}
}

/*
	Name		Terrain::reset
	Syntax		Terrain::reset()
	Brief		Resets the terrain generation
*/
void Terrain::reset()
{
	for (int i = 0; i < verticesNo_; ++i)
	{
		vertices_[i].pos.y = heightMap_[i] = 0.0f;
		vertices_[i].type = ROCK;
	}

	int index;
	// Drop edges of terrain 
	for (int i = 0; i < width_; i++)
	{
		vertices_[i].pos.y = -100.0f;
		index = (width_-1) * width_ + i;
		vertices_[index].pos.y = -100.0f;
	}

	for (int j = 0; j < height_; j++)
	{
		index = j * width_;
		vertices_[index].pos.y = -100.0f;
		index = j * width_ + height_-1;
		vertices_[index].pos.y = -100.0f;
	}

	currentGenStage_ = GEN_FLAT;
	age_ = 5.0f;
	clearEmitters();
	isComplete_ = false;

	calculateNormals();

	// Initialize the vertex buffer pointer
	void* verts = 0;

	// Lock the vertex buffer
	HRESULT hr = vertexBuffer_->Map(D3D10_MAP_WRITE_DISCARD, 0, (void**)&verts);
	if(FAILED(hr))
	{
		MessageBox(0, "Updating terrain vertices - Failed", "Error", MB_OK);
	}

	// Copy the data into the vertex buffer
	memcpy(verts, (void*)vertices_, (sizeof(Vertex) * verticesNo_));

	// Unlock the vertex buffer
	vertexBuffer_->Unmap();
}

/*
	Name		Terrain::increasePosX
	Syntax		Terrain::increasePosX(float x)
	Param		float x - Value to increase the x position by
	Brief		Increases the Terrain's x position
*/
void Terrain::increasePosX(float x)
{
	pos_.x += x;
	setTrans();
}

/*
	Name		Terrain::increasePosY
	Syntax		Terrain::increasePosY(float y)
	Param		float y - Value to increase the y position by
	Brief		Increases the Terrain's y position
*/
void Terrain::increasePosY(float y)
{
	pos_.y += y;
	setTrans();
}

/*
	Name		Terrain::increasePosZ
	Syntax		Terrain::increasePosZ(float z)
	Param		float z - Value to increase the z position by
	Brief		Increases the Terrain's z position
*/
void Terrain::increasePosZ(float z)
{
	pos_.z += z;
	setTrans();
}

/*
	Name		Terrain::setPos
	Syntax		Terrain::setPos(D3DXVECTOR3 pos)
	Param		D3DXVECTOR3 pos - Position vector to move Terrain to
	Brief		Sets the Terrain's position
*/
void Terrain::setPos(D3DXVECTOR3 pos)
{
	pos_ = pos;
	setTrans();
}

/*
	Name		Terrain::setPos
	Syntax		Terrain::setPos(float x, float y, float z)
	Param		float x - Value to set position on the x axis to
	Param		float y - Value to set position on the y axis to
	Param		float z - Value to set position on the z axis to
	Brief		Sets the Terrain's position
*/
void Terrain::setPos(float x, float y, float z)
{
	pos_.x = x;
	pos_.y = y;
	pos_.z = z;
	setTrans();
}

/*
	Name		Terrain::increaseThetaX
	Syntax		Terrain::increaseThetaX(float x)
	Param		float x - Value to increase rotation on the x axis by
	Brief		Increases the Terrain's x axis rotation
*/
void Terrain::increaseThetaX(float x)
{
	theta_.x += x;
	setTrans();
}

/*
	Name		Terrain::increaseThetaY
	Syntax		Terrain::increaseThetaY(float y)
	Param		float y - Value to increase rotation on the y axis by
	Brief		Increases the Terrain's y axis rotation
*/
void Terrain::increaseThetaY(float y)
{
	theta_.y += y;
	setTrans();
}

/*
	Name		Terrain::increaseThetaZ
	Syntax		Terrain::increaseThetaZ(float z)
	Param		float z - Value to increase rotation on the z axis by
	Brief		Increases the Terrain's z axis rotation
*/
void Terrain::increaseThetaZ(float z)
{
	theta_.z += z;
	setTrans();
}

/*
	Name		Terrain::setTheta
	Syntax		Terrain::setTheta(float x, float y, float z)
	Param		float x - Value to set rotation on the x axis to
	Param		float y - Value to set rotation on the y axis to
	Param		float z - Value to set rotation on the z axis to
	Brief		Set the Terrain's rotation
*/
void Terrain::setTheta(float x, float y, float z)
{
	theta_.x = x;
	theta_.y = y;
	theta_.z = z;
	setTrans();
}

/*
	Name		Terrain::increaseScaleX
	Syntax		Terrain::increaseScaleX(float x)
	Param		float x - Value to increase scale along the z axis by
	Brief		Increases the Terrain's x axis scaling
*/
void Terrain::increaseScaleX(float x)
{
	scale_.x += x;
	setTrans();
}

/*
	Name		Terrain::increaseScaleY
	Syntax		Terrain::increaseScaleY(float y)
	Param		float y - Value to increase scale along the y axis by
	Brief		Increases the Terrain's z axis scaling
*/
void Terrain::increaseScaleY(float y)
{
	scale_.y += y;
	setTrans();
}

/*
	Name		Terrain::increaseScaleZ
	Syntax		Terrain::increaseScaleZ(float z)
	Param		float z - Value to increase scale along the z axis by
	Brief		Increases the Terrain's z axis scaling
*/
void Terrain::increaseScaleZ(float z)
{
	scale_.z += z;
	setTrans();
}

/*
	Name		Terrain::setScale
	Syntax		Terrain::setScale(float x, float y, float z)
	Param		float x - Value to set scale along the x axis to
	Param		float y - Value to set scale along the y axis to
	Param		float z - Value to set scale along the z axis to
	Brief		Set the Terrain's scale
*/
void Terrain::setScale(float x, float y, float z)
{
	scale_.x = x;
	scale_.y = y;
	scale_.z = z;
	setTrans();
}

/*
	Name		Terrain::autoComplete
	Syntax		Terrain::autoComplete()
	Brief		Autocompletes the terrain generation and updates all the vertices to their final positions
*/
void Terrain::autoComplete()
{
	int i;
	if (currentGenStage_ == GEN_FLAT)
	{
		generateMountain();
		currentGenStage_ = GEN_MOUNTAIN;
	}
	if (currentGenStage_ == GEN_MOUNTAIN)
	{
		generateCrater();
		currentGenStage_ = GEN_NOISE;
	}
	if (currentGenStage_ == GEN_NOISE)
	{
		generateNoise();
		currentGenStage_ = GEN_LAVA_FLOWS;
		age_ = 41;
	}
	if (currentGenStage_ == GEN_LAVA_FLOWS)
	{
		int flows;
		if (age_ > 40)
			flows = 4;
		else if (age_ > 25)
			flows = 3;
		else if (age_ >15)
			flows = 2;
		else
			flows = 1;

		for (i = 0; i < flows; ++i)
		{
			generateLavaFlow();
		}
		currentGenStage_ = GEN_COMPLETE;
		isComplete_ = true;
		createHeightMap();
		setEmitters();
	}
		
	age_ = 0.0f;

	for (i = 0; i < verticesNo_; ++i)
	{
		vertices_[i].pos.y = heightMap_[i];
	}

	calculateNormals();

	// Initialize the vertex buffer pointer
	void* verts = 0;

	// Lock the vertex buffer
	HRESULT hr = vertexBuffer_->Map(D3D10_MAP_WRITE_DISCARD, 0, (void**)&verts);
	if(FAILED(hr))
	{
		MessageBox(0, "Updating terrain vertices - Failed", "Error", MB_OK);
	}

	// Copy the data into the vertex buffer
	memcpy(verts, (void*)vertices_, (sizeof(Vertex) * verticesNo_));

	// Unlock the vertex buffer
	vertexBuffer_->Unmap();
}

/*
	Name		Terrain::createHeightMap
	Syntax		Terrain::createHeightMap()
	Brief		Creates a 2D heightmap for the terrain
*/
void Terrain::createHeightMap()
{
	// This does not work as the heights stored in the height map are in local space
	// and need to be translated to world space first before they are loaded into the
	// texture.
	D3D10_SUBRESOURCE_DATA initData;
	initData.pSysMem = heightMap_;
	initData.SysMemPitch = width_ * sizeof(float);
	initData.SysMemSlicePitch = height_ * sizeof(float);

	// Create the texture
	D3D10_TEXTURE2D_DESC texDesc;
	texDesc.Width = width_;
	texDesc.Height = height_;
	texDesc.MipLevels = 1;
	texDesc.Format = DXGI_FORMAT_R32_FLOAT; 
	 
	texDesc.Usage = D3D10_USAGE_IMMUTABLE;
	texDesc.BindFlags = D3D10_BIND_SHADER_RESOURCE;
	texDesc.CPUAccessFlags = 0;
	texDesc.MiscFlags = 0;
	texDesc.ArraySize = 1;

	DXGI_SAMPLE_DESC sampleDesc;
	sampleDesc.Count = 1;
	sampleDesc.Quality = 0;
	texDesc.SampleDesc = sampleDesc;

	ID3D10Texture2D* tex = 0;
	HRESULT hr = Scene::instance()->getDevice()->CreateTexture2D(&texDesc, &initData, &tex);
	if (FAILED(hr))
	{
		MessageBox(0, "Create perturbance texture - Failed", "Error", MB_OK);
		return;
	}

	// Create the resource view
	D3D10_SHADER_RESOURCE_VIEW_DESC viewDesc;
	viewDesc.Format = texDesc.Format;
	viewDesc.ViewDimension = D3D10_SRV_DIMENSION_TEXTURE2D;
	viewDesc.Texture2D.MipLevels = texDesc.MipLevels;
	viewDesc.Texture2D.MostDetailedMip = 0;

	hr = Scene::instance()->getDevice()->CreateShaderResourceView(tex, &viewDesc, &heightMapRV_);

	if (FAILED(hr))
	{
		MessageBox(0, "Create perturbance tex RV - Failed", "Error", MB_OK);
		return;
	}

	tex->Release();
	tex = 0;
}

/*
	Name		Terrain::setEmitters
	Syntax		Terrain::setEmitters()
	Brief		Sets the fire and smoke emitters by picking random lava vertices
*/
void Terrain::setEmitters()
{
	int i, j;
	for (i = 0; i < NUM_FIRE_SYSTEMS; ++i)
	{
		j = rand() % lavaVertices_.size();
		fireEmitters_.push_back(lavaVertices_[j]);
	}

	for (i = 0; i < NUM_SMOKE_SYSTEMS; ++i)
	{
		j = rand() % lavaVertices_.size();
		smokeEmitters_.push_back(lavaVertices_[j]);
	}
}

/*
	Name		Terrain::clearEmitters
	Syntax		Terrain::clearEmitters()
	Brief		Clears the fire and smoke emitters
*/
void Terrain::clearEmitters()
{
	fireEmitters_.clear();
	smokeEmitters_.clear();
	lavaVertices_.clear();
}
