/*
	Created 	Elinor Townsend 2012
*/

/*	
	Name		Sky Sphere
	Brief		Definition of Sky Sphere Class used for creating a sphere
*/

#ifndef SKYSPHERE_H
#define SKYSPHERE_H

#include <d3dx10.h>

class SkySphere
{
public:
	SkySphere();
	~SkySphere();

	void initialise(ID3D10Device* device);
	void render();
	DWORD getNumVertices() const { return verticesNo_; };
	DWORD getNumIndices() const { return facesNo_ * 3; };
	D3DXMATRIX getWorld() const { return world_; };
	void setTrans();
	void setPos(D3DXVECTOR3 pos);
	void setScale(float x, float y, float z);

private:
	DWORD verticesNo_;
	DWORD facesNo_;

	ID3D10Device* d3dDevice_;
	ID3D10Buffer* vertexBuffer_;
	ID3D10Buffer* indexBuffer_;

	D3DXMATRIX world_;
	D3DXVECTOR3 pos_, scale_;
};

#endif // SKYSPHERE_H