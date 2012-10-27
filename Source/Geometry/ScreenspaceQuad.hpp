/*
	Created 	Elinor Townsend 2012
*/

/*	
	Name		Screenspace Quad
	Brief		Definition of Screenspace Quad Class used for creating a quad in screenspace
*/

#ifndef SCREENSPACEQUAD_H
#define SCREENSPACEQUAD_H

#include <d3dx10.h>

class ScreenspaceQuad
{
public:
	ScreenspaceQuad();
	~ScreenspaceQuad();

	void initialise(ID3D10Device* device);
	void render();
	int getNumIndices() const { return facesNo_ * 3; };
	void updateVertices();
private:
	DWORD verticesNo_;
	DWORD facesNo_;

	ID3D10Device* d3dDevice_;
	ID3D10Buffer* vertexBuffer_;
	ID3D10Buffer* indexBuffer_;

	bool initialised_;
};

#endif // SCREENSPACEQUAD_H