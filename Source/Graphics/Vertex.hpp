/*
	Created 	Elinor Townsend 2012
				Based on Frank Luna (C) 2008 All Rights Reserved.
*/

/*	
	Name		Vertex
	Brief		Custom vertex structures to hold vertex data
*/

#ifndef VERTEX_H
#define VERTEX_H

#include <d3dx10.h>

/*
	Name		Vertex
	Syntax		Vertex
	Brief		A vertex structure to hold position, normal, and color
*/
struct Vertex
{
	D3DXVECTOR3 pos;
	D3DXVECTOR3 normal;
	D3DXVECTOR2 texC;
	unsigned int type;
};

/*
	Name		ParticleVertex
	Syntax		ParticleVertex
	Brief		A vertex structure to hold position, velocity, size, age, and type 
				of particles
*/
struct ParticleVertex
{
	D3DXVECTOR3 initialPos;
	D3DXVECTOR3 initialVel;
	D3DXVECTOR2 size;
	float age;
	unsigned int type;
};

/*
	Name		SimpleVertex
	Syntax		SimpleVertex
	Brief		A vertex structure to position and texture coordinates
*/
struct SimpleVertex 
{
	D3DXVECTOR3 pos;
	D3DXVECTOR2 texC;
};

#endif 