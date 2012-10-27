/*
	Created 	Elinor Townsend 2012
*/

/*	
	Name		Light
	Brief		Struct for holding light data
*/

#ifndef LIGHT_H
#define LIGHT_H

#include <d3dx10.h>

struct Light
{
	Light()
	{
		ZeroMemory(this, sizeof(Light));
	}

	D3DXCOLOR ambientColour;
	D3DXCOLOR diffuseColour;
	D3DXCOLOR specularColour;
	D3DXVECTOR3 diffuseDirection;
	float ambientIntensity;
   	float diffuseIntensity;	 
	float specularIntensity;
	float shininess;
};

#endif // LIGHT_H