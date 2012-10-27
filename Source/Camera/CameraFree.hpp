/*
	Created 	Elinor Townsend 2012
*/

/*	
	Name		CameraFree
	Brief		Definition of CameraFree class 
*/

#ifndef CAMERA_FREE_H
#define CAMERA_FREE_H

#include <d3dx10.h>
#include "Camera/Camera.hpp"

class CameraFree : public Camera
{
public:
	CameraFree();
	~CameraFree();
	void initialise();
	void rotate(float yaw, float pitch, float roll);
	void move(float moveX, float moveY, float moveZ);
	void update();
	void reset();
	void zoom(float direction);
	void setZoomFactor(float zoom);

private:
	void setCameraProjectionMatrix();
	const float RADIANS_CONVERSION;
};

#endif