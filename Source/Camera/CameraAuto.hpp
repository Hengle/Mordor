/*
	Created 	Elinor Townsend 2012
*/

/*	
	Name		CameraAuto
	Brief		Definition of CameraAuto class 
*/

#ifndef CAMERA_AUTO_H
#define CAMERA_AUTO_H

#include <d3dx10.h>
#include "Camera/Camera.hpp"

class CameraAuto : public Camera
{
public:
	CameraAuto();
	~CameraAuto();
	void initialise();
	void rotate(float yaw, float pitch, float roll);
	void move(float moveX, float moveY, float moveZ);
	void update();
	void reset();
	void zoom(float direction);
	void setZoomFactor(float zoom);

private:
	void setCameraProjectionMatrix();
	float orbitalAngle_;
	const D3DXVECTOR3 CENTRE_OF_ORBIT;
	const float ORBITAL_RADIUS;
	const float RADIANS_CONVERSION;
};

#endif