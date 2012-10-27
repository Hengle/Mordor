/*
	Created 	Elinor Townsend 2012
*/

/*	
	Name		Terrain
	Brief		Declaration of Terrain Class
*/

#ifndef TERRAIN_H
#define TERRAIN_H

#include <d3dx10.h>
#include <d3dx10math.h>
#include <stdio.h>
#include <fstream>
#include <vector>

struct Vertex;

enum TerrainGenerationStage 
{
	GEN_FLAT,
	GEN_MOUNTAIN,
	GEN_CRATER,
	GEN_NOISE,
	GEN_LAVA_FLOWS,
	GEN_COMPLETE
};

enum TerrainType
{
	ROCK,
	LAVA,
};

class Terrain
{
public:
	Terrain();
	~Terrain();
	void initialise(ID3D10Device* device, int gridSize);
	void update(float deltaTime);
	void render(); 
	DWORD getNumVertices()	const { return verticesNo_; };
	DWORD getNumIndices()	const { return facesNo_ * 3; };
	D3DXMATRIX getWorld()	const { return world_; };
	D3DXVECTOR3 getAshEmitter() const { return ashEmitter_; };
	D3DXVECTOR3 getFireEmitter(int i) const { return fireEmitters_[i]; };
	D3DXVECTOR3 getSmokeEmitter(int i) const { return smokeEmitters_[i]; };
	ID3D10ShaderResourceView* getHeightMap() const { return heightMapRV_; };
	void reset();
	void autoComplete();
	void increasePosX(float x);
	void increasePosY(float y);
	void increasePosZ(float z);
	void setPos(D3DXVECTOR3 pos);
	void setPos(float x, float y, float z);
	void increaseThetaX(float x);
	void increaseThetaY(float y);
	void increaseThetaZ(float z);
	void setTheta(float x, float y, float z);
	void increaseScaleX(float x);
	void increaseScaleY(float y);
	void increaseScaleZ(float z);
	void setScale(float x, float y, float z);
	bool isComplete() const { return isComplete_; };

private:
	bool createTerrain();
	void createBuffers();
	void calculateNormals();
	void setTrans();
	void generateMountain();
	void generateCrater();
	void generateNoise();
	void generateLavaFlow();
	void updateVertices(float deltaTime);
	void createHeightMap();
	void setEmitters();
	void clearEmitters();
		
	TerrainGenerationStage currentGenStage_;

	float age_;

	D3DXMATRIX world_;
	D3DXVECTOR3 pos_, theta_, scale_;

	DWORD verticesNo_;
	DWORD facesNo_;

	Vertex* vertices_;
	float* heightMap_;
	DWORD* indices_;

	ID3D10Device* d3dDevice_;
	ID3D10Buffer* vertexBuffer_;
	ID3D10Buffer* indexBuffer_;
	
	UINT width_;
	UINT height_;

	float craterX_;
	float craterZ_;
	float craterRadius_;

	D3DXVECTOR3 ashEmitter_;
	std::vector<D3DXVECTOR3> fireEmitters_;
	std::vector<D3DXVECTOR3> smokeEmitters_;
	std::vector<D3DXVECTOR3> lavaVertices_;

	bool isComplete_;

	ID3D10ShaderResourceView* heightMapRV_;
};

#endif